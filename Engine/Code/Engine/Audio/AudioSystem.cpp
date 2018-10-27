#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"

AudioSystem* g_audioSystemInstance = nullptr;

/* static */
AudioSystem* AudioSystem::GetInstance()
{
#if defined( ENGINE_DISABLE_AUDIO )
	ERROR_RECOVERABLE( "ERROR: AudioSystem::GetInstance(): Audio System disabled. This will return nullptr." );
#endif
	return g_audioSystemInstance;
}

//-----------------------------------------------------------------------------------------------
// To disable audio entirely (and remove requirement for fmod.dll / fmod64.dll) for any game,
//	#define ENGINE_DISABLE_AUDIO in your game's Code/Game/EngineBuildPreferences.hpp file.
//
// Note that this #include is an exception to the rule "engine code doesn't know about game code".
//	Purpose: Each game can now direct the engine via #defines to build differently for that game.
//	Downside: ALL games must now have this Code/Game/EngineBuildPreferences.hpp file.
//
#include "Game/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_AUDIO )


//-----------------------------------------------------------------------------------------------
// Link in the appropriate FMOD static library (32-bit or 64-bit)
//
#if defined( _WIN64 )
#pragma comment( lib, "ThirdParty/fmod/fmod64_vc.lib" )
#else
#pragma comment( lib, "ThirdParty/fmod/fmod_vc.lib" )
#endif


//-----------------------------------------------------------------------------------------------
// Initialization code based on example from "FMOD Studio Programmers API for Windows"
//
AudioSystem::AudioSystem()
	: m_fmodSystem( nullptr )
{
	FMOD_RESULT result;
	result = FMOD::System_Create( &m_fmodSystem );
	ValidateResult( result );

	result = m_fmodSystem->init( 512, FMOD_INIT_NORMAL, nullptr );
	ValidateResult( result );

	g_audioSystemInstance = this;

	InitFromXML();
}


//-----------------------------------------------------------------------------------------------
AudioSystem::~AudioSystem()
{
	FMOD_RESULT result = m_fmodSystem->release();
	ValidateResult( result );

	m_fmodSystem = nullptr; // #Fixme: do we delete/free the object also, or just do this?
}

