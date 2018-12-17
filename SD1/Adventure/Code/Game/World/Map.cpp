#include "Game/GameCommon.hpp"
#include "Game/World/Map.hpp"
#include "Game/World/MapGenStep.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"

Map::Map( const std::string& mapName, const MapDefinition* mapDefinition, const Adventure& adventure )
	:	m_adventure( *( const_cast< Adventure* >( &adventure ) ) )
{
	m_name = std::string( mapName );
	m_mapDefinition = const_cast< MapDefinition* >( mapDefinition );
	
	IntRange widthRange = m_mapDefinition->GetWidthRange();
	IntRange heightRange = m_mapDefinition->GetHeightRange();
	m_dimensions = IntVector2( GetRandomIntInRange( widthRange.min, widthRange.max ), GetRandomIntInRange( heightRange.min, heightRange.max ) );
	
	PopulateTilesFromMapDefinitionAndMapGenSteps();
	InitializeMapForLoad();
}

Map::Map( const std::string& mapName, const std::string& mapDefinitionName, const Adventure& adventure )
	:	m_adventure( *( const_cast< Adventure* >( &adventure ) ) )
{
	m_name = std::string( mapName );

	ASSERT_OR_DIE( MapDefinition::s_definitions.find( mapDefinitionName ) != MapDefinition::s_definitions.end(), "Map() constructor: invalid MapDefinition name found. Aborting..." );
	m_mapDefinition = MapDefinition::s_definitions[ mapDefinitionName ];

	IntRange widthRange = m_mapDefinition->GetWidthRange();
	IntRange heightRange = m_mapDefinition->GetHeightRange();
	m_dimensions = IntVector2( GetRandomIntInRange( widthRange.min, widthRange.max ), GetRandomIntInRange( heightRange.min, heightRange.max ) );

	PopulateTilesFromMapDefinitionAndMapGenSteps();
	InitializeMapForLoad();
}

Map::~Map()
{
	g_mainCamera->StopTracking();
	m_playerEntity = nullptr;
	DeleteEntities();
}

void Map::PopulateTilesFromMapDefinitionAndMapGenSteps()
{
	for ( int verticalTileCounter = 0; verticalTileCounter < m_dimensions.y; verticalTileCounter++ )
	{
		for ( int horizontalTileCounter = 0; horizontalTileCounter < m_dimensions.x; horizontalTileCounter++ )
		{
			m_tiles.push_back( Tile( *m_mapDefinition->GetDefaultTileDefinition(), IntVector2( horizontalTileCounter, verticalTileCounter ) ) );
		}
	}

	for ( const MapGenStep* mapGenStep : m_mapDefinition->GetMapGenSteps() )
	{
		mapGenStep->Run( *this );
	}
}

void Map::AddExistingPlayer( const Player* playerEntity )
{
	m_playerEntity = const_cast< Player* >( playerEntity );
	m_playerEntity->SetPosition( GetRandomUnpopulatedTileOfType( *m_mapDefinition->GetDefaultTileDefinition() ).GetTileWorldBounds().GetCenter() );
	m_actors.push_back( m_playerEntity );
	m_allEntitiesInDrawOrder.push_back( m_playerEntity );
}

void Map::AddExistingItem( Item* itemEntity )
{
	itemEntity->SetPosition( GetRandomUnpopulatedTileOfType( *m_mapDefinition->GetDefaultTileDefinition() ).GetTileWorldBounds().GetCenter() );
	m_items.push_back( itemEntity );
	m_allEntitiesInDrawOrder.push_back( itemEntity );
}

Player* Map::SpawnPlayer( const TileDefinition& onTileType )
{
	m_playerEntity = new Player( "Player", GetRandomUnpopulatedTileOfType( onTileType ).GetTileWorldBounds().GetCenter(), ActorDefinition::s_definitions[ "Player" ], *this );
	m_actors.push_back( m_playerEntity );
	m_allEntitiesInDrawOrder.push_back( m_playerEntity );
	g_mainCamera->TrackEntity( m_playerEntity );
	return m_playerEntity;
}

Actor* Map::SpawnActor( const ActorDefinition* actorDefinition, const TileDefinition& onTileType )
{
	Actor* newActor = new Actor( actorDefinition->GetName(), GetRandomUnpopulatedTileOfType( onTileType ).GetTileWorldBounds().GetCenter(), const_cast< ActorDefinition* >( actorDefinition ), *this );
	m_actors.push_back( newActor );
	m_allEntitiesInDrawOrder.push_back( newActor );
	return newActor;
}

Item* Map::SpawnItem( const ItemDefinition* itemDefinition, const TileDefinition& onTileType )
{
	Item* newItem = new Item( itemDefinition->GetName(), GetRandomUnpopulatedTileOfType( onTileType ).GetTileWorldBounds().GetCenter(), const_cast< ItemDefinition* >( itemDefinition ), *this );
	m_items.push_back( newItem );
	m_allEntitiesInDrawOrder.push_back( newItem );
	return newItem;
}

