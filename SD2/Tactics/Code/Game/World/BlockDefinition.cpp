#include "Game/World/BlockDefinition.hpp"

std::map< std::string, BlockDefinition* > BlockDefinition::s_definitions;

BlockDefinition* BlockDefinition::GetDefinition( const std::string& name )
{
	return BlockDefinition::s_definitions[ name ];
}

BlockDefinition::BlockDefinition( std::string name, Texture* texture, const AABB2& uvTop /* = AABB2::ZERO_TO_ONE */, const AABB2& uvSide /* = AABB2::ZERO_TO_ONE */, const AABB2& uvBottom /* = AABB2::ZERO_TO_ONE */ )
	:	m_name( name ),
		m_texture( texture ),
		m_uvTop( uvTop ),
		m_uvSide( uvSide ),
		m_uvBottom( uvBottom )
{
	BlockDefinition::s_definitions[ name ] = this;
}

BlockDefinition::~BlockDefinition()
{

}

AABB2 BlockDefinition::GetTopUVs() const
{
	return m_uvTop;
}

AABB2 BlockDefinition::GetSideUVs() const
{
	return m_uvSide;
}

AABB2 BlockDefinition::GetBottomUVs() const
{
	return m_uvBottom;
}
