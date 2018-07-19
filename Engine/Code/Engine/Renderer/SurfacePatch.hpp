#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include <functional>

Vector3 SphereSurfacePatch( float u, float v );	// Parametrization of a unit sphere centered at the origin
Vector3 CylinderSurfacePatch( float u, float v );	// Parametrization of a cylinder of unit radius and unit length centered at the origin

Vector3 GetSurfacePatchNormal( std::function< Vector3( float, float ) > surfacePatchCallback, float u, float v, float sampleSizeU, float sampleSizeV ); // Computes normal = (bi-tangent x tangent)
Vector3 GetSurfacePatchTangent( std::function< Vector3( float, float ) > surfacePatchCallback, float u, float v, float sampleSizeU );
Vector3 GetSurfacePatchBiTangent( std::function< Vector3( float, float ) > surfacePatchCallback, float u, float v, float sampleSizeV );
Vector2 GetSurfacePatchPreviousUV( float u, float v, float sampleSizeU, float sampleSizeV );
Vector2 GetSurfacePatchNextUV( float u, float v, float sampleSizeU, float sampleSizeV );