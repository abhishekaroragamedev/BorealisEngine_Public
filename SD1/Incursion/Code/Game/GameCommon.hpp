#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/TheGame.hpp"
#include "Game/IncursionAudioManager.hpp"
#include "Game/World/TheWorld.hpp"

constexpr float CLIENT_ASPECT = ( 16.0f / 9.0f );
constexpr float SCREEN_HEIGHT = 1000.0f;

extern AudioSystem* g_audioSystem;
extern InputSystem* g_inputSystem;
extern Renderer* g_renderer;		// Defined in EngineCommon
extern TheGame* g_theGame;
extern TheWorld* g_theWorld;