#include "Game/GameCommon.hpp"
#include "Game/World/Map.hpp"

Map::Map( int horizontalTileWidth, int verticalTileWidth, SpriteSheet& terrainSpriteSheet ) : m_terrainSpriteSheet( terrainSpriteSheet )
{
	m_horizontalTileWidth = horizontalTileWidth;
	m_verticalTileWidth = verticalTileWidth;
	m_liquidTileUvOffset = 0.0f;
	m_liquidTileFlowDirection = 1.0f;

	m_camera = new Camera();
}

Map::~Map()
{
	DeleteAllEntities();
}

bool Map::Update( float deltaSeconds )
{
	bool hasPlayerCompletedLevel = HasPlayerCompletedLevel();

	if ( !HasPlayerCompletedLevel() )
	{
		PreventCollisions( deltaSeconds );

		for ( int entityType = EntityType::ENTITY_TYPE_PLAYER_TANK; entityType < EntityType::NUM_ENTITY_TYPES; entityType++ )
		{
			for ( std::vector< Entity* >::const_iterator entityIterator = m_entitiesByType[ entityType ].begin(); entityIterator != m_entitiesByType[ entityType ].end(); entityIterator++ )		// Update entities
			{
				Entity* currentEntity = *entityIterator;
				currentEntity->Update( deltaSeconds );
			}
		}

		HandleCollisions();
		RemoveEntitiesMarkedForDeath();
	}

	UpdateLiquidTileUvOffset( deltaSeconds );

	if ( m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].size() > 0 )		// This condition will not be satisified on Victory
	{
		m_camera->Update( deltaSeconds );
	}

	return hasPlayerCompletedLevel;
}

void Map::Render( bool developerModeEnabled ) const
{
	for ( std::vector< Tile >::const_iterator tileIterator = m_tiles.begin(); tileIterator != m_tiles.end(); tileIterator++ )		// Render tiles
	{
		Rgba tileColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );
		AABB2 tilePosition = GetTileWorldPosition( *tileIterator );
		AABB2 terrainTextureCoordinates = tileIterator->GetTileDefinition().GetTerrainTextureBounds();

		if ( !tileIterator->GetTileDefinition().IsLiquid() )
		{
			g_renderer->DrawTexturedAABB( tilePosition, *( m_terrainSpriteSheet.GetTexture() ), terrainTextureCoordinates.mins, terrainTextureCoordinates.maxs, tileColor );
		}
		else	// Flow!
		{
			float textureOffset = m_liquidTileUvOffset * terrainTextureCoordinates.GetDimensions().x;
			float tileOffset = m_liquidTileUvOffset * tilePosition.GetDimensions().x;

			AABB2 leftPartOfTile = AABB2( tilePosition );
			AABB2 rightPartOfTile = AABB2( tilePosition );
			leftPartOfTile.maxs.x = leftPartOfTile.mins.x + ( tilePosition.GetDimensions().x - tileOffset );
			rightPartOfTile.mins.x = rightPartOfTile.mins.x + ( tilePosition.GetDimensions().x - tileOffset );

			AABB2 leftPartOfTexture = AABB2( terrainTextureCoordinates );
			AABB2 rightPartOfTexture = AABB2( terrainTextureCoordinates );

			// Horizontal movement
			leftPartOfTexture.maxs.x = leftPartOfTexture.mins.x + textureOffset;
			rightPartOfTexture.mins.x = rightPartOfTexture.mins.x + textureOffset;

			g_renderer->DrawTexturedAABB( leftPartOfTile, *( m_terrainSpriteSheet.GetTexture() ), rightPartOfTexture.mins, rightPartOfTexture.maxs, tileColor );
			g_renderer->DrawTexturedAABB( rightPartOfTile, *( m_terrainSpriteSheet.GetTexture() ), leftPartOfTexture.mins, leftPartOfTexture.maxs, tileColor );
		}
	}

	for ( int entityType = 0; entityType < EntityType::NUM_ENTITY_TYPES; entityType++ )
	{
		for ( std::vector< Entity* >::const_iterator entityIterator = m_entitiesByType[ entityType ].begin(); entityIterator != m_entitiesByType[ entityType ].end(); entityIterator++ )		// Render entities
		{
			Entity* currentEntity = *entityIterator;
			Vector2 currentEntityLocation = currentEntity->GetLocation();
			float currentEntityOrientationDegrees = currentEntity->GetOrientationDegrees();

			g_renderer->PushMatrix();
			g_renderer->Translate( currentEntityLocation.x, currentEntityLocation.y, 0.0f );
			g_renderer->Rotate( currentEntityOrientationDegrees, 0.0f, 0.0f, 1.0f );

			currentEntity->Render( developerModeEnabled );

			g_renderer->PopMatrix();
		}
	}

	m_camera->RenderPlayerHealthInformation();
	m_camera->RenderPauseOrDeathScreenOverlay();
}

