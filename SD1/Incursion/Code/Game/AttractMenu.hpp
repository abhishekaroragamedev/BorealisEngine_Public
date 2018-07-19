#pragma once

#include <vector>
#include "Engine/Renderer/Texture.hpp"

class AttractMenu
{

public:
	AttractMenu();
	~AttractMenu();

public:
	void Update( float deltaSeconds );
	void Render() const;
	void InitializeAttractMode();
	void FlushSoundsPlaying();
	int GetFramesSinceAttractModeFirstLoaded() const;

private:
	void UpdateGameStartTextAlpha( float deltaSeconds );
	void HandleKeyboardInput() const;
	void HandleXboxControllerInput() const;
	void RenderBackground() const;		// Draws a horizontal Cyan-Red gradient to serve as the title background
	void RenderInstructions() const;
	void RenderAuthorName() const;
	void RenderTitle() const;
	void RenderLoadingText() const;
	void RenderStartPrompt() const;

private:
	const std::string TITLE_TEXT_PATH = "Data/Images/TitleText.png";
	const std::string LOADING_TEXT_PATH = "Data/Images/LoadingText.png";
	const std::string INSTRUCTION_TEXT_PATH = "Data/Images/InstructionText.png";
	const std::string START_PROMPT_TEXT_PATH = "Data/Images/GameStartPrompt.png";
	const std::string AUTHOR_NAME_TEXT_PATH = "Data/Images/AuthorName.png";
	const float START_TEXT_FLASH_TIME_SECONDS = 1.0f;

	Texture* m_titleTexture;
	Texture* m_loadingTexture;
	Texture* m_instructionTexture;
	Texture* m_gameStartTexture;
	Texture* m_authorNameTexture;
	int m_framesSinceAttractMenuCreated;
	float m_gameStartTextAlpha;
	float m_gameStartTextAlphaChangeDirection;
	std::vector<size_t> m_playbackIdsOfSoundsPlaying;

};
