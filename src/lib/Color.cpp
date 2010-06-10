#include "Color.h"

using namespace Sexy;

Color Color::Black(0, 0, 0);
Color Color::White(255, 255, 255);

Color::Color() :
    mRed(0),
    mGreen(0),
    mBlue(0),
    mAlpha(255)
{
}

Color::Color(uint32_t theColor)
{
    // The caller must be aware that the uint32_t is filled as follows:
    // 31.[alpha].24 | 23.[red].16 | 15.[green].8 | 7.[blue].0
    mAlpha = (theColor >> 24) & 0xFF;
    mRed   = (theColor >> 16) & 0xFF;
    mGreen = (theColor >> 8 ) & 0xFF;
    mBlue  = (theColor      ) & 0xFF;
    if(mAlpha==0)
        mAlpha = 0xff;
}

Color::Color(uint32_t theColor, int theAlpha)
{
    // The caller must be aware that the uint32_t is filled as follows:
    // 31.[...].24 | 23.[red].16 | 15.[green].8 | 7.[blue].0
    mAlpha = theAlpha;
    mRed   = (theColor >> 16) & 0xFF;
    mGreen = (theColor >> 8 ) & 0xFF;
    mBlue  = (theColor      ) & 0xFF;
}

Color::Color(int theRed, int theGreen, int theBlue) :
    mRed(theRed),
    mGreen(theGreen),
    mBlue(theBlue),
    mAlpha(0xFF)
{
}

Color::Color(int theRed, int theGreen, int theBlue, int theAlpha) :
    mRed(theRed),
    mGreen(theGreen),
    mBlue(theBlue),
    mAlpha(theAlpha)
{
}

Color::Color(const SexyRGBA &theColor)
{
    mRed   = theColor.r();
    mGreen = theColor.g();
    mBlue  = theColor.b();
    mAlpha = theColor.a();
}

Color::Color(const uchar* theElements)
{
    // The input is a 3 element array with RGB
    mRed   = theElements[0];
    mGreen = theElements[1];
    mBlue  = theElements[2];
    mAlpha = 0xFF;
}

Color::Color(const int* theElements)
{
    // The input is a 3 element array with RGB
    mRed   = theElements[0] & 0xFF;
    mGreen = theElements[1] & 0xFF;
    mBlue  = theElements[2] & 0xFF;
    mAlpha = 0xFF;
}

int Color::GetRed() const
{
    return mRed;    
}

int Color::GetGreen() const
{
    return mGreen;
}

int Color::GetBlue() const
{
    return mBlue;
}

int Color::GetAlpha() const
{
    return mAlpha;
}

uint32_t Color::ToInt() const
{
    // ARGB
    return (mAlpha << 24) | (mRed << 16) | (mGreen << 8) | (mBlue);
}

SexyRGBA Color::ToRGBA() const
{
    return SexyRGBA(mRed, mGreen, mBlue, mAlpha);
}

bool Sexy::operator==(const Color& theColor1, const Color& theColor2)
{
    return 
        (theColor1.mRed == theColor2.mRed) &&
        (theColor1.mGreen == theColor2.mGreen) &&
        (theColor1.mBlue == theColor2.mBlue) && 
        (theColor1.mAlpha == theColor2.mAlpha);
}

bool Sexy::operator!=(const Color& theColor1, const Color& theColor2)
{
    return 
        (theColor1.mRed != theColor2.mRed) ||
        (theColor1.mGreen != theColor2.mGreen) ||
        (theColor1.mBlue != theColor2.mBlue) ||
        (theColor1.mAlpha != theColor2.mAlpha);
}

