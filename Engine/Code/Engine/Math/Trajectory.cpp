#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Trajectory.hpp"
#include "Engine/Tools/DevConsole.hpp"

Vector2 Trajectory::Evaluate( float gravity, Vector2 launchVelocity, float time )
{
	/*
		f( time ) = Vector2 [ ( launchVelocity.x * time ), ( -0.5f * gravity * time^2 + launchVelocity.y * time ) ]
	*/

	Vector2 positionOnTrajectory;
	positionOnTrajectory.x = launchVelocity.x * time;
	positionOnTrajectory.y = ( -0.5f * ( gravity * time * time ) ) + ( launchVelocity.y * time );
	return positionOnTrajectory;
}

Vector2 Trajectory::Evaluate( float gravity, float launchSpeed, float launchAngle, float time )
{
	Vector2 launchVelocity = Vector2( ( launchSpeed * CosDegrees( launchAngle ) ), ( launchSpeed * SinDegrees( launchAngle ) ) );
	return Evaluate( gravity, launchVelocity, time );
}

float Trajectory::GetMinimumLaunchSpeed( float gravity, float distance )
{
	/*
		launchSpeed * cos( 45 ) * time = distance				[ as u(x) * t = S ]
		-> launchSpeed =	  distance							----- (1)
						--------------------
						( cos( 45 ) * time )

		launchSpeed * sin( 45 ) * time = 0.5f * g * time^2		[ as u(y) * t - 0.5 * g * t^2 = S(y), and S(y) = 0 ]
		
		Substituting (1),
		( distance * sin ( 45 ) * time )	=	0.5f * g * time^2
		--------------------------------
			  ( cos( 45 ) * time )

		-> distance	= 0.5f * g * time^2							[ as tan( 45 ) = 1 ]
		-> time = sqrtf( 2.0f * distance )
						---------------
							   g

		Use (1) again to get the launchSpeed.
	*/

	float time = sqrtf( ( 2.0f * distance ) / gravity );
	float launchSpeed = ( ROOT_TWO * distance ) / time;		//	[ as cos( 45 ) = ONE_BY_ROOT_TWO ]
	return launchSpeed;
}

bool Trajectory::GetLaunchAngles( Vector2& out_angles, float gravity, float launchSpeed, float distance, float heightDifference /* = 0.0f */ )
{
	/*
		launchSpeed * cos( x ) * time = distance
		-> time	=				 distance												----- (1)
						--------------------------
						( launchSpeed * cos( x ) )

		( launchSpeed * sin( x ) * time ) - ( 0.5f * g * time^2 ) = heightDifference

		Substituting (1),
		( distance * tan( x ) ) -	( 0.5f * g * distance^2 )		=	H
								  ------------------------------
								  ( launchSpeed^2 * cos^2( x ) )
		
		Eventually, we get the quadratic equation in tan(x)
		[ ( 0.5f * g * distance^2 ) tan^2( x ) ] + [ ( -launchSpeed^2 * distance ) tan( x ) ] + [ ( 0.5 * g * distance^2 ) + ( heightDifference * launchSpeed^2 ) ] 
					a												b													   c
		
		The roots of this equation give us the possible tangent values, which can be inversed to get the angles we need.
	*/

	float a = 0.5f * ( gravity * ( distance * distance ) );
	float b = -1.0f * ( ( launchSpeed * launchSpeed ) * distance );
	float c = ( 0.5f * ( gravity * ( distance * distance ) ) ) + ( heightDifference * ( launchSpeed * launchSpeed ) );

	bool solvable = SolveQuadratic( out_angles, a, b, c );
	if ( solvable )
	{
		out_angles.x = ATanDegrees( out_angles.x );
		out_angles.y = ATanDegrees( out_angles.y );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Trajectory::GetLaunchAngles: The provided parameters result in an unsolvable equation." );
	}
	
	return solvable;
}

