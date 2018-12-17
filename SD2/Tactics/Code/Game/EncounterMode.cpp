#include "Game/Action.hpp"
#include "Game/Actor.hpp"
#include "Game/EncounterMode.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TacticsNetMessage.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Tools/DevConsole.hpp"

#pragma region EncounterMode

EncounterMode::EncounterMode()
{

}

EncounterMode::~EncounterMode()
{

}

#pragma endregion

#pragma region DefaultMode

DefaultMode::DefaultMode()
{

}

DefaultMode::~DefaultMode()
{

}

void DefaultMode::TopOfStackInit()
{
	g_encounterGameState->SetRenderFlags(
		EncounterRenderFlag::RENDER_FLAG_MAP |
		EncounterRenderFlag::RENDER_FLAG_ACTORS |
		EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT |
		EncounterRenderFlag::RENDER_FLAG_TURNINFO
	);
	if ( !CheckForVictory() )
	{
		g_encounterGameState->SetTurnNumber( ++m_turnNumber );
		InitializeTurnParameters();
		HandleInput();
	}
}

void DefaultMode::Tick()
{
	
}

bool DefaultMode::CheckForVictory()
{
	Team winningTeam = g_encounterGameState->GetCurrentEncounter()->CheckGameOverAndGetWinningTeam();
	if ( winningTeam != Team::TEAM_INVALID )
	{
		g_encounterGameState->PushMode( new VictoryMode( winningTeam ) );
		return true;
	}
	return false;
}

void DefaultMode::InitializeTurnParameters()
{
	Actor* nextActor = g_encounterGameState->GetCurrentEncounter()->AdvanceTimeToNextTurnAndGetNextActor();

	if ( g_encounterGameState->CanClientPlayThisTurn( nextActor ) )
	{
		PlayerType nextActorPlayerType = PlayerType::PLAYER_HUMAN;
		g_encounterGameState->PushMode( new SelectActionMode( nextActor, nextActorPlayerType ) );
	}
	else
	{
		PlayerType nextActorPlayerType = PlayerType::PLAYER_HUMAN;
		g_encounterGameState->PushMode( new WaitForOpponentMode( nextActor, nextActorPlayerType ) );
	}
}

void DefaultMode::HandleInput()
{

}

#pragma endregion

#pragma region SelectActionMode

