#include "pch.h"
#include "Utils.h"

void Utils::Random::Init()
{
	/* initialize random seed: */
	srand( time( NULL ) );
}

const int Utils::Random::Random0ToN( const int t_max )
{
	// value from 0 to max; 
	return rand() % t_max;
}

const int Utils::Random::RandomBetween( const int t_min, const int t_max )
{
	// Range from minx to max
	return rand() % t_max + t_min;
}