#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Tools/Command.hpp"
#include "Game/TheGame.hpp"

constexpr int NUM_TIME_SCALE_HOTKEY_VALUES = 5;
constexpr double TIME_SCALE_HOTKEY_VALUES[] = { 1.0f, 2.0f, 4.0f, 0.25f, 0.5f };
constexpr char APP_NAME[] = "Volumetrics - Abhishek Arora";
constexpr char GAME_CONFIG_XML_PATH[] = "Data/GameConfig.xml";

class TheApp
{

public:
	TheApp();
	~TheApp();

public:
	void RunFrame();
	void Update();
	void Render();
	void ForceQuit();
	bool IsQuitting() const;

private:
	void LoadGameConfigXML();
	void HandleKeyboardInput();

private:
	bool m_isQuitting = false;
	int m_nextTimeScaleHotkeyIndex = 1;
};