void Map::UpdateLiquidTileUvOffset( float deltaSeconds )
{
	static float s_totalDistanceTravelledThisFlow;

	float smoothingFactor = SmoothStep3( 1.0f - ( s_totalDistanceTravelledThisFlow - 0.5f ) ) / 2.0f;
	m_liquidTileUvOffset += m_liquidTileFlowDirection * ( ( smoothingFactor + LIQUID_TILE_MIN_FLOW_SPEED_WORLD_UNITS_PER_SECOND ) * deltaSeconds );
	s_totalDistanceTravelledThisFlow += ( smoothingFactor + LIQUID_TILE_MIN_FLOW_SPEED_WORLD_UNITS_PER_SECOND ) * deltaSeconds;

	if ( m_liquidTileUvOffset > 1.0f )
	{
		m_liquidTileUvOffset -= 1.0f;
	}
	else if ( m_liquidTileUvOffset < 0.0f )
	{
		m_liquidTileUvOffset += 1.0f;
	}

	if ( s_totalDistanceTravelledThisFlow > 1.0f )
	{
		m_liquidTileFlowDirection *= -1.0f;
		s_totalDistanceTravelledThisFlow = 0.0f;
	}
}

void Map::AddTile( const Tile& newTile )
{
	m_tiles.push_back( newTile );
}

Tile Map::GetTileAt( int x, int y ) const
{
	int index = ( m_horizontalTileWidth * y ) + x;
	return m_tiles[ index ];
}

void Map::UnloadMap()	// Remove the PlayerTank from the map
{
	DeleteAllEntities();
}

void Map::SpawnPlayerTank( PlayerTank* playerTank )		// Current spawn logic: spawn tank at the first grass tile in the vector of tiles
{
	Vector2 spawnLocation = Vector2( 0.0f, 0.0f );

	for ( std::vector< Tile >::iterator tileIterator = m_tiles.begin(); tileIterator != m_tiles.end(); tileIterator++ )
	{
		if ( !tileIterator->GetTileDefinition().DoesBlockMovement() )
		{
			spawnLocation = GetTileWorldPosition( *tileIterator ).GetCenter();
			break;
		}
	}

	playerTank->SetLocation( spawnLocation );

	m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].push_back( playerTank );
}

void Map::SpawnEnemyTanks( int numberToSpawn )
{
	std::vector<IntVector2> playerTankLocations;
	for ( std::vector<Entity*>::iterator playerTankIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].begin(); playerTankIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].end(); playerTankIterator++ )
	{
		playerTankLocations.push_back( GetTileAtWorldPosition( ( *playerTankIterator )->GetLocation() ).GetTileCoordinates() );
	}

	for ( int enemyTankIndex = 0; enemyTankIndex < numberToSpawn; enemyTankIndex++ )
	{
		IntVector2 tileIndex = IntVector2( GetRandomIntLessThan( m_horizontalTileWidth ), GetRandomIntLessThan( m_verticalTileWidth ) );

		while ( GetTileAt( tileIndex.x, tileIndex.y ).GetTileDefinition().DoesBlockMovement() || std::find( playerTankLocations.begin(), playerTankLocations.end(), tileIndex ) != playerTankLocations.end() )
		{
			tileIndex = IntVector2( GetRandomIntLessThan( m_horizontalTileWidth ), GetRandomIntLessThan( m_verticalTileWidth ) );
		}

		Tile tileAtSpawnLocation = GetTileAt( tileIndex.x, tileIndex.y );
		Vector2 spawnLocation = GetTileWorldPosition( tileAtSpawnLocation ).GetCenter();

		EnemyTank* enemyTank = static_cast<EnemyTank*>( m_entityFactory.CreateEntityOfType( EntityType::ENTITY_TYPE_ENEMY_TANK , spawnLocation, Vector2( 0.0f, 0.0f ) ) );
		m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TANK ].push_back( enemyTank );
	}
}

void Map::SpawnEnemyTurrets( int numberToSpawn )
{
	std::vector<IntVector2> tankPositions;
	for ( std::vector<Entity*>::iterator playerTankIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].begin(); playerTankIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].end(); playerTankIterator++ )
	{
		tankPositions.push_back( GetTileAtWorldPosition( ( *playerTankIterator )->GetLocation() ).GetTileCoordinates() );
	}
	for ( std::vector<Entity*>::iterator enemyTankIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TANK ].begin(); enemyTankIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TANK ].end(); enemyTankIterator++ )
	{
		tankPositions.push_back( GetTileAtWorldPosition( ( *enemyTankIterator )->GetLocation() ).GetTileCoordinates() );
	}

	for ( int enemyTurretIndex = 0; enemyTurretIndex < numberToSpawn; enemyTurretIndex++ )
	{
		IntVector2 tileIndex = IntVector2( GetRandomIntLessThan( m_horizontalTileWidth ), GetRandomIntLessThan( m_verticalTileWidth ) );

		while ( GetTileAt( tileIndex.x, tileIndex.y ).GetTileDefinition().IsSolid() || std::find( tankPositions.begin(), tankPositions.end(), tileIndex ) != tankPositions.end() )
		{
			tileIndex = IntVector2( GetRandomIntLessThan( m_horizontalTileWidth ), GetRandomIntLessThan( m_verticalTileWidth ) );
		}

		Tile tileAtSpawnLocation = GetTileAt( tileIndex.x, tileIndex.y );
		Vector2 spawnLocation = GetTileWorldPosition( tileAtSpawnLocation ).GetCenter();

		EnemyTurret* enemyTurret = static_cast<EnemyTurret*>( m_entityFactory.CreateEntityOfType( EntityType::ENTITY_TYPE_ENEMY_TURRET , spawnLocation, Vector2( 0.0f, 0.0f ) ) );
		m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TURRET ].push_back( enemyTurret );
	}
}

