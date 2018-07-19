#pragma once

#include "Game/AttractMenu.hpp"
#include "Game/VictoryScreen.hpp"

enum GameState
{
	STATE_NONE = -1,
	STATE_ATTRACT,
	STATE_GAME,
	STATE_VICTORY,
	NUM_STATES
};

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update( float deltaSeconds );
	void Render() const;
	bool IsQuitting() const;

private:
	void HandleKeyboardInput();
	void HandleXboxControllerInput();
	void UpdateNextStateIfPlayerHasWonGame();
	void TransitionToNextState();

private:
	GameState m_gameState;
	AttractMenu* m_attractMenu;
	VictoryScreen* m_victoryScreen;
	GameState m_nextState;		// For transitions
	bool m_isQuitting;

};
