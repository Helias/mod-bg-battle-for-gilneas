/*
 * Copyright (C) ArkCORE
 * Copyright (C) 2019+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: http://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
*/

#ifndef __BATTLEGROUNDBFG_H
#define __BATTLEGROUNDBFG_H

#include "Battleground.h"
#include "Object.h"

enum BG_BFG_Events
{
    BG_BFG_EVENT_UPDATE_BANNER_LIGHTHOUSE    = 1,
    BG_BFG_EVENT_UPDATE_BANNER_WATERWORKS    = 2,
    BG_BFG_EVENT_UPDATE_BANNER_MINE          = 3,

    BG_BFG_EVENT_CAPTURE_LIGHTHOUSE          = 4,
    BG_BFG_EVENT_CAPTURE_WATERWORKS          = 5,
    BG_BFG_EVENT_CAPTURE_MINE                = 6,

    BG_BFG_EVENT_ALLIANCE_TICK               = 7,
    BG_BFG_EVENT_HORDE_TICK                  = 8
};

enum BG_BFG_Misc
{
    BG_BFG_OBJECTIVE_ASSAULT_BASE        = 122,
    BG_BFG_OBJECTIVE_DEFEND_BASE         = 123,
    BG_BFG_EVENT_START_BATTLE            = 9158, // Achievement: Let's Get This Done
    BG_BFG_QUEST_CREDIT_BASE             = 15001,

    BG_BFG_HONOR_TICK_NORMAL             = 260,
    BG_BFG_HONOR_TICK_WEEKEND            = 160,
    BG_BFG_REP_TICK_NORMAL               = 160,
    BG_BFG_REP_TICK_WEEKEND              = 120,

    BG_BFG_FLAG_CAPTURING_TIME           = 60000,
    BG_BFG_BANNER_UPDATE_TIME            = 2000
};


enum BattlegroundCriteriaId
{
    BG_CRITERIA_CHECK_RESILIENT_VICTORY,
    BG_CRITERIA_CHECK_SAVE_THE_DAY,
    BG_CRITERIA_CHECK_EVERYTHING_COUNTS,
    BG_CRITERIA_CHECK_AV_PERFECTION,
    BG_CRITERIA_CHECK_DEFENSE_OF_THE_ANCIENTS,
    BG_CRITERIA_CHECK_NOT_EVEN_A_SCRATCH,
};

enum BattleForGilneasStrings {
    // Battle For Gilneas
    LANG_BG_BFG_START_TWO_MINUTES           = 12015,
    LANG_BG_BFG_START_ONE_MINUTE            = 12016,
    LANG_BG_BFG_START_HALF_MINUTE           = 12017,
    LANG_BG_BFG_HAS_BEGUN                   = 12018,

    LANG_BG_BFG_ALLY                        = 12019,
    LANG_BG_BFG_HORDE                       = 12020,

    LANG_BG_BFG_NODE_LIGHTHOUSE             = 12021,
    LANG_BG_BFG_NODE_WATERWORKS             = 12022,
    LANG_BG_BFG_NODE_MINE                   = 12023,
    LANG_BG_BFG_NODE_TAKEN                  = 12024,
    LANG_BG_BFG_NODE_DEFENDED               = 12025,
    LANG_BG_BFG_NODE_ASSAULTED              = 12026,
    LANG_BG_BFG_NODE_CLAIMED                = 12027,

    LANG_BG_BFG_A_NEAR_VICTORY              = 12028,
    LANG_BG_BFG_H_NEAR_VICTORY              = 12029,
};

enum GILNEAS_BG_WorldStates
{
    GILNEAS_BG_OP_OCCUPIED_BASES_HORDE      = 6201,
    GILNEAS_BG_OP_OCCUPIED_BASES_ALLY       = 6200,
    GILNEAS_BG_OP_RESOURCES_ALLY            = 6202,
    GILNEAS_BG_OP_RESOURCES_HORDE           = 6203,
    GILNEAS_BG_OP_RESOURCES_MAX             = 6204,
    GILNEAS_BG_OP_RESOURCES_WARNING         = 1955,

