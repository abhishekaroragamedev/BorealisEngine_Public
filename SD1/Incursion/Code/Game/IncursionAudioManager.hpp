#pragma once

#include <string>
#include <vector>

constexpr float AUDIO_MUSIC_VOLUME = 0.4f;

#pragma region AudioFileNames

constexpr char AUDIO_EXPLOSION_1[] = "Data/Audio/Explosion1.wav";
constexpr char AUDIO_EXPLOSION_2[] = "Data/Audio/Explosion2.wav";
constexpr char AUDIO_EXPLOSION_3[] = "Data/Audio/Explosion3.wav";
constexpr char AUDIO_SOFT_FIRE_1[] = "Data/Audio/TankFireSmallBullet1.wav";
constexpr char AUDIO_SOFT_FIRE_2[] = "Data/Audio/TankFireSmallBullet2.wav";
constexpr char AUDIO_SOFT_FIRE_3[] = "Data/Audio/TankFireSmallBullet3.wav";
constexpr char AUDIO_SOFT_FIRE_4[] = "Data/Audio/TankFireSmallBullet4.wav";
constexpr char AUDIO_HARD_FIRE_1[] = "Data/Audio/TankFireLargeBullet1.wav";
constexpr char AUDIO_MENU_LOAD[] = "Data/Audio/Anticipation.mp3";
constexpr char AUDIO_PAUSE[] = "Data/Audio/Pause.mp3";
constexpr char AUDIO_UNPAUSE[] = "Data/Audio/Unpause.mp3";
constexpr char AUDIO_START_GAME[] = "Data/Audio/StartGame.mp3";
constexpr char AUDIO_QUIT_GAME[] = "Data/Audio/QuitGame.mp3";
constexpr char AUDIO_EXIT_MAP[] = "Data/Audio/MapExited.mp3";
constexpr char AUDIO_VICTORY[] = "Data/Audio/Victory.mp3";
constexpr char AUDIO_GAME_OVER[] = "Data/Audio/GameOver.mp3";
constexpr char AUDIO_MUSIC_ATTRACT[] = "Data/Audio/AttractMusic.mp3";
constexpr char AUDIO_MUSIC_LEVEL_1[] = "Data/Audio/Level1Music.mp3";
constexpr char AUDIO_MUSIC_LEVEL_2[] = "Data/Audio/Level2Music.mp3";
constexpr char AUDIO_MUSIC_LEVEL_3[] = "Data/Audio/Level3Music.mp3";
constexpr char AUDIO_MUSIC_DAMAGE[] = "Data/Audio/DamageMusic.mp3";


#pragma endregion

enum IncursionAudioType
{
	EXPLOSION,
	SOFT_FIRE,
	HARD_FIRE,
	MENU_LOAD,
	PAUSE,
	UNPAUSE,
	START_GAME,
	QUIT_GAME,
	EXIT_MAP,
	VICTORY,
	GAME_OVER,
	NUM_AUDIO_TYPES
};

size_t PlayRandomSoundFor( IncursionAudioType audioType, float volume = 1.0f );		// For actions which have multiple different sounds

std::vector<std::string> GetAllAudioFilenames();