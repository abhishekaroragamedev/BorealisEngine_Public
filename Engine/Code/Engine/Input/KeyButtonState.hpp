#pragma once

struct KeyButtonState
{
	KeyButtonState();

	bool m_isDown = false;
	bool m_wasJustPressed = false;
	bool m_wasJustReleased = false;
};