void Map::SpawnBullet( EntityType firingEntityType, const Vector2& spawnLocation, float spawnOrientation )
{
	if ( firingEntityType == EntityType::ENTITY_TYPE_PLAYER_TANK )
	{
		PlayerBullet* playerBullet = static_cast<PlayerBullet*>( m_entityFactory.CreateEntityOfType( EntityType::ENTITY_TYPE_PLAYER_BULLET, spawnLocation, Vector2::MakeDirectionAtDegrees( spawnOrientation ) ) );
		m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].push_back( playerBullet );

		Explosion* bulletSpawnLocation = m_entityFactory.CreateExplosion( playerBullet->GetLocation(), EXPLOSION_BULLET_SPAWN_SIZE_WORLD_UNITS, EXPLOSION_BULLET_STRIKE_DURATION_SECONDS );
		m_entitiesByType[ EntityType::ENTITY_TYPE_EXPLOSION ].push_back( bulletSpawnLocation );
	}
	else if ( firingEntityType == EntityType::ENTITY_TYPE_ENEMY_TANK || firingEntityType == EntityType::ENTITY_TYPE_ENEMY_TURRET )
	{
		EnemyBullet* enemyBullet = static_cast<EnemyBullet*>( m_entityFactory.CreateEntityOfType( EntityType::ENTITY_TYPE_ENEMY_BULLET, spawnLocation, Vector2::MakeDirectionAtDegrees( spawnOrientation ) ) );
		m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_BULLET ].push_back( enemyBullet );

		Explosion* bulletSpawnLocation = m_entityFactory.CreateExplosion( enemyBullet->GetLocation(), EXPLOSION_BULLET_SPAWN_SIZE_WORLD_UNITS, EXPLOSION_BULLET_STRIKE_DURATION_SECONDS );
		m_entitiesByType[ EntityType::ENTITY_TYPE_EXPLOSION ].push_back( bulletSpawnLocation );
	}

	SoundPlaybackID bulletFireSound = PlayRandomSoundFor( IncursionAudioType::SOFT_FIRE, ( g_theWorld->GetPlayerSoundMuteFraction() * BULLET_FIRE_VOLUME ) );
	Vector2 positionRelativeToPlayer = spawnLocation - g_theWorld->GetPlayerTank()->GetLocation();
	positionRelativeToPlayer.x /= g_theWorld->GetCurrentMap()->GetBoundsInWorldCoordinates().maxs.x;
	g_audioSystem->SetSoundPlaybackBalance( bulletFireSound, SmoothStop4( positionRelativeToPlayer.x ) );
	g_theWorld->AddSoundPlaying( bulletFireSound );
}

Tile Map::GetTileAtWorldPosition( const Vector2& worldPosition ) const
{
	float tileIndexX = ( worldPosition.x - 0.0f ) / TILE_SIDE_LENGTH_WORLD_UNITS;
	float tileIndexY = ( worldPosition.y - 0.0f ) / TILE_SIDE_LENGTH_WORLD_UNITS;

	return GetTileAt( static_cast< int > ( tileIndexX ), static_cast< int > ( tileIndexY ) );
}

AABB2 Map::GetTileWorldPosition( const Tile& tile ) const
{
	float minX;
	float minY;
	float maxX;
	float maxY;

	IntVector2 tileCoordinates = tile.GetTileCoordinates();
	minX = TILE_SIDE_LENGTH_WORLD_UNITS * static_cast<float>( tileCoordinates.x );
	minY = TILE_SIDE_LENGTH_WORLD_UNITS * static_cast<float>( tileCoordinates.y );
	maxX = minX + TILE_SIDE_LENGTH_WORLD_UNITS;
	maxY = minY + TILE_SIDE_LENGTH_WORLD_UNITS;
	
	return AABB2( minX, minY, maxX, maxY );
}

void Map::HandleBulletDeath( Bullet* bullet )
{
	Explosion* bulletExplosion = m_entityFactory.CreateExplosion( bullet->GetLocation(), EXPLOSION_BULLET_STRIKE_SIZE_WORLD_UNITS, EXPLOSION_BULLET_STRIKE_DURATION_SECONDS );
	m_entitiesByType[ EntityType::ENTITY_TYPE_EXPLOSION ].push_back( bulletExplosion );
}

void Map::HandleEnemyTankDeath( EnemyTank* enemyTank )
{
	Explosion* tankExplosion = m_entityFactory.CreateExplosion( enemyTank->GetLocation(), EXPLOSION_TANK_EXPLOSION_SIZE_WORLD_UNITS, EXPLOSION_TANK_EXPLOSION_DURATION_SECONDS );
	m_entitiesByType[ EntityType::ENTITY_TYPE_EXPLOSION ].push_back( tankExplosion );
}

void Map::HandleEnemyTurretDeath( EnemyTurret* enemyTurret )
{
	Explosion* turretExplosion = m_entityFactory.CreateExplosion( enemyTurret->GetLocation(), EXPLOSION_TURRET_EXPLOSION_SIZE_WORLD_UNITS, EXPLOSION_TURRET_EXPLOSION_DURATION_SECONDS );
	m_entitiesByType[ EntityType::ENTITY_TYPE_EXPLOSION ].push_back( turretExplosion );
}

