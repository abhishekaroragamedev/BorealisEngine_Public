#include "Game/GameCommon.hpp"
#include "Game/GameStates/EncounterGameState.hpp"
#include "Game/TacticsNetMessage.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Tools/DevConsole.hpp"

FlyoutText::FlyoutText( const std::string& text, const Vector3& location, const Rgba& color, bool shouldPersist, float duration /* = 0.0f */, ShaderProgram* shader /* = nullptr */, float cellHeight /* = 0.25f */ )
	:	m_text( text ),
		m_location( location ),
		m_color( color ),
		m_shouldPersist( shouldPersist ),
		m_duration( duration ),
		m_shader( shader ),
		m_cellHeight( cellHeight )
	{
		if ( m_shader == nullptr )
		{
			m_shader = g_renderer->CreateOrGetShaderProgram( DEFAULT_SHADER_NAME );
		}
	}

EncounterGameState::EncounterGameState()
{
	m_encounterClock = new Clock();
}

EncounterGameState::~EncounterGameState()
{
	g_encounterGameState = nullptr;

	if ( m_encounter != nullptr )
	{
		delete m_encounter;
		m_encounter = nullptr;;
	}
	if ( m_orbitCamera != nullptr )
	{
		delete m_orbitCamera;
		m_orbitCamera = nullptr;
	}
	delete m_encounterClock;
	m_encounterClock = nullptr;
}

void EncounterGameState::PushDelayedAction( DelayedAction* action )
{
	m_delayedActionQueue.push_back( action );
}

void EncounterGameState::PushAction( Action* action )
{
	m_actionQueue.push_back( action );
}

void EncounterGameState::PushActionTo( Action* action, int positionInQueue )
{
	std::deque<Action*>::iterator insertionPoint = m_actionQueue.begin() + positionInQueue;
	m_actionQueue.insert( insertionPoint, action );
}

void EncounterGameState::PushMode( EncounterMode* mode )
{
	m_modeStack.push( mode );
	m_modeStack.top()->TopOfStackInit();
}

void EncounterGameState::PopAction()
{
	Action* poppedAction = m_actionQueue.front();
	m_actionQueue.pop_front();
	delete poppedAction;
}

void EncounterGameState::PopMode()
{
	EncounterMode* poppedMode = m_modeStack.top();
	m_modeStack.pop();
	m_modeStack.top()->TopOfStackInit();
	delete poppedMode;
}

void EncounterGameState::ClearActionQueue()
{
	while( !m_actionQueue.empty() )
	{
		PopAction();
	}
}

void EncounterGameState::ClearModeStack()
{
	while( !m_modeStack.empty() )
	{
		EncounterMode* poppedMode = m_modeStack.top();
		m_modeStack.pop();
		delete poppedMode;
	}
}

void EncounterGameState::SendActionMessage( NetMessage& message )
{
	if (m_isNetworked)
	{
		NetSession::GetInstance()->BroadcastMessage(message);
	}
}

void EncounterGameState::InvokeMoveAction( Actor* movingActor, const IntVector2& targetPos )
{
	movingActor->SetLastAction(MOVE);

	std::vector< IntVector2  > path = GetCurrentEncounter()->GetMap()->GetPath( movingActor->GetPosition(), targetPos, movingActor->GetStats().m_moveSpeed, movingActor->GetStats().m_jumpHeight );
	PushAction( new MoveAction( movingActor, targetPos, path ) );
	SetRenderFlags(
		EncounterRenderFlag::RENDER_FLAG_MAP |
		EncounterRenderFlag::RENDER_FLAG_ACTORS |
		EncounterRenderFlag::RENDER_FLAG_TURNINFO
	);
}

void EncounterGameState::InvokeAttackAction( Actor* attackingActor, const IntVector2& targetPos, bool isBlocked, bool isCritical )
{
	attackingActor->SetLastAction(ATTACK);
	PushAction( new AttackAction( attackingActor, targetPos, isBlocked, isCritical ) );
}

void EncounterGameState::InvokeBowAction( Actor* firingActor, const IntVector2& targetPos, bool isBlocked, bool isCritical )
{
	firingActor->SetLastAction(BOW);
	PushAction( new BowAction( firingActor, targetPos, isBlocked, isCritical ) );
}

void EncounterGameState::InvokeHealAction( Actor* healingActor, const IntVector2& targetPos )
{
	healingActor->SetLastAction(HEAL);
	PushAction( new HealAction( healingActor, targetPos ) );
}

void EncounterGameState::InvokeDefendAction( Actor* defendingActor )
{
	defendingActor->SetLastAction(DEFEND);
	PushAction( new DefendAction( defendingActor, ACTOR_DEFEND_ACTION_AUGMENTED_BLOCK_CHANCE ) );
}

void EncounterGameState::InvokeCastFireAction( Actor* firingActor, const IntVector2& targetPos )
{
	firingActor->SetLastAction(CAST_FIRE);
	PushAction( new CastFireAction( firingActor, targetPos ) );
}

void EncounterGameState::InvokeWaitAction( Actor* waitingActor )
{
	waitingActor->SetLastAction(WAIT);
	PushAction( new WaitAction(waitingActor) );
	if (!CanClientPlayThisTurn(m_currentActor))
	{
		// This was a remote invocation - this client is currently in WaitForOpponentMode
		m_popModeNextFrame = true;
	}
}

void EncounterGameState::SetCameraTargetLookAt( const Vector3& target, bool interpolate /*= true*/ )
{
	m_targetLookAt = target;
	m_lookAtLerpSecondsElapsed = 0.0f;

	if ( !interpolate )
	{
		m_currentLookAt = m_targetLookAt;
		m_orbitCamera->SetTarget( m_currentLookAt );
	}
}

