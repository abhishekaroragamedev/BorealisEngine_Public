#pragma once

#include "Game/TheGame.hpp"
#include "Game/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

constexpr float SCREEN_HEIGHT = 1000.0f;
constexpr float CLIENT_ASPECT = 16.0f / 9.0f;

extern AudioSystem* g_audioSystem;
extern Renderer* g_renderer;
extern SpriteSheet* g_tileSpriteSheet;
extern InputSystem* g_inputSystem;
extern Camera* g_mainCamera;
extern TheGame* g_theGame;