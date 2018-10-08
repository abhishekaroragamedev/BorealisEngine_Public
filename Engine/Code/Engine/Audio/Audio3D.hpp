#pragma once

#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Vector3.hpp"

typedef size_t SoundID;			// Forward-declared duplicate of typedef in AudioSystem.hpp
typedef size_t SoundPlaybackID;	// Forward-declared duplicate of typedef in AudioSystem.hpp

class AudioListener3D
{
public:
	AudioListener3D() {}
	~AudioListener3D() {}

	Vector3 GetListeningDirection();
	Vector3 GetPosition();

public:
	Transform m_transform;	// Listening direction is the Forward basis
	float m_backwardListeningMagnitude = 0.2f;
};

class AudioSource3D
{
public:
	AudioSource3D() {}
	AudioSource3D( float volume, Vector3 attenuation = Vector3::ZERO )
		:	m_volume( volume ),
			m_attenuation( attenuation )
	{

	}
	~AudioSource3D() {}

	void UpdateCurrentSoundForListener( AudioListener3D& listener, float masterVolume = 1.0f );								// Assumes the sound is already playing with m_currentPlaybackID
	void UpdateCurrentSoundForListeners( AudioListener3D& lListener, AudioListener3D& rListener, float masterVolume = 1.0f );	// Assumes the sound is already playing with m_currentPlaybackID; Does L-R balancing

	float GetVolumeAtListener( AudioListener3D& listener );
	float GetAttenuationFactorForPosition( const Vector3& position );

public:
	float m_volume = 1.0f;
	Transform m_transform;
	Vector3 m_attenuation = Vector3::ZERO;
	SoundID m_currentSound = 0;
	SoundPlaybackID m_currentPlaybackID = 0;
};
