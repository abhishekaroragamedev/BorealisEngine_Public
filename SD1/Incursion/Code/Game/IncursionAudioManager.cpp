#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/IncursionAudioManager.hpp"

SoundPlaybackID PlayRandomSoundFor( IncursionAudioType audioType, float volume )
{
	std::string audioFileToPlay = "";

	switch( audioType )
	{
		case IncursionAudioType::EXPLOSION:
		{
			int randomInt = GetRandomIntInRange( 0, 2 );
			switch (randomInt)
			{
				case 0:
				{
					audioFileToPlay = AUDIO_EXPLOSION_1;
					break;
				}
				case 1:
				{
					audioFileToPlay = AUDIO_EXPLOSION_2;
					break;
				}
				case 2:
				{
					audioFileToPlay = AUDIO_EXPLOSION_3;
					break;
				}
				default:
				{
					audioFileToPlay = AUDIO_EXPLOSION_1;
					break;
				}
			}

			break;
		}
		case IncursionAudioType::SOFT_FIRE:
		{
			int randomInt = GetRandomIntInRange( 0, 3 );
			switch (randomInt)
			{
				case 0:
				{
					audioFileToPlay = AUDIO_SOFT_FIRE_1;
					break;
				}
				case 1:
				{
					audioFileToPlay = AUDIO_SOFT_FIRE_2;
					break;
				}
				case 2:
				{
					audioFileToPlay = AUDIO_SOFT_FIRE_3;
					break;
				}
				case 3:
				{
					audioFileToPlay = AUDIO_SOFT_FIRE_4;
					break;
				}
				default:
				{
					audioFileToPlay = AUDIO_SOFT_FIRE_1;
					break;
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}

	if ( audioFileToPlay.compare( "" ) != 0 )
	{
		return g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( audioFileToPlay ), false, volume );
	}
	else
	{
		return 0;
	}
}

std::vector<std::string> GetAllAudioFilenames()
{
	std::vector<std::string> fileNames;

	fileNames.push_back( AUDIO_EXPLOSION_1 );
	fileNames.push_back( AUDIO_EXPLOSION_2 );
	fileNames.push_back( AUDIO_EXPLOSION_3 );
	fileNames.push_back( AUDIO_SOFT_FIRE_1 );
	fileNames.push_back( AUDIO_SOFT_FIRE_2 );
	fileNames.push_back( AUDIO_SOFT_FIRE_3 );
	fileNames.push_back( AUDIO_SOFT_FIRE_4 );
	fileNames.push_back( AUDIO_HARD_FIRE_1 );
	fileNames.push_back( AUDIO_MENU_LOAD );
	fileNames.push_back( AUDIO_PAUSE );
	fileNames.push_back( AUDIO_UNPAUSE );
	fileNames.push_back( AUDIO_START_GAME );
	fileNames.push_back( AUDIO_QUIT_GAME );
	fileNames.push_back( AUDIO_EXIT_MAP );
	fileNames.push_back( AUDIO_VICTORY );
	fileNames.push_back( AUDIO_GAME_OVER );
	fileNames.push_back( AUDIO_MUSIC_ATTRACT );
	fileNames.push_back( AUDIO_MUSIC_LEVEL_1 );
	fileNames.push_back( AUDIO_MUSIC_LEVEL_2 );
	fileNames.push_back( AUDIO_MUSIC_LEVEL_3 );
	fileNames.push_back( AUDIO_MUSIC_DAMAGE );

	return fileNames;
}
