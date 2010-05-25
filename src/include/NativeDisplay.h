#ifndef __NATIVEDISPLAY_H__
#define __NATIVEDISPLAY_H__
#include "Common.h"

namespace Sexy
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class NativeDisplay
{
public:
	int						mRGBBits;
	uint32_t					mRedMask;
	uint32_t					mGreenMask;
	uint32_t					mBlueMask;
	int						mRedBits;
	int						mGreenBits;
	int						mBlueBits;
	int						mRedShift;
	int						mGreenShift;
	int						mBlueShift;

public:
	NativeDisplay();
	virtual ~NativeDisplay();
};
}

#endif