float Trajectory::GetMaxHeight( float gravity, float launchSpeed, float distance )
{
	/*
		At the apex of the parabola, the distance = totalDistance * 0.5f.
		From the quadratic equation in tan(x) under Trajectory::GetLaunchAngles,
		height =	-[ ( 0.5f * g * ( distance * 0.5f )^2 ) tan^2( x ) ] + [ ( launchSpeed^2 * distance * 0.5f ) tan( x ) ] - [ 0.5f * g * ( distance * 0.5f )^2 ]	------- (1)
						------------------------------------------------------------------------------------------------------------------------------------------
																		launchSpeed^2
		For max height, d/d( tan( x ) )[ height ] = 0.
		The first derivative on tan( x ) yields
		-[ ( 0.125f * g * distance^2 ) * ( 2 * tan( x ) ) ] + [ ( 0.5f * launchSpeed^2 * distance ) ] = 0															------- (2)
		
		The second derivative on tan( x ) yields
		-0.25f * g * distance^2
		Since g is positive (having applied a negative sign in front of it in the first equation instead),
		this expression is negative -> this is a local maximum, and thus the Max Height.

		Solving (2) for tan( x ) yields
		tan( x ) = ( 2.0f * launchSpeed^2 )
				   ------------------------
					   ( distance * g )
		
		This is the angle used for firing, and the max height can be obtained by substituting this value of tan( x ) back in (1).
	*/
	
	float tanTheta = ( 2.0f * launchSpeed * launchSpeed ) / ( distance * gravity );
	
	float tanThetaSquared = tanTheta * tanTheta;
	float distanceSquared = distance * distance;
	float launchSpeedSquared = launchSpeed * launchSpeed;

	float maxHeight =
		(	- ( 0.125f * ( gravity * ( distanceSquared * tanThetaSquared ) ) )	// a
			+ ( 0.5f * ( distance * ( launchSpeedSquared * tanTheta ) ) )		// b		
			- ( 0.125f * gravity * distance )									// c
		) / launchSpeedSquared;

	return maxHeight;
}

Vector2 Trajectory::GetLaunchVelocity( float gravity, float apexHeight, float distance, float targetHeight )
{
	/*
		distance = Vx * t								--- (1)
		targetHeight = ( Vy * t ) - ( 0.5f * g * t^2 )  --- (2)
		Vy^2 = 2.0f * g * apexHeight					--- (3)	[ as v^2 = u^2 + 2aS ], and at the apex, v = 0 in the y-direction

		From (1),
		t = distance
			--------
			   Vx
		From (3),
		Vy = sqrtf( 2.0f * g *apexHeight )

		Substituting the value of t in (2) yields
		targetHeight = ( Vy * distance )	-	( 0.5f * g * distance^2 )
					   -----------------		-------------------------
							  Vx						  Vx^2

		Thus, the following quadratic equation in Vx is built:
		( targetHeight ) Vx^2 - ( Vy * distance ) Vx + ( 0.5f * g * distance^2 ) = 0
			   a									b			  c

		The two values of Vx represent two cases:
		1. The lower value has the projectile hitting its apex before landing on the target (a tall parabola).
		2. The higher value has the projectile hitting the target before it reaches its apex (a long parabola).

		We consider the first one here, as we want the value after it hits its apex and falls down (unless the lower value is negative).
	*/

	if ( apexHeight < targetHeight )
	{
		targetHeight = 0.0f;
	}

	Vector2 launchVelocity;
	launchVelocity.y = sqrtf( 2.0f * gravity * apexHeight );

	float a = targetHeight;
	float b = -1.0f * ( launchVelocity.y * distance );
	float c = 0.5f * ( gravity * ( distance * distance ) );

	Vector2 possibleXVelocityValues;
	bool solvable = SolveQuadratic( possibleXVelocityValues, a, b, c );
	if ( solvable )
	{
		if ( IsFloatGreaterThanOrEqualTo( possibleXVelocityValues.x, 0.0f ) )
		{
			launchVelocity.x = possibleXVelocityValues.x;
		}
		else
		{
			launchVelocity.x = possibleXVelocityValues.y;
		}
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Trajectory::GetLaunchVelocity: The provided parameters result in an unsolvable equation. Returning zero launch velocity." );
		launchVelocity = Vector2::ZERO;
	}

	return launchVelocity;
}