Item* Map::SpawnItem( const ItemDefinition* itemDefinition, const Vector2& spawnPosition )
{
	Item* newItem = new Item( itemDefinition->GetName(), spawnPosition, const_cast< ItemDefinition* >( itemDefinition ), *this );
	m_items.push_back( newItem );
	m_allEntitiesInDrawOrder.push_back( newItem );
	return newItem;
}

void Map::RemoveItemFromMap( Item* itemToRemove )
{
	std::vector< Item* >::iterator itemIterator = std::find( m_items.begin(), m_items.end(), itemToRemove );
	std::vector< Entity* >::iterator entityIterator = std::find( m_allEntitiesInDrawOrder.begin(), m_allEntitiesInDrawOrder.end(), static_cast< Entity* >( itemToRemove ) );
	if ( itemIterator != m_items.end() )
	{
		m_items.erase( itemIterator );
	}
}

void Map::DeleteItemPermanently( Item* itemToDelete )
{
	std::vector< Item* >::iterator itemIterator = std::find( m_items.begin(), m_items.end(), itemToDelete );
	std::vector< Entity* >::iterator entityIterator = std::find( m_allEntitiesInDrawOrder.begin(), m_allEntitiesInDrawOrder.end(), static_cast< Entity* >( itemToDelete ) );
	if ( itemIterator != m_items.end() )
	{
		delete itemToDelete;
		m_items.erase( itemIterator );
	}
	if ( entityIterator != m_allEntitiesInDrawOrder.end() )
	{
		m_allEntitiesInDrawOrder.erase( entityIterator );
	}
}

Portal* Map::SpawnPortal( const PortalDefinition* portalDefinition, const TileDefinition& onTileType )
{
	Portal* newPortal = new Portal( portalDefinition->GetName(), GetRandomUnpopulatedTileOfType( onTileType ).GetTileWorldBounds().GetCenter(), const_cast< PortalDefinition* >( portalDefinition ), *this );
	m_portals.push_back( newPortal );
	m_allEntitiesInDrawOrder.push_back( newPortal );
	return newPortal;
}

Projectile* Map::SpawnProjectile( const ProjectileDefinition* projectileDefinition, Actor* attacker, WeaponType sourceWeaponType, const Vector2& spawnPosition, const Vector2& spawnVelocity, int statMultipliers[ StatID::NUM_STATS ] )
{
	Projectile* newProjectile = new Projectile( projectileDefinition->GetName(), attacker, sourceWeaponType, spawnPosition, spawnVelocity, const_cast< ProjectileDefinition* >( projectileDefinition ), *this, statMultipliers );
	m_projectiles.push_back( newProjectile );
	m_allEntitiesInDrawOrder.push_back( newProjectile );
	return newProjectile;
}

void Map::InitializeMapForLoad()
{
	AABB2 mapBounds = AABB2( Vector2::ZERO, GetDimensionsInWorldCoordinates() );
	g_mainCamera->SetMapBounds( mapBounds );
}

void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
	HandleCollisions();
	BubbleSortEntityRenderVector();
}

void Map::HandleCollisions()
{
	if( !HasPlayerReachedAPortal()  )
	{
		HandleActorCollisions();
		HandleProjectileLifetimeAndCollisions();
	}
}