void Map::HandlePlayerTankDeath( PlayerTank* playerTank )
{
	Explosion* tankExplosion = m_entityFactory.CreateExplosion( playerTank->GetLocation(), EXPLOSION_PLAYER_TANK_EXPLOSION_SIZE_WORLD_UNITS, EXPLOSION_PLAYER_TANK_EXPLOSION_DURATION_SECONDS );
	m_entitiesByType[ EntityType::ENTITY_TYPE_EXPLOSION ].push_back( tankExplosion );

	SoundPlaybackID gameOverSound = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_GAME_OVER ) );
	g_theWorld->AddSoundPlaying( gameOverSound );
}

bool Map::HasPlayerCompletedLevel()
{
	PlayerTank* playerTank = static_cast<PlayerTank*>( m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ][ 0 ] );
	Tile tileAtTankPosition = GetTileAtWorldPosition( playerTank->GetLocation() );

	return tileAtTankPosition.GetTileDefinition().IsTerminalTile();
}

void Map::RemoveEntitiesMarkedForDeath()		// Excluding player tanks
{
	for ( int entityTypeIndex = EntityType::ENTITY_TYPE_ENEMY_TANK; entityTypeIndex < EntityType::NUM_ENTITY_TYPES; entityTypeIndex++ )
	{
		size_t newEntityIndex = 0;

		for ( size_t entityIndex = 0; entityIndex < m_entitiesByType[ entityTypeIndex ].size(); entityIndex++ )
		{
			Entity* entity = m_entitiesByType[ entityTypeIndex ][ entityIndex ];

			if ( entity->IsDead() )
			{
				delete entity;		// Deallocate from heap
			}
			else
			{
				m_entitiesByType[ entityTypeIndex ][ newEntityIndex ] = entity;		// Move alive entities to the beginning of the vector
				newEntityIndex++;
			}
		}

		m_entitiesByType[ entityTypeIndex ].resize( newEntityIndex );
	}
}

void Map::PreventCollisions( float deltaSeconds )
{
	std::vector<Entity*> bulletsToCheck;
	bulletsToCheck.insert( bulletsToCheck.end(), m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].begin(), m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].end() );
	bulletsToCheck.insert( bulletsToCheck.end(), m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_BULLET ].begin(), m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_BULLET ].end() );

	AABB2 levelBounds = GetBoundsInWorldCoordinates();

	for ( std::vector<Entity*>::iterator bulletIterator = bulletsToCheck.begin(); bulletIterator != bulletsToCheck.end(); bulletIterator++ )
	{
		Bullet* bulletToCheck = static_cast<Bullet*>( *bulletIterator );

		Tile tileAtCurrentPosition = GetTileAtWorldPosition( bulletToCheck->GetLocation() );

		Vector2 bulletNextPosition = bulletToCheck->GetNextPosition( deltaSeconds );		// This position could potentially be dangerous (out of bounds), so clamp it first
		bulletNextPosition.x = ClampFloat( bulletNextPosition.x, levelBounds.mins.x, levelBounds.maxs.x );
		bulletNextPosition.y = ClampFloat( bulletNextPosition.y, levelBounds.mins.y, levelBounds.maxs.y );

		Tile tileAtNextPosition = GetTileAtWorldPosition( bulletNextPosition );
		if ( tileAtNextPosition.GetTileDefinition().IsSolid() )
		{
			Vector2 bulletVelocity = bulletToCheck->GetVelocity();
			IntVector2 reflectionNormalIntVector = tileAtCurrentPosition.GetTileCoordinates() - tileAtNextPosition.GetTileCoordinates();
			if ( reflectionNormalIntVector.x == 0 && reflectionNormalIntVector.y == 0 )		// This can happen if the bullet is fired right into a solid tile
			{
				if ( fabs( bulletVelocity.x ) >= fabs( bulletVelocity.y ) )
				{
					reflectionNormalIntVector.x = static_cast<int>( -bulletVelocity.x / fabs( bulletVelocity.x ) );
				}
				else
				{
					reflectionNormalIntVector.y = static_cast<int>( -bulletVelocity.y / fabs( bulletVelocity.y ) );
				}
			}

			Vector2 reflectionNormal = Vector2( static_cast<float>( reflectionNormalIntVector.x ), static_cast<float>( reflectionNormalIntVector.y ) );
			Vector2 reflectionTangent = RotateVector2RightAngle( reflectionNormal, true );

			Vector2 bulletVelocityInTNSpace = GetTransformedIntoBasis( bulletVelocity, reflectionTangent, reflectionNormal );
			bulletVelocityInTNSpace.y *= -1;	// Reverse the normal component of the velocity, and preserve the tangent

			Vector2 newBulletVelocity = GetTransformedOutOfBasis( bulletVelocityInTNSpace, reflectionTangent, reflectionNormal );

			bulletToCheck->RedirectBullet( newBulletVelocity );

			if ( bulletToCheck->GetHealth() == 0 )
			{
				HandleBulletDeath( bulletToCheck );
			}
		}
	}
}

