#pragma once

#include "Game/Entities/Entity.hpp"
#include "Engine/Math/AABB2.hpp"

class Camera
{

public:
	Camera( float numTilesInViewVertically, const AABB2& mapBounds );
	~Camera();

public:
	void Update();
	void StopTracking();
	void TrackEntity( Entity* entityToTrack );
	void SetRendererOrtho() const;
	void SetMapBounds( const AABB2& mapBounds );
	AABB2 GetViewportBounds() const;

private:
	void UpdateViewport();
	void CorrectViewportBoundsToFitInMap();

private:
	Entity* m_entityToTrack = nullptr;
	float m_numTilesInViewVertically = 0.0f;
	bool m_isTrackingEntity = false;
	AABB2 m_mapBounds;
	AABB2 m_viewport;

};
