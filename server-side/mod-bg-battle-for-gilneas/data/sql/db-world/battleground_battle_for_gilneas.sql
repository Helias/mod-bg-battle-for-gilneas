-- acore_string
DELETE FROM `acore_string` WHERE entry BETWEEN 12015 AND 12029;
INSERT INTO `acore_string` (`entry`,`content_default`) VALUES
(12015,'The Battle for Gilneas begins in 2 minutes.'),
(12016,'The Battle for Gilneas begins in 1 minute.'),
(12017,'The Battle for Gilneas begins in 30 seconds. Prepare yourselves!'),
(12018,'The Battle for Gilneas has begun!'),
(12019,'Alliance'),
(12020,'Horde'),
(12021,'Lighthouse'),
(12022,'Waterworks'),
(12023,'Mine'),
(12024,'The %s has taken the %s'),
(12025,'$N has defended the %s'),
(12026,'$N has assaulted the %s'),
(12027,'$N claims the %s! If left unchallenged, the %s will control it in 1 minute!'),
(12028,'The Alliance has gathered $6202W resources, and is near victory!'),
(12029,'The Horde has gathered $6203W resources, and is near victory!');

-- battleground_template
DELETE FROM `battleground_template` WHERE id=120;
INSERT INTO `battleground_template` (`id`,`MinPlayersPerTeam`,`MaxPlayersPerTeam`,`MinLvl`,`MaxLvl`,`AllianceStartLoc`,`AllianceStartO`,`HordeStartLoc`,`HordeStartO`,`StartMaxDist`,`Weight`,`ScriptName`,`Comment`) VALUES
(120,5,10,80,80,1739,0,1738,0,75,1,'','Battle for Gilneas');

-- game_graveyard
DELETE FROM `game_graveyard` WHERE ID IN (1735,1736,1737,1738,1739,1798,1799);
INSERT INTO `game_graveyard` (`ID`, `Map`, `x`, `y`, `z`, `Comment`) VALUES
(1735,761,1252.22998047,836.54699707,27.7894992828,"Gilneas BG - Graveyard (H-Mid)"),
(1736,761,1034.81994629,1335.57995605,12.0094995499,"Gilneas BG - Graveyard (A-Mid)"),
(1737,761,887.57800293,937.336975098,23.7737007141,"Gilneas BG - Graveyard (Mid)"),
(1738,761,1401.38000488,977.125,7.44215011597,"Gilneas BG - Graveyard (Horde Start)"),
(1739,761,908.273986816,1338.59997559,27.6448993683,"Gilneas BG - Graveyard (Alliance Start)"),
(1798,761,909.46,1337.36,27.6449,"Gilneas BG - Graveyard (Alliance Spawn)"),
(1799,761,1430.44,983.861,0.225735,"Gilneas BG - Graveyard (Horde Spawn)");

-- area trigger
DELETE FROM `areatrigger` WHERE entry IN (6265, 6266, 6267, 6268, 6269, 6447, 6448);
INSERT INTO `areatrigger` (`entry`,`map`,`x`,`y`,`z`,`radius`,`length`,`width`,`height`,`orientation`) VALUES
(6265,761,990.267,984.076,12.9949,4,0,0,0,0),
(6266,761,1110.99,921.877,27.545,5,0,0,0,0),
(6267,761,966.826,1044.16,13.1475,4,0,0,0,0),
(6268,761,1195.37,1020.52,7.97874,4,0,0,0,0),
(6269,761,1064.07,1309.32,4.91045,4,0,0,0,0),
(6447,761,870.908,1349.82,27.6449,53.8516,40,100,76.78,6.255),
(6448,761,1418.56,996.026,7.39786,42.4679,40,100,64.14,0);

-- gameobject_template
DELETE FROM `gameobject_template` WHERE entry IN (208779, 208780, 208781, 207177, 207178);
INSERT INTO `gameobject_template` (`entry`,`type`,`displayId`,`name`,`IconName`,`castBarCaption`,`unk1`,`size`,`Data0`,`Data1`,`Data2`,`Data3`,`Data4`,`Data5`,`Data6`,`Data7`,`Data8`,`Data9`,`Data10`,`Data11`,`Data12`,`Data13`,`Data14`,`Data15`,`Data16`,`Data17`,`Data18`,`Data19`,`Data20`,`Data21`,`Data22`,`Data23`,`AIName`,`ScriptName`,`VerifiedBuild`) VALUES
(208779,10,6271,"Lighthouse Banner","","Capturing","",1,1479,0,0,3000,0,0,0,0,0,0,23932,1,0,1,37190,0,1,0,0,0,0,0,0,0,"","",12340),
(208780,10,6271,"Waterworks banner","","Capturing","",1,1479,0,0,3000,0,0,0,0,0,0,23936,1,0,1,0,0,1,0,0,0,0,0,0,0,"","",12340),
(208781,10,6271,"Mines banner","","Capturing","",1,1479,0,0,3000,0,0,0,0,0,0,23936,1,0,1,0,0,1,0,0,0,0,0,0,0,"","",12340),
(207177,0,9062,"Alliance Door","","","",0.34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"","",12340),
(207178,0,10215,"Horde Gate","","","",1.05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"","",12340);

-- gameobject_template_addon
DELETE FROM `gameobject_template_addon` WHERE entry IN (208779, 208780, 208781, 207177, 207178);
INSERT INTO `gameobject_template_addon` (`entry`,`faction`,`flags`,`mingold`,`maxgold`) VALUES
(208779,35,32,0,0),
(208780,35,0,0,0),
(208781,35,0,0,0),
(207177,114,32,0,0),
(207178,114,32,0,0);
