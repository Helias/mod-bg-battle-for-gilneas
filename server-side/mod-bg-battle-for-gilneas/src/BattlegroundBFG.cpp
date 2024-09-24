/*
 * Copyright (C) Project SkyFire
 * Copyright (C) 2019+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: http://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
*/

#include "AchievementMgr.h"
#include "Battleground.h"
#include "BattlegroundBFG.h"
#include "World.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "BattlegroundMgr.h"
#include "Creature.h"
#include "Language.h"
#include "Object.h"
#include "Player.h"
#include "Util.h"
#include "WorldSession.h"

#include "GameGraveyard.h"
#include <unordered_map>

#include "ScriptMgr.h"
#include "Config.h"

// adding Battleground to the core battlegrounds list
BattlegroundTypeId BATTLEGROUND_BFG = BattlegroundTypeId(120); // value from BattlemasterList.dbc
BattlegroundQueueTypeId BATTLEGROUND_QUEUE_BFG = BattlegroundQueueTypeId(13);


void BattlegroundBFGScore::BuildObjectivesBlock(WorldPacket& data)
{
    data << uint32(2);
    data << uint32(BasesAssaulted);
    data << uint32(BasesDefended);
}

BattlegroundBFG::BattlegroundBFG()
{
    m_BuffChange = true;
    BgObjects.resize(GILNEAS_BG_OBJECT_MAX);
    BgCreatures.resize(GILNEAS_BG_ALL_NODES_COUNT + GILNEAS_BG_DYNAMIC_NODES_COUNT); // +GILNEAS_BG_DYNAMIC_NODES_COUNT buff triggers

    _controlledPoints[TEAM_ALLIANCE] = 0;
    _controlledPoints[TEAM_HORDE] = 0;
    _teamScores500Disadvantage[TEAM_ALLIANCE] = false;
    _teamScores500Disadvantage[TEAM_HORDE] = false;
    _honorTics = 0;
    _reputationTics = 0;

    StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_BG_BFG_START_TWO_MINUTES;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_BFG_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_BFG_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_BFG_HAS_BEGUN;
}

BattlegroundBFG::~BattlegroundBFG() {}