SelectActionMode::SelectActionMode( Actor* currentActor, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	m_actor( currentActor ),
		m_playerType( playerType ),
		m_actorInitialPosition( m_actor->GetPosition() ),
		m_actorInitialFacingDirection( m_actor->GetFacingDirection() )
{
	m_actor->RefreshActionsForTurn();
}

SelectActionMode::~SelectActionMode()
{

}

void SelectActionMode::TopOfStackInit()
{
	g_encounterGameState->SetRenderFlags(
		EncounterRenderFlag::RENDER_FLAG_MAP |
		EncounterRenderFlag::RENDER_FLAG_ACTORS |
		EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT |
		EncounterRenderFlag::RENDER_FLAG_MENU |
		EncounterRenderFlag::RENDER_FLAG_TURNINFO
	);
	ReinitializeMenuItems();
	g_encounterGameState->SetCurrentActor( m_actor );
	g_encounterGameState->SetCurrentCursorPosition( m_actor->GetPosition() );
}

void SelectActionMode::Tick()
{
	if ( m_actor->IsDead() )
	{
		ReinitializeMenuItems();
	}
	HandleInput();
}

void SelectActionMode::ReinitializeMenuItems()
{
	if ( m_actor->IsDead() )
	{
		m_actor->DisableAllActionsButWait();
	}

	for ( ActionType action : m_actor->GetActions() )
	{
		if ( m_actor->IsActionAvailableThisTurn( action ) )
		{
			m_currentSelectedAction = action;
			g_encounterGameState->SetCurrentlySelectedAction( m_currentSelectedAction );
			break;
		}
	}
}

void SelectActionMode::HandleInput()
{
	static float s_timeSinceAnalogInput = XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS;
	s_timeSinceAnalogInput += GetMasterDeltaSecondsF();

	bool controllerInputAllowed = s_timeSinceAnalogInput >= XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS && g_inputSystem->GetController( 0 ).IsConnected();

	{	// Item selection
		float analogStickY = ( controllerInputAllowed )? g_inputSystem->GetController( 0 ).GetJoystick( 0 ).GetPosition().y : 0.0f;
		if ( analogStickY < 0.0f || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_DOWN_ARROW ) )
		{
			m_currentSelectedAction = m_actor->GetNextAvailableAction( m_currentSelectedAction );
			g_encounterGameState->SetCurrentlySelectedAction( m_currentSelectedAction );
			s_timeSinceAnalogInput = 0.0f;
		}
		else if ( analogStickY > 0.0f || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_UP_ARROW ) )
		{
			m_currentSelectedAction = m_actor->GetPreviousAvailableAction( m_currentSelectedAction );
			g_encounterGameState->SetCurrentlySelectedAction( m_currentSelectedAction );
			s_timeSinceAnalogInput = 0.0f;
		}
	}
	{	// Confirm action
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) )
		{
			switch( m_currentSelectedAction )
			{
				case ActionType::WAIT	:
				{
					g_encounterGameState->InvokeWaitAction(m_actor);
					
					if (g_encounterGameState->IsNetworked())
					{
						AddWaitMessage();
						FlushActionMessages();
					}
					
					g_encounterGameState->PopMode();
					break;
				}
				case ActionType::MOVE	:	g_encounterGameState->PushMode( new MoveMode( m_actor, this ) );		break;
				case ActionType::ATTACK	:	g_encounterGameState->PushMode( new AttackMode( m_actor, this ) );	break;
				case ActionType::BOW	:	g_encounterGameState->PushMode( new BowMode( m_actor, this ) );	break;
				case ActionType::HEAL	:	g_encounterGameState->PushMode( new HealMode( m_actor, this ) );	break;
				case ActionType::CAST_FIRE	:	g_encounterGameState->PushMode( new CastFireMode( m_actor, this ) );	break;
				case ActionType::DEFEND	:
				{
					g_encounterGameState->InvokeDefendAction(m_actor);
					if (g_encounterGameState->IsNetworked())
					{
						AddDefendMessage();
					}

					TopOfStackInit();
					break;
				}
			}
		}
		// Undo Move
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_B ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) )
		{
			if ( !m_actor->IsActionAvailableThisTurn( ActionType::MOVE ) && m_actor->GetLastAction() == ActionType::MOVE )
			{
				UndoMove();
			}
		}
	}
}

void SelectActionMode::UndoMove()
{
	// Functionally undo the last move
	m_actor->SetPosition( m_actorInitialPosition );
	m_actor->SetFacingDirection( m_actorInitialFacingDirection );
	m_actor->EnableAction( ActionType::MOVE );
	m_actor->UndoLastWaitAddition();
	
	if ( g_encounterGameState->IsNetworked() && m_actionMessagesForTurn.size() > 0U )
	{
		// Assume we're removing a move message
		m_actionMessagesForTurn.erase( m_actionMessagesForTurn.begin() + (m_actionMessagesForTurn.size() - 1U) );
	}
	
	// Visually undo the last move
	m_actor->SetRenderPositionFromMapPosition();
	g_encounterGameState->SetCurrentCursorPosition( m_actorInitialPosition );
	g_encounterGameState->SetCameraTargetLookAt( m_actor->GetRenderPosition() );
}

void SelectActionMode::AddMoveMessage( const IntVector2& targetPos )
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_move");
	NetMessage msg( msgIndex );
	msg.Write<IntVector2>( &targetPos );
	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::AddAttackMessage( const IntVector2& targetPos, bool isBlocked, bool isCritical )
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_attack");
	
	char blocked = (isBlocked)? 1 : 0;
	char critical = (isCritical)? 1 : 0;
	NetMessage msg( msgIndex );
	msg.Write<IntVector2>( &targetPos );
	msg.WriteBytes(1U, &blocked);
	msg.WriteBytes(1U, &critical);

	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::AddBowMessage( const IntVector2& targetPos, bool isBlocked, bool isCritical )
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_bow");

	char blocked = (isBlocked)? 1 : 0;
	char critical = (isCritical)? 1 : 0;
	NetMessage msg( msgIndex );
	msg.Write<IntVector2>( &targetPos );
	msg.WriteBytes(1U, &blocked);
	msg.WriteBytes(1U, &critical);

	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::AddHealMessage( const IntVector2& targetPos )
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_heal");
	NetMessage msg( msgIndex );
	msg.Write<IntVector2>( &targetPos );
	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::AddDefendMessage()
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_defend");
	NetMessage msg( msgIndex );
	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::AddCastFireMessage( const IntVector2& targetPos )
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_fire");
	NetMessage msg( msgIndex );
	msg.Write<IntVector2>( &targetPos );
	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::AddWaitMessage()
{
	uint8_t msgIndex = NetSession::GetInstance()->GetMessageIndex("net_wait");
	NetMessage msg( msgIndex );
	m_actionMessagesForTurn.push_back(msg);
}

