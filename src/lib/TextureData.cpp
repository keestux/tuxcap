#include "TextureData.h"
#include "Common.h"
#include "D3DInterface.h"
#include "GLExtensions.h"

#include <vector>
#include <assert.h>
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

using namespace Sexy;
using namespace std;

static int gMinTextureWidth = 64;
//static int gMinTextureHeight = 64;
static int gMaxTextureWidth = 64;
static int gMaxTextureHeight = 64;
static int gMaxTextureAspectRatio = 1;
#if 0
static Uint32 gSupportedPixelFormats;
#endif
//static bool gTextureSizeMustBePow2;
static const int MAX_TEXTURE_SIZE = 512;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TextureData::TextureData()
{
    mWidth = 0;
    mHeight = 0;
    mTexVecWidth = 0;
    mTexVecHeight = 0;
    mBitsChangedCount = 0;
    mTexMemSize = 0;
    mTexPieceWidth = 64;
    mTexPieceHeight = 64;

#if 0
    mPalette = NULL;
#endif
    mPixelFormat = PixelFormat_Unknown;
    mImageFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TextureData::~TextureData()
{
    ReleaseTextures();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::ReleaseTextures()
{
    for (int i = 0; i < (int) mTextures.size(); i++) {
        //if (glIsTexture(mTextures[i].mTexture) == GL_TRUE)
        glDeleteTextures(1, &mTextures[i].mTexture);
    }

    mTextures.clear();

    mTexMemSize = 0;
#if 0
    if (mPalette != NULL) {
        mPalette->Release();
        mPalette = NULL;
    }
#endif
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::CreateTextureDimensions(MemoryImage *theImage)
{
    int aWidth = theImage->GetWidth();
    int aHeight = theImage->GetHeight();
    int i;
    /**/
    // Calculate inner piece sizes
    mTexPieceWidth = aWidth;
    mTexPieceHeight = aHeight;
    bool usePow2 = true; //gTextureSizeMustBePow2 || mPixelFormat==PixelFormat_Palette8;
    GetBestTextureDimensions(mTexPieceWidth, mTexPieceHeight, false, usePow2, mImageFlags);

    // Calculate right boundary piece sizes
    int aRightWidth = aWidth % mTexPieceWidth;
    int aRightHeight = mTexPieceHeight;
    if (aRightWidth > 0)
        GetBestTextureDimensions(aRightWidth, aRightHeight, true, usePow2, mImageFlags);
    else
        aRightWidth = mTexPieceWidth;

    // Calculate bottom boundary piece sizes
    int aBottomWidth = mTexPieceWidth;
    int aBottomHeight = aHeight % mTexPieceHeight;
    if (aBottomHeight > 0)
        GetBestTextureDimensions(aBottomWidth, aBottomHeight, true, usePow2, mImageFlags);
    else
        aBottomHeight = mTexPieceHeight;

    // Calculate corner piece size
    int aCornerWidth = aRightWidth;
    int aCornerHeight = aBottomHeight;
    GetBestTextureDimensions(aCornerWidth, aCornerHeight, true, usePow2, mImageFlags);
    /**/

    // Allocate texture array
    mTexVecWidth = (aWidth + mTexPieceWidth - 1) / mTexPieceWidth;
    mTexVecHeight = (aHeight + mTexPieceHeight - 1) / mTexPieceHeight;
    mTextures.resize(mTexVecWidth * mTexVecHeight);

    // Assign inner pieces
    for (i = 0; i < (int) mTextures.size(); i++) {
        TextureDataPiece &aPiece = mTextures[i];
        aPiece.mTexture = 0;
        aPiece.mWidth = mTexPieceWidth;
        aPiece.mHeight = mTexPieceHeight;
    }

    // Assign right pieces
    /**/
    for (i = mTexVecWidth - 1; i < (int) mTextures.size(); i += mTexVecWidth) {
        TextureDataPiece &aPiece = mTextures[i];
        aPiece.mWidth = aRightWidth;
        aPiece.mHeight = aRightHeight;
    }

    // Assign bottom pieces
    for (i = mTexVecWidth * (mTexVecHeight - 1); i < (int) mTextures.size(); i++) {
        TextureDataPiece &aPiece = mTextures[i];
        aPiece.mWidth = aBottomWidth;
        aPiece.mHeight = aBottomHeight;
    }

    // Assign corner piece
    mTextures.back().mWidth = aCornerWidth;
    mTextures.back().mHeight = aCornerHeight;
    /**/

    mMaxTotalU = aWidth / (float) mTexPieceWidth;
    mMaxTotalV = aHeight / (float) mTexPieceHeight;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int gcd(int i, int j)
{
    // Use Euclid's algorithm with a a recursive function
    if (j != 0) {
        return gcd(j, i % j);
    }
    if (i < 0) {
        return -i;
    }
    return i;
}

static inline int GetClosestPowerOf2Above(int theNum)
{
    int aPower2 = 1;
    while (aPower2 < theNum)
        aPower2 <<= 1;

    return aPower2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline bool IsPowerOf2(int theNum)
{
    if (theNum < 0) {
        return false;
    }
    return (theNum & (theNum - 1)) == 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::GetBestTextureDimensions(int &theWidth, int &theHeight, bool isEdge, bool usePow2, Uint32 theImageFlags)
{
    if (theImageFlags & D3DImageFlag_Use64By64Subdivisions) {
        theWidth = theHeight = 64;
        return;
    }

    if (gMaxTextureAspectRatio != 1) {
        assert(0);
    }

    int g = gcd(theWidth, theHeight);
    if (g < gMinTextureWidth)
        g = gMinTextureWidth;
    if (g > gMaxTextureWidth)
        g = gMaxTextureWidth;

    if (!usePow2 || (g > gMinTextureWidth && IsPowerOf2(g))) {
        theWidth = g;
        theHeight = g;
        return;
    }

    int try_g[] ={
        64, 128, 256, 512, 1024,
    };
    vector<double> ratios(5);
    vector<int> pixels(5);
    vector<int> textures(5);
    for (unsigned int i = 0; i < sizeof(try_g)/sizeof(try_g[0]); i++) {
        int g1 = try_g[i];
        int nr_w = (theWidth + g1 - 1) / g1;        // number of textures horizontal
        int nr_h = (theHeight + g1 - 1) / g1;       // number of textures vertical
        textures[i] = nr_w * nr_h;                  // number of textures
        pixels[i] = nr_w * g1 * nr_h * g1;          // number of pixels using the textures
        ratios[i] = (double)pixels[i] / (theWidth * theHeight);
    }

    // pick the best
    double r = 0.0;
    int nr_textures = 0;
    for (unsigned int i = 0; i < sizeof(try_g)/sizeof(try_g[0]); i++) {
        if (theImageFlags & D3DImageFlag_MinimizeNumSubdivisions) {
            // Pick a choice with the least number of textures.
            // If the ratio is greater than 1.5 it seems a waste of memory. Don't do that.
            if (ratios[i] < 1.5) {
                if (textures[i] < nr_textures) {
                    nr_textures = textures[i];
                    r = ratios[i];
                    g = try_g[i];
                }
            }
        }
        else {
            // Pick a choice with the smallest ratio compared to the optimum number of pixels
            if (ratios[i] < r || (ratios[i] == r && textures[i] < nr_textures)) {
                nr_textures = textures[i];
                r = ratios[i];
                g = try_g[i];
            }
        }

        // Nothing picked so far. Use this one.
        if (r == 0.0) {
            nr_textures = textures[i];
            r = ratios[i];
            g = try_g[i];
        }
    }
    theWidth = g;
    theHeight = g;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GLuint TextureData::GetTexture(int x, int y, int &width, int &height, float &u1, float &v1, float &u2, float &v2)
{
    int tx = x / mTexPieceWidth;
    int ty = y / mTexPieceHeight;

    TextureDataPiece &aPiece = mTextures[ty * mTexVecWidth + tx];

    int left = x % mTexPieceWidth;
    int top = y % mTexPieceHeight;
    int right = left + width;
    int bottom = top + height;

    if (right > aPiece.mWidth)
        right = aPiece.mWidth;

    if (bottom > aPiece.mHeight)
        bottom = aPiece.mHeight;

    width = right - left;
    height = bottom - top;

    u1 = left / (float) aPiece.mWidth;
    v1 = top / (float) aPiece.mHeight;
    u2 = right / (float) aPiece.mWidth;
    v2 = bottom / (float) aPiece.mHeight;

    return aPiece.mTexture;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GLuint TextureData::GetTextureF(float x, float y, float &width, float &height, float &u1, float &v1, float &u2, float &v2)
{
    int tx = (int) (x / mTexPieceWidth);
    int ty = (int) (y / mTexPieceHeight);

    TextureDataPiece &aPiece = mTextures[ty * mTexVecWidth + tx];

    float left = x - tx*mTexPieceWidth;
    float top = y - ty*mTexPieceHeight;
    float right = left + width;
    float bottom = top + height;

    if (right > aPiece.mWidth)
        right = aPiece.mWidth;

    if (bottom > aPiece.mHeight)
        bottom = aPiece.mHeight;

    width = right - left;
    height = bottom - top;

    u1 = left / aPiece.mWidth;
    v1 = top / aPiece.mHeight;
    u2 = right / aPiece.mWidth;
    v2 = bottom / aPiece.mHeight;

    return aPiece.mTexture;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::CreateTextures(MemoryImage *theImage)
{
    theImage->DeleteSWBuffers(); // don't need these buffers for 3d drawing

    theImage->CommitBits();

    // Release texture if image size has changed
    bool createTextures = false;
    if (mWidth != theImage->GetWidth() || mHeight != theImage->GetHeight() || theImage->mD3DFlags != mImageFlags) {
        ReleaseTextures();
        mImageFlags = theImage->mD3DFlags;
        CreateTextureDimensions(theImage);
        createTextures = true;
    }

    int i, x, y;

    int aHeight = theImage->GetHeight();
    int aWidth = theImage->GetWidth();

    int aFormatSize = 4;
#if 0
    if (aFormat == PixelFormat_Palette8)
        aFormatSize = 1;
    else if (aFormat == PixelFormat_R5G6B5)
        aFormatSize = 2;
    else if (aFormat == PixelFormat_A4R4G4B4)
        aFormatSize = 2;
#endif

    i = 0;
    for (y = 0; y < aHeight; y += mTexPieceHeight) {
        for (x = 0; x < aWidth; x += mTexPieceWidth) {
            TextureDataPiece &aPiece = mTextures[i++];              // dynamically expanded vector<>
            if (createTextures) {

                aPiece.mTexture = theImage->CreateTexture(x, y, aPiece.mWidth, aPiece.mHeight);
                if (aPiece.mTexture == 0) // create texture failure
                {
                    // TODO. This dies silently. Maybe assert(0) is better.
                    assert(0);
                    return;
                }
#if 0
                SexyRGBA rgba = Color::Black.ToRGBA();

                //create vbo

                (*GLExtensions::glGenBuffers_ptr)(1, &aPiece.mVBO);

                (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, aPiece.mVBO);

                //allocate memory for vbo

                (*GLExtensions::glBufferData_ptr)(GL_ARRAY_BUFFER, 4*sizeof(D3DTLVERTEX), 0, GL_STATIC_DRAW);
                //fill vbo

                D3DTLVERTEX aVertex[4] =
                    {
                        {0, 0,                            rgba, x,                    y           },
                        {0, mTexPieceHeight,              rgba, x,                    y + mTexPieceHeight },
                        {mTexPieceWidth, 0,               rgba, x + mTexPieceWidth,   y           },
                        {mTexPieceWidth, mTexPieceHeight, rgba, x + mTexPieceWidth,   y + mTexPieceHeight },
                    };

                //fixme use GL_WRITE_ONLY_OES for iphone
                GLvoid* vbo_buffer = (*GLExtensions::glMapBuffer_ptr)(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                memcpy(vbo_buffer, &aVertex, 4*sizeof(D3DTLVERTEX));

                (*GLExtensions::glUnmapBuffer_ptr)(GL_ARRAY_BUFFER); 

                //tell opengl about the vertex buffer memory layout
                
                glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), (GLvoid*)((char*)NULL));
                glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), (GLvoid*)((char*)NULL+2*sizeof(GL_FLOAT)));
                glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), (GLvoid*)((char*)NULL+2*sizeof(GL_FLOAT)+4*sizeof(GL_UNSIGNED_BYTE)));
#endif

#if 0
                if (mPalette != NULL)
                    aPiece.mTexture->SetPalette(mPalette);
#endif

                // For diagnostic?
                mTexMemSize += aPiece.mWidth * aPiece.mHeight*aFormatSize;
            }
        }
    }

    mWidth = theImage->GetWidth();
    mHeight = theImage->GetHeight();
    mBitsChangedCount = theImage->mBitsChangedCount;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::CheckCreateTextures(MemoryImage *theImage)
{
    if (theImage->GetWidth() != mWidth || theImage->GetHeight() != mHeight
            || theImage->mBitsChangedCount != mBitsChangedCount
            || theImage->mD3DFlags != mImageFlags)
        CreateTextures(theImage);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor)
{
    int srcLeft = theSrcRect.mX;
    int srcTop = theSrcRect.mY;
    int srcRight = srcLeft + theSrcRect.mWidth;
    int srcBottom = srcTop + theSrcRect.mHeight;
    int srcX, srcY;
    float dstX, dstY;
    int aWidth, aHeight;
    float u1, v1, u2, v2;

    srcY = srcTop;
    dstY = theY;

    if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
        return;

    SexyRGBA rgba = theColor.ToRGBA();

    while (srcY < srcBottom) {
        srcX = srcLeft;
        dstX = theX;
        while (srcX < srcRight) {
            aWidth = srcRight - srcX;
            aHeight = srcBottom - srcY;

            GLuint aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);
            float x = dstX; // - 0.5f;
            float y = dstY; // 0.5f;

            glBindTexture(GL_TEXTURE_2D, aTexture);

            D3DTLVERTEX aVertex[4] =
            {
                {u1, v1, rgba, x,            y           },
                {u1, v2, rgba, x,            y + aHeight },
                {u2, v1, rgba, x + aWidth,   y           },
                {u2, v2, rgba, x + aWidth,   y + aHeight },
            };


            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
            glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
            glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].tu));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


            srcX += aWidth;
            dstX += aWidth;

        }

        srcY += aHeight;
        dstY += aHeight;
    }
}

void TextureData::SetMaxTextureDimension(int maxWidth, int maxHeight)
{
    gMaxTextureWidth = maxWidth;
    gMaxTextureHeight = maxHeight;
    if (gMaxTextureWidth > MAX_TEXTURE_SIZE)
        gMaxTextureWidth = MAX_TEXTURE_SIZE;
    if (gMaxTextureHeight > MAX_TEXTURE_SIZE)
        gMaxTextureHeight = MAX_TEXTURE_SIZE;
}

void TextureData::SetMaxTextureAspectRatio(int maxAspectRatio)
{
    gMaxTextureAspectRatio = maxAspectRatio;

    if (gMaxTextureAspectRatio == 0)
        gMaxTextureAspectRatio = 65536;             // Huh?
}