    GILNEAS_BG_OP_LIGHTHOUSE_ICON           = 1842,
    GILNEAS_BG_OP_LIGHTHOUSE_STATE_ALLIANCE = 1767,
    GILNEAS_BG_OP_LIGHTHOUSE_STATE_HORDE    = 1768,
    GILNEAS_BG_OP_LIGHTHOUSE_STATE_CON_ALI  = 1769,
    GILNEAS_BG_OP_LIGHTHOUSE_STATE_CON_HOR  = 1770,

    GILNEAS_BG_OP_WATERWORKS_ICON           = 1846,
    GILNEAS_BG_OP_WATERWORKS_STATE_ALLIANCE = 1782,
    GILNEAS_BG_OP_WATERWORKS_STATE_HORDE    = 1783,
    GILNEAS_BG_OP_WATERWORKS_STATE_CON_ALI  = 1784,
    GILNEAS_BG_OP_WATERWORKS_STATE_CON_HOR  = 1785,

    GILNEAS_BG_OP_MINE_ICON                 = 1845,
    GILNEAS_BG_OP_MINE_STATE_ALLIANCE       = 1772,
    GILNEAS_BG_OP_MINE_STATE_HORDE          = 1773,
    GILNEAS_BG_OP_MINE_STATE_CON_ALI        = 1774,
    GILNEAS_BG_OP_MINE_STATE_CON_HOR        = 1775
};

// const uint32 GILNEAS_BG_OP_NODESTATES[3] = { 1767, 1772, 1782 };
// const uint32 GILNEAS_BG_OP_NODEICONS[3] =  { 1842, 1845, 1846 };

enum GILNEAS_BG_NodeObjectId
{
    GILNEAS_BG_OBJECTID_NODE_BANNER_0 = 208779,       // Lighthouse banner
    GILNEAS_BG_OBJECTID_NODE_BANNER_1 = 208780,       // Waterworks banner
    GILNEAS_BG_OBJECTID_NODE_BANNER_2 = 208781,       // Mines banner
};

enum GILNEAS_BG_ObjectType
{
    GILNEAS_BG_OBJECT_BANNER_NEUTRAL          = 0,
    GILNEAS_BG_OBJECT_BANNER_ALLY             = 1,
    GILNEAS_BG_OBJECT_BANNER_HORDE            = 2,
    GILNEAS_BG_OBJECT_BANNER_CONT_A           = 3,
    GILNEAS_BG_OBJECT_BANNER_CONT_H           = 4,
    GILNEAS_BG_OBJECT_AURA_ALLY               = 5,
    GILNEAS_BG_OBJECT_AURA_HORDE              = 6,
    GILNEAS_BG_OBJECT_AURA_CONTESTED          = 7,
    GILNEAS_BG_OBJECT_PER_NODE                = 8,

    GILNEAS_BG_OBJECT_GATE_A_1                = 24,
    GILNEAS_BG_OBJECT_GATE_H_1                = 25,
    // GILNEAS_BG_OBJECT_GATE_A_2                = 26,
    // GILNEAS_BG_OBJECT_GATE_H_2                = 27,

    //buffs
    GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE    = 28,
    GILNEAS_BG_OBJECT_REGENBUFF_LIGHTHOUSE    = 29,
    GILNEAS_BG_OBJECT_BERSERKBUFF_LIGHTHOUSE  = 30,
    GILNEAS_BG_OBJECT_SPEEDBUFF_WATERWORKS    = 31,
    GILNEAS_BG_OBJECT_REGENBUFF_WATERWORKS    = 32,
    GILNEAS_BG_OBJECT_BERSERKBUFF_WATERWORKS  = 33,
    GILNEAS_BG_OBJECT_SPEEDBUFF_MINE          = 34,
    GILNEAS_BG_OBJECT_REGENBUFF_MINE          = 35,
    GILNEAS_BG_OBJECT_BERSERKBUFF_MINE        = 36,
    GILNEAS_BG_OBJECT_MAX                     = 37,
};

