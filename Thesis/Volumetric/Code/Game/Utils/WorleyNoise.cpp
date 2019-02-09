#include "Game/Utils/WorleyNoise.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Tools/DevConsole.hpp"

WorleyNoiseCube::WorleyNoiseCube( const IntVector3& dimensions, const IntVector3& numOctaves )
	:	m_dimensions( dimensions ),
		m_numOctaves( numOctaves )
{
	ComputeWorleyNoise();
}

WorleyNoiseCube::WorleyNoiseCube( int x, int y, int z, int numOctavesX, int numOctavesY, int numOctavesZ )
	:	m_dimensions( x, y, z ),
		m_numOctaves( numOctavesX, numOctavesY, numOctavesZ )
{
	ComputeWorleyNoise();
}

void WorleyNoiseCube::ComputeWorleyNoise()
{
	IntVector3 numCells = m_numOctaves;

	Vector3 cellWidth;
	cellWidth.x = 1.0f / static_cast< float >( numCells.x );
	cellWidth.y = 1.0f / static_cast< float >( numCells.y );
	cellWidth.z = 1.0f / static_cast< float >( numCells.z );

	// Compute feature points
	m_featurePoints = new Vector3[ numCells.x * numCells.y * numCells.z ];
	for ( int k = 0; k < numCells.z; k++ )
	{
		for ( int i = 0; i < numCells.x; i++ )
		{
			for ( int j = 0; j < numCells.y; j++ )
			{
				int index = Get1DFeaturePointIndex( i, j, k );
				float cellStartX = static_cast< float >( i ) * cellWidth.x;
				float cellStartY = static_cast< float >( j ) * cellWidth.y;
				float cellStartZ = static_cast< float >( k ) * cellWidth.z;
				Vector3 featurePoint = Vector3(
					GetRandomFloatInRange( cellStartX, ( cellStartX + cellWidth.x ) ),
					GetRandomFloatInRange( cellStartY, ( cellStartY + cellWidth.y ) ),
					GetRandomFloatInRange( cellStartZ, ( cellStartZ + cellWidth.z ) )
				);
				m_featurePoints[ index ] = featurePoint;
			}
		}
	}

	Vector3 dimWidth;
	dimWidth.x = 1.0f / static_cast< float >( m_dimensions.x );
	dimWidth.y = 1.0f / static_cast< float >( m_dimensions.y );
	dimWidth.z = 1.0f / static_cast< float >( m_dimensions.z );


	float maxDistance = 0.0f;

	// Compute noise
	m_values = new float[ m_dimensions.x * m_dimensions.y * m_dimensions.z ];
	for ( int k = 0; k < m_dimensions.z; k++ )
	{
		for ( int i = 0; i < m_dimensions.x; i++ )
		{
			for ( int j = 0; j < m_dimensions.y; j++ )
			{
				Vector3 currentPoint = Vector3(
					( static_cast< float >( i ) * dimWidth.x ),
					( static_cast< float >( j ) * dimWidth.y ),
					( static_cast< float >( k ) * dimWidth.z )
				);
				IntVector3 cellIndex = ConvertVector3ToIntVector3( currentPoint / cellWidth );	// Which cell is the point in?

				// Get the neighboring cells for the point
				int back = cellIndex.z - 1;
				if ( back < 0 )
				{
					back = numCells.z - 1;
				}
				int left = cellIndex.x - 1;
				if ( left < 0 )
				{
					left = numCells.x - 1;
				}
				int bottom = cellIndex.y - 1;
				if ( bottom < 0 )
				{
					bottom = numCells.y - 1;
				}

				float minDistance = FLT_MAX;
				// SUPER NESTED LOOP-YIKES!
				IntVector3 numIterations = IntVector3( 3, 3, 3 );	// Check 27 closest cells
				for ( int nK = back; numIterations.z > 0; nK = ( nK + 1 ) % numCells.z )
				{
					numIterations.z--;
					numIterations.x = 3;

					Vector3 hasWrapped = Vector3::ZERO;
					if ( ( nK + 1 >= numCells.z && numIterations.z == 2 ) || ( nK - 1 < 0 && numIterations.z == 0 ) )
					{
						hasWrapped.z = 1.0f;
					}

					for ( int nI = left; numIterations.x > 0; nI = ( nI + 1 ) % numCells.x )
					{
						numIterations.x--;
						numIterations.y = 3;

						hasWrapped.x = 0.0f;
						if ( ( nI + 1 >= numCells.x && numIterations.x == 2 ) || ( nI - 1 < 0 && numIterations.x == 0 ) )
						{
							hasWrapped.x = 1.0f;
						}

						for ( int nJ = bottom; numIterations.y > 0; nJ = ( nJ + 1 ) % numCells.y )
						{
							numIterations.y--;

							hasWrapped.y = 0.0f;
							if ( ( nJ + 1 >= numCells.y && numIterations.y == 2 ) || ( nJ - 1 < 0 && numIterations.y == 0 ) )
							{
								hasWrapped.y = 1.0f;
							}

							int featurePointIndex = Get1DFeaturePointIndex( nI, nJ, nK );
							Vector3 featurePoint = m_featurePoints[ featurePointIndex ];

							Vector3 displacement = featurePoint - currentPoint;
							// "Wrap around" the displacements for any dimensions that crossed boundaries this iteration - otherwise, boundary crosses will have unfairly large distances and never compete with minDistance
							displacement.x -= Sign( displacement.x ) * hasWrapped.x;
							displacement.y -= Sign( displacement.y ) * hasWrapped.y;
							displacement.z -= Sign( displacement.z ) * hasWrapped.z;

							float distanceSquared = displacement.GetLengthSquared();
							distanceSquared = ClampFloat( distanceSquared, 0.0f, 1.0f );
							if ( distanceSquared < minDistance )
							{
								minDistance = distanceSquared;
							}
						}
					}
				}
				minDistance = sqrtf( minDistance );
				//minDistance = 1.0f - minDistance;	// Check if this works better than just minDistance - black vs. white?
				int pointIndex = Get1DValueIndex( i, j, k );
				m_values[ pointIndex ] = minDistance;

				if ( minDistance > maxDistance )
				{
					maxDistance = minDistance;
				}
			}
		}
	}

	// Scale noise by maxDistance
	for ( int k = 0; k < m_dimensions.z; k++ )
	{
		for ( int i = 0; i < m_dimensions.x; i++ )
		{
			for ( int j = 0; j < m_dimensions.y; j++ )
			{
				int pointIndex = Get1DValueIndex( i, j, k );
				m_values[ pointIndex ] = RangeMapFloat( m_values[ pointIndex ], 0.0f, maxDistance, 0.0f, 1.0f );
			}
		}
	}
}

WorleyNoiseCube::~WorleyNoiseCube()
{
	if ( m_featurePoints != nullptr )
	{
		delete[] m_featurePoints;
		m_featurePoints = nullptr;
	}
	if ( m_values != nullptr )
	{
		delete[] m_values;
		m_values = nullptr;
	}
}

float WorleyNoiseCube::GetValue( const IntVector3& index ) const
{
	int actualIndex = Get1DValueIndex( index.x, index.y, index.z );
	return m_values[ actualIndex ];
}

float WorleyNoiseCube::GetValue( int x, int y, int z ) const
{
	return GetValue( IntVector3( x, y, z ) );
}

int WorleyNoiseCube::Get1DValueIndex( int x, int y, int z ) const
{
	int actualIndex = ( m_dimensions.x * m_dimensions.y ) * z + ( m_dimensions.y ) * x + y;
	return actualIndex;
}

int WorleyNoiseCube::Get1DFeaturePointIndex( int x, int y, int z ) const
{
	int actualIndex = ( m_numOctaves.x * m_numOctaves.y ) * z + ( m_numOctaves.y ) * x + y;
	return actualIndex;
}
