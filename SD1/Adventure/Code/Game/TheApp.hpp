#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/TheGame.hpp"

constexpr char ADVENTURE_APP_NAME[] = "Adventure - Abhishek Arora";

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
	void PopulateBlackboardFromGameConfigXml();
	float ComputeDeltaSeconds() const;

private:
	bool m_isQuitting = false;
};