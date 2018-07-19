#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/TheGame.hpp"

constexpr char INCURSION_2D_APP_NAME[] = "Incursion Gold - Abhishek Arora";

class TheApp
{

public:
	TheApp();
	~TheApp();

public:
	void RunFrame();
	void Update();
	void Render();
	bool IsQuitting() const;
	
private:
	void LoadSoundsForApp();		// Pre-load audio files
	float ComputeDeltaSeconds() const;

private:
	bool m_audioLoaded;
};