#pragma once

enum GameStateType
{
	STATE_INVALID = -1,
	STATE_LOADING = 0,
	STATE_MENU = 1,
	STATE_ENCOUNTER = 2,
	NUM_STATES
};

class GameState
{

public:
	virtual void Enter() = 0;
	virtual void Exit() = 0;
	virtual void Update() = 0;
	virtual void Render() const = 0;

protected:
	float m_timeInState = 0.0f;

};
