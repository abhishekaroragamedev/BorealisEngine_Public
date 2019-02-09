#pragma once

constexpr char DEFAULT_VERTEX_SHADER_TEXT[] =
"// define the shader version (this is required)\n"
"#version 420 core\n"
"// Attributes - input to this shasder stage (constant as far as the code is concerned)\n"
"in vec3 POSITION;\n"
"// Entry point - required.  What does this stage do?\n"
"void main( void )\n"
"{\n"
"gl_Position = vec4( POSITION, 1 );\n"
"}\n";

constexpr char DEFAULT_FRAGMENT_SHADER_TEXT[] =
"#version 420 core\n"
"out vec4 outColor;\n"
"void main( void )\n"
"{\n"
"outColor = vec4( 1, 1, 1, 1 );\n"
"}\n";