#include "Game/Camera.hpp"
#include "Game/GameCommon.hpp"

Camera::Camera()
{
	m_worldPosition = Vector2( 0.0f, 0.0f );
	m_orientationDegrees = 0.0f;
	m_numTilesInViewVertically = CAMERA_NUMBER_OF_TILES_IN_VIEW_VERTICALLY;
	m_pauseScreenTexture = g_renderer->CreateOrGetTexture( PAUSE_SCREEN_OVERLAY_PATH );
	m_deathScreenTexture = g_renderer->CreateOrGetTexture( DEATH_SCREEN_OVERLAY_PATH );
	m_renderPauseScreen = false;
	m_renderDeathScreen = false;
	m_renderFullMap = false;
	m_deathScreenOverlayAlpha = 0.0f;
	m_playerDamageAlphaPulseBlend = 0.0f;
	m_playerDamageAlphaPulseBlendChangeDirection = 1.0f;
	m_screenShakeTheta = 0.0f;
	m_shouldShakeScreenForPlayerDamage = false;
	m_currentScreenShakeDisplacement = Vector2( 0.0f, 0.0f );
}

Camera::~Camera()
{
	
}

void Camera::Update( float deltaSeconds )
{
	HandleKeyboardInput();

	if ( g_theWorld->GetPlayerTank()->IsDeadAndCanRespawn() )
	{
		m_renderDeathScreen = true;
		m_renderPauseScreen = false;
	}
	else if ( g_theWorld->IsGamePaused() )
	{
		m_renderDeathScreen = false;
		m_renderPauseScreen = true;
	}
	else
	{
		m_renderDeathScreen = false;
		m_renderPauseScreen = false;
	}

	if ( m_renderFullMap )
	{
		SetOrthoForFullMapRender();
	}
	else
	{
		SetOrthoFromPlayerTank();
	}

	ShakeScreenIfPlayerJustGotHit( deltaSeconds );
	ShakeScreenIfPlayerJustDied( deltaSeconds );
	UpdateDeathScreenAlpha( deltaSeconds );
	UpdatePlayerHealthPulseBlend();
}

void Camera::ShakeScreenIfPlayerJustGotHit( float deltaSeconds )
{
	static float s_timeSinceScreenShakeStart = 0.0f;

	if ( g_theWorld->GetPlayerTank()->WasDamagedThisFrame() || ( !g_theWorld->GetPlayerTank()->IsDead() &&  m_shouldShakeScreenForPlayerDamage ) )
	{
		m_currentScreenShakeDisplacement.y = 0.0f;
		m_shouldShakeScreenForPlayerDamage = true;
		float remainingTimeToShakeScreen = PLAYER_DAMAGE_SCREEN_SHAKE_DURATION_SECONDS - s_timeSinceScreenShakeStart;		// This also governs the magnitude of the screenshake

		if ( remainingTimeToShakeScreen <= 0.0f )
		{
			s_timeSinceScreenShakeStart = 0.0f;
			m_screenShakeTheta = 0.0f;
			m_shouldShakeScreenForPlayerDamage = false;
			m_currentScreenShakeDisplacement.x = 0.0f;
		}
		else
		{
			AABB2 currentOrtho = GetCurrentOrtho();
			m_currentScreenShakeDisplacement.x = SinDegrees( m_screenShakeTheta ) * ( PLAYER_DAMAGE_SCREEN_SHAKE_MAX_FRACTION_OF_ORTHO_WIDTH * currentOrtho.GetDimensions().x );
			m_currentScreenShakeDisplacement.x = SmoothStop4( remainingTimeToShakeScreen / PLAYER_DAMAGE_SCREEN_SHAKE_DURATION_SECONDS ) * m_currentScreenShakeDisplacement.x;

			g_renderer->SetOrtho( ( currentOrtho.mins + m_currentScreenShakeDisplacement ), ( currentOrtho.maxs + m_currentScreenShakeDisplacement ) );

			m_screenShakeTheta += PLAYER_DAMAGE_SHAKE_THETA_INCREMENT_DEGREES_PER_SECOND * deltaSeconds;

			s_timeSinceScreenShakeStart += deltaSeconds;
		}
	}
	else
	{
		m_currentScreenShakeDisplacement.x = 0.0f;
	}
}

