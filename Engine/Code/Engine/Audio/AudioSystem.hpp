#pragma once


//-----------------------------------------------------------------------------------------------
#include "Engine/Math/Vector3.hpp"
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs


//-----------------------------------------------------------------------------------------------
class AudioSystem;

constexpr char AUDIO_XML_FILEPATH[] = "Data/Audio/Audio.xml";

/////////////////////////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
	friend class OVRHeadset; // For Oculus Rift Audio with FMOD
public:
	AudioSystem();
	virtual ~AudioSystem();

public:
	virtual void				BeginFrame();
	virtual void				EndFrame();

	virtual SoundID				CreateOrGetSound( const std::string& soundFilePath, bool spatial = false );
	virtual SoundPlaybackID		PlaySound( SoundID soundID, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false );
	virtual void				StopSound( SoundPlaybackID soundPlaybackID );
	virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)
	virtual void				SetSoundPlayback3DAttributes( SoundPlaybackID soundPlaybackID, const Vector3& position, const Vector3& velocity );
	virtual void				SetListener3DAttributes( const Vector3& position, const Vector3& velocity, const Vector3& up, const Vector3& forward, int listenerID = 0 );
	virtual int					GetNumListeners3D() const;
	virtual void				SetNumListeners3D( int numListeners );

	virtual SoundPlaybackID		PlayOneOffSoundFromGroup( const std::string& groupName, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false );
	virtual bool				RegisterSoundWithGroup( SoundID soundID, const std::string& groupName, bool spatial = false );
	virtual bool				RegisterSoundWithGroup( const std::string& soundFilePath, const std::string& groupName, bool spatial = false );
	virtual bool				UnregisterSoundFromGroup( SoundID soundID, const std::string& groupName );
	virtual bool				UnregisterSoundFromGroup( const std::string& soundFilePath, const std::string& groupName );

	virtual void				ValidateResult( FMOD_RESULT result );

private:
	void InitFromXML();

public:
	static AudioSystem* GetInstance();

protected:
	FMOD::System*										m_fmodSystem;
	std::map< std::string, SoundID >					m_registeredSoundIDs;
	std::vector< FMOD::Sound* >							m_registeredSounds;
	std::map< std::string, std::vector< SoundID > >		m_registeredSoundIDsByGroup;
};

