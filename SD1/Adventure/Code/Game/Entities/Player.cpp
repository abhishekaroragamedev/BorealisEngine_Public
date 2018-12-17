#include "Game/GameCommon.hpp"
#include "Game/Entities/Player.hpp"
#include "Engine/Input/XboxController.hpp"

Player::Player( const std::string instanceName, const Vector2& position, ActorDefinition* actorDefinition, const Map& map )
	:	Actor( instanceName, position, actorDefinition, map )
{

}

Player::~Player()
{

}

bool Player::IsInvincible() const
{
	return m_isInvincible;
}

void Player::SetMap( const Map* newMap )
{
	m_map = const_cast< Map* >( newMap );
}

void Player::Damage( int damageStats[ StatID::STAT_STRENGTH ] )
{
	if ( !m_isInvincible )
	{
		Actor::Damage( damageStats );
	}
}

void Player::Respawn()
{
	m_stats[ StatID::STAT_HEALTH ] = GetBaseStat( StatID::STAT_HEALTH );
}

void Player::HandleActorDeath()
{
	
}

void Player::PerformAction()
{
	HandleXboxControllerInput();
	HandleKeyboardInput();
}

void Player::HandleKeyboardInput()
{
	Vector2 netVelocity = Vector2::ZERO;
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_W ) )
	{
		netVelocity += Vector2::UP;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_S ) )
	{
		netVelocity += Vector2::DOWN;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_A ) )
	{
		netVelocity += Vector2::LEFT;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_D ) )
	{
		netVelocity += Vector2::RIGHT;
	}
	if ( netVelocity != Vector2::ZERO )
	{
		m_facingDirection = netVelocity;
		if ( !m_isBeingKnocked )
		{
			m_velocity = netVelocity;
			m_speed = static_cast< float >( GetStat( StatID::STAT_AGILITY ) );
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_SPACE ) && CanAttack() )
	{
		Attack( EquipSlot::EQUIP_SLOT_WEAPON );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_X ) && CanAttack() )
	{
		Attack( EquipSlot::EQUIP_SLOT_SPELL );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_E ) )
	{
		TryAddItemAtCurrentLocationToInventory();
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_G ) )
	{
		m_isInvincible = !m_isInvincible;
	}
}

void Player::HandleXboxControllerInput()
{
	XboxController controllerOne = g_inputSystem->GetController( 0 );
	if ( controllerOne.IsConnected() )
	{
		AnalogJoystick leftJoystick = controllerOne.GetJoystick( 0 );
		if ( leftJoystick.GetMagnitude() > 0.0f )
		{
			m_facingDirection = leftJoystick.GetPosition();
			if ( !m_isBeingKnocked )
			{
				m_velocity = leftJoystick.GetPosition();
				m_speed = static_cast< float >( GetStat( StatID::STAT_AGILITY ) );
			}
		}

		if ( controllerOne.WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) && CanAttack() )
		{
			Attack( EquipSlot::EQUIP_SLOT_WEAPON );
		}
		if ( controllerOne.WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_X ) && CanAttack() )
		{
			Attack( EquipSlot::EQUIP_SLOT_SPELL );
		}
		if ( controllerOne.WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_B ) )
		{
			TryAddItemAtCurrentLocationToInventory();
		}
	}
}
