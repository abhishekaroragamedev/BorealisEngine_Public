#pragma once

#include <string>
#include "Engine/Renderer/Texture.hpp"

constexpr float VICTORY_SCREEN_DISABLE_INPUT_SECONDS = 1.0f;

class VictoryScreen
{

public:
	VictoryScreen();
	~VictoryScreen();

public:
	void Update( float deltaSeconds );
	void Render() const;
	void InitializeVictoryScreen();
	void FlushSoundsPlaying();
	float GetTimeSinceLastInit() const;

private:
	void UpdateAlpha( float deltaSeconds );
	void RenderVictoryScreen() const;

private:
	const std::string VICTORY_SCREEN_TEXTURE_PATH = "Data/Images/VictoryScreen.png";
	const float VICTORY_SCREEN_FADE_IN_TIME_SECONDS = 0.5f;

	Texture* m_victoryScreenTexture;
	float m_timeSinceLastInit;
	float m_victoryScreenAlpha;
	std::vector<size_t> m_playbackIdsOfSoundsPlaying;

};