void EncounterGameState::LoadEncounter( const EncounterDefinition& encounterDefinition )
{
	if ( m_encounter != nullptr )
	{
		delete m_encounter;
	}
	m_encounter = new Encounter( encounterDefinition );
}

void EncounterGameState::SetNetworkingProperties( bool isNetworked, bool isHost )
{
	m_isNetworked = isNetworked;
	m_isHost = isHost;

	if ( m_isHost )
	{
		CommandRun( "host" );
	}
	else
	{
		m_connectionState = ENC_CONN_CONNECTED;	// Client cannot load this state without first connecting
	}
}

void EncounterGameState::SetRenderFlags( unsigned int bits )
{
	m_renderFlags = bits;
}

void EncounterGameState::SetRenderFlag( EncounterRenderFlag flag )
{
	m_renderFlags |= flag;
}

void EncounterGameState::RemoveRenderFlag( EncounterRenderFlag flag )
{
	m_renderFlags &= ~flag;
}

void EncounterGameState::AdvanceTimeForActiveActions( int waitAmount )
{
	for ( Action* activeAction : m_delayedActionQueue )
	{
		activeAction->DecayWait( waitAmount );
	}
}

void EncounterGameState::SetCurrentCursorPosition( const IntVector2& cursorPosition )
{
	m_currentCursorPosition = cursorPosition;
	SetCameraTargetLookAt( m_encounter->GetMap()->GetWorldPositionFrom2DMapPosition( m_currentCursorPosition ) );
}

void EncounterGameState::SetSelectableTiles( const std::vector< IntVector2 >& selectableTiles )
{
	m_selectableTiles.clear();
	m_selectableTiles.insert( m_selectableTiles.begin(), selectableTiles.begin(), selectableTiles.end() );
}

void EncounterGameState::SetProjectilePosition( const Vector3& position )
{
	m_projectilePosition = position;
}

void EncounterGameState::SetTurnNumber( unsigned int turnNumber )
{
	m_turnNumber = turnNumber;
}

void EncounterGameState::SetCurrentActor( Actor* currentActor )
{
	m_currentActor = currentActor;
	m_availableActions = currentActor->GetActions();
	SetCameraTargetLookAt(m_currentActor->GetRenderPosition());
}

void EncounterGameState::SetCurrentlySelectedAction( ActionType actionType )
{
	m_currentlySelectedAction = actionType;
}

void EncounterGameState::AddFlyoutText( const std::string& key, const FlyoutText& flyoutText )
{
	if ( m_flyoutTextByKey.find( key ) != m_flyoutTextByKey.end() && m_flyoutTextByKey[ key ] != nullptr )
	{
		delete m_flyoutTextByKey[ key ];
	}
	m_flyoutTextByKey[ key ] = new FlyoutText( flyoutText );
}

void EncounterGameState::EnableFlyoutText( const std::string& key )
{
	m_flyoutTextByKey[ key ]->m_enabled = true;
}

void EncounterGameState::DisableFlyoutText( const std::string& key )
{
	m_flyoutTextByKey[ key ]->m_enabled = false;
}

void EncounterGameState::RemoveFlyoutTextIfPresent( const std::string& key )
{
	if ( m_flyoutTextByKey.find( key ) != m_flyoutTextByKey.end() )
	{
		m_flyoutTextByKey.erase( key );
	}
}

void EncounterGameState::Enter()
{
	ConsolePrintf( Rgba::CYAN, "Entered Encounter State." );

	m_orbitCamera = new OrbitCamera( ORBIT_CAMERA_MIN_RADIUS, ORBIT_CAMERA_MAX_RADIUS, ORBIT_CAMERA_MIN_AZIMUTH_DEGREES, ORBIT_CAMERA_MAX_AZIMUTH_DEGREES, g_renderer->GetDefaultColorTarget(), g_renderer->GetDefaultDepthStencilTarget() );
	m_orbitCamera->SetProjectionOrtho( ORBIT_CAMERA_MIN_RADIUS, CLIENT_ASPECT, ORBIT_CAMERA_NEAR_Z, ORBIT_CAMERA_FAR_Z );
	SetCameraTargetLookAt( m_encounter->GetMap()->GetWorldPositionFrom2DMapPosition( m_currentCursorPosition ) );

	m_debugUICamera = new Camera( g_renderer->GetDefaultColorTarget() );
	m_debugUICamera->SetProjectionOrtho( 2.0f, CLIENT_ASPECT, -1.0f, 1.0f );

	CommandRegister( "add_actor", AddActorCommand, "TACTICS: Adds an actor to the current Encounter." );
	CommandRegister( "damage_actor", DamageActorCommand, "TACTICS: Damages an actor with the specified name, if present." );
	CommandRegister( "kill_actor", KillActorCommand, "TACTICS: Kills an actor with the specified name, if present." );

	m_encounterClock->Reset();

	AABB2 projectileBounds;
	projectileBounds.mins.x = 5.0f / 13.0f;
	projectileBounds.mins.y = 6.0f / 13.0f;
	projectileBounds.maxs.x = 6.0f / 13.0f;
	projectileBounds.maxs.y = 7.0f / 13.0f;
	m_projectileSprite = new Sprite(
		g_theGame->GetItemsTextureAtlas()->GetTexture(),
		projectileBounds,
		16.0f,
		Vector2( 0.5f, 0.5f )
	);

	PushMode( new DefaultMode );
}

void EncounterGameState::Exit()
{
	ClearModeStack();
	ClearActionQueue();

	m_isNetworked = false;
	m_isHost = false;
	m_connectionState = ENC_CONN_DISCONNECTED;
	m_otherConnectionIndex = INVALID_CONNECTION_INDEX;

	delete m_projectileSprite;
	m_projectileSprite = nullptr;

	delete m_encounter;
	m_encounter = nullptr;;
	delete m_orbitCamera;
	m_orbitCamera = nullptr;

	CommandUnregister( "add_actor" );
	CommandUnregister( "damage_actor" );
	CommandUnregister( "kill_actor" );
	ConsolePrintf( Rgba::MAGENTA, "Leaving Encounter State." );
}

