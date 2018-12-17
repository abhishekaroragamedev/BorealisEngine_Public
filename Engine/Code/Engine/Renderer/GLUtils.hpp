#pragma once

#define GL_CHECK_ERROR()  GLCheckError( __FILE__, __LINE__ )

bool GLCheckError( char const *file, int line );

bool GLFailed();

bool GLSucceeded();

size_t GLGetTypeSize( int type );