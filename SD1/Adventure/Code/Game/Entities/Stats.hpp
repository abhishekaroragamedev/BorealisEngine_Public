#pragma once

enum StatID
{
	INVALID_STAT_ID = -1,
	STAT_HEALTH,	// Determines maximum health
	STAT_STRENGTH,	// Determines how much damage attacks do (offensive check)
	STAT_RESILIENCE,	// Determines how much damage is absorbed (defensive check)
	STAT_AGILITY,	// Determines how fast an Entity moves
	STAT_DEXTERITY,		// Determines how fast an Actor can attack (offensive check)
	STAT_AIM,	// Determines how accurately an Actor fires projectiles (offensive check)
	STAT_ACCURACY,	// Determines how likely melee attacks are to land (offensive check)
	STAT_EVASIVENESS,	// Determines how likely melee and projectile attacks are to miss (defensive check)
	STAT_POISON_USAGE,		// Can be used for both offensive and defensive stat checks, according to context
	STAT_FIRE_USAGE,		// Can be used for both offensive and defensive stat checks, according to context
	STAT_ICE_USAGE,		// Can be used for both offensive and defensive stat checks, according to context
	STAT_ELECTRICITY_USAGE,		// Can be used for both offensive and defensive stat checks, according to context
	NUM_STATS
};