Encounter* EncounterGameState::GetCurrentEncounter() const
{
	return m_encounter;
}

Actor* EncounterGameState::GetCurrentActor() const
{
	return m_currentActor;
}

OrbitCamera* EncounterGameState::GetOrbitCamera() const
{
	return m_orbitCamera;
}

Clock* EncounterGameState::GetClock() const
{
	return m_encounterClock;
}

unsigned int EncounterGameState::GetRenderFlags() const
{
	return m_renderFlags;
}

bool EncounterGameState::IsActionQueueEmpty() const
{
	return m_actionQueue.empty();
}

bool EncounterGameState::IsModeStackEmpty() const
{
	return m_modeStack.empty();
}

bool EncounterGameState::IsCurrentActorIdle() const
{
	return (
		m_currentActor != nullptr &&
		m_currentActor->GetIsometricSpriteAnimationSet()->GetSpriteAnimationSet()->GetCurrentSpriteAnimation()->GetDefinition()->GetName().find("Idle") != std::string::npos
	);
}

bool EncounterGameState::IsNetworked() const
{
	return m_isNetworked;
}

bool EncounterGameState::IsConnected() const
{
	return ( m_connectionState == ENC_CONN_CONNECTED );
}

bool EncounterGameState::IsInErrorState() const
{
	return ( m_connectionState == ENC_CONN_ERROR );
}

bool EncounterGameState::CanClientPlayThisTurn( Actor* currentActor ) const
{
	return (
		!m_isNetworked ||
		(
			( m_isHost && currentActor != nullptr && currentActor->GetTeam() == TEAM_RED ) ||	// Host is always red
			( !m_isHost && currentActor != nullptr && currentActor->GetTeam() == TEAM_BLUE )	// Client is always blue
		)
	);
}

FlyoutText* EncounterGameState::GetFlyoutText( const std::string& key )
{
	FlyoutText* text = nullptr;
	if ( m_flyoutTextByKey.find( key ) != m_flyoutTextByKey.end() )
	{
		text = m_flyoutTextByKey[ key ];
	}
	return text;
}

void EncounterGameState::Update()
{
	m_timeInState += GetMasterDeltaSecondsF();

	if (ShouldWaitForNetwork())
	{
		return;
	}

	if ( !DevConsole::GetInstance()->IsOpen() )
	{
		HandlePauseInput();
		HandleDebugInput();
		if ( !m_encounterClock->IsPaused() )
		{
			MoveDelayedActionsToActionQueueIfReady();

			bool actionRun = false;
			if ( !m_actionQueue.empty() )
			{
				for ( unsigned int actionIndex = 0; actionIndex < m_actionQueue.size(); actionIndex++ )
				{
					if ( m_actionQueue[ actionIndex ]->CanTick() )
					{
						m_actionQueue[ actionIndex ]->Tick();
						actionRun = true;
						break;
					}
					else
					{
						DelayedAction* action = reinterpret_cast<DelayedAction*>( m_actionQueue[ actionIndex ] );
						if (!action->HasInitialized())
						{
							action->Init();
						}

						if (IsCurrentActorIdle())	// Check if the animation for this delayed action's start has finished
						{
							m_actionQueue.erase( m_actionQueue.begin() + actionIndex );
							m_delayedActionQueue.push_back(action);
							actionIndex--;
						}

						actionRun = true; // Let more delayed actions initialize, if present
						break;
					}
				}
			}
			if ( !actionRun )
			{
				if (m_popModeNextFrame)	// Deferred since this should not interrupt any existing actions
				{
					PopMode();
					m_popModeNextFrame = false;
				}
				m_modeStack.top()->Tick();
			}
			
			UpdateLookAt();
			UpdateActors();
			UpdateFlyoutText();
		}
	}
}

bool EncounterGameState::ShouldWaitForNetwork()
{
	if ( m_isNetworked )
	{
		if (IsInErrorState())
		{
			static float s_secondsInErrorState = 0.0f;
			s_secondsInErrorState += GetMasterDeltaSecondsF();
			if ( s_secondsInErrorState > 5.0f )
			{
				g_theGame->SetCanJoin( false );
				g_theGame->ChangeGameState(STATE_MENU);
				s_secondsInErrorState = 0.0f;
			}
			return true;
		}

		if ( m_isHost && !IsConnected() )
		{
			TrySendJoinAcceptMessage();
			return true;
		}
		else if (IsConnected())
		{
			return TryHandleDisconnect();
		}
	}

	return false;
}

void EncounterGameState::TrySendJoinAcceptMessage()
{
	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
	{
		NetConnection* currentConnection = NetSession::GetInstance()->GetConnection( connIndex );
		if (currentConnection && currentConnection != NetSession::GetInstance()->GetMyConnection() && currentConnection->IsConnected() )
		{
			NetMessage msg( NetSession::GetInstance()->GetMessageIndex("game_join_accept") );
			currentConnection->Send(msg);
			m_connectionState = ENC_CONN_CONNECTED;
			m_otherConnectionIndex = connIndex;
			break;
		}
	}
}

bool EncounterGameState::TryHandleDisconnect()
{
	if (
		( m_isHost && NetSession::GetInstance()->GetConnection( m_otherConnectionIndex ) == nullptr ) ||
		( NetSession::GetInstance()->GetHostConnection() == nullptr )
	)
	{
		m_connectionState = ENC_CONN_ERROR;
		return true;
	}

	return false;
}

void EncounterGameState::HandleDebugInput()
{
	if (g_inputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_N))
	{
		m_sessionOverlayVisible = !m_sessionOverlayVisible;
	}
}