void Map::HandleCollisions()
{
	HandleBulletCollisionsWithTanksAndTurrets();
	HandleTankCollisionsWithTanks();
	HandleTankCollisionsWithTurrets();
	HandleTankCollisionWithWalls();
}

void Map::HandleBulletCollisionsWithTanksAndTurrets()
{
	for ( std::vector<Entity*>::iterator playerBulletIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].begin(); playerBulletIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].end(); playerBulletIterator++ )	// Collisions between player bullets and enemies
	{
		for ( std::vector<Entity*>::iterator enemyTankIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TANK ].begin(); enemyTankIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TANK ].end(); enemyTankIterator++ )
		{
			PlayerBullet* playerBullet = static_cast<PlayerBullet*>( *playerBulletIterator );
			EnemyTank* enemyTank = static_cast<EnemyTank*>( *enemyTankIterator );

			if ( !playerBullet->IsDead() && enemyTank->GetDiscCollider().IsPointInside( playerBullet->GetDiscCollider().center ) )
			{
				playerBullet->Destroy();
				enemyTank->DoDamage();

				HandleBulletDeath( playerBullet );

				if ( enemyTank->GetHealth() == 0 )
				{
					HandleEnemyTankDeath( enemyTank );
				}
			}
		}
	}

	for ( std::vector<Entity*>::iterator playerBulletIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].begin(); playerBulletIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_BULLET ].end(); playerBulletIterator++ )	// Collisions between player bullets and enemies
	{
		for ( std::vector<Entity*>::iterator enemyTurretIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TURRET ].begin(); enemyTurretIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_TURRET ].end(); enemyTurretIterator++ )
		{
			PlayerBullet* playerBullet = static_cast<PlayerBullet*>( *playerBulletIterator );
			EnemyTurret* enemyTurret = static_cast<EnemyTurret*>( *enemyTurretIterator );

			if ( !playerBullet->IsDead() && enemyTurret->GetDiscCollider().IsPointInside( playerBullet->GetDiscCollider().center ) )
			{
				playerBullet->Destroy();
				enemyTurret->DoDamage();

				HandleBulletDeath( playerBullet );

				if ( enemyTurret->GetHealth() == 0 )
				{
					HandleEnemyTurretDeath( enemyTurret );
				}
			}
		}
	}

	for ( std::vector<Entity*>::iterator enemyBulletIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_BULLET ].begin(); enemyBulletIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_ENEMY_BULLET ].end(); enemyBulletIterator++ )		// Collisions between enemy bullets and players
	{
		for ( std::vector<Entity*>::iterator playerIterator = m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].begin(); playerIterator != m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].end(); playerIterator++ )
		{
			EnemyBullet* enemyBullet = static_cast<EnemyBullet*>( *enemyBulletIterator );
			PlayerTank* playerTank = static_cast<PlayerTank*>( *playerIterator );

			if ( !enemyBullet->IsDead() && !playerTank->IsDead()  && playerTank->GetDiscCollider().IsPointInside( enemyBullet->GetDiscCollider().center ) )
			{
				enemyBullet->Destroy();

				HandleBulletDeath( enemyBullet );

				if ( !playerTank->IsGodModeEnabled()  )
				{
					playerTank->DoDamage();
				}

				if ( playerTank->GetHealth() == 0 )
				{
					HandlePlayerTankDeath( playerTank );
				}
			}
		}
	}
}

void Map::HandleTankCollisionsWithTanks()
{
	std::vector<Entity*> tanksToHandle;
	tanksToHandle.insert( tanksToHandle.end(), m_entitiesByType[ ENTITY_TYPE_PLAYER_TANK ].begin(), m_entitiesByType[ ENTITY_TYPE_PLAYER_TANK ].end() );
	tanksToHandle.insert( tanksToHandle.end(), m_entitiesByType[ ENTITY_TYPE_ENEMY_TANK ].begin(), m_entitiesByType[ ENTITY_TYPE_ENEMY_TANK ].end() );

	for ( std::vector<Entity*>::iterator firstTankIterator = tanksToHandle.begin(); firstTankIterator != tanksToHandle.end(); firstTankIterator++ )
	{
		for ( std::vector<Entity*>::iterator secondTankIterator = tanksToHandle.begin(); secondTankIterator != tanksToHandle.end(); secondTankIterator++ )
		{
			if ( firstTankIterator != secondTankIterator && !(*firstTankIterator)->IsDead() && !(*secondTankIterator)->IsDead() )
			{
				Tank* firstTank = static_cast<Tank*>( *firstTankIterator );
				Tank* secondTank = static_cast<Tank*>( *secondTankIterator );

				Disc2 firstDisc = firstTank->GetDiscCollider();
				Disc2 secondDisc = secondTank->GetDiscCollider();

				if ( DoDiscsOverlap( firstDisc, secondDisc ) )
				{
					Vector2 directionBetweenCenters = secondDisc.center - firstDisc.center;
					float distanceBetweenCenters = directionBetweenCenters.NormalizeAndGetLength();
					Vector2 midPoint = firstDisc.center + ( ( distanceBetweenCenters / 2.0f ) * directionBetweenCenters );
					
					Vector2 firstTankFinalPosition = midPoint - ( directionBetweenCenters * firstDisc.radius );
					Vector2 secondTankFinalPosition = midPoint + ( directionBetweenCenters * secondDisc.radius );

					firstTank->TranslateFromCollision( firstTankFinalPosition - firstDisc.center );
					secondTank->TranslateFromCollision( secondTankFinalPosition - secondDisc.center );
				}
			}
		}
	}
}

