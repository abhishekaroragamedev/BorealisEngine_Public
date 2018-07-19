#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector4.hpp"

struct FogUBO		/* std140 */
{

public:
	FogUBO()	{}
	FogUBO(	const Vector4& fogColor,
			float fogNearFactor,
			float fogFarFactor,
			float fogNearZ,
			float fogFarZ
	)
		:	m_fogColor( fogColor ),
			m_fogNearFactor( fogNearFactor ),
			m_fogFarFactor( fogFarFactor ),
			m_fogNearZ( fogNearZ ),
			m_fogFarZ( fogFarZ )
	{

	}

	FogUBO(	const Rgba& fogColor,
		float fogNearFactor,
		float fogFarFactor,
		float fogNearZ,
		float fogFarZ
	)
		:	m_fogColor( fogColor.GetAsFloats() ),
			m_fogNearFactor( fogNearFactor ),
			m_fogFarFactor( fogFarFactor ),
			m_fogNearZ( fogNearZ ),
			m_fogFarZ( fogFarZ )
	{

	}

public:
	Vector4 m_fogColor = Vector4::ZERO_DISPLACEMENT;
	float m_fogNearFactor = 0.0f;
	float m_fogFarFactor = 1.0f;
	float m_fogNearZ = 0.0f;
	float m_fogFarZ = 0.0f;

};