void EncounterGameState::HandlePauseInput()
{
	if	(	g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) ||
			( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START ) )
		)
	{
		if ( m_encounterClock->IsPaused() )
		{
			m_encounterClock->Unpause();
		}
		else
		{
			m_encounterClock->Pause();
		}
	}
	if	(	g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) ||
			( g_inputSystem->GetController( 0 ).IsConnected() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_B ) )
		)
	{
		if ( m_encounterClock->IsPaused() )
		{
			if ( m_isNetworked )
			{
				NetSession::GetInstance()->Disconnect();
			}
			m_encounterClock->Unpause();
			g_theGame->ChangeGameState( GameStateType::STATE_MENU );
		}
	}
}

void EncounterGameState::MoveDelayedActionsToActionQueueIfReady()
{
	for ( size_t index = 0; index < m_delayedActionQueue.size(); index++ )
	{
		if ( m_delayedActionQueue[ index ]->CanTick() )
		{
			m_actionQueue.push_back( m_delayedActionQueue[ index ] );
			m_delayedActionQueue.erase( m_delayedActionQueue.begin() + index );
			index--;
		}
	}
}

void EncounterGameState::UpdateLookAt()
{
	if ( m_lookAtLerpSecondsElapsed < ORBIT_CAMERA_LERP_TO_LOOK_AT_SECONDS )
	{
		m_currentLookAt = Interpolate( m_currentLookAt, m_targetLookAt, ( m_lookAtLerpSecondsElapsed / ORBIT_CAMERA_LERP_TO_LOOK_AT_SECONDS ) );
		m_orbitCamera->SetTarget( m_currentLookAt );
		m_lookAtLerpSecondsElapsed += m_encounterClock->GetDeltaSecondsF();
	}
}

void EncounterGameState::UpdateActors()
{
	for ( Actor* actor : m_encounter->m_actors )
	{
		actor->Update();
	}
}

void EncounterGameState::UpdateFlyoutText()
{
	std::vector< std::string > keysToDelete;

	for ( std::map< std::string, FlyoutText* >::iterator mapIterator = m_flyoutTextByKey.begin(); mapIterator != m_flyoutTextByKey.end(); mapIterator++ )
	{
		if ( !mapIterator->second->m_shouldPersist )
		{
			if ( mapIterator->second->m_duration > 0.0f )
			{
				mapIterator->second->m_duration -= m_encounterClock->GetDeltaSecondsF();
			}
			else
			{
				keysToDelete.push_back( mapIterator->first );
			}
		}
	}

	for ( std::string keyToDelete : keysToDelete )
	{
		delete m_flyoutTextByKey[ keyToDelete ];
		m_flyoutTextByKey.erase( keyToDelete );
	}
}

void EncounterGameState::Render() const
{
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->BindModelMatrixToCurrentShader( Matrix44::IDENTITY );

	g_renderer->SetCamera( m_orbitCamera );
	g_renderer->EnableDepth( COMPARE_LESS, true );
	g_renderer->ClearColor();
	g_renderer->WriteDepthImmediate(true); 
	g_renderer->ClearDepth(1.0f);

	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_MAP ) ) )
	{
		m_encounter->GetMap()->Render();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_TILES ) ) )
	{
		RenderVisibleSelectableTilesAndCursor();
		RenderHiddenSelectableTilesAndCursor();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_ACTORS ) ) )
	{
		RenderActors();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_PROJECTILE ) ) )
	{
		RenderProjectile();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT ) ) )
	{
		RenderFlyoutText();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_ACTORS ) ) )
	{
		RenderHighlightedActorInfo();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_MENU ) ) )
	{
		RenderEncounterMenu();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_TURNINFO ) ) )
	{
		RenderTurnInfo();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_VICTORY ) ) )
	{
		RenderVictoryScreen();
	}
	if ( ( m_renderFlags & static_cast< unsigned int >( EncounterRenderFlag::RENDER_FLAG_WAITING_FOR_OPPONENT ) ) )
	{
		RenderWaitingForOpponentMessage();
	}

	RenderPauseEffectsAndMenu();

	if (m_isNetworked)
	{
		if (m_sessionOverlayVisible)
		{
			RenderNetSessionOverlay();
		}
		if ( m_isHost && !IsConnected() )
		{
			RenderWaitingForConnectionMessage();
		}
		if (IsInErrorState())
		{
			RenderConnectionErrorMessage();
		}
	}
}

void EncounterGameState::RenderEncounterMenu() const
{
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->EnableDepth( COMPARE_ALWAYS, false );
	g_renderer->SetCamera( m_debugUICamera );
	
	std::vector< Rgba > menuItemColors;
	unsigned int currentlySelectedIndex = 0;
	for ( unsigned int actionIndex = 0; actionIndex < m_availableActions.size(); actionIndex++ )	// Grey out used items
	{
		menuItemColors.push_back( Rgba::WHITE );
		if ( !m_currentActor->IsActionAvailableThisTurn( m_availableActions[ actionIndex ] ) )
		{
			menuItemColors[ actionIndex ] = Rgba( 100, 100, 100, 255 );
		}
		if ( m_currentlySelectedAction == m_availableActions[ actionIndex ] )
		{
			currentlySelectedIndex = actionIndex;
		}
	}
	menuItemColors[ currentlySelectedIndex ] = Rgba::YELLOW;		// Selected item

	AABB2 menuBackgroundBounds;
	menuBackgroundBounds.mins = Vector2( -Window::GetAspectMultipliers().x, -Window::GetAspectMultipliers().y );
	menuBackgroundBounds.maxs = menuBackgroundBounds.mins + Vector2( ( 0.6f * Window::GetAspectMultipliers().x ), ( ( static_cast< float >( m_availableActions.size() ) * 0.15f ) * Window::GetAspectMultipliers().y )  );
	g_renderer->DrawAABB( menuBackgroundBounds, Rgba::BLACK.GetWithAlpha( ENCOUNTER_MENU_RENDER_ALPHA ) );

	Vector2 actionDrawEndPosition = Vector2( -0.95f, -0.95f );
	float actionDrawYIncrement = 0.15f;
	for ( unsigned int actionIndex = static_cast< unsigned int >( m_availableActions.size() - 1 ); actionIndex < m_availableActions.size(); actionIndex-- )
	{
		g_renderer->DrawText2D( Vector2( ( actionDrawEndPosition.x * Window::GetAspectMultipliers().x ), ( actionDrawEndPosition.y * Window::GetAspectMultipliers().y ) ), Action::GetName( m_availableActions[ actionIndex ] ), 0.075f, menuItemColors[ actionIndex ], 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ) );
		actionDrawEndPosition.y += actionDrawYIncrement;
	}

	// Render a prompt to undo the last move if the current actor just moved
	if ( !m_currentActor->IsActionAvailableThisTurn( ActionType::MOVE ) && m_currentActor->GetLastAction() == ActionType::MOVE )
	{
		g_renderer->DrawTextInBox2D( "Cancel Move : Esc/B", AABB2( ( -1.0f * Window::GetAspectMultipliers() ), Window::GetAspectMultipliers() ), 0.05f, Rgba::WHITE, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2::ONE );
	}
}

