/* 
 * File:   TestureData.h
 * Author: kees
 *
 * Created on March 13, 2011, 3:36 PM
 */

#ifndef TESTUREDATA_H
#define	TESTUREDATA_H

#include "Common.h"
#include "Color.h"
#include "Rect.h"
#include "MemoryImage.h"

#include <vector>

#include <SDL.h>
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

namespace Sexy
{

class MemoryImage;
class SexyMatrix3;
class TriVertex;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TextureDataPiece
{
    GLuint              mTexture;
    GLuint              mVBO;
    int                 mWidth, mHeight;
    int                 mX0, mY0;
    int                 mX1, mY1;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum PixelFormat
{
    PixelFormat_Unknown             =           0x0000,
    PixelFormat_A8R8G8B8            =           0x0001,
    PixelFormat_A4R4G4B4            =           0x0002,
    PixelFormat_R5G6B5              =           0x0004,
    PixelFormat_Palette8            =           0x0008
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TextureData
{
private:
    std::vector<TextureDataPiece> mTextures;
    int                 mWidth, mHeight;
    int                 mTexVecWidth, mTexVecHeight;
    int                 mTexPieceWidth, mTexPieceHeight;
    int                 mBitsChangedCount;
    PixelFormat         mPixelFormat;
    Uint32              mImageFlags;             // See MemoryImage::mD3DFlags and enum D3DImageFlags
    float               mMaxTotalU, mMaxTotalV;
    int                 mTexMemSize;

public:
    TextureData();
    ~TextureData();

    void    CheckCreateTextures(MemoryImage *theImage);
    void    Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor);
    void    BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor, const Rect *theClipRect = NULL, float theX = 0, float theY = 0, bool center = false);
    void    BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, Uint32 theColor, float tx = 0, float ty = 0);

    static void SetMinMaxTextureDimension(int minWidth, int miHeight, int maxWidth, int maxHeight, int maxAspectRatio);
    static void SetMaxTextureDimension(int maxWidth, int maxHeight);
    static void SetMaxTextureAspectRatio(int maxAspectRatio);

private:
    void    ReleaseTextures();
    void    CreateTextureDimensions(MemoryImage *theImage);
    GLuint  GetTexture(int x, int y, int &width, int &height, float &u1, float &v1, float &u2, float &v2);
    void    CreateTextures(MemoryImage *theImage);
    void    GetBestTextureDimensions(int &theWidth, int &theHeight, bool isEdge, bool usePow2, Uint32 theImageFlags);
};

}

#endif	/* TESTUREDATA_H */
