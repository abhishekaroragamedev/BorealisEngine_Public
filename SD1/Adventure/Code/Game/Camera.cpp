#include "Game/Camera.hpp"
#include "Game/GameCommon.hpp"

Camera::Camera( float numTilesInViewVertically, const AABB2& mapBounds )
		:	m_numTilesInViewVertically( numTilesInViewVertically ),
			m_mapBounds( mapBounds )
{
	float viewportHalfHeight = ( m_numTilesInViewVertically * TILE_SIDE_LENGTH_WORLD_UNITS ) / 2.0f;
	float viewportHalfWidth = viewportHalfHeight * CLIENT_ASPECT;
	m_viewport = AABB2 (	Vector2( ( mapBounds.GetCenter().x - viewportHalfWidth ), ( mapBounds.GetCenter().y - viewportHalfHeight ) ),
							Vector2( ( mapBounds.GetCenter().x + viewportHalfWidth ), ( mapBounds.GetCenter().y + viewportHalfHeight ) )
						);
}

Camera::~Camera()
{

}

AABB2 Camera::GetViewportBounds() const
{
	return m_viewport;
}

void Camera::SetMapBounds( const AABB2& mapBounds )
{
	m_mapBounds = AABB2( mapBounds );
	
	float viewportHalfHeight = ( m_numTilesInViewVertically * TILE_SIDE_LENGTH_WORLD_UNITS ) / 2.0f;
	float viewportHalfWidth = viewportHalfHeight * CLIENT_ASPECT;
	m_viewport = AABB2 (	Vector2( ( mapBounds.GetCenter().x - viewportHalfWidth ), ( mapBounds.GetCenter().y - viewportHalfHeight ) ),
							Vector2( ( mapBounds.GetCenter().x + viewportHalfWidth ), ( mapBounds.GetCenter().y + viewportHalfHeight ) )
				);
}

void Camera::StopTracking()
{
	m_isTrackingEntity = false;
	m_entityToTrack = nullptr;
}

void Camera::TrackEntity( Entity* entityToTrack )
{
	m_isTrackingEntity = true;
	m_entityToTrack = entityToTrack;
}

void Camera::Update()
{
	UpdateViewport();
	CorrectViewportBoundsToFitInMap();
}

void Camera::UpdateViewport()
{
	if ( m_isTrackingEntity && m_entityToTrack != nullptr )
	{
		Vector2 cameraCenter = m_entityToTrack->GetLocation();

		float viewportHalfHeight = ( m_numTilesInViewVertically * TILE_SIDE_LENGTH_WORLD_UNITS ) / 2.0f;
		float viewportHalfWidth = viewportHalfHeight * CLIENT_ASPECT;

		m_viewport.mins = Vector2( ( cameraCenter.x - viewportHalfWidth ), ( cameraCenter.y - viewportHalfHeight ) );
		m_viewport.maxs = Vector2( ( cameraCenter.x + viewportHalfWidth ), ( cameraCenter.y + viewportHalfHeight ) );
	}
}

void Camera::CorrectViewportBoundsToFitInMap()
{
	Vector2 netTranslation = Vector2::ZERO;

	if ( m_viewport.mins.x < m_mapBounds.mins.x )
	{
		netTranslation.x = m_mapBounds.mins.x - m_viewport.mins.x;
	}
	else if ( m_viewport.maxs.x > m_mapBounds.maxs.x )
	{
		netTranslation.x = m_mapBounds.maxs.x - m_viewport.maxs.x;
	}

	if ( m_viewport.mins.y < m_mapBounds.mins.y )
	{
		netTranslation.y =  m_mapBounds.mins.y - m_viewport.mins.y;
	}
	else if ( m_viewport.maxs.y > m_mapBounds.maxs.y )
	{
		netTranslation.y = m_mapBounds.maxs.y - m_viewport.maxs.y;
	}

	m_viewport.Translate( netTranslation );
}

void Camera::SetRendererOrtho() const
{
	g_renderer->SetOrtho( m_viewport.mins, m_viewport.maxs );
}
