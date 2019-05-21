#ifndef PCH_H
#define PCH_H

#include "Utils.h"

#if defined ( _MSC_VER )

#define COMPILER_BARRIER	_ReadWriteBarrier();

#elif defined( __GNUC__ )

#define COMPILER_BARRIER	asm volatile ("" ::: "memory");

#else

#define COMPILER_BARRIER	asm volatile("" ::: "memory");

#endif


// Debug macros ------------------
#ifdef _DEBUG

#define DEBUG_PRINT( a, ... ) printf( "%s: %d(): " a "\n", __FILE__, __LINE__, __VA_ARGS__ );

#else

#define DEBUG_PRINT( a, ... )

#endif


// Memory leak detection
#if defined( _WIN32 ) || defined( _WIN64 )

#if defined( _DEBUG ) 

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>
#include <crtdbg.h> 

// Redefine new
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )

#else

#define DBG_NEW new

#endif  // defined( _DEBUG ) 

#endif  // defined( _WIN32 ) || defined( _WIN64 )

// Common libraries --------------
#include <iostream>

#endif //PCH_H
