#include "Game/Entities/Actor.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/Entities/Projectile.hpp"

Projectile::Projectile( const std::string& instanceName, Actor* attacker, WeaponType sourceWeaponType, const Vector2& position, const Vector2& velocity, ProjectileDefinition* projectileDefinition, const Map& map, int statMultipliers[ StatID::NUM_STATS ] )
	:	Entity( instanceName, position, projectileDefinition, map ),
		m_attacker( attacker ),
		m_sourceWeaponType( sourceWeaponType ),
		m_projectileDefinition( projectileDefinition )
{
	m_velocity = velocity;
	m_orientation = m_velocity.GetOrientationDegrees();
	m_remainingLifeTime = m_projectileDefinition->GetLifetime();
	InitializeStatsFromBaseAndMultipliers( statMultipliers );
}

Projectile::~Projectile()
{

}

void Projectile::InitializeStatsFromBaseAndMultipliers( int statMultipliers[ StatID::NUM_STATS ] )
{
	m_stats[ StatID::STAT_HEALTH ] = m_projectileDefinition->GetBaseStat( StatID::STAT_HEALTH ) + statMultipliers[ StatID::STAT_HEALTH ];

	m_stats[ StatID::STAT_STRENGTH ] = m_projectileDefinition->GetBaseStat( StatID::STAT_STRENGTH );
	if ( m_stats[ StatID::STAT_STRENGTH ] > 0 )
	{
		m_stats[ StatID::STAT_STRENGTH ] += statMultipliers[ StatID::STAT_STRENGTH ];
	}

	m_stats[ StatID::STAT_AGILITY ] = m_projectileDefinition->GetBaseStat( StatID::STAT_AGILITY ) + statMultipliers[ StatID::STAT_AGILITY ];

	m_stats[ StatID::STAT_POISON_USAGE ] = m_projectileDefinition->GetBaseStat( StatID::STAT_POISON_USAGE );
	if ( m_stats[ StatID::STAT_POISON_USAGE ] > 0 )
	{
		m_stats[ StatID::STAT_POISON_USAGE ] += statMultipliers[ StatID::STAT_POISON_USAGE ];
	}
	m_stats[ StatID::STAT_FIRE_USAGE ] = m_projectileDefinition->GetBaseStat( StatID::STAT_FIRE_USAGE );
	if ( m_stats[ StatID::STAT_FIRE_USAGE ] > 0 )
	{
		m_stats[ StatID::STAT_FIRE_USAGE ] += statMultipliers[ StatID::STAT_FIRE_USAGE ];
	}
	m_stats[ StatID::STAT_ICE_USAGE ] = m_projectileDefinition->GetBaseStat( StatID::STAT_ICE_USAGE );
	if ( m_stats[ StatID::STAT_ICE_USAGE ] > 0 )
	{
		m_stats[ StatID::STAT_ICE_USAGE ] += statMultipliers[ StatID::STAT_ICE_USAGE ];
	}
	m_stats[ StatID::STAT_ELECTRICITY_USAGE ] = m_projectileDefinition->GetBaseStat( StatID::STAT_ELECTRICITY_USAGE );
	if ( m_stats[ StatID::STAT_ELECTRICITY_USAGE ] > 0 )
	{
		m_stats[ StatID::STAT_ELECTRICITY_USAGE ] += statMultipliers[ StatID::STAT_ELECTRICITY_USAGE ];
	}
}

void Projectile::Update( float deltaSeconds )
{
	UpdateLifetime( deltaSeconds );
	Entity::Update( deltaSeconds );
}

void Projectile::UpdateLifetime( float deltaSeconds )
{
	if ( m_sourceWeaponType == WeaponType::WEAPON_TYPE_MELEE )		// TODO: Add other weapon types when they exist
	{
		m_remainingLifeTime -= deltaSeconds;
	}
}

bool Projectile::ShouldActorBeHit( const Actor* collidingActor ) const
{
	switch ( m_projectileDefinition->GetAttackActorSet() )
	{
		case ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_OTHER_ACTORS	:
		{
			if ( collidingActor != m_attacker )
			{
				return true;
			}
			break;
		}
		case ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_OTHER_FACTIONS	:
		{
			if ( collidingActor->GetActorDefinition()->GetFactionName() != m_attacker->GetActorDefinition()->GetFactionName() )
			{
				return true;
			}
			break;
		}
		case ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_SPECIFIC_FACTION	:
		{
			if ( collidingActor->GetActorDefinition()->GetFactionName() == m_projectileDefinition->GetVulnerableFactionName() )
			{
				return true;
			}
			break;
		}
		default	:	return false;
	}

	return false;
}

Actor* Projectile::GetAttacker() const
{
	return m_attacker;
}

ProjectileDefinition* Projectile::GetProjectileDefinition() const
{
	return m_projectileDefinition;
}

int Projectile::GetStat( StatID statID ) const
{
	return m_stats[ statID ];
}

Vector2 Projectile::GetVelocityWithAgility() const
{
	return ( static_cast< float >( GetStat( StatID::STAT_AGILITY ) ) * m_velocity );
}

void Projectile::Translate( float deltaSeconds )
{
	if ( m_sourceWeaponType == WeaponType::WEAPON_TYPE_PROJECTILE )
	{
		m_position = m_position + ( GetVelocityWithAgility() *  deltaSeconds );
	}
	else if ( m_sourceWeaponType == WeaponType::WEAPON_TYPE_MELEE )
	{
		m_position = m_attacker->GetLocation() + ( ( m_projectileDefinition->GetPhysicsRadius() + m_attacker->GetActorDefinition()->GetPhysicsRadius() ) * m_attacker->GetFacingDirection() );
	}
}

bool Projectile::ShouldBeDestroyed() const
{
	return ( ( m_sourceWeaponType == WeaponType::WEAPON_TYPE_MELEE ) && m_remainingLifeTime <= 0.0f );
}

WeaponType Projectile::GetSourceWeaponType() const
{
	return m_sourceWeaponType;
}
