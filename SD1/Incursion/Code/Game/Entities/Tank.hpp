#pragma once

#include "Game/Entities/Entity.hpp"
#include "Game/World/Tile.hpp"

class Tank : public Entity
{
protected:
	Tank( const Vector2& spawnLocation );
	~Tank();

public:
	virtual void Update( float deltaSeconds ) override;
	void TranslateFromCollision( const Vector2& locationCorrection );
	virtual void Render( bool developerModeEnabled ) const override;
	float GetTurretOrientationDegrees() const;
	Disc2 GetDiscCollider() const;

protected:
	void Translate( float deltaSeconds ) override;
	void RestoreTurretToCenterPosition( float deltaSeconds );
	bool CanFireBullets() const;
	void UpdateBulletFireCooldown( float deltaSeconds );

protected:
	const float TANK_COSMETIC_DISC_RADIUS_WORLD_UNITS = 0.5f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TANK_PHYSICAL_DISC_RADIUS_WORLD_UNITS = 0.4f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TANK_TURRET_MAX_OFFSET = 0.15f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TANK_TURRET_RECOIL_MAX_RECOVERY_PER_SECOND = 0.15f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TANK_TURRET_TURN_SPEED_DEGREES_PER_SECOND = 360.0f;

	Disc2 m_discCollider;
	float m_tankTurretOrientation;
	float m_tankTurretOffset;
	float m_timeSinceBulletFired;
	float m_bulletCooldownTime;	// This has to be set by the child entity
	Texture* m_tankTexture;		// This has to be set by the child entity
	Texture* m_tankTurretTexture;	// This has to be set by the child entity

};