enum GILNEAS_BG_ObjectTypes
{
    GILNEAS_BG_OBJECTID_BANNER_A      = 180058,
    GILNEAS_BG_OBJECTID_BANNER_CONT_A = 180059,
    GILNEAS_BG_OBJECTID_BANNER_H      = 180060,
    GILNEAS_BG_OBJECTID_BANNER_CONT_H = 180061,

    GILNEAS_BG_OBJECTID_AURA_A        = 180100,
    GILNEAS_BG_OBJECTID_AURA_H        = 180101,
    GILNEAS_BG_OBJECTID_AURA_C        = 180102,

    GILNEAS_BG_OBJECTID_GATE_A_1      = 207177,
    // GILNEAS_BG_OBJECTID_GATE_A_2      = 180322,
    GILNEAS_BG_OBJECTID_GATE_H_1      = 207178,
    // GILNEAS_BG_OBJECTID_GATE_H_2      = 180322,
};

enum GILNEAS_BG_Timers
{
    GILNEAS_BG_FLAG_CAPTURING_TIME = 60000,
};

enum GILNEAS_BG_Score
{
    GILNEAS_BG_WARNING_NEAR_VICTORY_SCORE = 1800,
    GILNEAS_BG_MAX_TEAM_SCORE             = 2000
};

enum GILNEAS_BG_BattlegroundNodes
{
    GILNEAS_BG_NODE_LIGHTHOUSE      = 0,
    GILNEAS_BG_NODE_WATERWORKS      = 1,
    GILNEAS_BG_NODE_MINE            = 2,

    GILNEAS_BG_DYNAMIC_NODES_COUNT  = 3,                        // Dynamic nodes that can be captured

    GILNEAS_BG_SPIRIT_ALLIANCE      = 3,
    GILNEAS_BG_SPIRIT_HORDE         = 4,

    GILNEAS_BG_ALL_NODES_COUNT      = 5,                        // All nodes (dynamic and static)
};

enum GILNEAS_BG_NodeStatus
{
    GILNEAS_BG_NODE_TYPE_NEUTRAL             = 0,
    GILNEAS_BG_NODE_STATUS_ALLY_OCCUPIED     = 1,
    GILNEAS_BG_NODE_STATUS_HORDE_OCCUPIED    = 2,
    GILNEAS_BG_NODE_STATUS_ALLY_CONTESTED    = 3,
    GILNEAS_BG_NODE_STATUS_HORDE_CONTESTED   = 4
};

enum GILNEAS_BG_Sounds
{
    GILNEAS_BG_SOUND_NODE_CLAIMED            = 8192,
    GILNEAS_BG_SOUND_NODE_CAPTURED_ALLIANCE  = 8173,
    GILNEAS_BG_SOUND_NODE_CAPTURED_HORDE     = 8213,
    GILNEAS_BG_SOUND_NODE_ASSAULTED_ALLIANCE = 8212,
    GILNEAS_BG_SOUND_NODE_ASSAULTED_HORDE    = 8174,
    GILNEAS_BG_SOUND_NEAR_VICTORY            = 8456
};

enum GILNEAS_BG_Objectives
{
    GILNEAS_BG_OBJECTIVE_ASSAULT_BASE = 370,
    GILNEAS_BG_OBJECTIVE_DEFEND_BASE  = 371
};

#define GILNEAS_BG_NotBGWeekendHonorTicks   260
#define GILNEAS_BG_BGWeekendHonorTicks      160
#define GILNEAS_BG_NotBGWeekendRepTicks     160
#define GILNEAS_BG_BGWeekendRepTicks        120

