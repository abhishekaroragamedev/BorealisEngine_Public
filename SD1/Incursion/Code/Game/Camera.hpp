#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/Entities/PlayerTank.hpp"

class Camera
{

public:
	Camera();
	~Camera();

public:
	void Update( float deltaSeconds );
	void RenderPauseOrDeathScreenOverlay() const;
	void RenderPauseScreenOverlay() const;
	void RenderDeathScreenOverlayWithFadeAlpha() const;
	void RenderDeathScreenMessage() const;
	void RenderPlayerHealthInformation() const;	// Render an inward growing red gradient

private:
	void HandleKeyboardInput();
	void UpdateDeathScreenAlpha( float deltaSeconds );
	void UpdatePlayerHealthPulseBlend();
	void ShakeScreenIfPlayerJustGotHit( float deltaSeconds );
	void ShakeScreenIfPlayerJustDied( float deltaSeconds );
	void SetOrthoFromPlayerTank();
	void SetOrthoForFullMapRender();
	void CorrectOrthoForMapBounds( AABB2& out_cameraOrtho ) const;
	AABB2 GetScreenShakeCorrectedCurrentOrtho() const;
	AABB2 GetCurrentOrtho() const;
	AABB2 GetOrthoForPlayerTankRender() const;
	AABB2 GetOrthoForFullMapRender() const;

private:
	const float CAMERA_NUMBER_OF_TILES_IN_VIEW_VERTICALLY = 7.0f;
	const float DEATH_SCREEN_OVERLAY_FADE_IN_TIME_SECONDS= 2.0f;
	const float DEATH_SCREEN_OVERLAY_FADE_OUT_TIME_SECONDS= 0.5f;
	const float DEATH_SCREEN_SHAKE_DURATION_SECONDS = 1.0f;
	const float DEATH_SCREEN_SHAKE_MAX_FRACTION_OF_ORTHO_HEIGHT = 0.1f;
	const float DEATH_SCREEN_SHAKE_THETA_INCREMENT_DEGREES_PER_SECOND = 1800.0f;
	const float PLAYER_DAMAGE_SCREEN_SHAKE_DURATION_SECONDS = 0.5f;
	const float PLAYER_DAMAGE_SHAKE_THETA_INCREMENT_DEGREES_PER_SECOND = 360.0f;
	const float PLAYER_DAMAGE_SCREEN_SHAKE_MAX_FRACTION_OF_ORTHO_WIDTH = 0.005f;
	const float PLAYER_DAMAGE_MIN_ALPHA = 0.5f;
	const float PLAYER_DAMAGE_MAX_ALPHA = 1.0f;
	const std::string PAUSE_SCREEN_OVERLAY_PATH = "Data/Images/PauseScreen.png";
	const std::string DEATH_SCREEN_OVERLAY_PATH = "Data/Images/DeathScreen.png";

	Vector2 m_worldPosition;
	float m_orientationDegrees;
	float m_numTilesInViewVertically;
	Texture* m_pauseScreenTexture;
	Texture* m_deathScreenTexture;
	bool m_renderDeathScreen;
	bool m_renderPauseScreen;
	bool m_renderFullMap;
	float m_deathScreenOverlayAlpha;
	float m_playerDamageAlphaPulseBlend;
	float m_playerDamageAlphaPulseBlendChangeDirection;
	float m_screenShakeTheta;
	float m_shouldShakeScreenForPlayerDamage;
	Vector2 m_currentScreenShakeDisplacement;

};
