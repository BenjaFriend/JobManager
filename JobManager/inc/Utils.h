#pragma once

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace Utils
{

class Random
{
public:

	static void Init();

	static const int Random0ToN( const int t_max );

	static const int RandomBetween( const int t_min, const int t_max );

};

};	// Namespace Helpers