void Map::HandleTankCollisionsWithTurrets()
{
	std::vector<Entity*> tanksToHandle;
	tanksToHandle.insert( tanksToHandle.end(), m_entitiesByType[ ENTITY_TYPE_PLAYER_TANK ].begin(), m_entitiesByType[ ENTITY_TYPE_PLAYER_TANK ].end() );
	tanksToHandle.insert( tanksToHandle.end(), m_entitiesByType[ ENTITY_TYPE_ENEMY_TANK ].begin(), m_entitiesByType[ ENTITY_TYPE_ENEMY_TANK ].end() );

	for ( std::vector<Entity*>::iterator tankIterator = tanksToHandle.begin(); tankIterator != tanksToHandle.end(); tankIterator++ )
	{
		for ( std::vector<Entity*>::iterator turretIterator = m_entitiesByType[ ENTITY_TYPE_ENEMY_TURRET ].begin(); turretIterator != m_entitiesByType[ ENTITY_TYPE_ENEMY_TURRET ].end(); turretIterator++ )
		{
			Tank* tank = static_cast<Tank*>( *tankIterator );
			EnemyTurret* turret = static_cast<EnemyTurret*>( *turretIterator );

			Disc2 tankDisc = tank->GetDiscCollider();
			Disc2 turretDisc = turret->GetDiscCollider();

			if ( DoDiscsOverlap( tankDisc, turretDisc ) )
			{
				Vector2 directionToPushTank = tankDisc.center - turretDisc.center;
				float distanceBetweenCenters = directionToPushTank.NormalizeAndGetLength();
				float sumOfRadii = tankDisc.radius + turretDisc.radius;
				float distanceToPushTank = sumOfRadii - distanceBetweenCenters;

				tank->TranslateFromCollision( distanceToPushTank * directionToPushTank);
			}
		}
	}
}

