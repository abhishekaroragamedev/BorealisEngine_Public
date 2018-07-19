#include "Game/GameCommon.hpp"
#include "Game/Entities/PlayerTank.hpp"

PlayerTank::PlayerTank( const Vector2& spawnLocation ) : Tank( spawnLocation )
{
	m_speed = PLAYER_TANK_SPEED;
	m_health = PLAYER_TANK_START_HEALTH;
	m_bulletCooldownTime = PLAYER_TANK_BULLET_FIRE_COOLDOWN_SECONDS;
	m_timeSinceDeath = 0.0f;
	m_timeSinceLastHit = 0.0f;
	m_timeSinceLastRegen = 1.0f;	// So that the first health regen can happen immediately
	m_godModeEnabled = false;
	m_wasDamagedThisFrame = false;

	m_tankTexture = g_renderer->CreateOrGetTexture( PLAYER_TANK_TEXTURE_PATH );
	m_tankTurretTexture = g_renderer->CreateOrGetTexture( PLAYER_TANK_TURRET_TEXTURE_PATH );
}

PlayerTank::~PlayerTank()
{
	
}

void PlayerTank::Update( float deltaSeconds )
{
	Tank::Update( deltaSeconds );
	HandleKeyboardInput();
	HandleXboxControllerInput( deltaSeconds );
	if ( m_isDead )
	{
		UpdateTimeSinceDeath( deltaSeconds );
	}
	else
	{
		UpdateTimeSinceLastHitAndRegenHealth( deltaSeconds );
	}

	StopControllerVibrationIfTimeHasPassed();

	m_wasDamagedThisFrame = false;
}

void PlayerTank::Render( bool developerModeEnabled ) const
{
	if ( !m_isDead )
	{
		Tank::Render( developerModeEnabled );
	}
}

void PlayerTank::DoDamage()
{
	Entity::DoDamage();
	m_timeSinceLastHit = 0.0f;
	m_wasDamagedThisFrame = true;

	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( m_health > 0 )
		{
			g_inputSystem->GetController( 0 ).VibrateController( PLAYER_TANK_HIT_VIBRATION_LEFT_MOTOR_VALUE, 0 );
		}
		else
		{
			g_inputSystem->GetController( 0 ).VibrateController( PLAYER_TANK_DEATH_VIBRATION_MOTOR_VALUE, PLAYER_TANK_DEATH_VIBRATION_MOTOR_VALUE );
		}
	}
}

void PlayerTank::StopControllerVibrationIfTimeHasPassed()
{
	if ( !IsDead() && ( m_timeSinceLastHit > PLAYER_TANK_HIT_VIBRATION_TIME_SECONDS ) && g_inputSystem->GetController( 0 ).IsConnected() )
	{
		g_inputSystem->GetController( 0 ).StopControllerVibrationLeftMotor();
	}
	else if ( IsDead() && ( m_timeSinceDeath > PLAYER_TANK_DEATH_VIBRATION_TIME_SECONDS ) && g_inputSystem->GetController( 0 ).IsConnected() )
	{
		g_inputSystem->GetController( 0 ).StopControllerVibration();
	}
}

void PlayerTank::SetLocation( Vector2 newLocation )
{
	m_location = newLocation;
	m_discCollider.center = m_location;
}

float PlayerTank::GetFractionOfDamageDone() const
{
	return ( ( GetMaxHealthAsFloat() - static_cast<float>( GetHealth() ) ) / GetMaxHealthAsFloat() );
}

float PlayerTank::GetFractionOfHealthRemaining() const
{
	return ( static_cast<float>( GetHealth() ) / GetMaxHealthAsFloat() );
}

float PlayerTank::GetMaxHealthAsFloat() const
{
	return static_cast<float>( PLAYER_TANK_START_HEALTH );
}

bool PlayerTank::WasDamagedThisFrame() const
{
	return m_wasDamagedThisFrame;
}

bool PlayerTank::IsDeadAndCanRespawn() const
{
	return ( m_isDead && ( m_timeSinceDeath >= PLAYER_TANK_RESPAWN_TIME_SECONDS ) );
}

bool PlayerTank::IsGodModeEnabled() const
{
	return m_godModeEnabled;
}

