(function(){
var totalExpRequired = [
	0,
	0,
	200,
	500,
	900,
	1400,
	2000,
	2600,
	3200,
	4400,
	5400,
	6500,
	7700,
	9000,
	10400,
	11900,
	13500,
	15200,
	17000,
	18900,
	20900,
	23000,
	25200,
	27500,
	29900,
	32400
];

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

dota.setHeroLevel = function(client, hero, level){
	var levelDiff = level - hero.netprops.m_iCurrentLevel;
	
	if(levelDiff != 0){
		hero.netprops.m_iCurrentLevel = level;
		hero.netprops.m_flStrength += hero.datamaps.m_flStrengthGain * levelDiff;
		hero.netprops.m_flAgility += hero.datamaps.m_flAgilityGain * levelDiff;
		hero.netprops.m_flIntellect += hero.datamaps.m_flIntellectGain * levelDiff;
		
		hero.netprops.m_iAbilityPoints = Math.max(0, hero.netprops.m_iAbilityPoints + levelDiff);
		
		if(client) playerManager.netprops.m_iLevel[client.netprops.m_iPlayerID] = level;
	}
	
	if(hero.netprops.m_iCurrentXP < totalExpRequired[level]){
		hero.netprops.m_iCurrentXP = totalExpRequired[level];
	}
}

dota.getTotalExpRequiredForLevel = function(level){
	return totalExpRequired[level];
}

Client.prototype.getPlayerID = function(){
	if(this._cachedPlayerID && this._cachedPlayerID != -1) return this._cachedPlayerID;
	return this._cachedPlayerID = this.netprops.m_iPlayerID;
}

new Client();

})();