void Map::HandleActorCollisions()
{
	for ( std::vector< Actor* >::iterator outerActorIterator = m_actors.begin(); outerActorIterator != m_actors.end(); outerActorIterator++ )		// Collisions with other actors
	{
		if ( ( *outerActorIterator )->GetActorDefinition()->IgnoresActorPhysics() )
		{
			continue;
		}

		for ( std::vector< Actor* >::iterator innerActorIterator = m_actors.begin(); innerActorIterator != m_actors.end(); innerActorIterator++ )
		{
			if ( ( *innerActorIterator )->GetActorDefinition()->IgnoresActorPhysics() )
			{
				continue;
			}

			Actor* firstActor = *outerActorIterator;
			Actor* secondActor = *innerActorIterator;
			if ( ( firstActor != secondActor ) && DoDiscsOverlap( firstActor->GetLocation(), firstActor->GetEntityDefinition()->GetPhysicsRadius(), secondActor->GetLocation(), secondActor->GetEntityDefinition()->GetPhysicsRadius() ) )
			{
				Vector2 displacementUnitVector = secondActor->GetLocation() - firstActor->GetLocation();
				float displacementLength = displacementUnitVector.NormalizeAndGetLength();
				float requiredDisplacementLength = firstActor->GetEntityDefinition()->GetPhysicsRadius() + secondActor->GetEntityDefinition()->GetPhysicsRadius();
				
				float correctionLength = ( requiredDisplacementLength - displacementLength );
				float totalActorMass = firstActor->GetActorDefinition()->GetPhysicsMass() + secondActor->GetActorDefinition()->GetPhysicsMass();
				float firstActorCorrection = ( secondActor->GetActorDefinition()->GetPhysicsMass() / totalActorMass ) * correctionLength;
				float secondActorCorrection = ( firstActor->GetActorDefinition()->GetPhysicsMass() / totalActorMass ) * correctionLength;

				firstActor->ApplyCorrectiveTranslation( firstActorCorrection * ( -1.0f * displacementUnitVector ) );
				secondActor->ApplyCorrectiveTranslation( secondActorCorrection * displacementUnitVector );
			}
		}
	}

	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )		// Collisions with walls
	{
		Tile tileAtEntityPosition = GetTileAtWorldPosition( ( *actorIterator )->GetLocation() );
		IntVector2 tileCoordinates = tileAtEntityPosition.GetTileCoordinates();

		bool wasNorthHandled = HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_NORTH ) );
		bool wasEastHandled = HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_EAST ) );
		bool wasSouthHandled = HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_SOUTH ) );
		bool wasWestHandled = HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_WEST ) );

		if ( !wasNorthHandled && !wasEastHandled ) { HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_NORTHEAST ) ); }
		if ( !wasNorthHandled && !wasWestHandled ) { HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_NORTHWEST ) ); }
		if ( !wasSouthHandled && !wasEastHandled ) { HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_SOUTHEAST ) ); }
		if ( !wasSouthHandled && !wasWestHandled ) { HandleCollisionBetweenEntityAndTileAtCoordinates( **actorIterator, ( tileCoordinates + IntVector2::STEP_SOUTHWEST ) ); }
	}
}

void Map::HandleProjectileLifetimeAndCollisions()
{
	std::vector< Projectile* > projectilesToDelete;

	for ( std::vector< Projectile* >::iterator projectileIterator = m_projectiles.begin(); projectileIterator != m_projectiles.end(); projectileIterator++ )
	{
		if ( ( ( *projectileIterator )->GetSourceWeaponType() != WeaponType::WEAPON_TYPE_PROJECTILE ) && ( *projectileIterator )->ShouldBeDestroyed() )
		{
			projectilesToDelete.push_back( *projectileIterator );
			continue;
		}

		Tile tileAtEntityPosition = GetTileAtWorldPosition( ( *projectileIterator )->GetLocation() );
		IntVector2 tileCoordinates = tileAtEntityPosition.GetTileCoordinates();
		if	( ( ( *projectileIterator )->GetSourceWeaponType() == WeaponType::WEAPON_TYPE_PROJECTILE ) && (
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_NORTH ) )		||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_EAST ) )		||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_SOUTH ) )		||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_WEST ) )		||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_NORTHEAST ) )	||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_NORTHWEST ) )	||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_SOUTHEAST ) )	||
				HandleCollisionBetweenEntityAndTileAtCoordinates( **projectileIterator, ( tileCoordinates + IntVector2::STEP_SOUTHWEST ) )
			) )
		{
			projectilesToDelete.push_back( *projectileIterator );
			continue;
		}

		for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
		{
			if ( DoDiscsOverlap( ( *projectileIterator )->GetLocation(), ( *projectileIterator )->GetEntityDefinition()->GetPhysicsRadius(), ( *actorIterator )->GetLocation(), ( *actorIterator )->GetEntityDefinition()->GetPhysicsRadius() ) )
			{
				if ( ( *projectileIterator )->ShouldActorBeHit( *actorIterator ) )
				{
					( *actorIterator )->Damage( (*projectileIterator )->m_stats );
					( *actorIterator )->ApplyKnockbackTranslation( (*projectileIterator )->GetProjectileDefinition()->GetKnockbackFraction() * (*projectileIterator )->GetVelocityWithAgility() );
					
					if ( ( *projectileIterator )->GetSourceWeaponType() == WeaponType::WEAPON_TYPE_PROJECTILE )		// Only proper Projectile weapons get destroyed on hitting someone
					{
						projectilesToDelete.push_back( *projectileIterator );
						break;
					}
				}
			}
		}
	}

	for ( std::vector< Projectile* >::iterator projectileToDeleteIterator = projectilesToDelete.begin(); projectileToDeleteIterator != projectilesToDelete.end(); projectileToDeleteIterator++ )
	{
		if ( std::find( m_projectiles.begin(), m_projectiles.end(), *projectileToDeleteIterator ) != m_projectiles.end() )
		{
			m_projectiles.erase( std::find( m_projectiles.begin(), m_projectiles.end(), *projectileToDeleteIterator )  );
		}
		if ( std::find( m_allEntitiesInDrawOrder.begin(), m_allEntitiesInDrawOrder.end(), *projectileToDeleteIterator ) != m_allEntitiesInDrawOrder.end() )
		{
			m_allEntitiesInDrawOrder.erase( std::find( m_allEntitiesInDrawOrder.begin(), m_allEntitiesInDrawOrder.end(), *projectileToDeleteIterator ) );
		}
		
		delete *projectileToDeleteIterator;
		*projectileToDeleteIterator = nullptr;
	}
}

