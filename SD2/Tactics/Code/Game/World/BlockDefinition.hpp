#pragma once
#include "Game/World/BlockDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <map>
#include <string>

class BlockDefinition
{

public:
	BlockDefinition( std::string name, Texture* texture, const AABB2& uvTop = AABB2::ZERO_TO_ONE, const AABB2& uvSide = AABB2::ZERO_TO_ONE, const AABB2& uvBottom = AABB2::ZERO_TO_ONE );
	~BlockDefinition(); 

	AABB2 GetTopUVs() const;
	AABB2 GetSideUVs() const;
	AABB2 GetBottomUVs() const;

public:
	static std::map< std::string, BlockDefinition* > s_definitions;
	static BlockDefinition* GetDefinition( const std::string& name );

private:
	std::string m_name = "";
	Texture* m_texture = nullptr;
	AABB2 m_uvTop;
	AABB2 m_uvSide;
	AABB2 m_uvBottom;

};