void Camera::ShakeScreenIfPlayerJustDied( float deltaSeconds )
{
	static float s_timeSinceScreenShakeStart = 0.0f;

	if ( g_theWorld->GetPlayerTank()->IsDead() && g_theWorld->GetPlayerTank()->GetTimeSinceDeath() <= DEATH_SCREEN_SHAKE_DURATION_SECONDS )
	{
		m_currentScreenShakeDisplacement.x = 0.0f;
		float remainingTimeToShakeScreen = DEATH_SCREEN_SHAKE_DURATION_SECONDS - s_timeSinceScreenShakeStart;		// This also governs the magnitude of the screenshake

		if ( remainingTimeToShakeScreen <= 0.0f )
		{
			s_timeSinceScreenShakeStart = 0.0f;
			m_screenShakeTheta = 0.0f;
			m_currentScreenShakeDisplacement.y = 0.0f;
		}
		else
		{
			AABB2 currentOrtho = GetCurrentOrtho();
			m_currentScreenShakeDisplacement.y = SinDegrees( m_screenShakeTheta ) * ( DEATH_SCREEN_SHAKE_MAX_FRACTION_OF_ORTHO_HEIGHT * currentOrtho.GetDimensions().y );
			m_currentScreenShakeDisplacement.y = SmoothStop4( remainingTimeToShakeScreen / DEATH_SCREEN_SHAKE_DURATION_SECONDS ) * m_currentScreenShakeDisplacement.y;

			g_renderer->SetOrtho( ( currentOrtho.mins + m_currentScreenShakeDisplacement ), ( currentOrtho.maxs + m_currentScreenShakeDisplacement ) );

			m_screenShakeTheta += ( SmoothStop4( remainingTimeToShakeScreen / DEATH_SCREEN_SHAKE_DURATION_SECONDS ) * DEATH_SCREEN_SHAKE_THETA_INCREMENT_DEGREES_PER_SECOND ) * deltaSeconds;

			s_timeSinceScreenShakeStart += deltaSeconds;
		}
	}
	else
	{
		m_currentScreenShakeDisplacement.y = 0.0f;
	}
}

void Camera::UpdateDeathScreenAlpha( float deltaSeconds )
{
	if ( g_theWorld->GetPlayerTank()->IsDead() && ( m_deathScreenOverlayAlpha < 1.0f ) )
	{
		float alphaIncrement = deltaSeconds / DEATH_SCREEN_OVERLAY_FADE_IN_TIME_SECONDS;
		m_deathScreenOverlayAlpha += alphaIncrement;
	}
	else if ( !g_theWorld->GetPlayerTank()->IsDead() && ( m_deathScreenOverlayAlpha > 0.0f ) )
	{
		float alphaDecrement = deltaSeconds / DEATH_SCREEN_OVERLAY_FADE_OUT_TIME_SECONDS;
		m_deathScreenOverlayAlpha -= alphaDecrement;
	}

	m_deathScreenOverlayAlpha = ClampFloat( m_deathScreenOverlayAlpha, 0.0f, 1.0f );
}

void Camera::UpdatePlayerHealthPulseBlend()
{
	PlayerTank* playerTank = g_theWorld->GetPlayerTank();
	m_playerDamageAlphaPulseBlend += m_playerDamageAlphaPulseBlendChangeDirection * playerTank->GetFractionOfDamageDone() * 0.1f;

	if ( m_playerDamageAlphaPulseBlend >= 1.0f )
	{
		m_playerDamageAlphaPulseBlendChangeDirection = -1.0f;
	}
	else if ( m_playerDamageAlphaPulseBlend <= 0.0f )
	{
		m_playerDamageAlphaPulseBlendChangeDirection = 1.0f;
	}

	m_playerDamageAlphaPulseBlend = ClampFloat( m_playerDamageAlphaPulseBlend, 0.0f, 1.0f );
}

