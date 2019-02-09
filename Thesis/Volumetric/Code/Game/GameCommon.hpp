#pragma once

#include "Game/TheApp.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

constexpr float SCREEN_HEIGHT = 1000.f;
constexpr float CLIENT_ASPECT = 16.f/9.f;
constexpr float RENDER_TARGET_HEIGHT = 512.f;

extern TheApp* g_theApp;
extern AudioSystem* g_audioSystem;
extern Renderer* g_renderer;
extern InputSystem* g_inputSystem;
extern TheGame* g_theGame;