void SelectActionMode::FlushActionMessages()
{
	if (g_encounterGameState->IsNetworked())
	{
		for( size_t msgIndex = 0U; msgIndex < m_actionMessagesForTurn.size(); msgIndex++ )
		{
			g_encounterGameState->SendActionMessage( m_actionMessagesForTurn[msgIndex] );
		}
		m_actionMessagesForTurn.clear();
	}
}

#pragma endregion

#pragma region ObserveMode

ObserveMode::ObserveMode( Actor* currentActor, SelectActionMode* parentMode /* = nullptr */, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	m_actor( currentActor ),
		m_playerType( playerType ),
		m_parentMode( parentMode )
{
	m_currentCursorPosition = m_actor->GetPosition();
	g_encounterGameState->SetCurrentCursorPosition( m_currentCursorPosition );
	m_orbitCamera = g_encounterGameState->GetOrbitCamera();
}

ObserveMode::~ObserveMode()
{

}

void ObserveMode::TopOfStackInit()
{
	g_encounterGameState->SetRenderFlags(
		EncounterRenderFlag::RENDER_FLAG_MAP |
		EncounterRenderFlag::RENDER_FLAG_ACTORS |
		EncounterRenderFlag::RENDER_FLAG_TILES |
		EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT |
		EncounterRenderFlag::RENDER_FLAG_TURNINFO
	);
}

void ObserveMode::Tick()
{
	HandleInput();
}

void ObserveMode::ComputeSelectableTiles()
{

}

void ObserveMode::HandleInput()
{
	HandleCursorInput();
	HandleOrbitCameraInput();
}

void ObserveMode::HandleOrbitCameraInput()
{
	{	// Rotation
		Vector2 rotationVector = ( g_inputSystem->GetController( 0 ).IsConnected() )? g_inputSystem->GetController( 0 ).GetJoystick( 1 ).GetPosition() : Vector2::ZERO;
		if ( rotationVector == Vector2::ZERO )
		{
			if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_A ) )
			{
				rotationVector.x = -1.0f;
			}
			else if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_D ) )
			{
				rotationVector.x = 1.0f;
			}
			if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_S ) )
			{
				rotationVector.y = -1.0f;
			}
			else if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_W ) )
			{
				rotationVector.y = 1.0f;
			}
		}

		if ( rotationVector != Vector2::ZERO )
		{
			m_orbitCamera->AddToRotation( ( rotationVector.x * ORBIT_CAMERA_ROTATION_SPEED_PER_SECOND ) * g_encounterGameState->GetClock()->GetDeltaSecondsF() );
			m_orbitCamera->AddToAzimuth( ( rotationVector.y * ORBIT_CAMERA_ROTATION_SPEED_PER_SECOND ) * g_encounterGameState->GetClock()->GetDeltaSecondsF() );
		}
	}
	{	
		{	// Zoom out
			float zoomOutMagnitude = ( g_inputSystem->GetController( 0 ).IsConnected() )? g_inputSystem->GetController( 0 ).GetLeftTriggerValue() : 0.0f;
			if ( IsFloatEqualTo( zoomOutMagnitude, 0.0f ) && g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_G ) )
			{
				zoomOutMagnitude = 1.0f;
			}
			m_orbitCamera->AddToRadius( ( zoomOutMagnitude * ORBIT_CAMERA_ZOOM_SPEED_PER_SECOND ) * g_encounterGameState->GetClock()->GetDeltaSecondsF() );
		}
		{	// Zoom in
			float zoomInMagnitude = ( g_inputSystem->GetController( 0 ).IsConnected() )? g_inputSystem->GetController( 0 ).GetRightTriggerValue() : 0.0f;
			if ( IsFloatEqualTo( zoomInMagnitude, 0.0f ) && g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_T ) )
			{
				zoomInMagnitude = 1.0f;
			}
			m_orbitCamera->AddToRadius( ( zoomInMagnitude * -ORBIT_CAMERA_ZOOM_SPEED_PER_SECOND ) * g_encounterGameState->GetClock()->GetDeltaSecondsF() );
		}
	}
	{	// Snap angle
		if	(	g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F ) ||
			( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_LB ) )
			)
		{
			m_orbitCamera->SnapToPreviousFixedAngle();
		}
		if	(	g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_H ) ||
			( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_RB ) )
			)
		{
			m_orbitCamera->SnapToNextFixedAngle();
		}
	}
	{	// Cancel
		if	(	g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) ||
			( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_B ) )
			)
		{
			g_encounterGameState->SetCurrentCursorPosition( m_actor->GetPosition() );
			g_encounterGameState->PopMode();	// Pops Move/AttackMode
		}
	}
}

