#pragma once

#include "Engine/Core/Blackboard.hpp"

#define byte_t unsigned char

// UNUSED Macro
#define UNUSED(x) (void)(x);

// TODO Macro
// Source from http://www.flipcode.com/archives/FIXME_TODO_Notes_As_Warnings_In_Compiler_Output.shtml
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define PRAGMA(p)  __pragma( p )
#define NOTE( x )  PRAGMA( message(x) )
#define FILE_LINE  NOTE( __FILE__LINE__ )

// THE IMPORANT BITS
#define TODO( x )  NOTE( __FILE__LINE__"\n"           \
        " --------------------------------------------------------------------------------------\n" \
        "|  TODO :   " ##x "\n" \
        " --------------------------------------------------------------------------------------\n" )

// UNIMPLEMENTED Macro
#define UNIMPLEMENTED()  TODO( "IMPLEMENT: " QUOTE(__FILE__) " (" QUOTE(__LINE__) ")" ); //ASSERT(0)

// BIT_SHIFT Macro
#define BIT_SHIFT( x ) 1<<x

extern Blackboard g_gameConfigBlackboard;

constexpr char* ENGINE_NAME = "Borealis Engine";
constexpr char* ENGINE_AUTHOR = "Abhishek Arora";
constexpr char* ENGINE_VERSION = "v0.3.2";
constexpr char* ENGINE_YEARS = "2017-2019";
