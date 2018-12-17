#include "Game/GameCommon.hpp"
#include "Game/TacticsNetMessage.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"

bool OnGameJoinAccept( NetMessage& message, NetConnection& connection )
{
	UNUSED( message );
	UNUSED( connection );

	g_theGame->SetCanJoin( true );
	return true;
}

bool OnNetMove( NetMessage& message, NetConnection& connection )
{
	UNUSED( connection );

	IntVector2 targetPos;
	if( !message.Read<IntVector2>(&targetPos) )
	{
		return false;
	}

	Actor* currentActor = g_encounterGameState->GetCurrentActor();
	g_encounterGameState->InvokeMoveAction( currentActor, targetPos );
	return true;
}

bool OnNetAttack( NetMessage& message, NetConnection& connection )
{
	UNUSED( connection );

	IntVector2 targetPos;
	if( !message.Read<IntVector2>(&targetPos) )
	{
		return false;
	}
	char isBlocked;
	if ( !message.ReadBytes(&isBlocked, 1U) )
	{
		return false;
	}
	char isCritical;
	if ( !message.ReadBytes(&isCritical, 1U) )
	{
		return false;
	}

	Actor* currentActor = g_encounterGameState->GetCurrentActor();
	g_encounterGameState->InvokeAttackAction( currentActor, targetPos, isBlocked, isCritical );
	return true;
}

bool OnNetBow( NetMessage& message, NetConnection& connection )
{
	UNUSED( connection );

	IntVector2 targetPos;
	if ( !message.Read<IntVector2>(&targetPos) )
	{
		return false;
	}
	char isBlocked;
	if ( !message.ReadBytes(&isBlocked, 1U) )
	{
		return false;
	}
	char isCritical;
	if ( !message.ReadBytes(&isCritical, 1U) )
	{
		return false;
	}

	Actor* currentActor = g_encounterGameState->GetCurrentActor();
	g_encounterGameState->InvokeBowAction( currentActor, targetPos, isBlocked, isCritical );
	return true;
}

bool OnNetHeal( NetMessage& message, NetConnection& connection )
{
	UNUSED( connection );

	IntVector2 targetPos;
	if( !message.Read<IntVector2>(&targetPos) )
	{
		return false;
	}

	Actor* currentActor = g_encounterGameState->GetCurrentActor();
	g_encounterGameState->InvokeHealAction( currentActor, targetPos );
	return true;
}

bool OnNetDefend( NetMessage& message, NetConnection& connection )
{
	UNUSED( message );
	UNUSED( connection );

	g_encounterGameState->InvokeDefendAction( g_encounterGameState->GetCurrentActor() );
	return true;
}

bool OnNetCastFire( NetMessage& message, NetConnection& connection )
{
	UNUSED( connection );

	IntVector2 targetPos;
	if( !message.Read<IntVector2>(&targetPos) )
	{
		return false;
	}

	Actor* currentActor = g_encounterGameState->GetCurrentActor();
	g_encounterGameState->InvokeCastFireAction( currentActor, targetPos );
	return true;
}

bool OnNetWait( NetMessage& message, NetConnection& connection )
{
	UNUSED( message );
	UNUSED( connection );

	Actor* currentActor = g_encounterGameState->GetCurrentActor();

	g_encounterGameState->InvokeWaitAction( currentActor );
	return true;
}