void BattlegroundBFG::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        _bgEvents.Update(diff);
        while (uint32 eventId =_bgEvents.ExecuteEvent())
            switch (eventId)
            {
                case BG_BFG_EVENT_UPDATE_BANNER_LIGHTHOUSE:
                case BG_BFG_EVENT_UPDATE_BANNER_WATERWORKS:
                case BG_BFG_EVENT_UPDATE_BANNER_MINE:
                    CreateBanner(eventId - BG_BFG_EVENT_UPDATE_BANNER_LIGHTHOUSE, false);
                    break;
                case BG_BFG_EVENT_CAPTURE_LIGHTHOUSE:
                case BG_BFG_EVENT_CAPTURE_WATERWORKS:
                case BG_BFG_EVENT_CAPTURE_MINE:
                {
                    uint8 node = eventId - BG_BFG_EVENT_CAPTURE_LIGHTHOUSE;
                    TeamId teamId = _capturePointInfo[node]._state == GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED ? TEAM_ALLIANCE : TEAM_HORDE;
                    DeleteBanner(node);
                    _capturePointInfo[node]._ownerTeamId = teamId;
                    _capturePointInfo[node]._state = teamId == TEAM_ALLIANCE ? GILNEAS_BG_NODE_STATUS_ALLY_OCCUPIED : GILNEAS_BG_NODE_STATUS_HORDE_OCCUPIED;
                    _capturePointInfo[node]._captured = true;

                    CreateBanner(node, false);
                    NodeOccupied(node);
                    SendNodeUpdate(node);

                    // SendBroadcastText(LANG_BG_BFG_NODE_TAKEN, teamId == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, NULL, teamId == TEAM_ALLIANCE ? LANG_BG_BFG_ALLY : LANG_BG_BFG_HORDE, LANG_BG_BFG_NODE_LIGHTHOUSE + node);
                    PlaySoundToAll(teamId == TEAM_ALLIANCE ? GILNEAS_BG_SOUND_NODE_CAPTURED_ALLIANCE : GILNEAS_BG_SOUND_NODE_CAPTURED_HORDE);
                    break;
                }
                case BG_BFG_EVENT_ALLIANCE_TICK:
                case BG_BFG_EVENT_HORDE_TICK:
                {
                    TeamId teamId = TeamId(eventId - BG_BFG_EVENT_ALLIANCE_TICK);
                    uint8 controlledPoints = _controlledPoints[teamId];
                    if (controlledPoints == 0)
                    {
                        _bgEvents.ScheduleEvent(eventId, 3000);
                        break;
                    }

                    // uint8 honorRewards = uint8(m_TeamScores[teamId] / _honorTics);
                    // uint8 reputationRewards = uint8(m_TeamScores[teamId] / _reputationTics);
                    uint8 information = uint8(m_TeamScores[teamId] / GILNEAS_BG_WARNING_NEAR_VICTORY_SCORE);
                    m_TeamScores[teamId] += GILNEAS_BG_TickPoints[controlledPoints];
                    if (m_TeamScores[teamId] > GILNEAS_BG_MAX_TEAM_SCORE)
                        m_TeamScores[teamId] = GILNEAS_BG_MAX_TEAM_SCORE;

                    // if (honorRewards < uint8(m_TeamScores[teamId] / _honorTics))
                    //     RewardHonorToTeam(GetBonusHonorFromKill(1), teamId);
                    // if (reputationRewards < uint8(m_TeamScores[teamId] / _reputationTics))
                    //     RewardReputationToTeam(teamId == TEAM_ALLIANCE ? 509 : 510, 10, teamId);

                    if (information < uint8(m_TeamScores[teamId] / GILNEAS_BG_WARNING_NEAR_VICTORY_SCORE))
                    {
                        SendBroadcastText(teamId == TEAM_ALLIANCE ? LANG_BG_BFG_A_NEAR_VICTORY : LANG_BG_BFG_H_NEAR_VICTORY, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                        PlaySoundToAll(GILNEAS_BG_SOUND_NEAR_VICTORY);
                    }

                    UpdateWorldState(teamId == TEAM_ALLIANCE ? GILNEAS_BG_OP_RESOURCES_ALLY : GILNEAS_BG_OP_RESOURCES_HORDE, m_TeamScores[teamId]);
                    if (m_TeamScores[teamId] > m_TeamScores[GetOtherTeamId(teamId)] + 500)
                        _teamScores500Disadvantage[GetOtherTeamId(teamId)] = true;
                    if (m_TeamScores[teamId] >= GILNEAS_BG_MAX_TEAM_SCORE)
                        EndBattleground(teamId);

                    _bgEvents.ScheduleEvent(eventId, GILNEAS_BG_TickIntervals[controlledPoints]);
                    break;
                }
            }
    }
}

void BattlegroundBFG::StartingEventCloseDoors()
{
    // despawn banners, auras and buffs
    for (uint32 obj = GILNEAS_BG_OBJECT_BANNER_NEUTRAL; obj < static_cast<uint8>(GILNEAS_BG_DYNAMIC_NODES_COUNT) * GILNEAS_BG_OBJECT_PER_NODE; ++obj)
        SpawnBGObject(obj, RESPAWN_ONE_DAY);
    for (uint32 i = 0; i < GILNEAS_BG_DYNAMIC_NODES_COUNT * 3; ++i)
        SpawnBGObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + i, RESPAWN_ONE_DAY);

    // Starting doors
    SpawnBGObject(GILNEAS_BG_OBJECT_GATE_A_1, RESPAWN_IMMEDIATELY);
    SpawnBGObject(GILNEAS_BG_OBJECT_GATE_H_1, RESPAWN_IMMEDIATELY);
    // SpawnBGObject(GILNEAS_BG_OBJECT_GATE_A_2, RESPAWN_IMMEDIATELY);
    // SpawnBGObject(GILNEAS_BG_OBJECT_GATE_H_2, RESPAWN_IMMEDIATELY);
    DoorClose(GILNEAS_BG_OBJECT_GATE_A_1);
    DoorClose(GILNEAS_BG_OBJECT_GATE_H_1);
    // DoorClose(GILNEAS_BG_OBJECT_GATE_A_2);
    // DoorClose(GILNEAS_BG_OBJECT_GATE_H_2);

    // // Starting base spirit guides
    // NodeOccupied(GILNEAS_BG_SPIRIT_ALLIANCE);
    // NodeOccupied(GILNEAS_BG_SPIRIT_HORDE);
}