void Camera::SetOrthoFromPlayerTank()
{
	m_worldPosition = g_theWorld->GetPlayerTank()->GetLocation();
	AABB2 orthoForPlayerTank = GetOrthoForPlayerTankRender();
	g_renderer->SetOrtho( orthoForPlayerTank.mins, orthoForPlayerTank.maxs );
}

void Camera::SetOrthoForFullMapRender()
{
	AABB2 mapWorldCoordinates = GetOrthoForFullMapRender();
	m_worldPosition = mapWorldCoordinates.GetCenter();
	g_renderer->SetOrtho( mapWorldCoordinates.mins, mapWorldCoordinates.maxs );
}

void Camera::HandleKeyboardInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_R ) )
	{
		m_renderFullMap = !m_renderFullMap;
	}
}

void Camera::RenderPlayerHealthInformation() const
{
	float fractionOfHealthInDamage = g_theWorld->GetPlayerHealthPulseFraction();
	float alphaAtBorder = BlendFloat( PLAYER_DAMAGE_MIN_ALPHA, PLAYER_DAMAGE_MAX_ALPHA, m_playerDamageAlphaPulseBlend );

	Rgba colorAtOuterBorder = Rgba( RGBA_MAX, 0, 0, static_cast<unsigned char>( alphaAtBorder * static_cast<float>( RGBA_MAX ) ) );
	Rgba colorAtInnerBorder = Rgba( RGBA_MAX, 0, 0, 0 );
	Rgba colorAtCurrentHealthBorder = Interpolate( colorAtOuterBorder, colorAtInnerBorder, SmoothStart4( fractionOfHealthInDamage ) );

	AABB2 outerBoundsAABB = GetScreenShakeCorrectedCurrentOrtho();

	Vector2 borderDimensions = ( PLAYER_HEALTH_MAX_BORDER_FRACTION * outerBoundsAABB.GetDimensions() ) / 2.0f;	// Divide by two to get the border width on one side
	AABB2 innerBoundsAABB = AABB2( Vector2( outerBoundsAABB.mins + ( fractionOfHealthInDamage * borderDimensions ) ), Vector2( outerBoundsAABB.maxs - ( fractionOfHealthInDamage * borderDimensions ) ) );

	g_renderer->DrawAABBBorder( innerBoundsAABB, colorAtInnerBorder, outerBoundsAABB, colorAtOuterBorder );
}

void Camera::RenderPauseOrDeathScreenOverlay() const
{
	if ( !IsFloatEqualTo( m_deathScreenOverlayAlpha, 0.0f ) )		// Draw this overlay even outside the death screen overlay function, because it needs to fade in and fade out
	{
		RenderDeathScreenOverlayWithFadeAlpha();
	}

	if ( m_renderDeathScreen )
	{
		RenderDeathScreenMessage();
	}
	else if ( m_renderPauseScreen )
	{
		RenderPauseScreenOverlay();
	}
}

void Camera::RenderDeathScreenOverlayWithFadeAlpha() const
{
	Rgba blackOverlayColor = Rgba( 0, 0, 0, static_cast<unsigned char>( ( m_deathScreenOverlayAlpha * static_cast<float>( RGBA_MAX ) ) * 0.5f ) );
	AABB2 screenBounds = GetScreenShakeCorrectedCurrentOrtho();
	g_renderer->DrawAABB( screenBounds, blackOverlayColor );
}

void Camera::RenderDeathScreenMessage() const
{
	Rgba neutralTintColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );

	AABB2 screenBounds = GetScreenShakeCorrectedCurrentOrtho();

	Texture* textureToRender = m_deathScreenTexture;
	g_renderer->DrawTexturedAABB( screenBounds, *textureToRender, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), neutralTintColor );
}