void AudioSystem::InitFromXML()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLError loadSuccess = xmlDoc.LoadFile( AUDIO_XML_FILEPATH );
	if ( loadSuccess == tinyxml2::XMLError::XML_SUCCESS )
	{
		tinyxml2::XMLElement* rootElement = xmlDoc.FirstChildElement();

		const tinyxml2::XMLElement* settingsElement = rootElement->FirstChildElement();
		float dopplerScale = ParseXmlAttribute( *settingsElement, "dopplerScale", 1.0f );
		float distanceFactor = ParseXmlAttribute( *settingsElement, "distanceFactor", 1.0f );
		float rollOffScale = ParseXmlAttribute( *settingsElement, "rollOffScale", 1.0f );
		m_fmodSystem->set3DSettings( dopplerScale, distanceFactor, rollOffScale );

		// Load lone clips
		const tinyxml2::XMLElement* clipsElement = settingsElement->NextSiblingElement();
		for ( const tinyxml2::XMLElement* clipElement = clipsElement->FirstChildElement(); clipElement != nullptr; clipElement = clipElement->NextSiblingElement() )
		{
			std::string filePath = ParseXmlAttribute( *clipElement, "filePath", "" );
			bool spatial = ParseXmlAttribute( *clipElement, "spatial", false );
			CreateOrGetSound( filePath, spatial );
		}

		// Load groups
		const tinyxml2::XMLElement* groupsElement = clipsElement->NextSiblingElement();
		for ( const tinyxml2::XMLElement* groupElement = groupsElement->FirstChildElement(); groupElement != nullptr; groupElement = groupElement->NextSiblingElement() )
		{
			std::string groupName = ParseXmlAttribute( *groupElement, "name", "" );
			for ( const tinyxml2::XMLElement* clipElement = groupElement->FirstChildElement(); clipElement != nullptr; clipElement = clipElement->NextSiblingElement() )
			{
				std::string filePath = ParseXmlAttribute( *clipElement, "filePath", "" );
				bool spatial = ParseXmlAttribute( *clipElement, "spatial", false );
				RegisterSoundWithGroup( filePath, groupName, spatial );
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
void AudioSystem::BeginFrame()
{
	m_fmodSystem->update();
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound( const std::string& soundFilePath, bool spatial /* = false */ )
{
	std::map< std::string, SoundID >::iterator found = m_registeredSoundIDs.find( soundFilePath );
	if( found != m_registeredSoundIDs.end() )
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		FMOD_MODE mode = ( spatial )? FMOD_3D : FMOD_DEFAULT;
		m_fmodSystem->createSound( soundFilePath.c_str(), mode, nullptr, &newSound );
		if( newSound )
		{
			SoundID newSoundID = m_registeredSounds.size();
			m_registeredSoundIDs[ soundFilePath ] = newSoundID;
			m_registeredSounds.push_back( newSound );
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::PlaySound( SoundID soundID, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	size_t numSounds = m_registeredSounds.size();
	if( soundID < 0 || soundID >= numSounds )
		return MISSING_SOUND_ID;

	FMOD::Sound* sound = m_registeredSounds[ soundID ];
	if( !sound )
		return MISSING_SOUND_ID;

	FMOD::Channel* channelAssignedToSound = nullptr;
	m_fmodSystem->playSound( sound, nullptr, isPaused, &channelAssignedToSound );
	if( channelAssignedToSound )
	{
		int loopCount = isLooped ? -1 : 0;
		unsigned int playbackMode = isLooped ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		float frequency;
		channelAssignedToSound->setMode(playbackMode);
		channelAssignedToSound->getFrequency( &frequency );
		channelAssignedToSound->setFrequency( frequency * speed );
		channelAssignedToSound->setVolume( volume );
		channelAssignedToSound->setPan( balance );
		channelAssignedToSound->setLoopCount( loopCount );
	}

	return (SoundPlaybackID) channelAssignedToSound;
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::StopSound( SoundPlaybackID soundPlaybackID )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set volume on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->stop();
}

//-----------------------------------------------------------------------------------------------
// Volume is in [0,1]
//
void AudioSystem::SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set volume on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setVolume( volume );
}


//-----------------------------------------------------------------------------------------------
// Balance is in [-1,1], where 0 is L/R centered
//
void AudioSystem::SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set balance on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setPan( balance );
}


//-----------------------------------------------------------------------------------------------
// Speed is frequency multiplier (1.0 == normal)
//	A speed of 2.0 gives 2x frequency, i.e. exactly one octave higher
//	A speed of 0.5 gives 1/2 frequency, i.e. exactly one octave lower
//
void AudioSystem::SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set speed on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	float frequency;
	FMOD::Sound* currentSound = nullptr;
	channelAssignedToSound->getCurrentSound( &currentSound );
	if( !currentSound )
		return;

	int ignored = 0;
	currentSound->getDefaults( &frequency, &ignored );
	channelAssignedToSound->setFrequency( frequency * speed );
}

//-----------------------------------------------------------------------------------------------
void AudioSystem::SetSoundPlayback3DAttributes( SoundPlaybackID soundPlaybackID, const Vector3& position, const Vector3& velocity )
{
	FMOD::Channel* channel = ( FMOD::Channel* ) soundPlaybackID;

	FMOD_VECTOR fmodPos;
	fmodPos.x = position.x;
	fmodPos.y = position.y;
	fmodPos.z = position.z;

	FMOD_VECTOR fmodVel;
	fmodVel.x = velocity.x;
	fmodVel.y = velocity.y;
	fmodVel.z = velocity.z;

	FMOD_RESULT result = channel->set3DAttributes( &fmodPos, &fmodVel );
	if ( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( "WARNING: error while setting 3D attributes of channel!" );
		return;
	}
}

//-----------------------------------------------------------------------------------------------
int AudioSystem::GetNumListeners3D() const
{
	int numListeners = -1;
	FMOD_RESULT result = m_fmodSystem->get3DNumListeners( &numListeners );
	if ( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( "WARNING: error while getting number of 3D listeners!" );
		return -1;
	}
	return numListeners;
}

//-----------------------------------------------------------------------------------------------
void AudioSystem::SetNumListeners3D( int numListeners )
{
	FMOD_RESULT result = m_fmodSystem->set3DNumListeners( numListeners );
	if ( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( "WARNING: error while setting number of 3D listeners!" );
		return;
	}
}

//-----------------------------------------------------------------------------------------------
void AudioSystem::SetListener3DAttributes( const Vector3& position, const Vector3& velocity, const Vector3& up, const Vector3& forward, int listenerID /* = 0 */ )
{
	FMOD_VECTOR fmodPos;
	fmodPos.x = position.x;
	fmodPos.y = position.y;
	fmodPos.z = position.z;

	FMOD_VECTOR fmodVel;
	fmodVel.x = velocity.x;
	fmodVel.y = velocity.y;
	fmodVel.z = velocity.z;

	FMOD_VECTOR fmodUp;
	fmodUp.x = up.x;
	fmodUp.y = up.y;
	fmodUp.z = up.z;

	FMOD_VECTOR fmodForward;
	fmodForward.x = forward.x;
	fmodForward.y = forward.y;
	fmodForward.z = forward.z;

	FMOD_RESULT result = m_fmodSystem->set3DListenerAttributes( listenerID, &fmodPos, &fmodVel, &fmodForward, &fmodUp );
	if ( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( "WARNING: error while setting 3D attributes of listener!" );
		return;
	}
}

//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::PlayOneOffSoundFromGroup( const std::string& groupName, bool isLooped/* =false */, float volume/* =1.f */, float balance/* =0.0f */, float speed/* =1.0f */, bool isPaused/* =false */ )
{
	if ( m_registeredSoundIDsByGroup.find( groupName ) == m_registeredSoundIDsByGroup.end() )
	{
		ERROR_RECOVERABLE( "ERROR: AudioSystem::PlayOneOffSoundFromGroup(): Group could not be found." );
		return MISSING_SOUND_ID;
	}
	if ( m_registeredSoundIDsByGroup.find( groupName )->second.size() == 0 )
	{
		ERROR_RECOVERABLE( "ERROR: AudioSystem::PlayOneOffSoundFromGroup(): No sounds in group." );
		return MISSING_SOUND_ID;
	}

	std::vector< SoundID >& group = m_registeredSoundIDsByGroup.find( groupName )->second;
	int randomIndex = GetRandomIntInRange( IntRange( 0, static_cast< int >( group.size() - 1 ) ) );
	SoundID soundToPlay = group[ randomIndex ];
	SoundPlaybackID playbackID = PlaySound( soundToPlay, isLooped, volume, balance, speed, isPaused );
	return playbackID;
}

//-----------------------------------------------------------------------------------------------
bool AudioSystem::RegisterSoundWithGroup( SoundID soundID, const std::string& groupName, bool spatial /* = false */ )
{
	if ( m_registeredSoundIDsByGroup.find( groupName ) != m_registeredSoundIDsByGroup.end() )
	{
		std::vector< SoundID >& group = m_registeredSoundIDsByGroup.find( groupName )->second;
		if ( std::find( group.begin(), group.end(), soundID ) != group.end() )
		{
			ERROR_RECOVERABLE( "ERROR: AudioSystem::RegisterSoundWithGroup(): Sound already exists within group." );
			return false;
		}
	}

	m_registeredSoundIDsByGroup[ groupName ].push_back( soundID );
	return true;
}

//-----------------------------------------------------------------------------------------------
bool AudioSystem::RegisterSoundWithGroup( const std::string& soundFilePath, const std::string& groupName, bool spatial /* = false */ )
{
	SoundID soundID = CreateOrGetSound( soundFilePath, spatial );
	return RegisterSoundWithGroup( soundID, groupName, spatial );
}

//-----------------------------------------------------------------------------------------------
bool AudioSystem::UnregisterSoundFromGroup( SoundID soundID, const std::string& groupName )
{
	if ( m_registeredSoundIDsByGroup.find( groupName ) != m_registeredSoundIDsByGroup.end() )
	{
		std::vector< SoundID >& group = m_registeredSoundIDsByGroup.find( groupName )->second;
		std::vector< SoundID >::iterator soundIterator = std::find( group.begin(), group.end(), soundID );
		if ( soundIterator != group.end() )
		{
			group.erase( soundIterator );
			return true;
		}
		else
		{
			ERROR_RECOVERABLE( "ERROR: AudioSystem::UnregisterSoundFromGroup(): Sound could not be found in group." );
		}
	}
	else
	{
		ERROR_RECOVERABLE( "ERROR: AudioSystem::UnregisterSoundFromGroup(): Group could not be found." );
	}

	return false;
}

//-----------------------------------------------------------------------------------------------
bool AudioSystem::UnregisterSoundFromGroup( const std::string& soundFilePath, const std::string& groupName )
{
	SoundID soundID = CreateOrGetSound( soundFilePath );
	return UnregisterSoundFromGroup( soundID, groupName );
}

//-----------------------------------------------------------------------------------------------
void AudioSystem::ValidateResult( FMOD_RESULT result )
{
	if( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( Stringf( "Engine/Audio SYSTEM ERROR: Got error result code %i - error codes listed in fmod_common.h\n", (int) result ) );
	}
}


#endif // !defined( ENGINE_DISABLE_AUDIO )