void BattlegroundBFG::StartingEventOpenDoors()
{
    // spawn neutral banners
    for (uint32 banner = GILNEAS_BG_OBJECT_BANNER_NEUTRAL, i = 0; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; banner += GILNEAS_BG_OBJECT_PER_NODE, ++i)
        SpawnBGObject(banner, RESPAWN_IMMEDIATELY);

    for (uint32 i = 0; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        SpawnBGObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + urand(0, 2) + i * 3, RESPAWN_IMMEDIATELY);

    DoorOpen(GILNEAS_BG_OBJECT_GATE_A_1);
    DoorOpen(GILNEAS_BG_OBJECT_GATE_H_1);
    // DoorOpen(GILNEAS_BG_OBJECT_GATE_A_2);
    // DoorOpen(GILNEAS_BG_OBJECT_GATE_H_2);

    // Achievement: Let's Get This Done
    StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, BG_BFG_EVENT_START_BATTLE);

    _bgEvents.ScheduleEvent(BG_BFG_EVENT_ALLIANCE_TICK, 3000);
    _bgEvents.ScheduleEvent(BG_BFG_EVENT_HORDE_TICK, 3000);
}


void BattlegroundBFG::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores.emplace(player->GetGUID().GetCounter(), new BattlegroundBFGScore(player->GetGUID()));
}

void BattlegroundBFG::RemovePlayer(Player* player) {
    player->SetPhaseMask(1, false);
}

void BattlegroundBFG::HandleAreaTrigger(Player* /* player*/, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (trigger) {
        case 6447:                                          // Alliance start
            // if (player->GetTeamId() != TEAM_ALLIANCE)
            //     player->GetSession()->SendNotification("Only The Alliance can use that portal");
            // else
            //     player->LeaveBattleground();
            // break;
        case 6448:                                          // Horde start
            // if (player->GetTeamId() != TEAM_HORDE)
            //     player->GetSession()->SendNotification("Only The Horde can use that portal");
            // else
            //     player->LeaveBattleground();
            // break;
        case 6265:                                          // Waterworks heal
        case 6266:                                          // Mine speed
        case 6267:                                          // Waterworks speed
        case 6268:                                          // Mine berserk
        case 6269:                                          // Lighthouse berserk
            break;
    }
}

void BattlegroundBFG::CreateBanner(uint8 node, bool delay)
{
    // Just put it into the queue
    if (delay)
    {
        _bgEvents.RescheduleEvent(BG_BFG_EVENT_UPDATE_BANNER_LIGHTHOUSE+node, BG_BFG_BANNER_UPDATE_TIME);
        return;
    }

    SpawnBGObject(node*GILNEAS_BG_OBJECT_PER_NODE + _capturePointInfo[node]._state, RESPAWN_IMMEDIATELY);
    SpawnBGObject(node*GILNEAS_BG_OBJECT_PER_NODE + GILNEAS_BG_OBJECT_AURA_ALLY + _capturePointInfo[node]._ownerTeamId, RESPAWN_IMMEDIATELY);
}

void BattlegroundBFG::DeleteBanner(uint8 node)
{
    SpawnBGObject(node*GILNEAS_BG_OBJECT_PER_NODE + _capturePointInfo[node]._state, RESPAWN_ONE_DAY);
    SpawnBGObject(node*GILNEAS_BG_OBJECT_PER_NODE + GILNEAS_BG_OBJECT_AURA_ALLY + _capturePointInfo[node]._ownerTeamId, RESPAWN_ONE_DAY);
}

