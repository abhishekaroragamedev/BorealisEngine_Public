#version 420 core

// Attributes
in vec3 POSITION;
in vec4 COLOR;

out vec3 PASSPOSITION;
out vec4 PASSCOLOR; 

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

void main( void )
{
   // 0, since I don't want to translate
   vec4 local_pos = vec4( POSITION, 0.0f );	
   PASSPOSITION = local_pos.xyz;

   vec4 camera_pos = VIEW * local_pos; 

   // projection relies on a 1 being present, so add it back
   vec4 clip_pos = PROJECTION * vec4(camera_pos.xyz, 1); 

   // we only render where depth is 1.0 (equal, ie, where have we not drawn)
   // so z needs to be one for all these
   clip_pos.z = clip_pos.w; // z must equal w.  We set the z, not the w, so we don't affect the x and y as well

   PASSCOLOR = COLOR; 
	gl_Position = clip_pos; // we pass out a clip coordinate
}
