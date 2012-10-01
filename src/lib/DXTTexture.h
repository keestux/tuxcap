/*
 * File:   DXTTexture.h
 * Author: W.P. van Paassen
 *
 * Code taken from SOIL by Jonathan Dummer
 * Public domain
 *
 * Created on May 7th, 2012 4:29 PM
 */

#ifndef DXTTEXTURE_H
#define	 DXTTEXTURE_H

#include <stdint.h>
#include <string>
#include <vector>
#include "Common.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

#include "Image.h"

/*	The dwFlags member of the original DDSURFACEDESC2 structure
	can be set to one or more of the following values.	*/
#define DDSD_CAPS	0x00000001
#define DDSD_HEIGHT	0x00000002
#define DDSD_WIDTH	0x00000004
#define DDSD_PITCH	0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE	0x00080000
#define DDSD_DEPTH	0x00800000

/*	DirectDraw Pixel Format	*/
#define DDPF_ALPHAPIXELS	0x00000001
#define DDPF_FOURCC	0x00000004
#define DDPF_RGB	0x00000040

/*	The dwCaps1 member of the DDSCAPS2 structure can be
	set to one or more of the following values.	*/
#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

/*	The dwCaps2 member of the DDSCAPS2 structure can be
	set to one or more of the following values.		*/
#define DDSCAPS2_CUBEMAP	0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x00008000
#define DDSCAPS2_VOLUME	0x00200000

#define COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

namespace Sexy
{

class DXTTexture : public Image
{
public:
    DXTTexture();
    ~DXTTexture();

    bool            initWithContentsOfFile(const std::string & fname);
    GLuint           CreateTexture(int x, int y, int w, int h);
    bool 			 unpackDXTData(uint8_t* data);

    /**	A bunch of DirectDraw Surface structures and flags **/
    typedef struct
    {
        unsigned int    dwMagic;
        unsigned int    dwSize;
        unsigned int    dwFlags;
        unsigned int    dwHeight;
        unsigned int    dwWidth;
        unsigned int    dwPitchOrLinearSize;
        unsigned int    dwDepth;
        unsigned int    dwMipMapCount;
        unsigned int    dwReserved1[ 11 ];

        /*  DDPIXELFORMAT	*/
        struct
        {
            unsigned int    dwSize;
            unsigned int    dwFlags;
            unsigned int    dwFourCC;
            unsigned int    dwRGBBitCount;
            unsigned int    dwRBitMask;
            unsigned int    dwGBitMask;
            unsigned int    dwBBitMask;
            unsigned int    dwAlphaBitMask;
        }
        sPixelFormat;

        /*  DDCAPS2	*/
        struct
        {
            unsigned int    dwCaps1;
            unsigned int    dwCaps2;
            unsigned int    dwDDSX;
            unsigned int    dwReserved;
        }
        sCaps;
        unsigned int    dwReserved2;
    }
    DDS_header ;

    enum
    {
    		DXT_FLAG_POWER_OF_TWO = 1,
    		DXT_FLAG_MIPMAPS = 2,
    		DXT_FLAG_TEXTURE_REPEATS = 4,
    		DXT_FLAG_MULTIPLY_ALPHA = 8,
    		DXT_FLAG_INVERT_Y = 16,
    		DXT_FLAG_COMPRESS_TO_DXT = 32,
    		DXT_FLAG_DDS_LOAD_DIRECT = 64,
    		DXT_FLAG_NTSC_SAFE_RGB = 128,
    		DXT_FLAG_CoCg_Y = 256,
    		DXT_FLAG_TEXTURE_RECTANGLE = 512
    };

private:
    virtual void    DoPurgeBits();

    std::vector<uint8_t *> mImageData;
    std::vector<int> mImageDataLength;
    uint32_t        mDXTTextureFlagType;

};

}

#endif	/* DXTTEXTURE_H */