void BattlegroundBFG::FillInitialWorldStates(WorldPacket& data)
{
    for (uint8 node = 0; node < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++node)
    {
        if (_capturePointInfo[node]._state == GILNEAS_BG_NODE_TYPE_NEUTRAL)
            data << uint32(_capturePointInfo[node]._iconNone) << uint32(1);

        for (uint8 i = GILNEAS_BG_NODE_STATUS_ALLY_OCCUPIED; i <= GILNEAS_BG_NODE_STATUS_HORDE_CONTESTED; ++i)
            data << uint32(_capturePointInfo[node]._iconCapture + i-1) << uint32(_capturePointInfo[node]._state == i);
    }

    data << uint32(GILNEAS_BG_OP_OCCUPIED_BASES_ALLY)  << uint32(_controlledPoints[TEAM_ALLIANCE]);
    data << uint32(GILNEAS_BG_OP_OCCUPIED_BASES_HORDE) << uint32(_controlledPoints[TEAM_HORDE]);
    data << uint32(GILNEAS_BG_OP_RESOURCES_MAX)        << uint32(GILNEAS_BG_MAX_TEAM_SCORE);
    data << uint32(GILNEAS_BG_OP_RESOURCES_WARNING)    << uint32(GILNEAS_BG_WARNING_NEAR_VICTORY_SCORE);
    data << uint32(GILNEAS_BG_OP_RESOURCES_ALLY)       << uint32(m_TeamScores[TEAM_ALLIANCE]);
    data << uint32(GILNEAS_BG_OP_RESOURCES_HORDE)      << uint32(m_TeamScores[TEAM_HORDE]);
    data << uint32(0x745) << uint32(0x2);           // 37 1861 unk
}


void BattlegroundBFG::SendNodeUpdate(uint8 node)
{
    UpdateWorldState(_capturePointInfo[node]._iconNone, 0);
    for (uint8 i = GILNEAS_BG_NODE_STATUS_ALLY_OCCUPIED; i <= GILNEAS_BG_NODE_STATUS_HORDE_CONTESTED; ++i)
        UpdateWorldState(_capturePointInfo[node]._iconCapture + i-1, _capturePointInfo[node]._state == i);

    UpdateWorldState(GILNEAS_BG_OP_OCCUPIED_BASES_ALLY, _controlledPoints[TEAM_ALLIANCE]);
    UpdateWorldState(GILNEAS_BG_OP_OCCUPIED_BASES_HORDE, _controlledPoints[TEAM_HORDE]);
}

void BattlegroundBFG::NodeOccupied(uint8 node)
{
    ApplyPhaseMask();
    AddSpiritGuide(node, GILNEAS_BG_SpiritGuidePos[node][0], GILNEAS_BG_SpiritGuidePos[node][1], GILNEAS_BG_SpiritGuidePos[node][2], GILNEAS_BG_SpiritGuidePos[node][3], _capturePointInfo[node]._ownerTeamId);

    ++_controlledPoints[_capturePointInfo[node]._ownerTeamId];
    // if (_controlledPoints[_capturePointInfo[node]._ownerTeamId] >= 5)
    //     CastSpellOnTeam(SPELL_AB_QUEST_REWARD_5_BASES, _capturePointInfo[node]._ownerTeamId);
    // if (_controlledPoints[_capturePointInfo[node]._ownerTeamId] >= 4)
    //     CastSpellOnTeam(SPELL_AB_QUEST_REWARD_4_BASES, _capturePointInfo[node]._ownerTeamId);

    // Creature* trigger = BgCreatures[node + 5] ? GetBGCreature(node + 5) : NULL; // 0-5 spirit guides
    Creature* trigger = GetBgMap()->GetCreature(BgCreatures[GILNEAS_BG_ALL_NODES_COUNT + node]);
    if (!trigger)
        trigger = AddCreature(WORLD_TRIGGER, GILNEAS_BG_ALL_NODES_COUNT + node, GILNEAS_BG_NodePositions[node][0], GILNEAS_BG_NodePositions[node][1], GILNEAS_BG_NodePositions[node][2], GILNEAS_BG_NodePositions[node][3]);

    if (trigger)
    {
        trigger->SetFaction(_capturePointInfo[node]._ownerTeamId == TEAM_ALLIANCE ? FACTION_ALLIANCE_GENERIC : FACTION_HORDE_GENERIC);
        trigger->CastSpell(trigger, SPELL_HONORABLE_DEFENDER_25Y, false);
    }
}

void BattlegroundBFG::NodeDeoccupied(uint8 node)
{
    --_controlledPoints[_capturePointInfo[node]._ownerTeamId];

    _capturePointInfo[node]._ownerTeamId = TEAM_NEUTRAL;
    RelocateDeadPlayers(BgCreatures[node]);

    DelCreature(node); // Delete spirit healer
    DelCreature(GILNEAS_BG_ALL_NODES_COUNT + node); // Delete aura trigger
}

