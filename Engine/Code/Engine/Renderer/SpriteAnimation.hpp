#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <vector>

class SpriteAnimation
{
	
public:
	explicit SpriteAnimation( SpriteSheet& spriteSheet, float durationSeconds, SpriteAnimMode playbackMode, int startSpriteIndex, int endSpriteIndex );
	explicit SpriteAnimation( SpriteSheet& spriteSheet, float durationSeconds, SpriteAnimMode playbackMode, std::vector< int > spriteIndices );
	explicit SpriteAnimation( const SpriteAnimationDefinition* spriteAnimDefinition );
	~SpriteAnimation();

	void Update( float deltaSeconds );
	AABB2 GetCurrentTexCoords() const;
	Sprite* GetCurrentSprite() const;
	const SpriteAnimationDefinition* GetDefinition() const;
	void Pause();
	void PlayOrResume();
	void Reset();
	bool IsFinished() const;
	bool IsPlaying() const;
	float GetSecondsElapsed() const;
	float GetSecondsRemaining() const;
	float GetFractionElapsed() const;
	float GetFractionRemaining() const;
	int GetCurrentIndex() const;
	void SetSecondsElapsed( float secondsElapsed );
	void SetFractionElapsed( float fractionElapsed );;

private:
	float GetCurrentSpriteMaxDuration() const;

private:
	bool m_isFinished = false;
	bool m_isPlaying = false;
	bool m_wasInitializedFromDefinition = false;
	float m_elapsedSeconds = 0.0f;
	int m_playDirection = 0;
	int m_currentSpriteIndexRelative = 0;
	const SpriteAnimationDefinition* m_spriteAnimDefinition = nullptr;

};
