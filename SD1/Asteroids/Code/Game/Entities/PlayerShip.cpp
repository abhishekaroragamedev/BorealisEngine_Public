#include "Game/Entities/PlayerShip.hpp"

PlayerShip::PlayerShip( PlayerIndex playerIndex )
{
	PopulateShipVertices();
	PopulateExhaustVertices();
	m_location = SHIP_SPAWN_POSITION;
	m_velocity = Vector2( 0.0f, 0.0f );
	m_orientation = SHIP_START_ORIENTATION_DEGREES;
	m_playerIndex = playerIndex;
	m_isDead = false;
	m_speed = SHIP_SPEED;
	m_deathRenderAlpha = 0.0f;

	m_cosmeticDisc2 = Disc2( m_location, SHIP_RADIUS ) ;
	m_physicalDisc2 = Disc2( m_location, SHIP_RADIUS * SHIP_PHYSICAL_TO_COSMETIC_RADIUS_RATIO );

	m_acceleration = SHIP_ACCELERATION;		// Assumes the ship points towards the right when not rotated

	PopulateDeveloperModeCirclesVertices();

	InitializeForSpawn();
}

PlayerShip::~PlayerShip()
{
	
}

void PlayerShip::Update( float deltaSeconds )
{
	if ( m_playerIndex == PlayerIndex::KEYBOARD )
	{
		HandleKeyboardInput( deltaSeconds );
	}
	else
	{
		HandleXboxControllerInput( deltaSeconds );
	}

	if ( m_isDead )
	{
		UpdateDeathStateRenderInformation( deltaSeconds );
	}

	Entity::Update( deltaSeconds );
}

void PlayerShip::Render( bool developerModeEnabled ) const
{
	if ( !m_isDead )
	{
		DrawShip();
		DrawExhaust();
	}
	else
	{
		DrawDeadShipFragments();
		ShakeScreen();
	}

	Entity::Render( developerModeEnabled );
}

Vector2 PlayerShip::GetBulletSpawnLocation() const
{
	return  ( m_location + RotateVector2( SHIP_TIP, m_orientation ) );		// The tip of the ship, rotated appropriately
}

Vector2 PlayerShip::GetBulletSpawnVelocity() const
{
	return Vector2::MakeDirectionAtDegrees( GetOrientationDegrees() );
}

bool PlayerShip::IsDead() const
{
	return m_isDead;
}

void PlayerShip::AccelerateShip( float deltaSeconds )
{
	m_drawExhaust = true;
	Vector2 rotatedAccelerationVector = RotateVector2( m_acceleration, m_orientation );
	m_velocity += rotatedAccelerationVector * deltaSeconds;
}

void PlayerShip::HandleKeyboardInput( float deltaSeconds )
{
	bool thrustPressed =  g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_E ) || g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_UP_ARROW );
	bool turnAntiClockwisePressed = g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_S ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_LEFT_ARROW );
	bool turnClockwisePressed = g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RIGHT_ARROW );
	bool turnAntiClockwiseReleased = g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_S ) || g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_LEFT_ARROW );
	bool turnClockwiseReleased = g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_F ) || g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_RIGHT_ARROW );

	if ( !m_isDead && thrustPressed )
	{
		AccelerateShip( deltaSeconds );
	}
	else
	{
		m_drawExhaust = false;
	}

	if ( !m_isDead && turnAntiClockwisePressed && !turnClockwisePressed )
	{
		m_rotationSpeed = SHIP_ROTATION_SPEED;
	}
	else if ( !m_isDead && turnClockwisePressed && !turnAntiClockwisePressed )
	{
		m_rotationSpeed = -SHIP_ROTATION_SPEED;
	}
	else if ( !m_isDead && ( turnAntiClockwiseReleased || turnClockwiseReleased || ( turnAntiClockwisePressed && turnClockwisePressed ) ) )
	{
		m_rotationSpeed = 0.0f;
	}

	if ( m_isDead && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_N ) )
	{
		Respawn();
	}
}

