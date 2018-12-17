#pragma once

#include "Game/Entities/ProjectileDefinition.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Entities/Stats.hpp"
#include "Engine/Math/Vector2.hpp"
#include <string>

enum WeaponType;
class Actor;

class Projectile : public Entity
{

	friend class Map;

public:
	~Projectile();

	bool ShouldActorBeHit( const Actor* collidingActor ) const;
	Actor* GetAttacker() const;
	ProjectileDefinition* GetProjectileDefinition() const;
	int GetStat( StatID statID ) const;
	Vector2 GetVelocityWithAgility() const;
	bool ShouldBeDestroyed() const;
	WeaponType GetSourceWeaponType() const;

	void Update( float deltaSeconds ) override;
	//void Render() const override;

protected:
	explicit Projectile( const std::string& instanceName, Actor* attacker, WeaponType sourceWeaponType, const Vector2& position, const Vector2& velocity, ProjectileDefinition* projectileDefinition, const Map& map, int statMultipliers[ StatID::NUM_STATS ] );

	void InitializeStatsFromBaseAndMultipliers( int statMultipliers[ StatID::NUM_STATS ] );
	void Translate( float deltaSeconds ) override;
	void UpdateLifetime( float deltaSeconds );

private:
	ProjectileDefinition* m_projectileDefinition = nullptr;
	Actor* m_attacker = nullptr;
	WeaponType m_sourceWeaponType = WeaponType( -1 );
	int m_stats[ StatID::NUM_STATS ];
	float m_remainingLifeTime = 0.0f;

};
