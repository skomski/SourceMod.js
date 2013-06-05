(function(){

var playerManager = null;

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
	unit.setDataEnt(0x2738, waypoint);
}

dota.setHeroLevel = function(hero, level){
	var levelDiff = level - hero.netprops.m_iCurrentLevel;
	
	if(levelDiff != 0){
		hero.netprops.m_iCurrentLevel = level;
		hero.netprops.m_flStrength += hero.datamaps.m_flStrengthGain * levelDiff;
		hero.netprops.m_flAgility += hero.datamaps.m_flAgilityGain * levelDiff;
		hero.netprops.m_flIntellect += hero.datamaps.m_flIntellectGain * levelDiff;
		
		hero.netprops.m_iAbilityPoints = Math.max(0, hero.netprops.m_iAbilityPoints + levelDiff);
	}
	
	var expRequired = dota.getTotalExpRequiredForLevel(level);
	if(hero.netprops.m_iCurrentXP < expRequired){
		hero.netprops.m_iCurrentXP = expRequired;
	}
}

dota.findClientByPlayerID = function(playerId){
	for(var i = 0; i < server.clients.length; ++i){
		var c = server.clients[i];
		if(c == null) continue;
		if(!c.isInGame()) continue;
		if(c.netprops.m_iPlayerID === playerId) return c;
	}
	return null;
}
/*
game.hook("OnMapStart", function(){
	playerManager = game.findEntityByClassname(-1, "dota_player_manager");
});*/

Client.prototype.getPlayerID = function(){
	if(this._cachedPlayerID && this._cachedPlayerID != -1) return this._cachedPlayerID;
	return this._cachedPlayerID = this.netprops.m_iPlayerID;
}

Client.prototype.getHeroes = function(){
	var hero = this.netprops.m_hAssignedHero;
	if(hero == null){
		return [];
	}
	
	// Cache the check if this client has meepo
	if(typeof this._isMeepo == 'undefined'){
		this._isMeepo = (hero.getClassname() == 'npc_dota_hero_meepo');
	}
	
	if(this._isMeepo){
		// Ensure that the primary meepo will always be the first one in the array
		var meepos = game.findEntitiesByClassname('npc_dota_hero_meepo');
		var i = 0;
		if((i = meepos.indexOf(hero)) != 0){
			var tmp = meepos[0];
			meepos[0] = hero;
			meepos[i] = tmp;
		}
		
		return meepos;
	}else{
		return [hero];
	}
}



})();