void ObserveMode::HandleCursorInput()
{
	static float s_timeSinceAnalogInput = XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS;
	s_timeSinceAnalogInput += GetMasterDeltaSecondsF();

	bool controllerInputAllowed = ( s_timeSinceAnalogInput > XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS ) && g_inputSystem->GetController( 0 ).IsConnected();

	Vector2 cursorMovementVector = ( controllerInputAllowed )? g_inputSystem->GetController( 0 ).GetJoystick( 0 ).GetPosition() : Vector2::ZERO;
	if ( cursorMovementVector == Vector2::ZERO )
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_UP_ARROW ) )
		{
			cursorMovementVector.y = 1.0f;
		}
		else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_DOWN_ARROW ) )
		{
			cursorMovementVector.y = -1.0f;
		}
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RIGHT_ARROW ) )
		{
			cursorMovementVector.x = 1.0f;
		}
		else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_LEFT_ARROW ) )
		{
			cursorMovementVector.x = -1.0f;
		}
	}

	if ( cursorMovementVector != Vector2::ZERO )
	{
		Vector3 strongestCandidateVector = Vector3::ZERO;
		float strongestDotProduct = 0.0f;

		float currentDotProduct = DotProduct( cursorMovementVector, Vector2::UP );
		if ( currentDotProduct > strongestDotProduct )
		{
			strongestDotProduct = currentDotProduct;
			strongestCandidateVector = g_encounterGameState->GetOrbitCamera()->GetForward();
		}
		currentDotProduct = DotProduct( cursorMovementVector, Vector2::DOWN );
		if ( currentDotProduct > strongestDotProduct )
		{
			strongestDotProduct = currentDotProduct;
			strongestCandidateVector = g_encounterGameState->GetOrbitCamera()->GetForward() * -1.0f;
		}
		currentDotProduct = DotProduct( cursorMovementVector, Vector2::LEFT );
		if ( currentDotProduct > strongestDotProduct )
		{
			strongestDotProduct = currentDotProduct;
			strongestCandidateVector = g_encounterGameState->GetOrbitCamera()->GetRight() * -1.0f;
		}
		currentDotProduct = DotProduct( cursorMovementVector, Vector2::RIGHT );
		if ( currentDotProduct > strongestDotProduct )
		{
			strongestDotProduct = currentDotProduct;
			strongestCandidateVector = g_encounterGameState->GetOrbitCamera()->GetRight();
		}

		IntVector2 coordinateDelta = Map::GetMapCoordinateDirection( strongestCandidateVector );
		if ( ( ( m_currentCursorPosition.x + coordinateDelta.x ) < 0 ) || ( ( m_currentCursorPosition.x + coordinateDelta.x ) > ( g_encounterGameState->GetCurrentEncounter()->GetMap()->GetWidth() - 1 ) ) )
		{
			coordinateDelta.x = 0;
		}
		if ( ( ( m_currentCursorPosition.y + coordinateDelta.y ) < 0 ) || ( ( m_currentCursorPosition.y + coordinateDelta.y ) > ( g_encounterGameState->GetCurrentEncounter()->GetMap()->GetDepth() - 1 ) ) )
		{
			coordinateDelta.y = 0;
		}

		m_currentCursorPosition = m_currentCursorPosition + coordinateDelta;
		g_encounterGameState->SetCameraTargetLookAt( m_actor->GetOwningMap()->GetWorldPositionFrom2DMapPosition( m_currentCursorPosition ) );

		s_timeSinceAnalogInput = 0.0f;
		g_encounterGameState->SetCurrentCursorPosition( m_currentCursorPosition );
	}
}

