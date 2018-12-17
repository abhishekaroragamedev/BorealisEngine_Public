#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

constexpr float SCREEN_WIDTH = 1000.0f;
constexpr float SCREEN_HEIGHT = 1000.0f;

extern Renderer* g_renderer;
extern InputSystem* g_inputSystem;