void EncounterGameState::RenderVisibleSelectableTilesAndCursor() const
{
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->EnableDepth( COMPARE_LESS, true );
	m_encounter->GetMap()->RenderCursorsAt( m_selectableTiles, std::vector< IntVector2 >( { m_currentCursorPosition } ), Rgba::BLUE.GetWithAlpha( VISIBLE_CURSOR_RENDER_ALPHA ) );

	g_renderer->EnableDepth( COMPARE_LESS, true );
	m_encounter->GetMap()->RenderCursorAt( m_currentCursorPosition, GetCurrentCursorColor().GetWithAlpha( VISIBLE_CURSOR_RENDER_ALPHA ) );
}

void EncounterGameState::RenderHiddenSelectableTilesAndCursor() const
{
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->EnableDepth( COMPARE_GREATER, false );	// Render tiles hidden beyond walls
	m_encounter->GetMap()->RenderCursorsAt( m_selectableTiles, std::vector< IntVector2 >( { m_currentCursorPosition } ), Rgba::CYAN.GetWithAlpha( HIDDEN_CURSOR_RENDER_ALPHA ) );

	g_renderer->EnableDepth( COMPARE_GREATER, false );	// Render cursor or part of it differently if hidden behind a wall
	m_encounter->GetMap()->RenderCursorAt( m_currentCursorPosition, GetCurrentCursorColor( true ).GetWithAlpha( HIDDEN_CURRENT_CURSOR_RENDER_ALPHA ) );
	g_renderer->EnableDepth( COMPARE_LESS, true );
}

void EncounterGameState::RenderActors() const
{
	std::vector< Actor* > drawOrder;
	std::vector< float > drawOrderDistance;

	for ( std::vector< Actor* >::iterator actorIterator = m_encounter->m_actors.begin(); actorIterator != m_encounter->m_actors.end(); actorIterator++ )
	{
		Actor* currentActor = *actorIterator;

		Vector3 actorCoordinatesInViewSpace = Vector3( m_orbitCamera->GetViewMatrix() * Vector4( currentActor->GetRenderPosition() ) );
		float distanceInViewSpace = actorCoordinatesInViewSpace.z;

		size_t actorIndex = 0;
		for ( actorIndex = 0; actorIndex < drawOrderDistance.size(); actorIndex++ )
		{
			if ( drawOrderDistance[ actorIndex ] < distanceInViewSpace )
			{
				break;
			}
		}
		drawOrder.insert( ( drawOrder.begin() + actorIndex ), currentActor );
		drawOrderDistance.insert( ( drawOrderDistance.begin() + actorIndex ), distanceInViewSpace );
	}

	for ( std::vector< Actor* >::iterator actorIterator = drawOrder.begin(); actorIterator != drawOrder.end(); actorIterator++ )
	{
		( *actorIterator )->Render();
	}
}

void EncounterGameState::RenderHighlightedActorInfo() const
{
	if ( m_encounter->GetActorAtPosition( m_currentCursorPosition ) != nullptr )
	{
		g_renderer->DisableDepth();
		Actor* actorAtCurrentPosition = m_encounter->GetActorAtPosition( m_currentCursorPosition );
		g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
		Vector3 boxDrawPosition = actorAtCurrentPosition->GetRenderPosition() + ( Vector3::UP * ( ( actorAtCurrentPosition->GetStats().m_height * 0.5f ) + ACTOR_STATUS_BOX_GAP_HEIGHT ) );

		g_renderer->UseShaderProgram( g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ) );
		g_renderer->DrawTextInBox3D( actorAtCurrentPosition->GetStatusString(), boxDrawPosition, m_orbitCamera->GetRight(), Vector2( ACTOR_STATUS_BOX_WIDTH, ACTOR_STATUS_BOX_HEIGHT ), 0.25f, Rgba::NO_COLOR, actorAtCurrentPosition->GetDrawColor(), 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), Vector2( 0.5f, 0.0f ), Vector2( 0.5f, 0.5f ) );
		g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	}
}

void EncounterGameState::RenderProjectile() const
{
	g_renderer->EnableDepth( COMPARE_LESS, true );
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->DrawCube( m_projectilePosition, ( Vector3::ONE * PROJECTILE_SIZE ), Rgba::WHITE );
}