void PlayerShip::HandleXboxControllerInput( float deltaSeconds )
{
	float shipMovementMagnitude = g_inputSystem->GetController( m_playerIndex ).GetJoystick( XBOXCONTROLLER_JOYSTICK_LEFT_INDEX ).GetMagnitude();
	float shipMovementAngleDegrees = g_inputSystem->GetController( m_playerIndex ).GetJoystick( XBOXCONTROLLER_JOYSTICK_LEFT_INDEX ).GetAngleDegrees();

	if ( !m_isDead && shipMovementMagnitude > 0.0f )
	{
		m_orientation = shipMovementAngleDegrees;
		AccelerateShip( deltaSeconds );
	}
	else
	{
		m_drawExhaust = false;
	}

	if ( m_isDead && g_inputSystem->GetController( m_playerIndex ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START ) )
	{
		Respawn();
	}
}

void PlayerShip::InitializeForSpawn()		// Relocate and reorient the player and its discs
{
	m_cosmeticDisc2.center = m_location;
	m_physicalDisc2.center = m_location;
	m_drawExhaust = false;
	m_rotationSpeed = 0.0f;
}

void PlayerShip::PopulateShipVertices()		// These vertices are all relative to the ship's center, assuming the ship points toward the right
{
	const float SHIP_REAR_TIP_X_LOCATION = 2.0f;

	m_shipVertices[0] = SHIP_TIP;
	m_shipVertices[1] = Vector2( -SHIP_RADIUS / SHIP_REAR_TIP_X_LOCATION, SHIP_RADIUS / SHIP_REAR_TIP_X_LOCATION );
	m_shipVertices[2] = SHIP_LEFT_EXHAUST_PORT_LOCATION;
	m_shipVertices[3] = SHIP_RIGHT_EXHAUST_PORT_LOCATION;
	m_shipVertices[4] = Vector2( -SHIP_RADIUS / SHIP_REAR_TIP_X_LOCATION, -SHIP_RADIUS / SHIP_REAR_TIP_X_LOCATION );
}

void PlayerShip::PopulateExhaustVertices()		// These vertices are all relative to the ship's center, assuming the ship points toward the right
{
	const float SHIP_EXHAUST_VERTEX_X_SMALL = 1.8f;
	const float SHIP_EXHAUST_VERTEX_X_MID = 1.65f;
	const float SHIP_EXHAUST_VERTEX_X_LARGE = 1.5f;

	m_exhaustVertices[0] = Vector2( -SHIP_RADIUS / SHIP_EXHAUST_VERTEX_X_SMALL, 0.0f );
	m_exhaustVertices[1] = Vector2( -SHIP_RADIUS / SHIP_EXHAUST_VERTEX_X_MID, 0.0f );
	m_exhaustVertices[2] = Vector2( -SHIP_RADIUS / SHIP_EXHAUST_VERTEX_X_LARGE, 0.0f );
}

Rgba PlayerShip::GetShipColorFromPlayerIndex() const
{
	Rgba shipColor;

	switch ( m_playerIndex )
	{
		case PlayerIndex::CONTROLLER_1:
		{
			shipColor.SetAsBytes( 0, RGBA_MAX, 0, RGBA_MAX );
			break;
		}
		
		case PlayerIndex::CONTROLLER_2:
		{
			shipColor.SetAsBytes( 0, 0, RGBA_MAX, RGBA_MAX );
			break;
		}

		case PlayerIndex::CONTROLLER_3:
		{
			shipColor.SetAsBytes( ( RGBA_MAX /5 ), RGBA_MAX, RGBA_MAX, RGBA_MAX );
			break;
		}

		case PlayerIndex::CONTROLLER_4:
		{
			shipColor.SetAsBytes( RGBA_MAX, RGBA_MAX, ( RGBA_MAX / 5 ), RGBA_MAX );
			break;
		}

		case PlayerIndex::KEYBOARD:
		{
			shipColor.SetAsBytes( RGBA_MAX, ( RGBA_MAX / 5 ), RGBA_MAX, RGBA_MAX );
			break;
		}

		default:
		{
			shipColor.SetAsBytes( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );
			break;
		}
	}

	return shipColor;
}

