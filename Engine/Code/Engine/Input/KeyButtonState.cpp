#include "Engine/Input/KeyButtonState.hpp"

KeyButtonState::KeyButtonState()
{
	m_isDown = false;
	m_wasJustPressed = false;
	m_wasJustReleased = false;
}
