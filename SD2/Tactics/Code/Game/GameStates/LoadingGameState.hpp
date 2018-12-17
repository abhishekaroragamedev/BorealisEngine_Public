#pragma once
#include "Game/GameStates/GameState.hpp"
#include "Engine/Renderer/Camera.hpp"

class LoadingGameState : public GameState
{

public:
	LoadingGameState();
	~LoadingGameState();
	
	void Enter() override;
	void Exit() override;
	void Update() override;
	void Render() const override;

private:
	Camera* m_camera = nullptr;

};