void PlayerShip::DrawShip() const
{
	g_renderer->ChangeRenderColor( GetShipColorFromPlayerIndex() );
	g_renderer->DrawPolygon( m_location, m_orientation, m_shipVertices, SHIP_NUM_VERTICES, RPolygonType::REGULAR );
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}

void PlayerShip::DrawExhaust() const
{
	static int s_currentExhaustVertexIndex = 0;

	const float SHIP_EXHAUST_RENDER_LINE_THICKNESS = 1.5f;

	if ( m_drawExhaust )
	{
		Vector2 leftExhaustPortInWorldSpace = m_location + RotateVector2( SHIP_LEFT_EXHAUST_PORT_LOCATION, m_orientation );
		Vector2 rightExhaustPortInWorldSpace = m_location + RotateVector2( SHIP_RIGHT_EXHAUST_PORT_LOCATION, m_orientation );
		Vector2 currentExhaustVertexInWorldSpace = m_location + RotateVector2( m_exhaustVertices[ s_currentExhaustVertexIndex ], m_orientation );

		g_renderer->DrawLine( leftExhaustPortInWorldSpace, currentExhaustVertexInWorldSpace, Rgba( Rgba( RGBA_MAX, 0, 0, 0 ) ), Rgba( RGBA_MAX, RGBA_MAX, ( RGBA_MAX / 2 ), RGBA_MAX ), SHIP_EXHAUST_RENDER_LINE_THICKNESS );
		g_renderer->DrawLine( rightExhaustPortInWorldSpace, currentExhaustVertexInWorldSpace, Rgba( Rgba( RGBA_MAX, 0, 0, 0 ) ), Rgba( RGBA_MAX, RGBA_MAX, ( RGBA_MAX / 2 ), RGBA_MAX ), SHIP_EXHAUST_RENDER_LINE_THICKNESS );
		g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );

		s_currentExhaustVertexIndex = ( s_currentExhaustVertexIndex + 1 ) % SHIP_EXHAUST_NUM_VERTICES;
	}
}

void PlayerShip::DrawDeadShipFragments() const
{
	const float SHIP_DEATH_RENDER_LINE_THICKNESS = 1.2f;

	Rgba shipColor = GetShipColorFromPlayerIndex();
	shipColor.a = static_cast<unsigned char>( m_deathRenderAlpha );
	
	for ( int i = 0; i <= SHIP_NUM_VERTICES; i++ )
	{
		Vector2 lineStart = RotateVector2( m_shipVertices[ i ], m_orientation );
		Vector2 lineEnd = RotateVector2( m_shipVertices[ ( i + 1 ) % SHIP_NUM_VERTICES ], m_orientation );
		Vector2 lineMidPoint = GetMidPoint( lineStart, lineEnd );		// Since the "center" of the ship is ( 0, 0 ), this is also effectively the displacement vector of the mid-point from the center of the ship

		lineStart += m_deathFragmentDistanceMultiplier * lineMidPoint;		// Displace these vectors using the mid-point vector, so that they move outward from the center of the ship
		lineEnd += m_deathFragmentDistanceMultiplier * lineMidPoint;
		lineStart += m_location;		// Convert these vectors to world space
		lineEnd += m_location;

		g_renderer->DrawLine( lineStart, lineEnd, shipColor, shipColor, SHIP_DEATH_RENDER_LINE_THICKNESS );
	}

	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}

void PlayerShip::MarkForDeathAndResetPositionAndOrientation()
{
	m_isDead = true;
	m_deathRenderAlpha = static_cast<float>( RGBA_MAX );
	m_deathFragmentDistanceMultiplier = 0.0f;
	m_timeSinceDeath = 0.0f;
	m_deathScreenShakeTheta = SHIP_DEATH_SCREEN_SHAKE_START_THETA_DEGREES;

	if ( this->m_playerIndex != PlayerIndex::KEYBOARD )
	{
		g_inputSystem->GetController( m_playerIndex ).VibrateController( SHIP_DEATH_CONTROLLER_VIBRATION_MOTOR_VALUE, SHIP_DEATH_CONTROLLER_VIBRATION_MOTOR_VALUE );
	}

	InitializeForSpawn();
}

