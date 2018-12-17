#pragma once
#include "Game/World/BlockDefinition.hpp"
#include "Engine/Math/IntVector3.hpp"

class Block
{

public:
	Block( IntVector3 mapCoordinates, BlockDefinition* definition = nullptr );
	~Block();

	BlockDefinition* GetType() const;
	void SetType( BlockDefinition* newDefinition = nullptr );
	bool IsAir() const;

public:
	BlockDefinition* m_definition = nullptr;
	IntVector3 m_mapCoordinates;

};