void EncounterGameState::RenderFlyoutText() const
{
	for ( std::map< std::string, FlyoutText* >::const_iterator mapIterator = m_flyoutTextByKey.begin(); mapIterator != m_flyoutTextByKey.end(); mapIterator++ )
	{
		FlyoutText* flyoutText = mapIterator->second;
		if ( flyoutText->m_enabled )
		{
			g_renderer->UseShaderProgram( flyoutText->m_shader );
			for ( ShaderFloatUniform floatUniform : flyoutText->m_shaderParams )
			{
				g_renderer->BindAttribToCurrentShader( floatUniform );
			}

			g_renderer->DrawTextInBox3D( flyoutText->m_text, flyoutText->m_location, m_orbitCamera->GetRight(), Vector2( ACTOR_STATUS_BOX_WIDTH, ACTOR_STATUS_BOX_HEIGHT ), flyoutText->m_cellHeight, Rgba::NO_COLOR, flyoutText->m_color, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), Vector2( 0.5f, 0.5f ), Vector2( 0.5f, 0.5f ) );
		}
	}
}

void EncounterGameState::RenderVictoryScreen() const
{
	g_renderer->EnableDepth( COMPARE_ALWAYS, false );
	g_renderer->SetCamera( m_debugUICamera );

	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	AABB2 fullscreenBounds = AABB2( -Window::GetAspectMultipliers().x, -Window::GetAspectMultipliers().y, Window::GetAspectMultipliers().x, Window::GetAspectMultipliers().y );
	g_renderer->DrawAABB( fullscreenBounds, Rgba::BLACK.GetWithAlpha( ENCOUNTER_MENU_RENDER_ALPHA ) );

	std::string winningTeamName = "";
	Rgba victoryTextColor;
	switch( m_encounter->CheckGameOverAndGetWinningTeam() )
	{
		case Team::TEAM_RED		:	winningTeamName = "Red"; victoryTextColor = Rgba::RED; break;
		case Team::TEAM_BLUE	:	winningTeamName = "Blue"; victoryTextColor = Rgba::BLUE; break;
	}

	g_renderer->DrawTextInBox2D( ( winningTeamName + " Team Wins!\n\nPress B or Enter to return to\nMain Menu" ), fullscreenBounds, 0.1f, victoryTextColor, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.5f ) );
}

void EncounterGameState::RenderWaitingForOpponentMessage() const
{
	if (!CanClientPlayThisTurn(m_currentActor))
	{
		g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );

		AABB2 fullscreenBounds = AABB2( -Window::GetAspectMultipliers().x, -Window::GetAspectMultipliers().y, Window::GetAspectMultipliers().x, Window::GetAspectMultipliers().y );
		g_renderer->DrawTextInBox2D(
			"Observing: Waiting for opponent",
			fullscreenBounds,
			0.1f,
			Rgba::WHITE,
			1.0f,
			g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ),
			TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT,
			Vector2( 0.5f, 0.9f )
		);
	}
}

void EncounterGameState::RenderTurnInfo() const
{
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->EnableDepth( COMPARE_ALWAYS, false );
	g_renderer->SetCamera( m_debugUICamera );

	g_renderer->DrawAABB( AABB2( -Window::GetAspectMultipliers().x, ( 0.65f * Window::GetAspectMultipliers().y ), ( -0.4f * Window::GetAspectMultipliers().x ), Window::GetAspectMultipliers().y ), Rgba::BLACK.GetWithAlpha( ENCOUNTER_MENU_RENDER_ALPHA ) );

	Rgba turnInfoRenderColor = Rgba::WHITE;
	
	switch( m_currentActor->GetTeam() )
	{
		case Team::TEAM_RED		:	turnInfoRenderColor = Rgba::RED; break;
		case Team::TEAM_BLUE	:	turnInfoRenderColor = Rgba::BLUE; break;
	}
	
	g_renderer->DrawText2D( Vector2( ( -0.95f * Window::GetAspectMultipliers().x ), ( 0.9f * Window::GetAspectMultipliers().y ) ), ("Turn " + std::to_string( m_turnNumber ) ), 0.05f, turnInfoRenderColor, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ) );
}

void EncounterGameState::RenderPauseEffectsAndMenu() const
{
	static float s_pauseMenuEffectBlendFactor = 0.0f;

	std::map< const char*, float > shaderParameters;
	shaderParameters[ "BLENDFACTOR" ] = ( s_pauseMenuEffectBlendFactor / PAUSE_MENU_EFFECT_BLEND_TIME_SECONDS );
	if ( m_encounterClock->IsPaused() || ( s_pauseMenuEffectBlendFactor > 0.0f ) )
	{
		g_renderer->ApplyEffect( g_renderer->CreateOrGetShaderProgram( FULLSCREEN_GRAYSCALE_SHADER_NAME ), shaderParameters );	
		g_renderer->FinishEffects();
		if ( m_encounterClock->IsPaused() && s_pauseMenuEffectBlendFactor < PAUSE_MENU_EFFECT_BLEND_TIME_SECONDS )
		{
			s_pauseMenuEffectBlendFactor += GetMasterDeltaSecondsF();
			s_pauseMenuEffectBlendFactor = ClampFloat( s_pauseMenuEffectBlendFactor, 0.0f, PAUSE_MENU_EFFECT_BLEND_TIME_SECONDS );
		}
		else if ( !m_encounterClock->IsPaused() && s_pauseMenuEffectBlendFactor > 0.0f )
		{
			s_pauseMenuEffectBlendFactor -= GetMasterDeltaSecondsF();
			s_pauseMenuEffectBlendFactor = ClampFloat( s_pauseMenuEffectBlendFactor, 0.0f, PAUSE_MENU_EFFECT_BLEND_TIME_SECONDS );
		}

		if ( m_encounterClock->IsPaused() )
		{
			g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
			g_renderer->SetCamera( m_debugUICamera );
			g_renderer->DrawTextInBox2D( "Paused\nPress start/P to resume\nPress B/escape to quit", AABB2( -0.6f, -0.3f, 0.6f, 0.3f ), 0.2f, Rgba::PURPLE, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.5f ) );
		}
	}
}

