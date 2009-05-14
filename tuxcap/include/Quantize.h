#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "Common.h"

namespace Sexy
{

bool Quantize8Bit(const uint32_t* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, uint32_t* theDestColorTable);

}

#endif //__QUANTIZE_H__

