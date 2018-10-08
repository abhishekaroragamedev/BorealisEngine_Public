#version 420 core

layout(binding=1) uniform sampler2D texNormal;

in vec2 PASSUV;
flat in vec3 PASSNORMAL;
flat in vec3 PASSTANGENT;
flat in vec3 PASSBITANGENT;

out vec4 outColor; 

void main( void )
{
   outColor = vec4( 0.0, 0.0, 0.0, 1.0 );
   
   #ifdef SURFACE_NORMAL
	vec3 normalColor = ( texture( texNormal, PASSUV ) ).xyz;
	#ifdef WORLD_NORMAL
		vec3 surfaceNormal = ( normalColor * vec3( 2.0, 2.0, 1.0 ) ) - vec3( 1.0, 1.0, 0.0 );
		mat3 surfaceToWorld = transpose( mat3( PASSTANGENT, PASSBITANGENT, PASSNORMAL ) );
		vec3 worldNormal = surfaceNormal * surfaceToWorld;
		vec3 worldNormalColor = ( worldNormal + vec3( 1.0, 1.0, 0.0 ) ) * vec3( 0.5, 0.5, 1.0 );
		outColor += vec4( worldNormalColor, 1.0 );
	#endif
	#ifndef WORLD_NORMAL
		outColor += vec4( normalColor, 1.0 );
	#endif
   #endif
   
   #ifdef VERTEX_NORMAL
	vec3 normalColor = ( PASSNORMAL + vec3(1.0) ) * vec3(0.5);	// RangeMap the Normal's [-1.0,1.0] range to [0.0, 1.0]
	outColor += vec4( normalColor, 0.0 );
   #endif
   
   #ifdef VERTEX_TANGENT
	vec3 tangentColor = ( PASSTANGENT + vec3(1.0) ) * vec3(0.5);	// RangeMap the Tangent's [-1.0,1.0] range to [0.0, 1.0]
	outColor += vec4( tangentColor, 0.0 );
   #endif
   
   #ifdef VERTEX_BITANGENT
	vec3 bitangentColor = ( PASSBITANGENT + vec3(1.0) ) * vec3(0.5);	// RangeMap the Bitangent's [-1.0,1.0] range to [0.0, 1.0]
	outColor += vec4( bitangentColor, 0.0 );
   #endif
}