bool ObserveMode::PopModeIfActionsComplete()
{
	if ( m_actionInitiated && g_encounterGameState->IsActionQueueEmpty() )
	{
		m_actor->SetLastAction( GetActionType() );
		g_encounterGameState->PopMode();	// Either MoveMode, AttackMode or HealMode pops itself
		return true;
	}
	return false;
}

#pragma endregion

#pragma region WaitForOpponentMode

WaitForOpponentMode::WaitForOpponentMode( Actor* currentActor, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	ObserveMode( currentActor, nullptr, playerType )
{

}

WaitForOpponentMode::~WaitForOpponentMode()
{

}

void WaitForOpponentMode::TopOfStackInit() /*override*/
{
	ObserveMode::TopOfStackInit();

	std::vector<IntVector2> noTiles;
	g_encounterGameState->SetSelectableTiles(noTiles);
	g_encounterGameState->SetRenderFlag(RENDER_FLAG_WAITING_FOR_OPPONENT);
	g_encounterGameState->SetCurrentActor( m_actor );
}

void WaitForOpponentMode::Tick() /*override*/
{
	HandleInput();
}

ActionType WaitForOpponentMode::GetActionType() const /*override*/
{
	return ActionType::ACTION_TYPE_INVALID;
}

void WaitForOpponentMode::HandleInput() /*override*/
{
	HandleOrbitCameraInput();
}

#pragma endregion

#pragma region MoveMode

MoveMode::MoveMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	ObserveMode( currentActor, parentMode, playerType )
{
	ComputeSelectableTiles();
	g_encounterGameState->SetSelectableTiles( m_selectableTiles );
}

MoveMode::~MoveMode()
{

}

void MoveMode::TopOfStackInit()
{
	ObserveMode::TopOfStackInit();
}

void MoveMode::Tick()
{
	if ( !PopModeIfActionsComplete())
	{
		HandleInput();
	}
}

void MoveMode::HandleInput()
{
	HandleActionConfirmationInput();
	ObserveMode::HandleInput();
}

void MoveMode::HandleActionConfirmationInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) || ( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) ) )
	{
		if ( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_currentCursorPosition ) != m_selectableTiles.end() )
		{
			m_actionInitiated = true;
			g_encounterGameState->InvokeMoveAction( m_actor, m_currentCursorPosition );
			if (g_encounterGameState->IsNetworked())
			{
				m_parentMode->AddMoveMessage( m_currentCursorPosition );
			}
		}
	}
}

void MoveMode::ComputeSelectableTiles()
{
	m_selectableTiles = g_encounterGameState->GetCurrentEncounter()->GetMap()->GetTraversableTiles( m_currentCursorPosition, m_actor->GetStats().m_moveSpeed, m_actor->GetStats().m_jumpHeight );

	// Remove tiles already occupied by actors
	for ( int tileIndex = ( static_cast< int >( m_selectableTiles.size() - 1 ) ); tileIndex > 0; tileIndex-- )
	{
		if ( !g_encounterGameState->GetCurrentEncounter()->CanPositionBeMovedInto( m_selectableTiles[ tileIndex ] ) )
		{
			m_selectableTiles.erase( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_selectableTiles[ tileIndex ] ) );
		}
	}
}

ActionType MoveMode::GetActionType() const
{
	return ActionType::MOVE;
}

#pragma endregion

#pragma region AttackMode

