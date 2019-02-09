#include "Engine/Audio/Audio3D.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

Vector3 AudioListener3D::GetListeningDirection()
{
	Matrix44 listenerMatrix = m_transform.GetAsMatrixWorld();
	Vector3 listeningDirection = listenerMatrix.GetKBasis();
	return listeningDirection;
}

Vector3 AudioListener3D::GetPosition()
{
	Matrix44 listenerMatrix = m_transform.GetAsMatrixWorld();
	Vector3 listenerPosition = listenerMatrix.GetTranslation();
	return listenerPosition;
}

void AudioSource3D::UpdateCurrentSoundForListener( AudioListener3D& listener, float masterVolume /*= 1.0f*/ )
{
	float volume = GetVolumeAtListener( listener );
	volume *= masterVolume;
	AudioSystem::GetInstance()->SetSoundPlaybackVolume( m_currentPlaybackID, volume );
}

void AudioSource3D::UpdateCurrentSoundForListeners( AudioListener3D& lListener, AudioListener3D& rListener, float masterVolume /*= 1.0f*/ )
{
	float lVolume = GetVolumeAtListener( lListener );
	float rVolume = GetVolumeAtListener( rListener );
	
	float balance = rVolume - lVolume;
	float maxPositiveBalance = 1.0f - Average( lListener.m_backwardListeningMagnitude, rListener.m_backwardListeningMagnitude );
	balance = ClampFloat( balance, -maxPositiveBalance, maxPositiveBalance );
	AudioSystem::GetInstance()->SetSoundPlaybackBalance( m_currentPlaybackID, balance );

	float volume = Average( lVolume, rVolume );
	volume *= masterVolume;
	AudioSystem::GetInstance()->SetSoundPlaybackVolume( m_currentPlaybackID, volume );
}

float AudioSource3D::GetVolumeAtListener( AudioListener3D& listener )
{
	float volume = m_volume;

	Matrix44 listenerMatrix = listener.m_transform.GetAsMatrixWorld();
	Vector3 listeningDirection = listener.GetListeningDirection();
	Vector3 listenerPosition = listener.GetPosition();

	float attenuationFactor = GetAttenuationFactorForPosition( listenerPosition );

	Vector3 sourcePosition = m_transform.GetWorldPosition();
	Vector3 sourceDirection = sourcePosition - listenerPosition;
	sourceDirection.NormalizeAndGetLength();
	float dotProduct = DotProduct( sourceDirection, listeningDirection );
	dotProduct = RangeMapFloat( dotProduct, -1.0f, 1.0f, listener.m_backwardListeningMagnitude, 1.0f );	// The sound should not be "negative" if the listener is oriented away from the source
	attenuationFactor *= dotProduct;

	volume *= attenuationFactor;
	return volume;
}

float AudioSource3D::GetAttenuationFactorForPosition( const Vector3& position )
{
	Vector3 displacement = position - m_transform.GetWorldPosition();
	float distance = displacement.GetLength();

	Vector3 attenuation = m_attenuation;
	if ( attenuation == Vector3::ZERO )
	{
		attenuation.x = 1.0f;
	}
	float attenuationFactor = attenuation.x + ( attenuation.y * distance ) + ( attenuation.z * distance * distance );
	attenuationFactor = 1.0f / attenuationFactor;
	return attenuationFactor;
}