bool Map::HasPlayerReachedAPortal()
{
	for ( std::vector< Portal* >::iterator portalIterator = m_portals.begin(); portalIterator != m_portals.end(); portalIterator++ )
	{
		if ( DoDiscsOverlap( ( *portalIterator )->GetLocation(), ( *portalIterator )->GetEntityDefinition()->GetPhysicsRadius(), m_playerEntity->GetLocation(), m_playerEntity->GetEntityDefinition()->GetPhysicsRadius() ) )
		{
			RemovePlayerEntityFromActorList();
			m_adventure.LoadMapAndMovePlayer( *( *portalIterator )->GetReciprocal()->GetMap(), m_playerEntity );
			return true;
		}
	}

	return false;
}

bool Map::HandleCollisionBetweenEntityAndTileAtCoordinates( Entity& entity, const IntVector2& tileCoordinates )
{
	if ( tileCoordinates.x < 0 || tileCoordinates.x >= m_dimensions.x || tileCoordinates.y < 0 || tileCoordinates.y >= m_dimensions.y )
	{
		return false;
	}

	Tile collisionTile = GetTileAt( tileCoordinates );
	AABB2 collisionTileBounds = collisionTile.GetTileWorldBounds();

	if	(	!( collisionTile.GetTileDefinition()->AllowsWalking() && entity.GetEntityDefinition()->CanWalk() )	&&
			!( collisionTile.GetTileDefinition()->AllowsFlying() && entity.GetEntityDefinition()->CanFly() )	&&
			!( collisionTile.GetTileDefinition()->AllowsSwimming() && entity.GetEntityDefinition()->CanSwim() )
		)
	{
		Tile tileAtEntityPosition = GetTileAtWorldPosition( entity.GetLocation() );

		if ( DoesDiscAndAABBOverlap( entity.GetLocation(), entity.GetEntityDefinition()->GetPhysicsRadius(), collisionTileBounds ) )
		{
			IntVector2 correctionDirection = tileAtEntityPosition.GetTileCoordinates() - tileCoordinates;
			Vector2 correctionDirectionAsVector2 = ConvertIntVector2ToVector2( correctionDirection );
			Vector2 tileCorrectionPoint = collisionTileBounds.GetCenter() + ( ( TILE_SIDE_LENGTH_WORLD_UNITS * 0.5f ) * correctionDirectionAsVector2 );
			Vector2 actualDisplacementFromTilePoint = entity.GetLocation() - tileCorrectionPoint;
			if ( IsFloatEqualTo( correctionDirectionAsVector2.x, 0.0f ) )
			{
				actualDisplacementFromTilePoint.x = 0.0f;
			}
			if ( IsFloatEqualTo( correctionDirectionAsVector2.y, 0.0f ) )
			{
				actualDisplacementFromTilePoint.y = 0.0f;
			}
			float actualDisplacementLength = actualDisplacementFromTilePoint.NormalizeAndGetLength();
			float differenceInDisplacement = entity.GetEntityDefinition()->GetPhysicsRadius() - actualDisplacementLength;

			entity.ApplyCorrectiveTranslation( differenceInDisplacement * actualDisplacementFromTilePoint );
			return true;
		}
	}

	return false;
}

void Map::UpdateEntities( float deltaSeconds )
{
	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		( *actorIterator )->Update( deltaSeconds );
	}
	for ( std::vector< Item* >::iterator itemIterator = m_items.begin(); itemIterator != m_items.end(); itemIterator++ )
	{
		( *itemIterator )->Update( deltaSeconds );
	}
	for ( std::vector< Portal* >::iterator portalIterator = m_portals.begin(); portalIterator != m_portals.end(); portalIterator++ )
	{
		( *portalIterator )->Update( deltaSeconds );
	}
	for ( std::vector< Projectile* >::iterator projectileIterator = m_projectiles.begin(); projectileIterator != m_projectiles.end(); projectileIterator++ )
	{
		( *projectileIterator )->Update( deltaSeconds );
	}
}