void EncounterGameState::RenderNetSessionOverlay() const
{
	g_renderer->SetCamera( m_debugUICamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "UI" ) );
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.95f ),
		"Session info:",
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	NetAddressIPv4 boundAddr = NetSession::GetInstance()->GetSocket()->m_netAddress;
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.925f ),
		Stringf( "\tBound address: %s", boundAddr.ToString().c_str() ),
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	float netTimeSeconds = NetSession::GetInstance()->GetNetTimeMS() * 0.001f;
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.9f ),
		Stringf( "\tNet Time: %.3fs", netTimeSeconds ),
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	float sessionSendRate = NetSession::GetInstance()->GetSendRate();
	FloatRange simLagMs = NetSession::GetInstance()->GetSimLag();
	float simLoss = NetSession::GetInstance()->GetSimLoss();
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.875f ),
		Stringf( "\tSendRate: %.2fHz\tSimLag: %.2fms-%.2fms\t SimLoss: %.2f%%", sessionSendRate, simLagMs.min, simLagMs.max, ( simLoss * 100.0f ) ),
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.85f ),
		"\tConnections:",
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	const char connectionRowFormat[] = "\t%-7s%-16s%-29s%-10s%-11s%-10s%-10s%-10s%-10s%-12s%-12s%-17s%-13s";
	std::string titleBar = Stringf(
		connectionRowFormat,
		"\tIndex",
		"ID",
		"Address",
		"SendRate",
		"RTT",
		"Loss",
		"LastSent",
		"LastRecv",
		"ACK",
		"LastConfACK",
		"LastRecvACK",
		"PrevACKs",
		"NumReliables"
	);
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.825f ),
		titleBar,
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	float drawY = 0.775f;
	uint8_t numConnections = NetSession::GetInstance()->GetNumConnections();
	for ( uint8_t index = 0U; index < numConnections; index++ )
	{
		NetConnection* connection = NetSession::GetInstance()->GetConnection( index );
		if ( connection == nullptr )
		{
			continue;
		}

		NetAddressIPv4 connectionAddr = connection->m_address;
		std::string addrStr = connectionAddr.ToString();
		if ( connection->IsHost() )
		{
			addrStr += "[h]";
		}
		if ( connection->IsMe() )
		{
			addrStr += "[l]";
		}
		float sendRate = connection->GetSendRate();
		float rtt = connection->GetRTT();
		float loss = connection->GetLoss();
		float timeSinceSend = connection->GetTimeSinceLastSendMS();
		float timeSinceReceive = connection->GetTimeSinceLastReceiveMS();
		uint16_t ack = connection->GetNextAck();
		uint16_t lastConfirmedAck = connection->GetLastConfirmedAck();
		uint16_t lastAck = connection->GetLastReceivedAck();
		std::string prevAcks = connection->GetPreviousAcks();
		size_t numUnconfirmedReliables = connection->GetNumUnconfirmedReliables();

		std::string connectionText = Stringf(
			connectionRowFormat,
			std::to_string( index ).c_str(),
			connection->GetInfo().m_id,
			addrStr.c_str(),
			Stringf( "%.2fHz", sendRate ).c_str(),
			Stringf( "%.3fs", rtt * 0.001f ).c_str(),
			Stringf( "%.2f", loss ).c_str(),
			Stringf( "%.2fs", timeSinceSend * 0.001f ).c_str(),
			Stringf( "%.2fs", timeSinceReceive * 0.001f ).c_str(),
			std::to_string( ack ).c_str(),
			std::to_string( lastConfirmedAck ).c_str(),
			std::to_string( lastAck ).c_str(),
			prevAcks.c_str(),
			std::to_string( numUnconfirmedReliables ).c_str()
		);

		g_renderer->DrawText2D(
			Vector2( ( -1.0f * Window::GetAspect() ), drawY ),
			connectionText,
			0.025f,
			Rgba::WHITE,
			0.85f,
			g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
		);
		drawY -= 0.025f;
	}
}

void EncounterGameState::RenderWaitingForConnectionMessage() const
{
	g_renderer->SetCamera( m_debugUICamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "UI" ) );

	Vector2 screenDimNDC = Vector2(Window::GetAspect(), 1.0f);
	g_renderer->DrawAABB(
		AABB2( (-1.0f * screenDimNDC), screenDimNDC ),
		Rgba::BLACK.GetWithAlpha( 0.75f )
	);

	NetAddressIPv4 myAddr = NetSession::GetInstance()->GetMyConnection()->m_address;
	g_renderer->DrawTextInBox2D(
		Stringf( "Waiting for connection on\n%s.", myAddr.ToString().c_str() ),
		AABB2( (-0.9f * screenDimNDC), (0.9f * screenDimNDC) ),
		0.2f,
		Rgba::WHITE,
		1.0f,
		g_renderer->CreateOrGetBitmapFont(DEFAULT_FONT_NAME),
		TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT,
		Vector2(0.5f, 0.5f)
	);
}

void EncounterGameState::RenderConnectionErrorMessage() const
{
	g_renderer->SetCamera( m_debugUICamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "UI" ) );

	Vector2 screenDimNDC = Vector2(Window::GetAspect(), 1.0f);
	g_renderer->DrawAABB(
		AABB2( (-1.0f * screenDimNDC), screenDimNDC ),
		Rgba::BLACK.GetWithAlpha( 0.75f )
	);

	g_renderer->DrawTextInBox2D(
		Stringf( "ERROR: Opponent disconnected. The game cannot proceed." ),
		AABB2( (-0.9f * screenDimNDC), (0.9f * screenDimNDC) ),
		0.2f,
		Rgba::RED,
		1.0f,
		g_renderer->CreateOrGetBitmapFont(DEFAULT_FONT_NAME),
		TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT,
		Vector2(0.5f, 0.5f)
	);
}

