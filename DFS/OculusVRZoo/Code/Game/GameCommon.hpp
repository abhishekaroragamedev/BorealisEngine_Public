#pragma once

#include "Game/TheApp.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureCube.hpp"

// Subsystems
extern TheApp* g_theApp;
extern AudioSystem* g_audioSystem;
extern Renderer* g_renderer;
extern InputSystem* g_inputSystem;
extern TheGame* g_theGame;

// Resource filenames
constexpr char DEFAULT_FONT_NAME[] = "SquirrelFixedFont";
constexpr char TERRAIN8x8_TEXTURE_FILEPATH[] ="Data/Images/Terrain_8x8.png";
constexpr char TERRAIN32x32_TEXTURE_FILEPATH[] ="Data/Images/Terrain_32x32.png";
constexpr char TEXTURE_CUBE_FILEPATH[] = "Data/Images/Skybox.jpg";

// Resources (Owned by LoadState)
extern BitmapFont* g_defaultFont; // Owned by TheApp, as this needs to be loaded before TheGame
extern Texture* g_terrainTexture;
extern TextureCube* g_skyboxTexture;
