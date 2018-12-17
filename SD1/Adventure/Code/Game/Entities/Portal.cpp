#include "Game/Entities/Portal.hpp"

Portal::Portal( const std::string instanceName, const Vector2& position, PortalDefinition* portalDefinition, const Map& map )
	:	Entity( instanceName, position, portalDefinition, map ),
		m_portalDefinition( portalDefinition )
{

}

Portal::~Portal()
{

}

void Portal::SetReciprocal( Portal* reciprocalPortal )
{
	m_reciprocalPortal = reciprocalPortal;
}

Portal* Portal::GetReciprocal() const
{
	return m_reciprocalPortal;
}

PortalDefinition* Portal::GetPortalDefinition() const
{
	return m_portalDefinition;
}