#define BG_EVENT_START_BATTLE               9158

const float GILNEAS_BG_NodePositions[GILNEAS_BG_DYNAMIC_NODES_COUNT][4] =
{
    { 1057.790f, 1278.285f, 3.1500f, 1.945662f },       // Lighthouse
    { 980.0446f, 948.7411f, 12.650f, 5.904071f },       // Mine
    { 1251.010f, 958.2685f, 5.6000f, 5.892280f }        // Waterworks
};

// x, y, z, o, rot0, rot1, rot2, rot3
const float GILNEAS_BG_DoorPositions[4][8] =
{
    { 918.160f, 1336.75f, 27.6299f, 2.87927f, 0.0f, 0.0f, 0.983231f, 0.182367f },
    { 918.160f, 1336.75f, 26.6299f, 2.87927f, 0.0f, 0.0f, 0.983231f, 0.182367f },
    { 1396.15f, 977.014f, 7.43169f, 6.27043f, 0.0f, 0.0f, 0.006378f, -0.99998f },
    { 1396.15f, 977.014f, 0.33169f, 6.27043f, 0.0f, 0.0f, 0.006378f, -0.99998f }
};

const uint32 GILNEAS_BG_TickIntervals[4] = { 0, 12000, 6000, 1000 };
const uint32 GILNEAS_BG_TickPoints[4] = { 0, 10, 10, 30 };

const uint32 GILNEAS_BG_GraveyardIds[GILNEAS_BG_ALL_NODES_COUNT] = { 1736, 1737, 1735, 1739, 1738 };

// x, y, z, o
const float GILNEAS_BG_SpiritGuidePos[GILNEAS_BG_ALL_NODES_COUNT][4] =
{
    { 1034.82f, 1335.58f, 12.0095f, 5.15f },     // Lighthouse
    { 1252.23f, 836.547f, 27.7895f, 1.60f },     // Waterworks
    { 887.578f, 937.337f, 23.7737f, 0.45f },     // Mine
    { 908.274f, 1338.60f, 27.6449f, 5.95f },     // Alliance
    { 1401.38f, 977.125f, 7.44215f, 3.04f }      // Horde
};

const float GILNEAS_BG_BuffPositions[GILNEAS_BG_DYNAMIC_NODES_COUNT][4] =
{
    { 1063.57f, 1313.42f, 4.91f, 4.14f },        // Lighthouse
    { 961.830f, 977.03f, 14.15f, 4.55f },        // Waterworks
    { 1193.09f, 1017.46f, 7.98f, 0.24f }         // Mine
};

struct GILNEAS_BG_BannerTimer
{
    uint32 timer;
    uint8  type;
    uint8  teamIndex;
};

struct BattlegroundBFGScore : public BattlegroundScore
{
    BattlegroundBFGScore(Player* player) : BattlegroundScore(player), BasesAssaulted(0), BasesDefended(0) {}
    ~BattlegroundBFGScore() {}
    uint32 BasesAssaulted;
    uint32 BasesDefended;
};

class BattlegroundBFG : public Battleground
{
public:
    BattlegroundBFG();
    ~BattlegroundBFG();

    void AddPlayer(Player* player);
    void StartingEventCloseDoors();
    void StartingEventOpenDoors();
    void RemovePlayer(Player* player);
    void HandleAreaTrigger(Player* player, uint32 trigger);
    bool SetupBattleground();
    void Init();
    void EndBattleground(TeamId winnerTeamId);
    GraveyardStruct const* GetClosestGraveyard(Player* player);

    void UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true);
    void FillInitialWorldStates(WorldPacket& data);
    void EventPlayerClickedOnFlag(Player* source, GameObject* gameObject);

    bool AllNodesConrolledByTeam(TeamId teamId) const;
    bool IsTeamScores500Disadvantage(TeamId teamId) const { return _teamScores500Disadvantage[teamId]; }
    
    TeamId GetPrematureWinner();
