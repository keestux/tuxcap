/*

*/
#include    "hgeRandom.h"
#include    "Common.h"

namespace HGE {

unsigned int g_seed=0;

void Random_Seed(int seed)
{
    if(!seed) g_seed=Sexy::Rand();
    else g_seed=seed;
}

float Random_Float(float min, float max)
{
    g_seed=214013*g_seed+2531011;

    return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}
}