float PlayerTank::GetTimeSinceDeath() const
{
	return m_timeSinceDeath;
}

void PlayerTank::HandleKeyboardInput()
{
	if ( IsDeadAndCanRespawn() && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) )
	{
		Respawn();
	}
	else if ( !IsDeadAndCanRespawn() && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_G ) )
	{
		m_godModeEnabled = !m_godModeEnabled;
	}
}

void PlayerTank::HandleXboxControllerInput( float deltaSeconds )
{
	if( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( !m_isDead )
		{
			float leftJoystickAngleDegrees = g_inputSystem->GetController( 0 ).GetJoystick( 0 ).GetAngleDegrees();		// Left joystick input
			float leftJoystickMagnitude = g_inputSystem->GetController( 0 ).GetJoystick( 0 ).GetMagnitude();
			if ( leftJoystickMagnitude > 0.0f )
			{
				m_velocity = leftJoystickMagnitude * Vector2::MakeDirectionAtDegrees( m_orientation );		// The tank goes the way it's currently oriented irrespective of the analog input direction
				m_velocity = m_velocity.GetNormalized();

				Translate( deltaSeconds );

				m_orientation = TurnToward( m_orientation, leftJoystickAngleDegrees, ( PLAYER_TANK_TURN_SPEED_DEGREES_PER_SECOND * deltaSeconds ) );		// Have the tank turn "slowly" instead of directly based on the analog input direction
			}
			else
			{
				m_velocity = Vector2( 0.0f, 0.0f );
			}

			float rightJoystickAngleDegrees = g_inputSystem->GetController( 0 ).GetJoystick( 1 ).GetAngleDegrees();		// Right joystick input
			float rightJoystickMagnitude = g_inputSystem->GetController( 0 ).GetJoystick( 1 ).GetMagnitude();
			if ( rightJoystickMagnitude > 0.0f )
			{
				m_tankTurretOrientation = TurnToward( m_tankTurretOrientation, rightJoystickAngleDegrees, ( TANK_TURRET_TURN_SPEED_DEGREES_PER_SECOND * deltaSeconds ) );		// Have the tank's turret turn "slowly" instead of directly based on the analog input direction
			}

			if ( CanFireBullets() )
			{
				float rightTriggerPressedValue = g_inputSystem->GetController( 0 ).GetRightTriggerValue();		// Trigger input - to fire bullets
				if ( rightTriggerPressedValue >= PLAYER_TANK_BULLET_XBOX_TRIGGER_THRESHOLD )
				{
					g_theWorld->GetCurrentMap()->SpawnBullet( EntityType::ENTITY_TYPE_PLAYER_TANK, m_location, m_tankTurretOrientation );
					m_tankTurretOffset = TANK_TURRET_MAX_OFFSET;
					g_inputSystem->GetController( 0 ).VibrateController( 0, PLAYER_TANK_FIRE_VIBRATION_RIGHT_MOTOR_VALUE );
					m_timeSinceBulletFired += 0.01f;
				}
				else
				{
					g_inputSystem->GetController( 0 ).StopControllerVibrationRightMotor();
				}
			}
		}
		else if ( IsDeadAndCanRespawn() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START ) )		// Respawn
		{
			Respawn();
		}
	}
}

void PlayerTank::UpdateTimeSinceLastHitAndRegenHealth( float deltaSeconds )
{
	m_timeSinceLastHit += deltaSeconds;

	if ( ( m_timeSinceLastHit > PLAYER_TANK_REGEN_START_TIME_SECONDS ) && ( m_health < PLAYER_TANK_START_HEALTH ) )
	{
		m_timeSinceLastRegen += deltaSeconds;

		if ( m_timeSinceLastRegen >= 1.0f )
		{
			m_health += PLAYER_HEALTH_REGEN_PER_SECOND;
			m_timeSinceLastRegen = 0.0f;
		}
	}
}

void PlayerTank::UpdateTimeSinceDeath( float deltaSeconds )
{
	m_timeSinceDeath += deltaSeconds;
}

void PlayerTank::Respawn()
{
	m_health = PLAYER_TANK_START_HEALTH;
	m_timeSinceDeath = 0.0f;
	m_isDead = false;
}