private:
    void PostUpdateImpl(uint32 diff);

    void DeleteBanner(uint8 node);
    void CreateBanner(uint8 node, bool delay);
    void SendNodeUpdate(uint8 node);
    void NodeOccupied(uint8 node);
    void NodeDeoccupied(uint8 node);
    void ApplyPhaseMask();

    struct CapturePointInfo
    {
        CapturePointInfo() : _ownerTeamId(TEAM_NEUTRAL), _iconNone(0), _iconCapture(0), _state(GILNEAS_BG_NODE_TYPE_NEUTRAL), _captured(false)
        {
        }

        TeamId _ownerTeamId;
        uint32 _iconNone;
        uint32 _iconCapture;
        uint8 _state;

        bool _captured;
    };

    CapturePointInfo _capturePointInfo[GILNEAS_BG_DYNAMIC_NODES_COUNT];
    EventMap _bgEvents;
    uint32 _honorTics;
    uint32 _reputationTics;
    uint8 _controlledPoints[BG_TEAMS_COUNT];
    bool _teamScores500Disadvantage[BG_TEAMS_COUNT];

//     void AddPlayer(Player* player);
//     void StartingEventCloseDoors();
//     void StartingEventOpenDoors();
//     void RemovePlayer(Player* player, uint64 guid, uint32 team);
//     void HandleAreaTrigger(Player* Source, uint32 Trigger);
//     bool SetupBattleground();
//     void Reset();
//     void EndBattleground(TeamId winner);
//     GraveyardStruct const* GetClosestGraveYard(Player* player);

//     /* Scorekeeping */
//     void UpdatePlayerScore(Player* Source, uint32 type, uint32 value, bool doAddHonor = true);

//     void FillInitialWorldStates(WorldPacket& builder);

//     /* Nodes occupying */
//     void EventPlayerClickedOnFlag(Player* source, GameObject* target_obj);

//     /* achievement req. */
//     bool IsAllNodesControlledByTeam(uint32 team) const;
//     bool CheckAchievementCriteriaMeet(uint32 /*criteriaId*/, Player const* /*player*/, Unit const* /*target*/ = NULL, uint32 /*miscvalue1*/ = 0);

//     TeamId GetPrematureWinner();
// private:
//     void PostUpdateImpl(uint32 diff);
//     /* Gameobject spawning/despawning */
//     void _CreateBanner(uint8 node, uint8 type, uint8 teamIndex, bool delay);
//     void _DelBanner(uint8 node, uint8 type, uint8 teamIndex);
//     void _SendNodeUpdate(uint8 node);

//     /* Creature spawning/despawning */
//     void NodeOccupied(uint8 node, TeamId team);
//     void NodeDeoccupied(uint8 node);

//     int32 _GetNodeNameId(uint8 node);
//     /* Nodes info:
//     0: neutral
//     1: ally contested
//     2: horde contested
//     3: ally occupied
//     4: horde occupied     */
//     uint8                    m_Nodes[GILNEAS_BG_DYNAMIC_NODES_COUNT];
//     uint8                    m_prevNodes[GILNEAS_BG_DYNAMIC_NODES_COUNT];
//     GILNEAS_BG_BannerTimer   m_BannerTimers[GILNEAS_BG_DYNAMIC_NODES_COUNT];
//     uint32                   m_NodeTimers[GILNEAS_BG_DYNAMIC_NODES_COUNT];
//     uint32                   m_lastTick[BG_TEAMS_COUNT];
//     uint32                   m_HonorScoreTics[BG_TEAMS_COUNT];
//     uint32                   m_ReputationScoreTics[BG_TEAMS_COUNT];
//     bool                     m_IsInformedNearVictory;
//     uint32                   m_HonorTics;
//     uint32                   m_ReputationTics;
//     // need for achievements
//     bool                     m_TeamScores500Disadvantage[BG_TEAMS_COUNT];
};

#endif
