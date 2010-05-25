/*
	provide some functions to provide random float value
*/

#ifndef	HGE_RANDOM_H
#define	HGE_RANDOM_H

namespace	HGE
{
	void	Random_Seed( int seed = 0 );
	float	Random_Float( float min, float max );
}

#endif