void EncounterGameState::PrintToDebugUI( const std::string& text, const Vector2& textMins ) const
{
	g_renderer->EnableDepth( COMPARE_ALWAYS, false );
	g_renderer->SetCamera( m_debugUICamera );
	g_renderer->DrawText2D( textMins, text, 0.02f, Rgba::WHITE, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ) );
}

Rgba EncounterGameState::GetCurrentCursorColor( bool getHiddenColors /* = false */ ) const
{
	Rgba cursorColor = ( getHiddenColors )? Rgba::WHITE : Rgba::GREEN;

	// Non-traversable tiles
	if ( ( std::find( m_selectableTiles.begin(), m_selectableTiles.end(), m_currentCursorPosition ) == m_selectableTiles.end() ) )
	{
		cursorColor = ( getHiddenColors )? Rgba::ORANGE : Rgba::RED;
	}

	// Moving into an actor's space
	/*
	if ( m_currentMenuState == EncounterMenuState::MENU_STATE_MOVE && !m_encounter->CanPositionBeMovedInto( m_currentCursorPosition ) )
	{
		cursorColor = ( getHiddenColors )? Rgba::ORANGE : Rgba::RED;
	}
	*/
	return cursorColor;
}

bool AddActorCommand( Command& command )
{
	static const char s_commandErrorString[] = "ERROR: add_actor actordefinitionname team name coordinateX,coordinateY";

	if ( command.GetName() == "add_actor" )
	{
		std::string actorDefinitionName = command.GetNextString();
		if ( ActorDefinition::s_definitions.find( actorDefinitionName ) == ActorDefinition::s_definitions.end() )
		{
			ConsolePrintf( Rgba::RED, "ERROR: add_actor: Invalid actor definition name provided." );
			ConsolePrintf( Rgba::RED, s_commandErrorString );
			return false;
		}
		ActorDefinition* actorDefinition = ActorDefinition::s_definitions[ actorDefinitionName ];

		std::string teamName = command.GetNextString();
		Team team = GetTeamFromTeamName( teamName );
		if ( team == Team::TEAM_INVALID )
		{
			ConsolePrintf( Rgba::RED, "ERROR: add_actor: Invalid team name provided. Allowed values: Red and Blue." );
			ConsolePrintf( Rgba::RED, s_commandErrorString );
			return false;
		}

		std::string actorName = command.GetNextString();

		IntVector2 coordinates;

		try
		{
			/*	DEPRECATED - Stats are determined by ActorDefinition
			int maxHealth = stoi( command.GetNextString() );
			int moveSpeed = stoi( command.GetNextString() );
			int jumpHeight = stoi( command.GetNextString() );
			int strength = stoi( command.GetNextString() );
			int actionSpeed = stoi( command.GetNextString() );
			int height = stoi( command.GetNextString() );
			stats = Stats( maxHealth, moveSpeed, jumpHeight, strength, actionSpeed, height );
			*/
			coordinates.SetFromText( command.GetNextString() );
		}
		catch ( std::invalid_argument& invalidArgument )
		{
			UNUSED( invalidArgument );
			ConsolePrintf( Rgba::RED, "ERROR: add_actor: Invalid coordinate values provided." );
			ConsolePrintf( Rgba::RED, s_commandErrorString );
			return false;
		}

		EncounterGameState* encounterState = reinterpret_cast< EncounterGameState* >( g_theGame->GetGameStateInstance( GameStateType::STATE_ENCOUNTER ) );
		if ( encounterState->GetCurrentEncounter()->AddActor( actorDefinition, team, actorName, coordinates ) )
		{
			ConsolePrintf( Rgba::GREEN, "Actor added successfully." );
			return true;
		}
		else
		{
			ConsolePrintf( Rgba::RED, "Failed to add Actor." );
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool DamageActorCommand( Command& command )
{
	if ( command.GetName() == "damage_actor" )
	{
		std::string actorName = command.GetNextString();
		if ( actorName == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: damage_actor: No actor name entered." );
			return false;
		}

		Actor* foundActor = g_encounterGameState->GetCurrentEncounter()->GetActorWithName(actorName);
		if ( foundActor == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: damage_actor: No actor with name %s found.", actorName.c_str() );
			return false;
		}

		if ( foundActor->IsDead() )
		{
			ConsolePrintf( Rgba::RED, "ERROR: damage_actor: Actor is already dead." );
			return false;
		}

		std::string damageStr = command.GetNextString();
		if ( damageStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: damage_actor: No damage amount entered." );
			return false;
		}
		int damage;
		try
		{
			damage = stoi(damageStr);
		}
		catch (std::invalid_argument& args)
		{
			UNUSED(args);
			ConsolePrintf( Rgba::RED, "ERROR: damage_actor: Invalid damage amount entered." );
			return false;
		}

		foundActor->Damage( damage );
		if (foundActor->GetHealth() == 0)
		{
			g_encounterGameState->PushAction( new DeathAction(foundActor) );
		}

		return true;
	}
	return false;
}

bool KillActorCommand( Command& command )
{
	if ( command.GetName() == "kill_actor" )
	{
		std::string actorName = command.GetNextString();
		if ( actorName == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: kill_actor: No actor name entered." );
			return false;
		}

		Actor* foundActor = g_encounterGameState->GetCurrentEncounter()->GetActorWithName(actorName);
		if ( foundActor == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: kill_actor: No actor with name %s found.", actorName.c_str() );
			return false;
		}

		if ( foundActor->IsDead() )
		{
			ConsolePrintf( Rgba::RED, "ERROR: kill_actor: Actor is already dead." );
			return false;
		}

		foundActor->Damage( 999999 );
		g_encounterGameState->PushAction( new DeathAction(foundActor) );

		return true;
	}
	return false;
}
