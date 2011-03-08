#ifndef __IMAGELIB_H__
#define __IMAGELIB_H__

#include <string>
#include <stdint.h>
#include "Logging.h"

namespace ImageLib
{

class Image
{
public:
    int                 mWidth;
    int                 mHeight;
    uint32_t*           mBits;

    LoggerFacil *       mLogFacil;

public:
    Image();
    Image(int width, int height);
    virtual ~Image();

    int                 GetWidth();
    int                 GetHeight();
    uint32_t*           GetBits();
};

#if 0
bool WriteJPEGImage(const std::string& theFileName, Image* theImage);
bool WritePNGImage(const std::string& theFileName, Image* theImage);
bool WriteTGAImage(const std::string& theFileName, Image* theImage);
bool WriteBMPImage(const std::string& theFileName, Image* theImage);
#endif

extern int gAlphaComposeColor;
extern bool gAutoLoadAlpha;
extern bool gIgnoreJPEG2000Alpha;  // I've noticed alpha in jpeg2000's that shouldn't have alpha so this defaults to true

Image* GetImage(std::string theFileName, bool lookForAlphaImage = true);

 void InitJPEG2000();
 void CloseJPEG2000();

}

#endif //__IMAGELIB_H__

