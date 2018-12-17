#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TheGame.hpp"

constexpr float CLIENT_ASPECT = 1.0f;
constexpr char ASTEROIDS_APP_NAME[] = "Asteroids Gold - Abhishek Arora";

class TheApp
{

public:
	TheApp();
	~TheApp();

public:
	void RunFrame();
	void Update();
	void Render();
	void QuitIfRequired();
	bool IsQuitting() const;
	
private:
	float ComputeDeltaSeconds() const;

private:
	TheGame* m_theGame;
	bool m_isQuitting;
};