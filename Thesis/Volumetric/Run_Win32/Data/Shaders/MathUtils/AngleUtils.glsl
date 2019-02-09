#define PI_BY_TWO 1.571428571428571f

vec2 CartesianToPolar( vec2 cartesian )
{
	vec2 polar = vec2(
		length( cartesian ),
		atan( cartesian.y, cartesian.x )
	); // TODO: Test x = 0
	return polar;
}

vec2 PolarToCartesian( vec2 polar )
{
	vec2 cartesian = vec2(
		( polar.x * cos( polar.y ) ),
		( polar.x * sin( polar.y ) )
	);
	return cartesian;
}
