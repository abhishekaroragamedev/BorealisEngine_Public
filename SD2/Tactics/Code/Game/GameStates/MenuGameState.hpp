#pragma once
#include "Game/GameStates/GameState.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Tools/Command.hpp"

enum GameMenuItem
{
	PLAY,
	HOST,
	QUIT,
	NUM_GAME_MENU_ITEMS
};

class MenuGameState : public GameState
{

public:
	MenuGameState();
	~MenuGameState();

	void Enter() override;
	void Exit() override;
	void Update() override;
	void Render() const override;

private:
	void TryJoinGame();
	void HandleInput();
	void HandleKeyboardInput();
	void HandleXboxControllerInput();

private:
	Camera* m_camera = nullptr;
	GameMenuItem m_selectedMenuItem = GameMenuItem::PLAY;	// 0 - Play, 1 - Quit

};

bool LoadEncounterCommand( Command& command );