/* Invoked if a player used a banner as a gameobject */
void BattlegroundBFG::EventPlayerClickedOnFlag(Player* player, GameObject* gameObject)
{
    if (GetStatus() != STATUS_IN_PROGRESS || !player->IsWithinDistInMap(gameObject, 10.0f))
        return;

    uint8 node = GILNEAS_BG_NODE_LIGHTHOUSE;
    for (; node < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++node)
        if (player->GetDistance2d(GILNEAS_BG_NodePositions[node][0], GILNEAS_BG_NodePositions[node][1]) < 10.0f)
            break;

    if (node == GILNEAS_BG_DYNAMIC_NODES_COUNT || _capturePointInfo[node]._ownerTeamId == player->GetTeamId() ||
        (_capturePointInfo[node]._state == GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED && player->GetTeamId() == TEAM_ALLIANCE) ||
        (_capturePointInfo[node]._state == GILNEAS_BG_NODE_STATUS_HORDE_CONTESTED && player->GetTeamId() == TEAM_HORDE))
        return;

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    uint32 sound = 0;
    // uint32 message = 0;
    // uint32 message2 = 0;
    DeleteBanner(node);
    CreateBanner(node, true);

    if (_capturePointInfo[node]._state == GILNEAS_BG_NODE_TYPE_NEUTRAL)
    {
        UpdatePlayerScore(player, SCORE_BASES_ASSAULTED, 1);
        _capturePointInfo[node]._state = static_cast<uint8>(GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED) + player->GetTeamId();
        _capturePointInfo[node]._ownerTeamId = TEAM_NEUTRAL;
        _bgEvents.RescheduleEvent(BG_BFG_EVENT_CAPTURE_LIGHTHOUSE + node, GILNEAS_BG_FLAG_CAPTURING_TIME);
        sound = GILNEAS_BG_SOUND_NODE_CLAIMED;
        // message = LANG_BG_BFG_NODE_CLAIMED;
        // message2 = player->GetTeamId() == TEAM_ALLIANCE ? LANG_BG_BFG_ALLY : LANG_BG_BFG_HORDE;
    }
    else if (_capturePointInfo[node]._state == GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED || _capturePointInfo[node]._state == GILNEAS_BG_NODE_STATUS_HORDE_CONTESTED)
    {
        if (!_capturePointInfo[node]._captured)
        {
            UpdatePlayerScore(player, SCORE_BASES_ASSAULTED, 1);
            _capturePointInfo[node]._state = static_cast<uint8>(GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED) + player->GetTeamId();
            _capturePointInfo[node]._ownerTeamId = TEAM_NEUTRAL;
            _bgEvents.RescheduleEvent(BG_BFG_EVENT_CAPTURE_LIGHTHOUSE + node, GILNEAS_BG_FLAG_CAPTURING_TIME);
            // message = LANG_BG_BFG_NODE_ASSAULTED;
        }
        else
        {
            UpdatePlayerScore(player, SCORE_BASES_DEFENDED, 1);
            _capturePointInfo[node]._state = static_cast<uint8>(GILNEAS_BG_NODE_STATUS_ALLY_OCCUPIED) + player->GetTeamId();
            _capturePointInfo[node]._ownerTeamId = player->GetTeamId();
            _bgEvents.CancelEvent(BG_BFG_EVENT_CAPTURE_LIGHTHOUSE + node);
            NodeOccupied(node); // after setting team owner
            // message = LANG_BG_BFG_NODE_DEFENDED;
        }
        sound = player->GetTeamId() == TEAM_ALLIANCE ? GILNEAS_BG_SOUND_NODE_ASSAULTED_ALLIANCE : GILNEAS_BG_SOUND_NODE_ASSAULTED_HORDE;
    }
    else
    {
        UpdatePlayerScore(player, SCORE_BASES_ASSAULTED, 1);
        NodeDeoccupied(node); // before setting team owner to neutral

        _capturePointInfo[node]._state = static_cast<uint8>(GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED) + player->GetTeamId();

        ApplyPhaseMask();
        _bgEvents.RescheduleEvent(BG_BFG_EVENT_CAPTURE_LIGHTHOUSE + node, GILNEAS_BG_FLAG_CAPTURING_TIME);
        // message = LANG_BG_BFG_NODE_ASSAULTED;
        sound = player->GetTeamId() == TEAM_ALLIANCE ? GILNEAS_BG_SOUND_NODE_ASSAULTED_ALLIANCE : GILNEAS_BG_SOUND_NODE_ASSAULTED_HORDE;
    }

    SendNodeUpdate(node);
    PlaySoundToAll(sound);
    // SendBroadcastText(message, player->GetTeamId() == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, player, LANG_BG_BFG_NODE_LIGHTHOUSE + node, message2);
}