void Camera::RenderPauseScreenOverlay() const
{
	Rgba blackOverlayColor = Rgba( 0, 0, 0, ( RGBA_MAX / 2 ) );
	Rgba neutralTintColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );

	AABB2 screenBounds = GetScreenShakeCorrectedCurrentOrtho();

	Texture* textureToRender = m_pauseScreenTexture;
	g_renderer->DrawAABB( screenBounds, blackOverlayColor );
	g_renderer->DrawTexturedAABB( screenBounds, *textureToRender, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), neutralTintColor );
}

AABB2 Camera::GetOrthoForPlayerTankRender() const
{
	Vector2 viewportCornerDifferenceFromCenter = Vector2( ( ( m_numTilesInViewVertically / 2.0f ) * CLIENT_ASPECT ), ( m_numTilesInViewVertically / 2.0f ) );
	Vector2 bottomLeft = m_worldPosition - viewportCornerDifferenceFromCenter;
	Vector2 topRight = m_worldPosition + viewportCornerDifferenceFromCenter;

	AABB2 cameraOrtho = AABB2( bottomLeft, topRight );
	CorrectOrthoForMapBounds( cameraOrtho );

	return cameraOrtho;
}

AABB2 Camera::GetScreenShakeCorrectedCurrentOrtho() const
{
	AABB2 currentOrtho = GetCurrentOrtho();
	currentOrtho.mins += m_currentScreenShakeDisplacement;
	currentOrtho.maxs += m_currentScreenShakeDisplacement;

	return currentOrtho;
}

AABB2 Camera::GetCurrentOrtho() const
{
	if ( m_renderFullMap )
	{
		return GetOrthoForFullMapRender();
	}
	else
	{
		return GetOrthoForPlayerTankRender();
	}
}

AABB2 Camera::GetOrthoForFullMapRender() const
{
	AABB2 mapWorldCoordinates = g_theWorld->GetCurrentMap()->GetBoundsInWorldCoordinates();

	float mapAspectRatio = mapWorldCoordinates.maxs.x / mapWorldCoordinates.maxs.y;		// This works because mins(x, y) = (0.0f, 0.0f), as the map starts from the origin

	if ( mapAspectRatio > CLIENT_ASPECT )
	{
		mapWorldCoordinates.maxs.y = mapWorldCoordinates.maxs.x / CLIENT_ASPECT;		// Avoid stretching the map; fit the aspect to the render window
	}
	else if ( mapAspectRatio < CLIENT_ASPECT )
	{
		mapWorldCoordinates.maxs.x = mapWorldCoordinates.maxs.y * CLIENT_ASPECT;		// Avoid stretching the map; fit the aspect to the render window
	}

	return mapWorldCoordinates;
}

void Camera::CorrectOrthoForMapBounds( AABB2& out_cameraOrtho ) const
{
	AABB2 mapBounds = g_theWorld->GetCurrentMap()->GetBoundsInWorldCoordinates();

	if ( out_cameraOrtho.maxs.x > mapBounds.maxs.x )
	{
		float difference = out_cameraOrtho.maxs.x - mapBounds.maxs.x;
		out_cameraOrtho.maxs.x -= difference;
		out_cameraOrtho.mins.x -= difference;
	}
	else if ( out_cameraOrtho.mins.x < mapBounds.mins.x )
	{
		float difference = mapBounds.mins.x - out_cameraOrtho.mins.x;
		out_cameraOrtho.maxs.x += difference;
		out_cameraOrtho.mins.x += difference;
	}

	if ( out_cameraOrtho.maxs.y > mapBounds.maxs.y )
	{
		float difference = out_cameraOrtho.maxs.y - mapBounds.maxs.y;
		out_cameraOrtho.maxs.y -= difference;
		out_cameraOrtho.mins.y -= difference;
	}
	else if ( out_cameraOrtho.mins.y < mapBounds.mins.y )
	{
		float difference = mapBounds.mins.y - out_cameraOrtho.mins.y;
		out_cameraOrtho.maxs.y += difference;
		out_cameraOrtho.mins.y += difference;
	}
}
