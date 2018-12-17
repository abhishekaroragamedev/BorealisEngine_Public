#include "Game/World/Block.hpp"

Block::Block( IntVector3 mapCoordinates, BlockDefinition* definition /* = nullptr */ )
	:	m_mapCoordinates( mapCoordinates ),
		m_definition( definition )
{

}

Block::~Block()
{

}

BlockDefinition* Block::GetType() const
{
	return m_definition;
}

void Block::SetType( BlockDefinition* newDefinition /* = nullptr */ )
{
	m_definition = newDefinition;
}

bool Block::IsAir() const
{
	return ( m_definition == nullptr );
}
