#pragma once

/*
Implemented by Abhishek Arora for Volumetric Cloud artifact based on:
Rurik Hogfeldt (Volumetric Clouds) - http://www.cse.chalmers.se/~uffe/xjobb/RurikH%C3%B6gfeldt.pdf
Steven Worley (Worley Noise) - Algorithm
*/

#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"

class Mesh;

class WorleyNoiseCube
{

public:
	WorleyNoiseCube( const IntVector3& dimensions, const IntVector3& numOctaves );							// Generates noise values in [0.0, 1.0]
	WorleyNoiseCube( int x, int y, int z, int numOctavesX, int numOctavesY, int numOctavesZ );				// Generates noise values in [0.0, 1.0]
	~WorleyNoiseCube();

	float GetValue( const IntVector3& index ) const;	// Indexed by Z(X(Y))
	float GetValue( int x, int y, int z ) const;		// Indexed by Z(X(Y))
	int Get1DValueIndex( int x, int y, int z ) const;

private:
	void ComputeWorleyNoise();
	int Get1DFeaturePointIndex( int x, int y, int z ) const;

private:
	IntVector3 m_dimensions;
	IntVector3 m_numOctaves;
	float* m_values = nullptr;
	Vector3* m_featurePoints = nullptr;

};
