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
dota.TEAM_RADIANT = 2;
dota.TEAM_DIRE = 3;
dota.TEAM_NEUTRAL = 4;


dota.DAMAGE_TYPE_PHYSICAL =   1 << 0;
dota.DAMAGE_TYPE_MAGICAL =    1 << 1;
dota.DAMAGE_TYPE_COMPOSITE =  1 << 2;
dota.DAMAGE_TYPE_PURE =       1 << 3;
dota.DAMAGE_TYPE_HP_REMOVAL = 1 << 4;

dota.UNIT_CAP_NO_ATTACK = 0;
dota.UNIT_CAP_MELEE_ATTACK = 1;
dota.UNIT_CAP_RANGED_ATTACK = 2;

dota.UNIT_CAP_MOVE_NONE = 0;
dota.UNIT_CAP_MOVE_GROUND = 1;
dota.UNIT_CAP_MOVE_FLY = 2;

dota.COMBAT_CLASS_ATTACK_LIGHT = 0;
dota.COMBAT_CLASS_ATTACK_HERO = 1;
dota.COMBAT_CLASS_ATTACK_BASIC = 2;
dota.COMBAT_CLASS_ATTACK_PIERCE = 3;
dota.COMBAT_CLASS_ATTACK_SIEGE = 4;

dota.COMBAT_CLASS_DEFEND_WEAK = 0;
dota.COMBAT_CLASS_DEFEND_BASIC = 1;
dota.COMBAT_CLASS_DEFEND_STRONG = 2;
dota.COMBAT_CLASS_DEFEND_STRUCTURE = 3;
dota.COMBAT_CLASS_DEFEND_HERO = 4;
dota.COMBAT_CLASS_DEFEND_SOFT = 5;

// The actual offset is around 0x2770, but it may change, so we store it as a relative
// offset from the closest prop we know
// This is found in the dota scripted spawner functions

var waypointOffset = game.getPropOffset("CDOTA_BaseNPC", "m_iDamageBonus") + 0x0014;
dota.setUnitWaypoint = function(unit, waypoint){
	unit.setDataEnt(waypointOffset, waypoint);
	dota._unitInvade(unit);
}

var moveCapabilitiesOffset = game.getPropOffset("CDOTA_BaseNPC", "m_iAttackCapabilities") + 0x0004;
dota.setMoveCapabilities = function(unit, cap){
	unit.setData(moveCapabilitiesOffset, 4, cap);
}

dota.setHeroLevel = function(hero, level){
	var levelDiff = level - hero.netprops.m_iCurrentLevel;
	if(levelDiff == 0) return;
	
	if(levelDiff > 0){
		for(var i = 0; i < levelDiff; ++i){
			dota.levelUpHero(hero, i == 0 /* playEffects only once */);
		}
	}else{
		// Deleveling a hero
		// This may be really buggy
		
		if(levelDiff != 0){
			hero.netprops.m_iCurrentLevel = level;
			hero.netprops.m_flStrength += hero.netprops.m_flStrengthGain * levelDiff;
			hero.netprops.m_flAgility += hero.netprops.m_flAgilityGain * levelDiff;
			hero.netprops.m_flIntellect += hero.netprops.m_flIntellectGain * levelDiff;
			
			hero.netprops.m_iAbilityPoints = Math.max(0, hero.netprops.m_iAbilityPoints + levelDiff);
		}
		
		var expRequired = dota.getTotalExpRequiredForLevel(level);
		if(hero.netprops.m_iCurrentXP < expRequired){
			hero.netprops.m_iCurrentXP = expRequired;
		}
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
		return game.findEntitiesByClassname('npc_dota_hero_meepo');
	}else{
		return [hero];
	}
}

dota.removeAll = function(type){
	game.findEntitiesByClassname(type).forEach(function(ent){
		dota.remove(ent);
	});
}

dota.clearMap = function(){
	dota.removeAll("npc_dota_tower*");
	dota.removeAll("npc_dota_fort*");
	dota.removeAll("npc_dota_barracks*");
	dota.removeAll("npc_dota_creep*");
	dota.removeAll("npc_dota_building*");
	dota.removeAll("ent_dota_fountain*");
	dota.removeAll("npc_dota_neutral_spawner*");
	dota.removeAll("npc_dota_roshan_spawner*");
	dota.removeAll("npc_dota_scripted_spawner*");
	dota.removeAll("npc_dota_spawner*");
	dota.removeAll("npc_dota_roshan*");
}

})();