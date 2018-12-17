#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"
#include <vector>

enum ActionType;
class Actor;

class EncounterMode
{

public:
	EncounterMode();
	~EncounterMode();

	virtual void TopOfStackInit() = 0;
	virtual void Tick() = 0;

protected:
	virtual void HandleInput() = 0;

};

class DefaultMode : public EncounterMode	// Handles which actor gets to go next, or declares victory if one side has won
{

public:
	DefaultMode();	// Invoked by EncounterGameState::Enter(); this mode always remains at the bottom of the stack
	~DefaultMode();

	void TopOfStackInit() override;	// Called at the beginning of every turn
	void Tick() override;

private:
	bool CheckForVictory();
	void InitializeTurnParameters();
	void HandleInput() override;

private:
	unsigned int m_turnNumber = 0;

};

class SelectActionMode : public EncounterMode	// Handles Action selection
{

public:
	SelectActionMode( Actor* currentActor, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed by DefaultMode::Tick() if the game is not over
	~SelectActionMode();

	void TopOfStackInit() override;	// Refreshes menu based on actions the player has taken this turn
	void Tick() override;

	void AddMoveMessage( const IntVector2& targetPos );
	void AddAttackMessage( const IntVector2& targetPos, bool isBlocked, bool isCritical );
	void AddBowMessage( const IntVector2& targetPos, bool isBlocked, bool isCritical );
	void AddHealMessage( const IntVector2& targetPos );
	void AddDefendMessage();
	void AddCastFireMessage( const IntVector2& targetPos );
	void AddWaitMessage();

private:
	void HandleInput() override;	// Changes selected action and creates a new mode depending on what action is chosen
	void ReinitializeMenuItems();	// Called whenever this Mode is on top of the stack, only once
	void UndoMove();
	void FlushActionMessages();

private:
	Actor* m_actor = nullptr;
	PlayerType m_playerType = PlayerType::PLAYER_HUMAN;
	ActionType m_currentSelectedAction = ActionType::ACTION_TYPE_INVALID;
	IntVector2 m_actorInitialPosition = IntVector2::ZERO;
	Vector3 m_actorInitialFacingDirection = Vector3::ZERO;
	std::vector<NetMessage> m_actionMessagesForTurn;

};

class ObserveMode : public EncounterMode
{

public:
	ObserveMode( Actor* currentActor, SelectActionMode* parentMode = nullptr, PlayerType playerType = PlayerType::PLAYER_HUMAN );
	~ObserveMode();

	virtual void TopOfStackInit() override;
	virtual void Tick() override;

	virtual ActionType GetActionType() const = 0;

protected:
	virtual void HandleInput() override;
	void HandleCursorInput();
	void HandleOrbitCameraInput();
	virtual void ComputeSelectableTiles();
	bool PopModeIfActionsComplete();

protected:
	Actor* m_actor = nullptr;
	PlayerType m_playerType = PlayerType::PLAYER_HUMAN;
	std::vector< IntVector2 > m_selectableTiles;
	IntVector2 m_currentCursorPosition = IntVector2::ZERO;
	OrbitCamera* m_orbitCamera = nullptr;
	SelectActionMode* m_parentMode = nullptr;	// Can remain nullptr for WaitForOpponentMode, but must be non-null for all other other child classes of ObserveMode
	bool m_actionInitiated = false;

};

class WaitForOpponentMode : public ObserveMode
{

public:
	WaitForOpponentMode( Actor* currentActor, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed onto stack by SelectActionMode if the "Move" action is chosen; populates selectable tiles here
	~WaitForOpponentMode();

	void TopOfStackInit() override;
	void Tick() override;

	ActionType GetActionType() const override;

private:
	void HandleInput() override;

};

class MoveMode : public ObserveMode	// Handles tile selection for Movement
{

public:
	MoveMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed onto stack by SelectActionMode if the "Move" action is chosen; populates selectable tiles here
	~MoveMode();

	void TopOfStackInit() override;
	void Tick() override;

	ActionType GetActionType() const override;

private:
	void HandleInput() override;	// Moves cursor around and allows a player to select a tile/cancel and pop this Mode off the stack
	void HandleActionConfirmationInput();
	void ComputeSelectableTiles() override;

};

class AttackMode : public ObserveMode	// Handles tile selection for Attack
{

public:
	AttackMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed onto stack by SelectActionMode if the "Move" action is chosen; populates selectable tiles here
	~AttackMode();

	void TopOfStackInit() override;
	void Tick() override;

	ActionType GetActionType() const override;
	Actor* GetTargetActor() const;

private:
	void HandleInput() override;	// Moves cursor around and allows a player to select a tile/cancel and pop this Mode off the stack
	void HandleActionConfirmationInput();
	void ComputeSelectableTiles() override;
	void CheckBlockAndCritical( bool* out_isBlocked, bool* out_isCritical );

};

class BowMode : public ObserveMode	// Handles tile selection for Bow
{

public:
	BowMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed onto stack by SelectActionMode if the "Move" action is chosen; populates selectable tiles here
	~BowMode();

	void TopOfStackInit() override;
	void Tick() override;

	ActionType GetActionType() const override;
	Actor* GetTargetActor() const;

private:
	void HandleInput() override;	// Moves cursor around and allows a player to select a tile/cancel and pop this Mode off the stack
	void HandleActionConfirmationInput();
	void ComputeSelectableTiles() override;
	void CheckBlockAndCritical( bool* out_isBlocked, bool* out_isCritical );

};

class HealMode : public ObserveMode	// Handles tile selection for Heal
{

public:
	HealMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed onto stack by SelectActionMode if the "Heal" action is chosen; populates selectable tiles here
	~HealMode();

	void TopOfStackInit() override;
	void Tick() override;

	ActionType GetActionType() const override;

private:
	void HandleInput() override;	// Moves cursor around and allows a player to select a tile/cancel and pop this Mode off the stack
	void HandleActionConfirmationInput();
	void ComputeSelectableTiles() override;

};

class CastFireMode : public ObserveMode	// Handles tile selection for Cast Fire
{

public:
	CastFireMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType = PlayerType::PLAYER_HUMAN );	// Pushed onto stack by SelectActionMode if the "Cast Fire" action is chosen; populates selectable tiles here
	~CastFireMode();

	void TopOfStackInit() override;
	void Tick() override;

	ActionType GetActionType() const override;

private:
	void HandleInput() override;	// Moves cursor around and allows a player to select a tile/cancel and pop this Mode off the stack
	void HandleActionConfirmationInput();
	void ComputeSelectableTiles() override;

};

class VictoryMode : public EncounterMode	// Handles victory screen and transition back to main menu
{

public:
	VictoryMode( Team winningTeam );	// Pushed onto stack by DefaultMode if the game is over
	~VictoryMode();

	void TopOfStackInit() override;	
	void Tick() override;

	Team GetWinningTeam() const;

private:
	void HandleInput() override;	// Waits for the user to press a button to call TheGame::SetNextGameState( MAIN_MENU )

private:
	Team m_winningTeam = Team::TEAM_INVALID;

};