TeamId BattlegroundBFG::GetPrematureWinner()
{
    if (_controlledPoints[TEAM_ALLIANCE] > _controlledPoints[TEAM_HORDE])
        return TEAM_ALLIANCE;
    return _controlledPoints[TEAM_HORDE] > _controlledPoints[TEAM_ALLIANCE] ? TEAM_HORDE : Battleground::GetPrematureWinner();
}

bool BattlegroundBFG::SetupBattleground() {

    AddObject(GILNEAS_BG_OBJECT_GATE_A_1, GILNEAS_BG_OBJECTID_GATE_A_1, GILNEAS_BG_DoorPositions[0][0], GILNEAS_BG_DoorPositions[0][1], GILNEAS_BG_DoorPositions[0][2], GILNEAS_BG_DoorPositions[0][3], GILNEAS_BG_DoorPositions[0][4], GILNEAS_BG_DoorPositions[0][5], GILNEAS_BG_DoorPositions[0][6], GILNEAS_BG_DoorPositions[0][7], RESPAWN_IMMEDIATELY);
    // AddObject(GILNEAS_BG_OBJECT_GATE_A_2, GILNEAS_BG_OBJECTID_GATE_A_2, GILNEAS_BG_DoorPositions[1][0], GILNEAS_BG_DoorPositions[1][1], GILNEAS_BG_DoorPositions[1][2], GILNEAS_BG_DoorPositions[1][3], GILNEAS_BG_DoorPositions[1][4], GILNEAS_BG_DoorPositions[1][5], GILNEAS_BG_DoorPositions[1][6], GILNEAS_BG_DoorPositions[1][7], RESPAWN_IMMEDIATELY);
    AddObject(GILNEAS_BG_OBJECT_GATE_H_1, GILNEAS_BG_OBJECTID_GATE_H_1, GILNEAS_BG_DoorPositions[2][0], GILNEAS_BG_DoorPositions[2][1], GILNEAS_BG_DoorPositions[2][2], GILNEAS_BG_DoorPositions[2][3], GILNEAS_BG_DoorPositions[2][4], GILNEAS_BG_DoorPositions[2][5], GILNEAS_BG_DoorPositions[2][6], GILNEAS_BG_DoorPositions[2][7], RESPAWN_IMMEDIATELY);
    // AddObject(GILNEAS_BG_OBJECT_GATE_H_2, GILNEAS_BG_OBJECTID_GATE_H_2, GILNEAS_BG_DoorPositions[3][0], GILNEAS_BG_DoorPositions[3][1], GILNEAS_BG_DoorPositions[3][2], GILNEAS_BG_DoorPositions[3][3], GILNEAS_BG_DoorPositions[3][4], GILNEAS_BG_DoorPositions[3][5], GILNEAS_BG_DoorPositions[3][6], GILNEAS_BG_DoorPositions[3][7], RESPAWN_IMMEDIATELY);

    // Buffs
    for (uint32 i = 0; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
    {
        AddObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + 3 * i,     Buff_Entries[0], GILNEAS_BG_BuffPositions[i][0], GILNEAS_BG_BuffPositions[i][1], GILNEAS_BG_BuffPositions[i][2], GILNEAS_BG_BuffPositions[i][3], 0, 0, sin(GILNEAS_BG_BuffPositions[i][3] / 2), cos(GILNEAS_BG_BuffPositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + 3 * i + 1, Buff_Entries[1], GILNEAS_BG_BuffPositions[i][0], GILNEAS_BG_BuffPositions[i][1], GILNEAS_BG_BuffPositions[i][2], GILNEAS_BG_BuffPositions[i][3], 0, 0, sin(GILNEAS_BG_BuffPositions[i][3] / 2), cos(GILNEAS_BG_BuffPositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + 3 * i + 2, Buff_Entries[2], GILNEAS_BG_BuffPositions[i][0], GILNEAS_BG_BuffPositions[i][1], GILNEAS_BG_BuffPositions[i][2], GILNEAS_BG_BuffPositions[i][3], 0, 0, sin(GILNEAS_BG_BuffPositions[i][3] / 2), cos(GILNEAS_BG_BuffPositions[i][3] / 2), RESPAWN_ONE_DAY);
    }

    AddSpiritGuide(GILNEAS_BG_SPIRIT_ALLIANCE, GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_ALLIANCE][0], GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_ALLIANCE][1], GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_ALLIANCE][2], GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_ALLIANCE][3], TEAM_ALLIANCE);
    AddSpiritGuide(GILNEAS_BG_SPIRIT_HORDE,    GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_HORDE][0],    GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_HORDE][1],   GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_HORDE][2],   GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_SPIRIT_HORDE][3], TEAM_HORDE);

    for (uint32 i = 0; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
    {
        AddObject(GILNEAS_BG_OBJECT_BANNER_NEUTRAL  + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_NODE_BANNER_0 + i, GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_BANNER_CONT_A   + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_BANNER_CONT_A,     GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_BANNER_CONT_H   + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_BANNER_CONT_H,     GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_BANNER_ALLY     + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_BANNER_A,          GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_BANNER_HORDE    + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_BANNER_H,          GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_AURA_ALLY       + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_AURA_A,            GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_AURA_HORDE      + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_AURA_H,            GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
        AddObject(GILNEAS_BG_OBJECT_AURA_CONTESTED  + GILNEAS_BG_OBJECT_PER_NODE * i, GILNEAS_BG_OBJECTID_AURA_C,            GILNEAS_BG_NodePositions[i][0], GILNEAS_BG_NodePositions[i][1], GILNEAS_BG_NodePositions[i][2], GILNEAS_BG_NodePositions[i][3], 0, 0, sin(GILNEAS_BG_NodePositions[i][3] / 2), cos(GILNEAS_BG_NodePositions[i][3] / 2), RESPAWN_ONE_DAY);
    }

    return true;
}

void BattlegroundBFG::Init()
{

      //call parent's class reset
    Battleground::Init();

    _bgEvents.Reset();

    // _honorTics = BattlegroundMgr::IsBGWeekend(GetBgTypeID()) ? BG_AB_HONOR_TICK_WEEKEND : BG_AB_HONOR_TICK_NORMAL;
    // _reputationTics = BattlegroundMgr::IsBGWeekend(GetBgTypeID()) ? BG_AB_REP_TICK_WEEKEND : BG_AB_REP_TICK_NORMAL;

    _capturePointInfo[GILNEAS_BG_NODE_LIGHTHOUSE]._iconNone    = GILNEAS_BG_OP_LIGHTHOUSE_ICON;
    _capturePointInfo[GILNEAS_BG_NODE_WATERWORKS]._iconNone    = GILNEAS_BG_OP_WATERWORKS_ICON;
    _capturePointInfo[GILNEAS_BG_NODE_MINE]._iconNone          = GILNEAS_BG_OP_MINE_ICON;
    _capturePointInfo[GILNEAS_BG_NODE_LIGHTHOUSE]._iconCapture = GILNEAS_BG_OP_LIGHTHOUSE_STATE_ALLIANCE;
    _capturePointInfo[GILNEAS_BG_NODE_WATERWORKS]._iconCapture = GILNEAS_BG_OP_WATERWORKS_STATE_ALLIANCE;
    _capturePointInfo[GILNEAS_BG_NODE_MINE]._iconCapture       = GILNEAS_BG_OP_MINE_STATE_ALLIANCE;
}

void BattlegroundBFG::EndBattleground(TeamId winnerTeamId)
{
    RewardHonorToTeam(GetBonusHonorFromKill(1), winnerTeamId);
    RewardHonorToTeam(GetBonusHonorFromKill(1), TEAM_HORDE);
    RewardHonorToTeam(GetBonusHonorFromKill(1), TEAM_ALLIANCE);
    Battleground::EndBattleground(winnerTeamId);
    _bgEvents.Reset();
}

GraveyardStruct const* BattlegroundBFG::GetClosestGraveyard(Player* player)
{
    GraveyardStruct const* entry = sGraveyard->GetGraveyard(GILNEAS_BG_GraveyardIds[static_cast<uint8>(GILNEAS_BG_SPIRIT_ALLIANCE) + player->GetTeamId()]);
    GraveyardStruct const* nearestEntry = entry;

    float pX = player->GetPositionX();
    float pY = player->GetPositionY();
    float dist = (entry->x - pX)*(entry->x - pX)+(entry->y - pY)*(entry->y - pY);
    float minDist = dist;

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePointInfo[i]._ownerTeamId == player->GetTeamId())
        {
            entry = sGraveyard->GetGraveyard(GILNEAS_BG_GraveyardIds[i]);
            dist = (entry->x - pX)*(entry->x - pX) + (entry->y - pY)*(entry->y - pY);
            if (dist < minDist)
            {
                minDist = dist;
                nearestEntry = entry;
            }
        }

    return nearestEntry;
}

