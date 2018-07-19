#pragma once

#include "Engine/Math/MathUtils.hpp"
#include <float.h>

struct TimeUBO		/* std140 */
{

public:
	TimeUBO( float currentTime, float lastFrameTime, unsigned int frameCount )
		:	m_currentTime( currentTime ),
			m_lastFrameTime( lastFrameTime ),
			m_frameCount( frameCount )
	{
		if ( !IsFloatEqualTo( m_lastFrameTime, 0.0f ) )
		{
			m_FPS = 1.0f / m_lastFrameTime;
		}
		else
		{
			m_FPS = FLT_MAX;
		}
	}
	~TimeUBO() {}

public:
	float m_currentTime = 0.0f;
	float m_lastFrameTime = 0.0f;
	float m_FPS = 0.0f;
	int m_frameCount = 0;

};
