dota.MAX_PLAYERS = 24;

dota.STATE_INIT = 0;
dota.STATE_WAIT_FOR_PLAYERS_TO_LOAD = 1;
dota.STATE_HERO_SELECTION = 2;
dota.STATE_STRATEGY_TIME = 3;
dota.STATE_PRE_GAME = 4;
dota.STATE_GAME_IN_PROGRESS = 5;
dota.STATE_POST_GAME = 6;
dota.STATE_DISCONNECT = 7;

dota.TEAM_NONE = 0;
dota.TEAM_SPEC = 1;
dota.TEAM_RADI = 2;
dota.TEAM_DIRE = 3;
dota.TEAM_NEUTRAL = 4;
dota.TEAM_RADIANT = 2;

dota.DAMAGE_TYPE_PHYSICAL =   1 << 0;
dota.DAMAGE_TYPE_MAGICAL =    1 << 1;
dota.DAMAGE_TYPE_COMPOSITE =  1 << 2;
dota.DAMAGE_TYPE_PURE =       1 << 3;
dota.DAMAGE_TYPE_HP_REMOVAL = 1 << 4;

dota.setUnitWaypoint = function(unit, waypoint){
	unit.setDataEnt(10036, waypoint);
}
