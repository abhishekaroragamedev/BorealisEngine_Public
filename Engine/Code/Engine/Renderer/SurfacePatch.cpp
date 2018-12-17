#include "Engine/Renderer/SurfacePatch.hpp"

Vector3 SphereSurfacePatch( float u, float v )	// Parametrization of a unit sphere centered at the origin
{
	u = RangeMapFloat( u, 0.0f, 1.0f, 0.0f, 360.0f );
	v = RangeMapFloat( v, 0.0f, 1.0f, -90.0f, 90.0f );

	return Vector3::ConvertToCartesian( Vector3( 1.0f, u, v ) );
}

Vector3 CylinderSurfacePatch( float u, float v )	// Parametrization of a cylinder of unit radius and unit length centered at the origin
{
	u = RangeMapFloat( u, 0.0f, 1.0f, 0.0f, 360.0f );

	Vector3 circularPoint = Vector3::ConvertToCartesian( Vector3( 1.0f, u, 0.0f ) );
	circularPoint.y = v;
	return circularPoint;
}

Vector3 GetSurfacePatchNormal( std::function< Vector3( float, float ) > surfacePatchCallback, float u, float v, float sampleSizeU, float sampleSizeV )
{
	Vector3 biTangent = GetSurfacePatchBiTangent( surfacePatchCallback, u, v, sampleSizeV );
	Vector3 tangent = GetSurfacePatchTangent( surfacePatchCallback, u, v, sampleSizeU );
	return CrossProduct( biTangent, tangent );
}

Vector3 GetSurfacePatchTangent( std::function< Vector3( float, float ) > surfacePatchCallback, float u, float v, float sampleSizeU )
{
	Vector2 previousUV = GetSurfacePatchPreviousUV( u, v, sampleSizeU, 0.0f );
	Vector2 nextUV = GetSurfacePatchNextUV( u, v, sampleSizeU, 0.0f );
	Vector3 dU = ( surfacePatchCallback( nextUV.x, v ) - surfacePatchCallback( previousUV.x, v ) ).GetNormalized();
	return dU;
}

Vector3 GetSurfacePatchBiTangent( std::function< Vector3( float, float ) > surfacePatchCallback, float u, float v, float sampleSizeV )
{
	Vector2 previousUV = GetSurfacePatchPreviousUV( u, v, 0.0f, sampleSizeV );
	Vector2 nextUV = GetSurfacePatchNextUV( u, v, 0.0f, sampleSizeV );
	Vector3 dV = ( surfacePatchCallback( u, nextUV.y ) - surfacePatchCallback( u, previousUV.y ) ).GetNormalized();
	return dV;
}

Vector2 GetSurfacePatchPreviousUV( float u, float v, float sampleSizeU, float sampleSizeV )
{
	u -= sampleSizeU;
	v -= sampleSizeV;

	return Vector2( u, v );
}

Vector2 GetSurfacePatchNextUV( float u, float v, float sampleSizeU, float sampleSizeV )
{
	u += sampleSizeU;
	v += sampleSizeV;

	return Vector2( u, v );
}