bool BattlegroundBFG::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
        case SCORE_BASES_ASSAULTED:
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, GILNEAS_BG_OBJECTIVE_ASSAULT_BASE);
            break;
        case SCORE_BASES_DEFENDED:
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, GILNEAS_BG_OBJECTIVE_DEFEND_BASE);
            break;
        default:
            break;
    }

    return true;
}

bool BattlegroundBFG::AllNodesConrolledByTeam(TeamId teamId) const
{
    return _controlledPoints[teamId] == GILNEAS_BG_DYNAMIC_NODES_COUNT;
}

// bool BattlegroundBFG::CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* player, Unit const* target, uint32 miscvalue)
// {
//     switch (criteriaId)
//     {
//         case BG_CRITERIA_CHECK_RESILIENT_VICTORY:
//             return m_TeamScores500Disadvantage[player->GetTeamId()];
//     }

//     //return CheckAchievementCriteriaMeet(criteriaId, player, target, miscvalue);
//     return false;
// }

void BattlegroundBFG::ApplyPhaseMask()
{
    uint32 phaseMask = 1;
    for (uint32 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePointInfo[i]._ownerTeamId != TEAM_NEUTRAL)
            phaseMask |= 1 << (i*2+1 + _capturePointInfo[i]._ownerTeamId);

    const BattlegroundPlayerMap& bgPlayerMap = GetPlayers();
    for (BattlegroundPlayerMap::const_iterator itr = bgPlayerMap.begin(); itr != bgPlayerMap.end(); ++itr)
    {
        itr->second->SetPhaseMask(phaseMask, false);
        itr->second->UpdateObjectVisibility(true, false);
    }
}