void Map::HandleTankCollisionWithWalls()
{
	std::vector<Entity*> tanksToHandle;
	tanksToHandle.insert( tanksToHandle.end(), m_entitiesByType[ ENTITY_TYPE_PLAYER_TANK ].begin(), m_entitiesByType[ ENTITY_TYPE_PLAYER_TANK ].end() );
	tanksToHandle.insert( tanksToHandle.end(), m_entitiesByType[ ENTITY_TYPE_ENEMY_TANK ].begin(), m_entitiesByType[ ENTITY_TYPE_ENEMY_TANK ].end() );

	for ( std::vector<Entity*>::iterator tankIterator = tanksToHandle.begin(); tankIterator != tanksToHandle.end(); tankIterator++ )
	{
		Tank* tank = static_cast<Tank*>( *tankIterator );
		Tile tankTile = GetTileAtWorldPosition( tank->GetLocation() );
		IntVector2 tankTileCoordinates = tankTile.GetTileCoordinates();

		bool wasEastCollisionChecked = false;
		bool wasWestCollisionChecked = false;
		bool wasNorthCollisionChecked = false;
		bool wasSouthCollisionChecked = false;
		bool wasEastCollisionHandled = false;
		bool wasWestCollisionHandled = false;
		bool wasNorthCollisionHandled = false;
		bool wasSouthCollisionHandled = false;

		if ( tankTileCoordinates.x + 1 < m_horizontalTileWidth )		// First, check the four main cardinal directions
		{
			wasEastCollisionChecked = true;
			Tile eastTile = GetTileAt( ( tankTileCoordinates.x + 1 ), tankTileCoordinates.y );

			if ( eastTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromEastTile = CorrectDiscAndAABBCollisionHeadOn( tank->GetDiscCollider(), GetTileWorldPosition( eastTile ) );
				if ( !IsFloatEqualTo( correctionFromEastTile.x, 0.0f ) )
				{
					wasEastCollisionHandled = true;
					tank->TranslateFromCollision( correctionFromEastTile );
				}
			}
		}
		if ( tankTileCoordinates.x - 1 >= 0 )
		{
			wasWestCollisionChecked = true;
			Tile westTile = GetTileAt( ( tankTileCoordinates.x - 1 ), tankTileCoordinates.y );

			if ( westTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromWestTile = CorrectDiscAndAABBCollisionHeadOn( tank->GetDiscCollider(), GetTileWorldPosition( westTile ) );
				if ( !IsFloatEqualTo( correctionFromWestTile.x, 0.0f ) )
				{
					wasWestCollisionHandled = true;
					tank->TranslateFromCollision( correctionFromWestTile );
				}
			}
		}
		if ( tankTileCoordinates.y + 1 < m_verticalTileWidth )
		{
			wasNorthCollisionChecked = true;
			Tile northTile = GetTileAt( tankTileCoordinates.x, ( tankTileCoordinates.y + 1 ) );

			if ( northTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromNorthTile = CorrectDiscAndAABBCollisionHeadOn( tank->GetDiscCollider(), GetTileWorldPosition( northTile ) );
				if ( !IsFloatEqualTo( correctionFromNorthTile.y, 0.0f ) )
				{
					wasNorthCollisionHandled = true;
					tank->TranslateFromCollision( correctionFromNorthTile );
				}
			}
		}
		if ( tankTileCoordinates.y - 1 >= 0 )
		{
			wasSouthCollisionChecked = true;
			Tile southTile = GetTileAt( tankTileCoordinates.x, ( tankTileCoordinates.y - 1 ) );

			if ( southTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromSouthTile = CorrectDiscAndAABBCollisionHeadOn( tank->GetDiscCollider(), GetTileWorldPosition( southTile ) );
				if ( !IsFloatEqualTo( correctionFromSouthTile.y, 0.0f ) )
				{
					wasSouthCollisionHandled = true;
					tank->TranslateFromCollision( correctionFromSouthTile );
				}
			}
		}

		if ( ( wasEastCollisionChecked && wasNorthCollisionChecked ) && ( !wasEastCollisionHandled && !wasNorthCollisionHandled ) )		// Next, check the four diagonal tiles
		{
			Tile northEastTile = GetTileAt( ( tankTileCoordinates.x + 1 ), ( tankTileCoordinates.y + 1 ) );
			if ( northEastTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromNorthEastTile = CorrectDiscAndAABBCollisionDiagonal( tank->GetDiscCollider(), GetTileWorldPosition( northEastTile ) );
				tank->TranslateFromCollision( correctionFromNorthEastTile );
			}
		}
		if ( ( wasWestCollisionChecked && wasNorthCollisionChecked ) && ( !wasWestCollisionHandled && !wasNorthCollisionHandled ) )
		{
			Tile northWestTile = GetTileAt( ( tankTileCoordinates.x - 1 ), ( tankTileCoordinates.y + 1 ) );
			if ( northWestTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromNorthWestTile = CorrectDiscAndAABBCollisionDiagonal( tank->GetDiscCollider(), GetTileWorldPosition( northWestTile ) );
				tank->TranslateFromCollision( correctionFromNorthWestTile );
			}
		}
		if ( ( wasEastCollisionChecked && wasSouthCollisionChecked ) && ( !wasEastCollisionHandled && !wasSouthCollisionHandled ) )
		{
			Tile southEastTile = GetTileAt( ( tankTileCoordinates.x + 1 ), ( tankTileCoordinates.y - 1 ) );
			if ( southEastTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromSouthEastTile = CorrectDiscAndAABBCollisionDiagonal( tank->GetDiscCollider(), GetTileWorldPosition( southEastTile ) );
				tank->TranslateFromCollision( correctionFromSouthEastTile );
			}
		}
		if ( ( wasWestCollisionChecked && wasSouthCollisionChecked ) && ( !wasWestCollisionHandled && !wasSouthCollisionHandled ) )
		{
			Tile southWestTile = GetTileAt( ( tankTileCoordinates.x - 1 ), ( tankTileCoordinates.y - 1 ) );
			if ( southWestTile.GetTileDefinition().DoesBlockMovement() )
			{
				Vector2 correctionFromSouthWestTile = CorrectDiscAndAABBCollisionDiagonal( tank->GetDiscCollider(), GetTileWorldPosition( southWestTile ) );
				tank->TranslateFromCollision( correctionFromSouthWestTile );
			}
		}
	}
}

Vector2 Map::CorrectDiscAndAABBCollisionHeadOn( const Disc2& discCollider, const AABB2& aabbCollider )
{
	Vector2 discLocationCorrection = Vector2( 0.0f, 0.0f );

	if ( DoesDiscAndAABBOverlap( discCollider, aabbCollider ) )
	{
		Vector2 discLocation  = discCollider.center;

		if ( ( discLocation.y >= aabbCollider.mins.y ) && ( discLocation.y <= aabbCollider.maxs.y ) )		// The tank is on the left or right of the tile
		{
			if ( discLocation.x <= aabbCollider.mins.x )		// The tank is to the left of the tile
			{
				discLocationCorrection.x = - ( discCollider.radius - ( aabbCollider.mins.x - discLocation.x ) );
			}
			else if ( discLocation.x >= aabbCollider.maxs.x )		// The tank is to the right of the tile
			{
				discLocationCorrection.x = discCollider.radius - ( discLocation.x - aabbCollider.maxs.x );
			}
		}

		if ( ( discLocation.x >= aabbCollider.mins.x ) && ( discLocation.x <= aabbCollider.maxs.x ) )		// The tank is above or below the tile
		{
			if ( discLocation.y <= aabbCollider.mins.y )		// The tank is below the tile
			{
				discLocationCorrection.y = - ( discCollider.radius - ( aabbCollider.mins.y - discLocation.y ) );
			}
			else if ( discLocation.y >= aabbCollider.maxs.y )		// The tank is above the tile
			{
				discLocationCorrection.y = discCollider.radius - ( discLocation.y - aabbCollider.maxs.y );
			}
		}
	}

	return discLocationCorrection;
}

