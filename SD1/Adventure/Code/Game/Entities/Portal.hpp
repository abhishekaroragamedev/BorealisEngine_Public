#pragma once

#include "Game/Entities/PortalDefinition.hpp"
#include "Game/Entities/Entity.hpp"
#include "Engine/Math/Vector2.hpp"
#include <string>

class Portal : public Entity
{

	friend class Map;

public:
	~Portal();

	void SetReciprocal( Portal* reciprocalPortal );
	Portal* GetReciprocal() const;
	PortalDefinition* GetPortalDefinition() const;

	//void Update( float deltaSeconds ) override;		// Override if really needed
	//void Render() const override;

protected:
	explicit Portal( const std::string instanceName, const Vector2& position, PortalDefinition* portalDefinition, const Map& map );

private:
	PortalDefinition* m_portalDefinition = nullptr;
	Portal* m_reciprocalPortal = nullptr;

};