void PlayerShip::UpdateDeathStateRenderInformation( float deltaSeconds )
{
	if ( m_isDead )
	{
		UpdateDeathRenderAlpha( deltaSeconds );
		UpdateDeathFragmentDistanceMultiplier( deltaSeconds );
		UpdateDeathScreenShakeTheta( deltaSeconds );
		UpdateTimeSinceDeathForControllerVibration( deltaSeconds );
	}
}

void PlayerShip::UpdateDeathRenderAlpha( float deltaSeconds )
{
	if ( m_deathRenderAlpha > 0.0f )		// This is used for the fade out on ship death
	{
		m_deathRenderAlpha -= SHIP_DEATH_RENDER_ALPHA_DECREMENT_PER_SECOND * deltaSeconds;
	}
	if ( m_deathRenderAlpha < 0.0f )
	{
		m_deathRenderAlpha = 0.0f;
	}
}

void PlayerShip::UpdateDeathFragmentDistanceMultiplier( float deltaSeconds )
{
	m_deathFragmentDistanceMultiplier += SHIP_DEATH_FRAGMENT_DISTANCE_MULTIPLIER_INCREMENT_PER_SECOND * deltaSeconds;		// This is used to make the ship's edges break apart when the ship dies
}

void PlayerShip::UpdateDeathScreenShakeTheta( float deltaSeconds )
{
	if ( IsFloatEqualTo( m_deathScreenShakeTheta, 0.0f ) )	// This is used to compute the screen offset when the ship dies
	{
		m_deathScreenShakeTheta = 0.0f;
		g_renderer->SetOrtho( Vector2( 0.0f, 0.0f ), Vector2( SCREEN_WIDTH, SCREEN_HEIGHT ) );
	}
	else if ( m_deathScreenShakeTheta > 0.0f )		
	{
		m_deathScreenShakeTheta -= SHIP_DEATH_SCREEN_SHAKE_THETA_DECREMENT_PER_SECOND * deltaSeconds;
		m_deathScreenShakeTheta *= -1;
	}
	else if ( m_deathScreenShakeTheta < 0.0f )
	{
		m_deathScreenShakeTheta += SHIP_DEATH_SCREEN_SHAKE_THETA_DECREMENT_PER_SECOND * deltaSeconds;
		m_deathScreenShakeTheta *= -1;
	}
}

void PlayerShip::UpdateTimeSinceDeathForControllerVibration( float deltaSeconds )
{
	m_timeSinceDeath += deltaSeconds;

	if ( m_playerIndex != PlayerIndex::KEYBOARD &&  m_timeSinceDeath >= SHIP_DEATH_CONTROLLER_VIBRATION_SECONDS )
	{
		g_inputSystem->GetController( m_playerIndex ).StopControllerVibration();
	}
}

void PlayerShip::ShakeScreen() const
{
	if ( m_deathScreenShakeTheta > 0.0f )
	{
		float orthoDisplacementFraction = SinDegrees( m_deathScreenShakeTheta );

		Vector2 bottomLeft = Vector2( 0.0f, 0.0f );
		Vector2 topRight = Vector2( SCREEN_HEIGHT, SCREEN_WIDTH );
		bottomLeft.x += SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE * orthoDisplacementFraction;
		bottomLeft.y += SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE * orthoDisplacementFraction;
		topRight.x += SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE * orthoDisplacementFraction;
		topRight.y += SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE * orthoDisplacementFraction;

		g_renderer->SetOrtho( bottomLeft, topRight );
	}
}

void PlayerShip::Respawn()
{
	m_isDead = false;
	m_location = SHIP_SPAWN_POSITION;
	m_velocity = Vector2( 0.0f, 0.0f );
	m_cosmeticDisc2.center = m_location;
	m_physicalDisc2.center = m_location;
	m_orientation = SHIP_START_ORIENTATION_DEGREES;

	if ( m_playerIndex != PlayerIndex::KEYBOARD )
	{
		g_inputSystem->GetController( m_playerIndex ).StopControllerVibration();
	}
}