AttackMode::AttackMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	ObserveMode( currentActor, parentMode, playerType )
{
	ComputeSelectableTiles();
	g_encounterGameState->SetSelectableTiles( m_selectableTiles );
}

AttackMode::~AttackMode()
{

}

void AttackMode::TopOfStackInit()
{
	ObserveMode::TopOfStackInit();
}

void AttackMode::Tick()
{
	if ( !PopModeIfActionsComplete())
	{
		HandleInput();
	}
}

void AttackMode::HandleInput()
{
	HandleActionConfirmationInput();
	ObserveMode::HandleInput();
}

void AttackMode::HandleActionConfirmationInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) || ( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) ) )
	{
		if ( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_currentCursorPosition ) != m_selectableTiles.end() )
		{
			m_actionInitiated = true;

			m_actor->TurnToward(m_currentCursorPosition);
			bool isBlocked = false;
			bool isCritical = false;
			CheckBlockAndCritical( &isBlocked, &isCritical );

			g_encounterGameState->InvokeAttackAction( m_actor, m_currentCursorPosition, isBlocked, isCritical );
			if (g_encounterGameState->IsNetworked())
			{
				m_parentMode->AddAttackMessage( m_currentCursorPosition, isBlocked, isCritical );
			}
		}
	}
}

void AttackMode::ComputeSelectableTiles()
{
	m_selectableTiles = g_encounterGameState->GetCurrentEncounter()->GetAttackableTiles( m_actor );
}

void AttackMode::CheckBlockAndCritical( bool* out_isBlocked, bool* out_isCritical )
{
	*out_isBlocked = GetTargetActor()->IsAttackBlocked( m_actor->GetFacingDirection() );
	*out_isCritical = GetTargetActor()->IsAttackCritical( m_actor->GetFacingDirection() );
}

ActionType AttackMode::GetActionType() const
{
	return ActionType::ATTACK;
}

Actor* AttackMode::GetTargetActor() const
{
	return g_encounterGameState->GetCurrentEncounter()->GetActorAtPosition( m_currentCursorPosition );
}

#pragma endregion

#pragma region BowMode

BowMode::BowMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	ObserveMode( currentActor, parentMode, playerType )
{
	ComputeSelectableTiles();
	g_encounterGameState->SetSelectableTiles( m_selectableTiles );
}

BowMode::~BowMode()
{

}

void BowMode::TopOfStackInit()
{
	ObserveMode::TopOfStackInit();
}

void BowMode::Tick()
{
	if ( !PopModeIfActionsComplete())
	{
		HandleInput();
	}
}

void BowMode::HandleInput()
{
	HandleActionConfirmationInput();
	ObserveMode::HandleInput();
}

void BowMode::HandleActionConfirmationInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) || ( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) ) )
	{
		if ( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_currentCursorPosition ) != m_selectableTiles.end() )
		{
			m_actionInitiated = true;

			m_actor->TurnToward(m_currentCursorPosition);
			bool isBlocked = false;
			bool isCritical = false;
			CheckBlockAndCritical( &isBlocked, &isCritical );

			g_encounterGameState->InvokeBowAction( m_actor, m_currentCursorPosition, isBlocked, isCritical );
			if (g_encounterGameState->IsNetworked())
			{
				m_parentMode->AddBowMessage( m_currentCursorPosition, isBlocked, isCritical );
			}
		}
	}
}

void BowMode::ComputeSelectableTiles()
{
	m_selectableTiles = g_encounterGameState->GetCurrentEncounter()->GetOccupiedTilesInRange( m_actor, IntRange( 2, 5 ), IntRange( -m_actor->GetOwningMap()->GetHeight(), m_actor->GetOwningMap()->GetHeight() ) );
}

void BowMode::CheckBlockAndCritical( bool* out_isBlocked, bool* out_isCritical )
{
	*out_isBlocked = GetTargetActor()->IsAttackBlocked( m_actor->GetFacingDirection() );
	*out_isCritical = GetTargetActor()->IsAttackCritical( m_actor->GetFacingDirection() );
}

ActionType BowMode::GetActionType() const
{
	return ActionType::BOW;
}

Actor* BowMode::GetTargetActor() const
{
	return g_encounterGameState->GetCurrentEncounter()->GetActorAtPosition( m_currentCursorPosition );
}

