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
    GLuint mTexture;
    int mWidth,mHeight;
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
public:
    typedef std::vector<TextureDataPiece> TextureVector;

    TextureVector mTextures;
#if 0
    LPDIRECTDRAWPALETTE mPalette;
#endif
    int mWidth,mHeight;
    int mTexVecWidth, mTexVecHeight;
    int mTexPieceWidth, mTexPieceHeight;
    int mBitsChangedCount;
    int mTexMemSize;
    float mMaxTotalU, mMaxTotalV;
    PixelFormat mPixelFormat;
    Uint32 mImageFlags;             // See MemoryImage::mD3DFlags and enum D3DImageFlags

    TextureData();
    ~TextureData();

    void ReleaseTextures();
    void CreateTextureDimensions(MemoryImage *theImage);
    GLuint GetTexture(int x, int y, int &width, int &height, float &u1, float &v1, float &u2, float &v2);
    GLuint GetTextureF(float x, float y, float &width, float &height, float &u1, float &v1, float &u2, float &v2);
    void CreateTextures(MemoryImage *theImage);
    void CheckCreateTextures(MemoryImage *theImage);
    void Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor);
    void BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor, const Rect *theClipRect = NULL, float theX = 0, float theY = 0, bool center = false);
    void BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, Uint32 theColor, float tx = 0, float ty = 0);
    void GetBestTextureDimensions(int &theWidth, int &theHeight, bool isEdge, bool usePow2, Uint32 theImageFlags);

    static void SetMinMaxTextureDimension(int minWidth, int miHeight, int maxWidth, int maxHeight, int maxAspectRatio);
    static void SetMaxTextureDimension(int maxWidth, int maxHeight);
    static void SetMaxTextureAspectRatio(int maxAspectRatio);
};

}

#endif	/* TESTUREDATA_H */
