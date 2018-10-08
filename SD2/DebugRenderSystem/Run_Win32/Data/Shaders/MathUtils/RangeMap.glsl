float RangeMapFloat( float value, float inMin, float inMax, float outMin, float outMax )
{
	float mappedValue = value;

	if ( ( inMax - inMin ) == 0.0f )
	{
		mappedValue = ( outMax + outMin ) * 0.5f;
	}
	else
	{
		mappedValue = outMin + ( ( ( mappedValue - inMin ) / ( inMax - inMin ) ) * ( outMax - outMin ) );
	}

	return mappedValue;
}

vec2 RangeMapVec2( vec2 value, vec2 inMin, vec2 inMax, vec2 outMin, vec2 outMax )
{
	vec2 mappedValue = value;

	if ( ( inMax - inMin ) == vec2( 0.0f ) )
	{
		mappedValue = ( outMax + outMin ) * 0.5f;
	}
	else
	{
		mappedValue = outMin + ( ( ( mappedValue - inMin ) / ( inMax - inMin ) ) * ( outMax - outMin ) );
	}

	return mappedValue;
}

vec3 RangeMapVec3( vec3 value, vec3 inMin, vec3 inMax, vec3 outMin, vec3 outMax )
{
	vec3 mappedValue = value;

	if ( ( inMax - inMin ) == vec3( 0.0f ) )
	{
		mappedValue = ( outMax + outMin ) * 0.5f;
	}
	else
	{
		mappedValue = outMin + ( ( ( mappedValue - inMin ) / ( inMax - inMin ) ) * ( outMax - outMin ) );
	}

	return mappedValue;
}

vec4 RangeMapVec4( vec4 value, vec4 inMin, vec4 inMax, vec4 outMin, vec4 outMax )
{
	vec4 mappedValue = value;

	if ( ( inMax - inMin ) == vec4( 0.0f ) )
	{
		mappedValue = ( outMax + outMin ) * 0.5f;
	}
	else
	{
		mappedValue = outMin + ( ( ( mappedValue - inMin ) / ( inMax - inMin ) ) * ( outMax - outMin ) );
	}

	return mappedValue;
}