#pragma endregion

#pragma region HealMode

HealMode::HealMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	ObserveMode( currentActor, parentMode, playerType )
{
	ComputeSelectableTiles();
	g_encounterGameState->SetSelectableTiles( m_selectableTiles );
}

HealMode::~HealMode()
{

}

void HealMode::TopOfStackInit()
{
	ObserveMode::TopOfStackInit();
}

void HealMode::Tick()
{
	if ( !PopModeIfActionsComplete())
	{
		HandleInput();
	}
}

void HealMode::HandleInput()
{
	HandleActionConfirmationInput();
	ObserveMode::HandleInput();
}

void HealMode::HandleActionConfirmationInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) || ( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) ) )
	{
		if ( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_currentCursorPosition ) != m_selectableTiles.end() )
		{
			m_actionInitiated = true;
			g_encounterGameState->InvokeHealAction( m_actor, m_currentCursorPosition );
			if (g_encounterGameState->IsNetworked())
			{
				m_parentMode->AddHealMessage(m_currentCursorPosition);
			}
		}
	}
}

void HealMode::ComputeSelectableTiles()
{
	m_selectableTiles = g_encounterGameState->GetCurrentEncounter()->GetOccupiedTilesInRange( m_actor, IntRange( 0, 3 ), IntRange( 0, 0 ) );
}

ActionType HealMode::GetActionType() const
{
	return ActionType::HEAL;
}

#pragma endregion

#pragma region CastFireMode

CastFireMode::CastFireMode( Actor* currentActor, SelectActionMode* parentMode, PlayerType playerType /* = PlayerType::PLAYER_HUMAN */ )
	:	ObserveMode( currentActor, parentMode, playerType )
{
	ComputeSelectableTiles();
	g_encounterGameState->SetSelectableTiles( m_selectableTiles );
}

CastFireMode::~CastFireMode()
{

}

void CastFireMode::TopOfStackInit()
{
	ObserveMode::TopOfStackInit();
}

void CastFireMode::Tick()
{
	if ( !PopModeIfActionsComplete())
	{
		HandleInput();
	}
}

void CastFireMode::HandleInput()
{
	HandleActionConfirmationInput();
	ObserveMode::HandleInput();
}

void CastFireMode::HandleActionConfirmationInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) || ( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) ) )
	{
		if ( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_currentCursorPosition ) != m_selectableTiles.end() )
		{
			m_actionInitiated = true;
			g_encounterGameState->InvokeCastFireAction( m_actor, m_currentCursorPosition );
			if (g_encounterGameState->IsNetworked())
			{
				m_parentMode->AddCastFireMessage(m_currentCursorPosition);
			}
		}
	}
}

void CastFireMode::ComputeSelectableTiles()
{
	m_selectableTiles = g_encounterGameState->GetCurrentEncounter()->GetOccupiedTilesInRange( m_actor, IntRange( 0, 1 ), IntRange( -2, 2 ) );
}

ActionType CastFireMode::GetActionType() const
{
	return ActionType::CAST_FIRE;
}

#pragma endregion

#pragma region VictoryMode

VictoryMode::VictoryMode( Team winningTeam )
	:	m_winningTeam( winningTeam )
{

}

VictoryMode::~VictoryMode()
{

}

void VictoryMode::TopOfStackInit()
{
	g_encounterGameState->SetRenderFlags(
		EncounterRenderFlag::RENDER_FLAG_MAP |
		EncounterRenderFlag::RENDER_FLAG_ACTORS |
		EncounterRenderFlag::RENDER_FLAG_VICTORY
	);
}

void VictoryMode::Tick()
{
	HandleInput();
}

void VictoryMode::HandleInput()
{
	if	(	g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) ||
			( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_B ) )
		)
	{
		g_encounterGameState->PopMode();	// Pops VictoryMode
		g_encounterGameState->PopMode();	// Pops DefaultMode
		g_theGame->ChangeGameState( GameStateType::STATE_MENU );
		if ( g_encounterGameState->IsNetworked() )
		{
			NetSession::GetInstance()->Disconnect();
		}
	}
}

Team VictoryMode::GetWinningTeam() const
{
	return m_winningTeam;
}

#pragma endregion
