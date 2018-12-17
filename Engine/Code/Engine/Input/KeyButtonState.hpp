#pragma once

struct KeyButtonState
{
	KeyButtonState();

	bool m_isDown;
	bool m_wasJustPressed;
	bool m_wasJustReleased;
};
