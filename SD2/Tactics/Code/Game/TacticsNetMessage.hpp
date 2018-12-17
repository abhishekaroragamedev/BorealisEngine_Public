#pragma once

class NetConnection;
class NetMessage;

enum TacticsNetMessage : unsigned int
{
	NETMSG_GAME_JOIN_ACCEPT = 120U,
	NETMSG_MOVE = 121U,
	NETMSG_BOW = 122U,
	NETMSG_ATTACK = 123U,
	NETMSG_HEAL = 124U,
	NETMSG_DEFEND = 125U,
	NETMSG_FIRE = 126U,
	NETMSG_WAIT = 127U,
	NUM_TACTICS_NET_MSGS
};

bool OnGameJoinAccept( NetMessage& message, NetConnection& connection );
bool OnNetMove( NetMessage& message, NetConnection& connection );
bool OnNetAttack( NetMessage& message, NetConnection& connection );
bool OnNetBow( NetMessage& message, NetConnection& connection );
bool OnNetHeal( NetMessage& message, NetConnection& connection );
bool OnNetDefend( NetMessage& message, NetConnection& connection );
bool OnNetCastFire( NetMessage& message, NetConnection& connection );
bool OnNetWait( NetMessage& message, NetConnection& connection );
