/*
 * File:   PVRTexture.h
 * Author: kees
 *
 * Created on April 3, 2011, 4:29 PM
 */

#ifndef PVRTEXTURE_H
#define	PVRTEXTURE_H

#include <stdint.h>
#include <string>
#include <vector>
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

#include "Image.h"

namespace Sexy
{

class PVRTexture : public Image
{
public:
    PVRTexture();
    ~PVRTexture();

    bool            initWithContentsOfFile(const std::string & fname);
    GLuint          CreateTexture(int x, int y, int w, int h);

private:
    bool            unpackPVRData(uint8_t * data);

    std::vector<uint8_t *> mImageData;
    std::vector<int> mImageDataLength;
    int             mWidth;
    int             mHeight;
    GLenum          mInternalFormat;
    bool            mHasAlpha;
};

}

#endif	/* PVRTEXTURE_H */