class BattleForGilneasWorld : public WorldScript
{
	public:
    	BattleForGilneasWorld() : WorldScript("BattleForGilneasWorld") { }
};

void AddBattleForGilneasScripts() {
	new BattleForGilneasWorld();

	// Add Battle for Gilneas to battleground list
	BattlegroundMgr::queueToBg[BATTLEGROUND_QUEUE_BFG] = BATTLEGROUND_BFG;
	BattlegroundMgr::bgToQueue[BATTLEGROUND_BFG] = BATTLEGROUND_QUEUE_BFG;
	BattlegroundMgr::bgtypeToBattleground[BATTLEGROUND_BFG] = new BattlegroundBFG;

	BattlegroundMgr::bgTypeToTemplate[BATTLEGROUND_BFG] = [](Battleground *bg_t) -> Battleground * { return new BattlegroundBFG(*(BattlegroundBFG *) bg_t); };

	// BattlegroundMgr::getBgFromTypeID[BATTLEGROUND_BFG] = [](WorldPacket* data, Battleground::BattlegroundScoreMap::const_iterator itr2, Battleground* /* bg */) {
    //     *data << uint32(0x00000002);            // count of next fields
    //     *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesAssaulted);      // bases asssaulted
    //     *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesDefended);       // bases defended
	// };

	// BattlegroundMgr::getBgFromMap[761] = [](WorldPacket* data, Battleground::BattlegroundScoreMap::const_iterator itr2) {
    //     *data << uint32(0x00000002);            // count of next fields
    //     *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesAssaulted);      // bases asssaulted
    //     *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesDefended);       // bases defended
	// };

    Player::bgZoneIdToFillWorldStates[5449] = [](Battleground* bg, WorldPacket& data) {
        if (bg && bg->GetBgTypeID(true) == BATTLEGROUND_BFG) {
          bg->FillInitialWorldStates(data);
        }
    };
}
