#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include <vector>

template< typename T >
T EvaluateQuadraticBezier( const T& startPos, const T& actualCurveMidPos, const T& endPos, float t )	// The provided point is actually on the curve; we need to get the control point
{
	T rawMidPoint = ( startPos + endPos ) * 0.5f;
	T displacementOfCurvePointFromRawMidPoint = actualCurveMidPos - rawMidPoint;	// Well, this works for Vectors, at least
	T controlPoint = actualCurveMidPos + displacementOfCurvePointFromRawMidPoint;

	T firstInterpolation = Interpolate( startPos, controlPoint, t );
	T secondInterpolation = Interpolate( controlPoint, endPos, t );
	return Interpolate( firstInterpolation, secondInterpolation, t );
}

template< typename T >
T EvaluateCubicBezier( const T& startPos, const T& guidePos1, const T& guidePos2, const T& endPos, float t )
{
	T firstLinearInterpolation = Interpolate( startPos, guidePos1, t );
	T secondLinearInterpolation = Interpolate( guidePos1, guidePos2, t );
	T thirdLinearInterpolation = Interpolate( guidePos2, endPos, t );

	T firstQuadraticInterpolation = Interpolate( firstLinearInterpolation, secondLinearInterpolation, t );
	T secondQuadraticInterpolation = Interpolate( secondLinearInterpolation, thirdLinearInterpolation, t );

	return Interpolate( firstQuadraticInterpolation, secondQuadraticInterpolation, t );
}

template< typename T >
T EvaluateCubicHermite( const T& startPos, const T& startVel, const T& endPos, const T& endVel, float t )
{
	const T& guidePos1 = startPos + ( startVel * ONE_BY_THREE );
	const T& guidePos2 = endPos - ( endVel * ONE_BY_THREE );
	return EvaluateCubicBezier( startPos, guidePos1, guidePos2, endPos, t );
}

class CubicSpline2D
{
public:
	CubicSpline2D() {}
	explicit CubicSpline2D( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray = nullptr );
	~CubicSpline2D() {}

	void		AppendPoint( const Vector2& position, const Vector2& velocity = Vector2::ZERO );
	void		AppendPoints( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray = nullptr );
	void		InsertPoint( int insertBeforeIndex, const Vector2& position, const Vector2& velocity = Vector2::ZERO );
	void		RemovePoint( int pointIndex );
	void		RemoveAllPoints();
	void		SetPoint( int pointIndex, const Vector2& newPosition, const Vector2& newVelocity );
	void		SetPosition( int pointIndex, const Vector2& newPosition );
	void		SetVelocity( int pointIndex, const Vector2& newVelocity );
	void		SetCardinalVelocities( float tension = 0.f, const Vector2& startVelocity = Vector2::ZERO, const Vector2& endVelocity = Vector2::ZERO );

	int				GetNumPoints() const;
	const Vector2	GetPosition( int pointIndex ) const;
	const Vector2	GetVelocity( int pointIndex ) const;
	int				GetPositions( std::vector<Vector2>& out_positions ) const;
	int				GetVelocities( std::vector<Vector2>& out_velocities ) const;
	Vector2			EvaluateAtCumulativeParametric( float t ) const;
	Vector2			EvaluateAtNormalizedParametric( float t ) const;

protected:
	std::vector< Vector2 >	m_positions;
	std::vector< Vector2 >	m_velocities;
};
