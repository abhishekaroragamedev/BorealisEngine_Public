#version 420 core

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

uniform float TOTALTIME;
uniform float TIMEREMAINING;

in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec4 PASSCOLOR;
out vec2 PASSUV;
out float PASSTOTALTIME;
out float PASSTIMEREMAINING;

void main( void )
{
	float timeElapsedFraction = (TOTALTIME - TIMEREMAINING)/TOTALTIME;

	vec4 ndc_pos = vec4( POSITION, 1 );
	ndc_pos = PROJECTION * VIEW * ndc_pos;
	ndc_pos.y = mix( ndc_pos.y, 1.0, timeElapsedFraction );
	ndc_pos.x = ndc_pos.x + ( 0.1 * sin( mix(0.0, 3.14, timeElapsedFraction) ) );		// Make the number shake about its mean position, completing 180 degrees at the top of the screen
	gl_Position = ndc_pos;

	PASSCOLOR = COLOR;
	PASSUV = UV;
	PASSTOTALTIME = TOTALTIME;
	PASSTIMEREMAINING = TIMEREMAINING;
}
