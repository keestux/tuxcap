#ifndef __COLOR_H__
#define __COLOR_H__

#include "Common.h"

namespace Sexy
{

// Order of the members of SexyRGBA is important
// TODO. ???? How much does this affect glTexImage2D?
// This struct is used as follows:
//    D3DTLVERTEX aVertex[2] =
//    {
//        { 0, 0, aColor, x1, y1, 0},
//        { 0, 0, aColor, x2, y2, 0}
//    };
//    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
// In other words, the members are used as a byte array. Thus the pack pragma.
#if 0
#pragma pack(push,1)
struct SexyRGBA {unsigned char b,g,r,a;};
#pragma pack(pop)
#else
// Byte order: rgba
struct SexyRGBA
{
public:
    SexyRGBA() {
        colorbytes[0] = 0;
        colorbytes[1] = 0;
        colorbytes[2] = 0;
        colorbytes[3] = 0;
    }
    SexyRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        colorbytes[0] = r;
        colorbytes[1] = g;
        colorbytes[2] = b;
        colorbytes[3] = a;
    }
    uint8_t r() const { return colorbytes[0]; }
    uint8_t g() const { return colorbytes[1]; }
    uint8_t b() const { return colorbytes[2]; }
    uint8_t a() const { return colorbytes[3]; }
private:
    uint8_t colorbytes[4];
};
#endif
class Color
{
public:
    int mRed;
    int mGreen;
    int mBlue;
    int mAlpha;

    static Color Black;
    static Color White;

public:
    Color();
    Color(uint32_t theColor);                   // ARGB
    Color(uint32_t theColor, int theAlpha);     // -RGB
    Color(int theRed, int theGreen, int theBlue);
    Color(int theRed, int theGreen, int theBlue, int theAlpha);
    Color(const SexyRGBA &theColor);
    Color(const uchar* theElements);    
    Color(const int* theElements);

    int                     GetRed() const;
    int                     GetGreen() const;
    int                     GetBlue() const;
    int                     GetAlpha() const;
    uint32_t                ToInt() const;
    SexyRGBA                ToRGBA() const;
};

bool operator==(const Color& theColor1, const Color& theColor2);
bool operator!=(const Color& theColor1, const Color& theColor2);

}

#endif //__COLOR_H__