void Map::BubbleSortEntityRenderVector()
{
	for ( unsigned int outerIndex = 0; outerIndex < m_allEntitiesInDrawOrder.size() - 1; outerIndex++ )
	{
		for ( unsigned int innerIndex = 0; innerIndex < ( m_allEntitiesInDrawOrder.size() - outerIndex - 1 ); innerIndex++ )
		{
			if ( m_allEntitiesInDrawOrder[ innerIndex ]->GetLocation().y < m_allEntitiesInDrawOrder[ innerIndex + 1 ]->GetLocation().y )
			{
				Entity* tempEntityPointer = m_allEntitiesInDrawOrder[ innerIndex ];
				m_allEntitiesInDrawOrder[ innerIndex ] = m_allEntitiesInDrawOrder[ innerIndex + 1 ];
				m_allEntitiesInDrawOrder[ innerIndex + 1 ] = tempEntityPointer;
			}
		}
	}
}

void Map::Render( float renderAlpha, bool developerModeEnabled ) const
{
	RenderTiles( renderAlpha );
	RenderEntities( renderAlpha, developerModeEnabled );
}

void Map::RenderTiles( float renderAlpha ) const
{
	static std::vector< Vertex_3DPCU > tileVerts;
	tileVerts.clear();

	AABB2 cameraViewportBounds = g_mainCamera->GetViewportBounds();
	IntVector2 minTileCoordinates = GetTileCoordinatesForWorldPosition( cameraViewportBounds.mins );
	IntVector2 maxTileCoordinates = GetTileCoordinatesForWorldPosition( cameraViewportBounds.maxs );		
	int minTileIndex = GetActualTileIndex( minTileCoordinates );
	int maxTileIndex = GetActualTileIndex( maxTileCoordinates );

	g_renderer->EnableTexturing( *g_tileSpriteSheet->GetTexture() );
	for ( int tileIndex = minTileIndex; tileIndex <= maxTileIndex; tileIndex++ )
	{
		Vertex_3DPCU vertsForTile[ 8 ];		// 4 for the base layer, 4 for the overlay
		AABB2 tileWorldBounds = m_tiles[ tileIndex ].GetTileWorldBounds();
		AABB2 baseTileUVs = m_tiles[ tileIndex ].GetTileDefinition()->GetBaseSpriteUvs();
		AABB2 overlayTileUVs = m_tiles[ tileIndex ].GetTileDefinition()->GetOverlaySpriteUvs();

		// Base Layer
		vertsForTile[ 0 ].m_position =  ConvertVector2ToVector3( tileWorldBounds.mins );
		vertsForTile[ 1 ].m_position =  ConvertVector2ToVector3( Vector2( tileWorldBounds.mins.x, tileWorldBounds.maxs.y ) );
		vertsForTile[ 2 ].m_position =  ConvertVector2ToVector3( tileWorldBounds.maxs );
		vertsForTile[ 3 ].m_position =  ConvertVector2ToVector3( Vector2( tileWorldBounds.maxs.x, tileWorldBounds.mins.y ) );

		vertsForTile[ 0 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetBaseSpriteTint().GetWithAlpha( renderAlpha );
		vertsForTile[ 1 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetBaseSpriteTint().GetWithAlpha( renderAlpha );
		vertsForTile[ 2 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetBaseSpriteTint().GetWithAlpha( renderAlpha );
		vertsForTile[ 3 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetBaseSpriteTint().GetWithAlpha( renderAlpha );

		vertsForTile[ 0 ].m_UVs =  baseTileUVs.mins;
		vertsForTile[ 1 ].m_UVs =  Vector2( baseTileUVs.mins.x, baseTileUVs.maxs.y );
		vertsForTile[ 2 ].m_UVs =  baseTileUVs.maxs;
		vertsForTile[ 3 ].m_UVs =  Vector2( baseTileUVs.maxs.x, baseTileUVs.mins.y );

		tileVerts.push_back( vertsForTile[ 0 ] );
		tileVerts.push_back( vertsForTile[ 1 ] );
		tileVerts.push_back( vertsForTile[ 2 ] );
		tileVerts.push_back( vertsForTile[ 3 ] );

		// Overlay
		vertsForTile[ 4 ].m_position =  ConvertVector2ToVector3( tileWorldBounds.mins );
		vertsForTile[ 5 ].m_position =  ConvertVector2ToVector3( Vector2( tileWorldBounds.mins.x, tileWorldBounds.maxs.y ) );
		vertsForTile[ 6 ].m_position =  ConvertVector2ToVector3( tileWorldBounds.maxs );
		vertsForTile[ 7 ].m_position =  ConvertVector2ToVector3( Vector2( tileWorldBounds.maxs.x, tileWorldBounds.mins.y ) );

		vertsForTile[ 4 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetOverlaySpriteTint().GetWithAlpha( renderAlpha );
		vertsForTile[ 5 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetOverlaySpriteTint().GetWithAlpha( renderAlpha );
		vertsForTile[ 6 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetOverlaySpriteTint().GetWithAlpha( renderAlpha );
		vertsForTile[ 7 ].m_color = m_tiles[ tileIndex ].GetTileDefinition()->GetOverlaySpriteTint().GetWithAlpha( renderAlpha );

		vertsForTile[ 4 ].m_UVs =  overlayTileUVs.mins;
		vertsForTile[ 5 ].m_UVs =  Vector2( overlayTileUVs.mins.x, overlayTileUVs.maxs.y );
		vertsForTile[ 6 ].m_UVs =  overlayTileUVs.maxs;
		vertsForTile[ 7 ].m_UVs =  Vector2( overlayTileUVs.maxs.x, overlayTileUVs.mins.y );

		tileVerts.push_back( vertsForTile[ 4 ] );
		tileVerts.push_back( vertsForTile[ 5 ] );
		tileVerts.push_back( vertsForTile[ 6 ] );
		tileVerts.push_back( vertsForTile[ 7 ] );
	}

	g_renderer->DrawMeshImmediate( tileVerts.data(), tileVerts.size(), DrawPrimitiveType::QUADS );
	g_renderer->DisableTexturing();
}

void Map::RenderEntities( float renderAlpha, bool developerModeEnabled ) const
{
	for ( std::vector< Entity* >::const_iterator entityIterator = m_allEntitiesInDrawOrder.begin(); entityIterator != m_allEntitiesInDrawOrder.end(); entityIterator++ )
	{
		if	(	std::find( m_items.begin(), m_items.end(), static_cast< Item* >( *entityIterator ) ) == m_items.end()	||		// For items, only render if they are not in inventory
				!( static_cast< Item* >( *entityIterator ) )->IsInInventory()
			)
		{
			( *entityIterator )->Render( renderAlpha, developerModeEnabled );
		}
	}
	for ( std::vector< Entity* >::const_iterator entityIterator = m_allEntitiesInDrawOrder.begin(); entityIterator != m_allEntitiesInDrawOrder.end(); entityIterator++ )	// Render health bars
	{
		if ( std::find( m_actors.begin(), m_actors.end(), static_cast< Actor* >( *entityIterator ) ) != m_actors.end() )
		{
			( static_cast< Actor* >( *entityIterator ) )->RenderHealthInfo( renderAlpha );
		}
	}
}

std::string Map::GetName() const
{
	return m_name;
}

IntVector2 Map::GetDimensions() const
{
	return m_dimensions;
}

Player* Map::GetPlayerEntity() const
{
	return m_playerEntity;
}

Vector2 Map::GetDimensionsInWorldCoordinates() const
{
	return Vector2( ( static_cast< float >( m_dimensions.x ) * TILE_SIDE_LENGTH_WORLD_UNITS ), ( static_cast< float >( m_dimensions.y ) * TILE_SIDE_LENGTH_WORLD_UNITS ) );
}

MapDefinition* Map::GetMapDefinition() const
{
	return m_mapDefinition;
}

int Map::GetActualTileIndex( const IntVector2& coordinates ) const
{
	return ( ( coordinates.y * m_dimensions.x ) + coordinates.x );
}

Tile Map::GetRandomUnpopulatedTileOfType( const TileDefinition& tileType )
{
	std::vector< Tile > tilesOfType;

	for ( Tile tile : m_tiles )
	{
		if ( tile.GetTileDefinition()->GetName() == tileType.GetName() )
		{
			tilesOfType.push_back( tile );
		}
	}
	ASSERT_OR_DIE( ( tilesOfType.size() > 0 ), Stringf( "Map::GetCenterOfRandomTileOfType - Tile of type %s does not exist in map with name %s. Aborting...", tileType.GetName().c_str(), m_name.c_str() ) );
	
	int tileIndex = 0;
	bool unpopulatedTileFound = false;
	while( !unpopulatedTileFound )
	{
		tileIndex = GetRandomIntLessThan( tilesOfType.size() );

		for ( Actor* actor : m_actors )
		{
			if ( GetTileAtWorldPosition( actor->GetLocation() ).GetTileCoordinates() == tilesOfType[ tileIndex ].GetTileCoordinates() )
			{
				continue;
			}
		}
		for ( Portal* portal : m_portals )
		{
			if ( GetTileAtWorldPosition( portal->GetLocation() ).GetTileCoordinates() == tilesOfType[ tileIndex ].GetTileCoordinates() )
			{
				continue;
			}
		}
		// TODO: Check Projectiles?

		unpopulatedTileFound = true;
	}

	return tilesOfType[ tileIndex ];
}

Tile Map::GetTileAt( const IntVector2& coordinates ) const
{
	ASSERT_OR_DIE( coordinates.x >= 0, "Map::GetTileAt() - Negative X coordinates are invalid. Aborting..." );
	ASSERT_OR_DIE( coordinates.y >= 0, "Map::GetTileAt() - Negative Y coordinates are invalid. Aborting..." );
	ASSERT_OR_DIE( coordinates.x < m_dimensions.x, "Map::GetTileAt() - X coordinates provided exceed map dimensions. Aborting..." );
	ASSERT_OR_DIE( coordinates.y < m_dimensions.y, "Map::GetTileAt() - Y coordinates provided exceed map dimensions. Aborting..." );

	return m_tiles[ GetActualTileIndex( coordinates ) ];
}

IntVector2 Map::GetTileCoordinatesForWorldPosition( const Vector2& worldCoordinates ) const		// Will Clamp the coordinates provided if not within map bounds
{
	Vector2 mapDimensionsInWorldCoordinates = GetDimensionsInWorldCoordinates();

	Vector2 clampedWorldCoordinates = Vector2( worldCoordinates );
	clampedWorldCoordinates.x = ClampFloat( worldCoordinates.x, 0.0f, ( mapDimensionsInWorldCoordinates.x - TILE_SIDE_LENGTH_WORLD_UNITS ) );
	clampedWorldCoordinates.y = ClampFloat( worldCoordinates.y, 0.0f, ( mapDimensionsInWorldCoordinates.y - TILE_SIDE_LENGTH_WORLD_UNITS ) );

	return IntVector2( ( static_cast< int >( clampedWorldCoordinates.x / TILE_SIDE_LENGTH_WORLD_UNITS ) ), ( static_cast< int >( clampedWorldCoordinates.y / TILE_SIDE_LENGTH_WORLD_UNITS ) ) );
}

Tile Map::GetTileAtWorldPosition( const Vector2& worldCoordinates ) const		// Will Clamp the coordinates provided if not within map bounds
{
	Vector2 mapDimensionsInWorldCoordinates = GetDimensionsInWorldCoordinates();
	
	Vector2 clampedWorldCoordinates = Vector2( worldCoordinates );
	clampedWorldCoordinates.x = ClampFloat( worldCoordinates.x, 0.0f, ( mapDimensionsInWorldCoordinates.x - TILE_SIDE_LENGTH_WORLD_UNITS ) );
	clampedWorldCoordinates.y = ClampFloat( worldCoordinates.y, 0.0f, ( mapDimensionsInWorldCoordinates.y - TILE_SIDE_LENGTH_WORLD_UNITS ) );

	IntVector2 tileCoordinates = IntVector2( ( static_cast< int >( clampedWorldCoordinates.x / TILE_SIDE_LENGTH_WORLD_UNITS ) ), ( static_cast< int >( clampedWorldCoordinates.y / TILE_SIDE_LENGTH_WORLD_UNITS ) ) );
	return GetTileAt( tileCoordinates );
}

std::vector< Item* > Map::GetItemsInContactWithActor( const Actor& actor ) const
{
	std::vector< Item* > itemsInContact;

	for ( std::vector< Item* >::const_iterator itemIterator = m_items.begin(); itemIterator != m_items.end(); itemIterator++ )
	{
		if ( !( *itemIterator )->IsInInventory() && DoDiscsOverlap( ( *itemIterator )->GetLocation(), ( *itemIterator )->GetEntityDefinition()->GetPhysicsRadius(), actor.GetLocation(), actor.GetEntityDefinition()->GetPhysicsRadius() ) )
		{
			itemsInContact.push_back( *itemIterator );
		}
	}

	return itemsInContact;
}

bool Map::IsEdgeTile( const Tile& tile, int edgeThickness ) const
{
	IntVector2 tileCoordinates = tile.GetTileCoordinates();
	return	( 
				( tileCoordinates.x < edgeThickness ) || ( tileCoordinates.x > ( m_dimensions.x - ( edgeThickness + 1 ) ) ) ||
				( tileCoordinates.y < edgeThickness ) || ( tileCoordinates.y > ( m_dimensions.y - ( edgeThickness + 1 ) ) )
			);
}

RaycastResult2D Map::Raycast( const Vector2& startPosition, const Vector2& direction, float maxDistance ) const
{
	RaycastResult2D result;

	int numSteps = static_cast<int>( maxDistance / TILE_SIDE_LENGTH_WORLD_UNITS ) * g_gameConfigBlackboard.GetValue( "raycastStepsPerTile", 1 );
	Vector2 singleStepDisplacement = ( direction * maxDistance ) / static_cast<float>( numSteps );
	Tile tileAtStartPosition = GetTileAtWorldPosition( startPosition );
	IntVector2 previousTileCoordinates = tileAtStartPosition.GetTileCoordinates();

	if( !tileAtStartPosition.GetTileDefinition()->AllowsSight() )		// Impact at start point
	{
		result.m_didImpact = true;
		result.m_impactNormal = Vector2( 0.0f, 0.0f );
		result.m_impactPosition = startPosition;
		result.m_impactTileCoordinates = previousTileCoordinates;
		result.m_impactDistanceToMaxDistanceRatio = 0.0f;
		return result;
	}

	for ( int raycastStep = 0; raycastStep <= numSteps; raycastStep++ )
	{
		Vector2 currentPosition = startPosition + singleStepDisplacement * static_cast<float>( raycastStep );
		Tile currentTile = GetTileAtWorldPosition( currentPosition );
		IntVector2 currentTileCoordinates = currentTile.GetTileCoordinates();

		if ( currentTileCoordinates == previousTileCoordinates )		// Same tile as before
		{
			continue;
		}

		if( !currentTile.GetTileDefinition()->AllowsSight() )		// Point of impact found
		{
			result.m_didImpact = true;
			IntVector2 impactNormalIntVector2 = previousTileCoordinates - currentTileCoordinates;
			result.m_impactNormal = Vector2( static_cast<float>( impactNormalIntVector2.x ), static_cast<float>( impactNormalIntVector2.y ) );
			result.m_impactPosition = currentPosition;
			result.m_impactTileCoordinates = currentTileCoordinates;
			result.m_impactDistanceToMaxDistanceRatio = maxDistance * ( static_cast<float>( raycastStep ) / static_cast<float>( numSteps ) );
			ImproveRaycastEndPositionPrecision( result );
			return result;
		}

		previousTileCoordinates = currentTileCoordinates;
	}

	result.m_didImpact = false;		// No impact
	result.m_impactNormal = Vector2( 0.0f, 0.0f );
	result.m_impactPosition = startPosition + singleStepDisplacement * static_cast<float>( numSteps );
	result.m_impactTileCoordinates = GetTileAtWorldPosition( result.m_impactPosition ).GetTileCoordinates();
	result.m_impactDistanceToMaxDistanceRatio = 1.0f;
	return result;
}

void Map::ImproveRaycastEndPositionPrecision( RaycastResult2D& out_raycastResult ) const		// Improves how precise the position of the endpoint is by clamping the position to the edge of a tile (only if the raycast hit)
{
	if ( out_raycastResult.m_didImpact )
	{
		Vector2 impactNormal = out_raycastResult.m_impactNormal;
		Tile impactTile = GetTileAtWorldPosition( out_raycastResult.m_impactPosition );
		AABB2 tileBounds =  impactTile.GetTileWorldBounds();

		if ( impactNormal.x < 0.0f )
		{
			out_raycastResult.m_impactPosition.x = tileBounds.mins.x;
		}
		else if ( impactNormal.x > 0.0f )
		{
			out_raycastResult.m_impactPosition.x = tileBounds.maxs.x;
		}

		if ( impactNormal.y < 0.0f )
		{
			out_raycastResult.m_impactPosition.y = tileBounds.mins.y;
		}
		else if ( impactNormal.y > 0.0f )
		{
			out_raycastResult.m_impactPosition.y = tileBounds.maxs.y;
		}
	}
}

bool Map::HasLineOfSight( const Vector2& startPosition, const Vector2& endPosition ) const
{
	Vector2 normalizedDisplacement = endPosition - startPosition;
	float maxDistance = normalizedDisplacement.NormalizeAndGetLength();
	RaycastResult2D raycastResult = Raycast( startPosition, normalizedDisplacement, maxDistance );

	return !raycastResult.m_didImpact;
}

void Map::RemovePlayerEntityFromActorList()
{
	std::vector< Actor* >::iterator playerIteratorInActorVector = m_actors.end();
	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		if ( ( *actorIterator )->GetInstanceName() == std::string( "Player" ) )
		{
			playerIteratorInActorVector = actorIterator;
			break;
		}
	}
	m_actors.erase( playerIteratorInActorVector );

	std::vector< Entity* >::iterator playerIteratorInEntityVector = m_allEntitiesInDrawOrder.end();
	for ( std::vector< Entity* >::iterator entityIterator = m_allEntitiesInDrawOrder.begin(); entityIterator != m_allEntitiesInDrawOrder.end(); entityIterator++ )
	{
		if ( ( *entityIterator )->GetInstanceName() == std::string( "Player" ) )
		{
			playerIteratorInEntityVector = entityIterator;
			break;
		}
	}
	m_allEntitiesInDrawOrder.erase( playerIteratorInEntityVector );

}

void Map::DeleteEntities()
{
	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		delete *actorIterator;
		*actorIterator = nullptr;
	}
	for ( std::vector< Item* >::iterator itemIterator = m_items.begin(); itemIterator != m_items.end(); itemIterator++ )
	{
		delete *itemIterator;
		*itemIterator = nullptr;
	}
	for ( std::vector< Portal* >::iterator portalIterator = m_portals.begin(); portalIterator != m_portals.end(); portalIterator++ )
	{
		delete *portalIterator;
		*portalIterator = nullptr;
	}
	for ( std::vector< Projectile* >::iterator projectileIterator = m_projectiles.begin(); projectileIterator != m_projectiles.end(); projectileIterator++ )
	{
		delete *projectileIterator;
		*projectileIterator = nullptr;
	}
}