Vector2 Map::CorrectDiscAndAABBCollisionDiagonal( const Disc2& discCollider, const AABB2& aabbCollider )
{
	Vector2 discLocationCorrection = Vector2( 0.0f, 0.0f );

	if ( DoesDiscAndAABBOverlap( discCollider, aabbCollider ) )
	{
		Vector2 discLocation  = discCollider.center;

		if ( discLocation.x <= aabbCollider.mins.x )
		{
			if ( discLocation.y <= aabbCollider.mins.y )		// Tank is to the southwest of the tile
			{
				float correctionAnglePositiveDegrees = ATan2Degrees( ( aabbCollider.mins.y - discLocation.y ), ( aabbCollider.mins.x - discLocation.x ) );
				discLocationCorrection.x = - ( ( discCollider.radius * CosDegrees( correctionAnglePositiveDegrees ) ) - ( aabbCollider.mins.x - discLocation.x ) );
				discLocationCorrection.y = - ( ( discCollider.radius * SinDegrees( correctionAnglePositiveDegrees ) ) - ( aabbCollider.mins.y - discLocation.y ) );
			}
			else if ( discLocation.y >= aabbCollider.maxs.y )		// Tank is to the northeast of the tile
			{
				float correctionAnglePositiveDegrees = ATan2Degrees( ( discLocation.y - aabbCollider.maxs.y ), ( aabbCollider.mins.x - discLocation.x ) );
				discLocationCorrection.x = - ( ( discCollider.radius * CosDegrees( correctionAnglePositiveDegrees ) ) - ( aabbCollider.mins.x - discLocation.x ) );
				discLocationCorrection.y = ( discCollider.radius * SinDegrees( correctionAnglePositiveDegrees ) ) - ( discLocation.y - aabbCollider.maxs.y );
			}
		}
		else if ( discLocation.x >= aabbCollider.maxs.x )
		{
			if ( discLocation.y <= aabbCollider.mins.y )			// Tank is to the southeast of the tile
			{
				float correctionAnglePositiveDegrees = ATan2Degrees( ( aabbCollider.mins.y - discLocation.y ), ( discLocation.x - aabbCollider.maxs.x ) );
				discLocationCorrection.x = ( discCollider.radius * CosDegrees( correctionAnglePositiveDegrees ) ) - ( discLocation.x - aabbCollider.maxs.x );
				discLocationCorrection.y = - ( ( discCollider.radius * SinDegrees( correctionAnglePositiveDegrees ) ) - ( aabbCollider.mins.y - discLocation.y ) );
			}
			else if ( discLocation.y >= aabbCollider.maxs.y )		// Tank is to the northeast of the tile
			{
				float correctionAnglePositiveDegrees = ATan2Degrees( ( discLocation.y - aabbCollider.maxs.y ), ( discLocation.x - aabbCollider.maxs.x ) );
				discLocationCorrection.x = ( discCollider.radius * CosDegrees( correctionAnglePositiveDegrees ) ) - ( discLocation.x - aabbCollider.maxs.x );
				discLocationCorrection.y = ( discCollider.radius * SinDegrees( correctionAnglePositiveDegrees ) ) - ( discLocation.y - aabbCollider.maxs.y );
			}
		}
	}

	return discLocationCorrection;
}

RaycastResult2D Map::Raycast( const Vector2& startPosition, const Vector2& direction, float maxDistance ) const
{
	RaycastResult2D result;

	int numSteps = static_cast<int>( maxDistance / TILE_SIDE_LENGTH_WORLD_UNITS ) * RAYCAST_STEPS_PER_TILE;
	Vector2 singleStepDisplacement = ( direction * maxDistance ) / static_cast<float>( numSteps );
	Tile tileAtStartPosition = GetTileAtWorldPosition( startPosition );
	IntVector2 previousTileCoordinates = tileAtStartPosition.GetTileCoordinates();

	if( tileAtStartPosition.GetTileDefinition().IsSolid() )		// Impact at start point
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

		if( currentTile.GetTileDefinition().IsSolid() )		// Point of impact found
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
		AABB2 tileBounds =  GetTileWorldPosition( impactTile );

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

AABB2 Map::GetBoundsInWorldCoordinates() const
{
	Vector2 bottomLeft = Vector2( 0.0f, 0.0f );
	Vector2 topRight = Vector2( ( m_horizontalTileWidth * TILE_SIDE_LENGTH_WORLD_UNITS ), ( m_verticalTileWidth * TILE_SIDE_LENGTH_WORLD_UNITS ) );

	return AABB2( bottomLeft, topRight );
}

void Map::DeleteAllEntities()
{
	if ( m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].size() > 0 )
	{
		m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].erase( m_entitiesByType[ EntityType::ENTITY_TYPE_PLAYER_TANK ].begin() );
	}

	for ( int entityType = EntityType::ENTITY_TYPE_ENEMY_TANK; entityType < EntityType::NUM_ENTITY_TYPES; entityType++ )		// Skip Player Tank here since TheWorld is supposed to manage it
	{
		for ( std::vector< Entity* >::const_iterator entityIterator = m_entitiesByType[ entityType ].begin(); entityIterator != m_entitiesByType[ entityType ].end(); entityIterator++ )		// Delete entities
		{
			Entity* currentEntity = *entityIterator;
			delete currentEntity;
			currentEntity = nullptr;
		}

		if ( m_entitiesByType[ entityType ].size() > 0 )
		{
			m_entitiesByType[ entityType ].erase( m_entitiesByType[ entityType ].begin(), m_entitiesByType[ entityType ].end() );
		}
	}
}
