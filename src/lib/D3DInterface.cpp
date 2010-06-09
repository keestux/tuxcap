#include "D3DInterface.h"
#include "DDInterface.h"
#include "Graphics.h"
#include "Color.h"
#if 0
#include "DirectXErrorString.h"
#endif
#include "SexyMatrix.h"
#include "SexyAppBase.h"
#include "TriVertex.h"
#include <assert.h>
#include <algorithm>

using namespace Sexy;

static int gMinTextureWidth;
static int gMinTextureHeight;
static int gMaxTextureWidth;
static int gMaxTextureHeight;
static int gMaxTextureAspectRatio;
static Uint32 gSupportedPixelFormats;
static bool gTextureSizeMustBePow2;
static const int MAX_TEXTURE_SIZE = 1024;
static bool gLinearFilter = false;


#ifndef WIN32
typedef struct {
    GLfloat tu;
    GLfloat tv;
    SexyRGBA color;
    GLfloat sx;
    GLfloat sy;
    GLfloat sz;
} D3DTLVERTEX;
#endif

std::string D3DInterface::mErrorString;
#if 0
static const int gVertexType = D3DFVF_TLVERTEX;
#endif

static void SetLinearFilter(bool linearFilter)
{
    if (gLinearFilter != linearFilter) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linearFilter ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linearFilter ? GL_LINEAR : GL_NEAREST);
        gLinearFilter = linearFilter;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static SDL_Surface* CreateTextureSurface(int theWidth, int theHeight/*, PixelFormat theFormat*/)
{

    SDL_Surface* aSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, theWidth, theHeight, 32, SDL_rmask, SDL_gmask, SDL_bmask, SDL_amask);

    return aSurface;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int GetClosestPowerOf2Above(int theNum)
{
    int aPower2 = 1;
    while (aPower2 < theNum)
        aPower2 <<= 1;

    return aPower2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool IsPowerOf2(int theNum)
{
    int aNumBits = 0;
    while (theNum > 0) {
        aNumBits += theNum & 1;
        theNum >>= 1;
    }

    return aNumBits == 1;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void CopyImageToSurface8888(void *theDest, Uint32 theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, bool rightPad)
{
    if (theImage->mColorTable == NULL) {
        uint32_t *srcRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;
        char *dstRow = (char*) theDest;

        for (int y = 0; y < theHeight; y++) {
            uint32_t *src = srcRow;
            uint32_t *dst = (uint32_t*) dstRow;

            for (int x = 0; x < theWidth; x++) {
                *dst++ = *src++;
            }

            if (rightPad)
                *dst = *(dst - 1);

            srcRow += theImage->GetWidth();
            dstRow += theDestPitch;
        }
    } else // palette
    {
        uchar *srcRow = (uchar*) theImage->mColorIndices + offy * theImage->GetWidth() + offx;
        uchar *dstRow = (uchar*) theDest;
        uint32_t *palette = theImage->mColorTable;

        for (int y = 0; y < theHeight; y++) {
            uchar *src = srcRow;
            uint32_t *dst = (uint32_t*) dstRow;
            for (int x = 0; x < theWidth; x++)
                *dst++ = palette[*src++];

            if (rightPad)
                *dst = *(dst - 1);

            srcRow += theImage->GetWidth();
            dstRow += theDestPitch;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void CopyImageToSurface(MemoryImage *theImage, SDL_Surface* surface, int offx, int offy, int texWidth, int texHeight)
{
    if (surface == NULL)
        return;

    int aWidth = std::min(texWidth, (theImage->GetWidth() - offx));
    int aHeight = std::min(texHeight, (theImage->GetHeight() - offy));

    bool rightPad = aWidth<texWidth;
    bool bottomPad = aHeight<texHeight;

    if (aWidth > 0 && aHeight > 0) {
        CopyImageToSurface8888((Uint32*) surface->pixels, surface->pitch, theImage, offx, offy, aWidth, aHeight, rightPad);
    }

    if (bottomPad) {
        uchar *dstrow = ((uchar*) surface->pixels) + surface->pitch*aHeight;
        memcpy(dstrow, dstrow - surface->pitch, surface->pitch);
    }
}

/* original taken from a post by Sam Lantinga, thanks Sam for this and for SDL :-)*/
static GLuint CreateTexture(MemoryImage* memImage, int x, int y, int width, int height)
{

    GLuint texture;
    int w, h;
    static SDL_Surface *image = NULL;

    /* Use the texture width and height expanded to powers of 2 */
    w = GetClosestPowerOf2Above(width);
    h = GetClosestPowerOf2Above(height);

    if (image != NULL) {

        if (image->w != w || image-> h != h) {

            SDL_FreeSurface(image);

            // TODO. Find out if we have an BGRA/RGBA issue
            image = SDL_CreateRGBSurface(
                    SDL_HWSURFACE,
                    w,
                    h,
                    32,
                    SDL_bmask,
                    SDL_gmask,
                    SDL_rmask,
                    SDL_amask
                    );

            assert(image != NULL);
        } else {
            //FIXME maybe better to clear the current image just in case
        }
    } else {
        // TODO. Find out if we have an BGRA/RGBA issue
        image = SDL_CreateRGBSurface(
                SDL_HWSURFACE,
                w,
                h,
                32,
                SDL_bmask,
                SDL_gmask,
                SDL_rmask,
                SDL_amask
                );

        assert(image != NULL);
    }

    CopyImageToSurface(memImage, image, x, y, w, h);

    /* Create an OpenGL texture for the image */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1);

    glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            w, h,
            0,
            GL_BGRA,                    // TODO ???? Why do we want BGRA instead of RGBA?
            GL_UNSIGNED_BYTE,
            image->pixels);

    return texture;
}

void D3DInterface::FillOldCursorAreaTexture(GLint x, GLint y)
{
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, 64, 64);
}

void D3DInterface::BltOldCursorArea(GLfloat x, GLfloat y, const Color& theColor)
{
    glDisable(GL_BLEND);

    //SetLinearFilter(false);

    SexyRGBA rgba = theColor.ToRGBA();
                        
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);

    D3DTLVERTEX aVertex[4] = 
    {
         {0.0f, 1.0f, rgba, x,          y,          0},
         {0.0f, 0.0f, rgba, x,          y + 64,     0},
         {1.0f, 0.0f, rgba, x + 64,     y + 64,     0},
         {1.0f, 1.0f, rgba, x + 64,     y,          0},
    };

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].tu));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_QUADS, 0, 4);

    glEnable(GL_BLEND);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void CopySurface8888ToImage(void *theDest, Uint32 theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight)
{
    char *srcRow = (char*) theDest;
    uint32_t *dstRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;

    for (int y = 0; y < theHeight; y++) {
        uint32_t *src = (uint32_t*) srcRow;
        uint32_t *dst = dstRow;

        for (int x = 0; x < theWidth; x++)
            *dst++ = *src++;

        dstRow += theImage->GetWidth();
        srcRow += theDestPitch;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GetBestTextureDimensions(int &theWidth, int &theHeight, bool isEdge, bool usePow2, Uint32 theImageFlags)
{
    if (theImageFlags & D3DImageFlag_Use64By64Subdivisions) {
        theWidth = theHeight = 64;
        return;
    }

    static int aGoodTextureSize[MAX_TEXTURE_SIZE];
    static bool haveInited = false;
    if (!haveInited) {
        haveInited = true;
        int i;
        int aPow2 = 1;
        for (i = 0; i < MAX_TEXTURE_SIZE; i++) {
            if (i > aPow2)
                aPow2 <<= 1;

            int aGoodValue = aPow2;
            if ((aGoodValue - i) > 64) {
                aGoodValue >>= 1;
                while (true) {
                    int aLeftOver = i % aGoodValue;
                    if (aLeftOver < 64 || IsPowerOf2(aLeftOver))
                        break;

                    aGoodValue >>= 1;
                }
            }
            aGoodTextureSize[i] = aGoodValue;
        }
    }

    int aWidth = theWidth;
    int aHeight = theHeight;

    if (usePow2) {
        if (isEdge || (theImageFlags & D3DImageFlag_MinimizeNumSubdivisions)) {
            aWidth = aWidth >= gMaxTextureWidth ? gMaxTextureWidth : GetClosestPowerOf2Above(aWidth);
            aHeight = aHeight >= gMaxTextureHeight ? gMaxTextureHeight : GetClosestPowerOf2Above(aHeight);
        } else {
            aWidth = aWidth >= gMaxTextureWidth ? gMaxTextureWidth : aGoodTextureSize[aWidth];
            aHeight = aHeight >= gMaxTextureHeight ? gMaxTextureHeight : aGoodTextureSize[aHeight];
        }
    }

    if (aWidth < gMinTextureWidth)
        aWidth = gMinTextureWidth;

    if (aHeight < gMinTextureHeight)
        aHeight = gMinTextureHeight;

    if (aWidth > aHeight) {
        while (aWidth > gMaxTextureAspectRatio * aHeight)
            aHeight <<= 1;
    } else if (aHeight > aWidth) {
        while (aHeight > gMaxTextureAspectRatio * aWidth)
            aWidth <<= 1;
    }

    theWidth = aWidth;
    theHeight = aHeight;
}

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
        //          if (glIsTexture(mTextures[i].mTexture) == GL_TRUE)
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
    if (mWidth != theImage->mWidth || mHeight != theImage->mHeight || theImage->mD3DFlags != mImageFlags) {
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
        for (x = 0; x < aWidth; x += mTexPieceWidth, i++) {
            TextureDataPiece &aPiece = mTextures[i];
            if (createTextures) {

                aPiece.mTexture = CreateTexture(theImage, x, y, aPiece.mWidth, aPiece.mHeight);

                if (aPiece.mTexture == 0) // create texture failure
                {
                    return;
                }

#if 0
                if (mPalette != NULL)
                    aPiece.mTexture->SetPalette(mPalette);
#endif

                mTexMemSize += aPiece.mWidth * aPiece.mHeight*aFormatSize;
            }
        }
    }

    mWidth = theImage->mWidth;
    mHeight = theImage->mHeight;
    mBitsChangedCount = theImage->mBitsChangedCount;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::CheckCreateTextures(MemoryImage *theImage)
{
    if (theImage->mWidth != mWidth || theImage->mHeight != mHeight || theImage->mBitsChangedCount != mBitsChangedCount || theImage->mD3DFlags != mImageFlags)
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
                {u1, v1, rgba, x,            y,           0},
                {u1, v2, rgba, x,            y + aHeight, 0},
                {u2, v1, rgba, x + aWidth,   y,           0},
                {u2, v2, rgba, x + aWidth,   y + aHeight, 0},
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


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VertexList {

    enum {
        MAX_STACK_VERTS = 100
    };
    D3DTLVERTEX mStackVerts[MAX_STACK_VERTS];
    D3DTLVERTEX *mVerts;
    int mSize;
    int mCapacity;

    typedef int size_type;

    VertexList() : mSize(0), mCapacity(MAX_STACK_VERTS), mVerts(mStackVerts)
    {
    }

    VertexList(const VertexList & theList) : mSize(theList.mSize), mCapacity(MAX_STACK_VERTS), mVerts(mStackVerts)
    {
        reserve(mSize);
        memcpy(mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
    }

    ~VertexList()
    {
        if (mVerts != mStackVerts)
            delete mVerts;
    }

    void reserve(int theCapacity)
    {
        if (mCapacity < theCapacity) {
            mCapacity = theCapacity;
            D3DTLVERTEX *aNewList = new D3DTLVERTEX[theCapacity];
            memcpy(aNewList, mVerts, mSize * sizeof (mVerts[0]));
            if (mVerts != mStackVerts)
                delete mVerts;

            mVerts = aNewList;
        }
    }

    void push_back(const D3DTLVERTEX & theVert)
    {
        if (mSize == mCapacity)
            reserve(mCapacity * 2);

        mVerts[mSize++] = theVert;
    }

    void operator=(const VertexList & theList)
    {
        reserve(theList.mSize);
        mSize = theList.mSize;
        memcpy(mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
    }

    D3DTLVERTEX & operator[](int thePos)
    {
        return mVerts[thePos];
    }

    int size()
    {
        return mSize;
    }

    void clear()
    {
        mSize = 0;
    }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline float GetCoord(const D3DTLVERTEX &theVertex, int theCoord)
{
    switch (theCoord) {
    case 0: return theVertex.sx;
    case 1: return theVertex.sy;
    case 2: return theVertex.sz;
    case 3: return theVertex.tu;
    case 4: return theVertex.tv;
    default: return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline D3DTLVERTEX Interpolate(const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, float t)
{
    D3DTLVERTEX aVertex = v1;
    aVertex.sx = v1.sx + t * (v2.sx - v1.sx);
    aVertex.sy = v1.sy + t * (v2.sy - v1.sy);
    aVertex.tu = v1.tu + t * (v2.tu - v1.tu);
    aVertex.tv = v1.tv + t * (v2.tv - v1.tv);
    Color c1(v1.color);
    Color c2(v2.color);
    if (c1 != c2) {
        // TODO ???? Don't we have to clip to 255?
        int r = c1.mRed   + (int) (t * (c2.mRed   - c1.mRed));
        int g = c1.mGreen + (int) (t * (c2.mGreen - c1.mGreen));
        int b = c1.mBlue  + (int) (t * (c2.mBlue  - c1.mBlue));
        int a = c1.mAlpha + (int) (t * (c2.mAlpha - c1.mAlpha));

        aVertex.color = Color(r, g, b, a).ToRGBA();
    }

    return aVertex;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

D3DInterface::D3DInterface()
{
#if 0
    mHWnd = NULL;
    mDD = NULL;
    mD3D = NULL;
    mD3DDevice = NULL;
#endif
    mWidth = 640;
    mHeight = 480;

    custom_cursor_texture = 0;

    mDDSDrawSurface = NULL;
    mZBuffer = NULL;

    mSceneBegun = false;
    mIsWindowed = true;

    gMinTextureWidth = 64;
    gMinTextureHeight = 64;
    gMaxTextureWidth = 64;
    gMaxTextureHeight = 64;
    gMaxTextureAspectRatio = 1;

    lastDrawMode = Graphics::DRAWMODE_NORMAL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

D3DInterface::~D3DInterface()
{
    Cleanup();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::UpdateViewport()
{
    /* Get available fullscreen/hardware modes */
    SDL_Rect **modes;

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

    if (gSexyAppBase->mIsWindowed) {
        glViewport(0, 0, gSexyAppBase->mCorrectedWidth, gSexyAppBase->mCorrectedHeight);
    } else {
        glViewport((modes[0]->w - gSexyAppBase->mCorrectedWidth) / 2, 0, gSexyAppBase->mCorrectedWidth, gSexyAppBase->mCorrectedHeight);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::InitD3D()
{

    GLint minimum_width = 1;
    GLint minimum_height = 1;

#if 0
    while (true) {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, minimum_width, minimum_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        GLint width = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        GLint height = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        if (width == 0)
            minimum_width <<= 1;

        if (height == 0)
            minimum_height <<= 1;

        if (width != 0 && height != 0)
            break;
    }
#endif

    //GLint try_width = minimum_width;
    //GLint try_height = minimum_height;

    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

#if 0
    while (true) {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, try_width << 1, try_height << 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        GLint width = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_MAX_TEXTURE_SIZE, &width);
        GLint height = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        if (width != 0)
            try_width <<= 1;

        if (height != 0)
            try_height <<= 1;

        if (width == 0 && height == 0)
            break;
    }
#endif

    gMaxTextureWidth = max_texture_size;
    gMaxTextureHeight = max_texture_size;
    //FIXME
    gMaxTextureAspectRatio = 1;

    if (gMaxTextureWidth > MAX_TEXTURE_SIZE)
        gMaxTextureWidth = MAX_TEXTURE_SIZE;
    if (gMaxTextureHeight > MAX_TEXTURE_SIZE)
        gMaxTextureHeight = MAX_TEXTURE_SIZE;

    if (gMaxTextureAspectRatio == 0)
        gMaxTextureAspectRatio = 65536;

#if 0
    if (gMinTextureWidth > gMaxTextureWidth)
        gMinTextureWidth = 64;
    if (gMinTextureHeight > gMaxTextureHeight)
        gMinTextureHeight = 64;
#endif

#if 0
    gSupportedPixelFormats = 0;
    mD3DDevice->EnumTextureFormats(PixelFormatsCallback, NULL);

    if (!(aCaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)) // need alpha in palettes
        gSupportedPixelFormats &= ~PixelFormat_Palette8;
#endif

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth (2.5);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_NORMALIZE);
    glDisable(GL_CULL_FACE);
    glShadeModel (GL_FLAT);

//   glReadBuffer(GL_BACK);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

//   glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
//   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glClear(GL_COLOR_BUFFER_BIT);
//   glDisable(GL_TEXTURE_GEN_S);
//   glDisable(GL_TEXTURE_GEN_T);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    UpdateViewport();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gSexyAppBase->mWidth, gSexyAppBase->mHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //create texture for mOldCursorArea
    glGenTextures(1, &custom_cursor_texture);
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    unsigned char tmp[64 * 64 * 4];
    memset(tmp, 0, 64 * 64 * 4);
    glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            64, 64,
            0,
            GL_BGRA,                    // TODO ???? Why do we want BGRA instead of RGBA?
            GL_UNSIGNED_BYTE,
            tmp);

    gLinearFilter = false;
    SetLinearFilter(true);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::InitFromDDInterface(DDInterface *theInterface)
{
    mErrorString.erase();
#if 0
    mDD = theInterface->mDD7;
    mHWnd = theInterface->mHWnd;
#endif
    mWidth = theInterface->mWidth;
    mHeight = theInterface->mHeight;

    //  mIsWindowed = true; //FIXME
    return InitD3D();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool gD3DInterfacePreDrawError = false;

bool D3DInterface::PreDraw()
{
    if (gSexyAppBase->mPhysMinimized)
        return false;

    if (!mSceneBegun) {
#if 0
        HRESULT hr;


        if (!SUCCEEDED(mD3DDevice->SetRenderTarget(mDDSDrawSurface, 0))) // this happens when there's been a mode switch (this caused the nvidia screensaver bluescreen)
        {
            gD3DInterfacePreDrawError = true;
            return false;
        } else
#endif
            gD3DInterfacePreDrawError = false;

#if 0
        hr = mD3DDevice->BeginScene();
#endif    

#if 0
        // alphablend states
        mD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

        // filter states
        mD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
        mD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        mD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFG_POINT);
        mD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        mD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        hr = mD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

        // Setup non-texture render states
        mD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
        hr = mD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
#endif
        mSceneBegun = true;
        //gLinearFilter = false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Pred>
struct PointClipper {
    Pred mPred;

    void ClipPoint(int n, float clipVal, const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, VertexList & out);
    void ClipPoints(int n, float clipVal, VertexList &in, VertexList & out);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Pred>
void PointClipper<Pred>::ClipPoint(int n, float clipVal, const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, VertexList &out)
{
    if (!mPred(GetCoord(v1, n), clipVal)) {
        if (!mPred(GetCoord(v2, n), clipVal)) // both inside
            out.push_back(v2);
        else // inside -> outside
        {
            float t = (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n));
            out.push_back(Interpolate(v1, v2, t));
        }
    } else {
        if (!mPred(GetCoord(v2, n), clipVal)) // outside -> inside
        {
            float t = (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n));
            out.push_back(Interpolate(v1, v2, t));
            out.push_back(v2);
        }
        //      else // outside -> outside
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Pred>
void PointClipper<Pred>::ClipPoints(int n, float clipVal, VertexList &in, VertexList &out)
{
    if (in.size() < 2)
        return;

    ClipPoint(n, clipVal, in[in.size() - 1], in[0], out);
    for (VertexList::size_type i = 0; i < in.size() - 1; i++)
        ClipPoint(n, clipVal, in[i], in[i + 1], out);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DrawPolyClipped(const Rect *theClipRect, const VertexList &theList)
{
    VertexList l1, l2;
    l1 = theList;

    int left = theClipRect->mX;
    int right = left + theClipRect->mWidth;
    int top = theClipRect->mY;
    int bottom = top + theClipRect->mHeight;

    VertexList *in = &l1, *out = &l2;
    PointClipper<std::less<float> > aLessClipper;
    PointClipper<std::greater_equal<float> > aGreaterClipper;

    aLessClipper.ClipPoints(0, left, *in, *out);
    std::swap(in, out);
    out->clear();
    aLessClipper.ClipPoints(1, top, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(0, right, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(1, bottom, *in, *out);

    VertexList &aList = *out;

    if (aList.size() >= 3) {
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].sx));
        glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].tu));
        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DoPolyTextureClip(VertexList &theList)
{
    VertexList l2;

    float left = 0;
    float right = 1;
    float top = 0;
    float bottom = 1;

    VertexList *in = &theList, *out = &l2;
    PointClipper<std::less<float> > aLessClipper;
    PointClipper<std::greater_equal<float> > aGreaterClipper;

    aLessClipper.ClipPoints(3, left, *in, *out);
    std::swap(in, out);
    out->clear();
    aLessClipper.ClipPoints(4, top, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(3, right, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(4, bottom, *in, *out);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor, const Rect *theClipRect, float theX, float theY, bool center)
{
    int srcLeft = theSrcRect.mX;
    int srcTop = theSrcRect.mY;
    int srcRight = srcLeft + theSrcRect.mWidth;
    int srcBottom = srcTop + theSrcRect.mHeight;
    int srcX, srcY;
    float dstX, dstY;
    int aWidth;
    int aHeight;
    float u1, v1, u2, v2;
    float startx = 0, starty = 0;
    float pixelcorrect = 0.0f; //0.5f;

    if (center) {
        startx = -theSrcRect.mWidth / 2.0f;
        starty = -theSrcRect.mHeight / 2.0f;
        pixelcorrect = 0.0f;
    }

    srcY = srcTop;
    dstY = starty;

    if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
        return;

    SexyRGBA rgba = theColor.ToRGBA();

    while (srcY < srcBottom) {
        srcX = srcLeft;
        dstX = startx;
        while (srcX < srcRight) {
            aWidth = srcRight - srcX;
            aHeight = srcBottom - srcY;
            GLuint aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

            float x = dstX; // - pixelcorrect; // - 0.5f; //FIXME correct??
            float y = dstY; // - pixelcorrect; // - 0.5f;

            SexyVector2 p[4] = {SexyVector2(x, y), SexyVector2(x, y + aHeight), SexyVector2(x + aWidth, y), SexyVector2(x + aWidth, y + aHeight)};
            SexyVector2 tp[4];

            int i;
            for (i = 0; i < 4; i++) {
                tp[i] = theTrans * p[i];
                tp[i].x -= pixelcorrect - theX;
                tp[i].y -= pixelcorrect - theY;
            }

            bool clipped = false;
            if (theClipRect != NULL) {
                int left = theClipRect->mX;
                int right = left + theClipRect->mWidth;
                int top = theClipRect->mY;
                int bottom = top + theClipRect->mHeight;
                for (i = 0; i < 4; i++) {
                    if (tp[i].x < left || tp[i].x >= right || tp[i].y < top || tp[i].y >= bottom) {
                        clipped = true;
                        break;
                    }
                }
            }

            glBindTexture(GL_TEXTURE_2D, aTexture);

            if (!clipped) {
                D3DTLVERTEX aVertex[4] =
                {
                    { u1,v1,rgba, tp[0].x,          tp[0].y,            0},
                    { u1,v2,rgba, tp[1].x,          tp[1].y,            0},
                    { u2,v1,rgba, tp[2].x,          tp[2].y,            0},
                    { u2,v2,rgba, tp[3].x,          tp[3].y,            0},
                };


                glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
                glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
                glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].tu));
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            } else {
                VertexList aList;

                D3DTLVERTEX vertex0 = {(GLfloat) u1, (GLfloat) v1, rgba, (GLfloat) tp[0].x, (GLfloat) tp[0].y};
                D3DTLVERTEX vertex1 = {(GLfloat) u1, (GLfloat) v2, rgba, (GLfloat) tp[1].x, (GLfloat) tp[1].y};
                D3DTLVERTEX vertex2 = {(GLfloat) u2, (GLfloat) v1, rgba, (GLfloat) tp[2].x, (GLfloat) tp[2].y};
                D3DTLVERTEX vertex3 = {(GLfloat) u2, (GLfloat) v2, rgba, (GLfloat) tp[3].x, (GLfloat) tp[3].y};

                aList.push_back(vertex0);
                aList.push_back(vertex1);
                aList.push_back(vertex3);
                aList.push_back(vertex2);

                DrawPolyClipped(theClipRect, aList);
            }
            srcX += aWidth;
            dstX += aWidth;

        }
        srcY += aHeight;
        dstY += aHeight;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color?theVertex.color:theColor)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, Uint32 theColor, float tx, float ty)
{
    if ((mMaxTotalU <= 1.0) && (mMaxTotalV <= 1.0)) {
        glBindTexture(GL_TEXTURE_2D, mTextures[0].mTexture);

        D3DTLVERTEX aVertexCache[300];
        int aVertexCacheNum = 0;

        glTexCoordPointer(2, GL_FLOAT, sizeof (D3DTLVERTEX), aVertexCache);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof (D3DTLVERTEX), &(aVertexCache[0].color));
        glVertexPointer(3, GL_FLOAT, sizeof (D3DTLVERTEX), &(aVertexCache[0].sx));

        for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++) {
            Color col;
            TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];
            D3DTLVERTEX* aD3DVertex = &aVertexCache[aVertexCacheNum];
            aVertexCacheNum += 3;

            aD3DVertex[0].sx = aTriVerts[0].x + tx;
            aD3DVertex[0].sy = aTriVerts[0].y + ty;
            aD3DVertex[0].sz = 0;
            //    aD3DVertex[0].rhw = 1;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[0].color = col.ToRGBA();
            //aD3DVertex[0].specular = 0;
            aD3DVertex[0].tu = aTriVerts[0].u * mMaxTotalU;
            aD3DVertex[0].tv = aTriVerts[0].v * mMaxTotalV;

            aD3DVertex[1].sx = aTriVerts[1].x + tx;
            aD3DVertex[1].sy = aTriVerts[1].y + ty;
            aD3DVertex[1].sz = 0;
            //aD3DVertex[1].rhw = 1;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[1].color = col.ToRGBA();
            //aD3DVertex[1].specular = 0;
            aD3DVertex[1].tu = aTriVerts[1].u * mMaxTotalU;
            aD3DVertex[1].tv = aTriVerts[1].v * mMaxTotalV;

            aD3DVertex[2].sx = aTriVerts[2].x + tx;
            aD3DVertex[2].sy = aTriVerts[2].y + ty;
            aD3DVertex[2].sz = 0;
            //aD3DVertex[2].rhw = 1;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[2].color = col.ToRGBA();
            //aD3DVertex[2].specular = 0;
            aD3DVertex[2].tu = aTriVerts[2].u * mMaxTotalU;
            aD3DVertex[2].tv = aTriVerts[2].v * mMaxTotalV;

            if ((aVertexCacheNum == 300) || (aTriangleNum == theNumTriangles - 1)) {


                glDrawArrays(GL_TRIANGLES, 0, aVertexCacheNum);
                aVertexCacheNum = 0;
            }
        }
    } else {
        for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++) {
            TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];

            D3DTLVERTEX aVertex[3];
            Color col = GetColorFromTriVertex(aTriVerts[0], theColor);

            D3DTLVERTEX vertex1 = {(GLfloat) (aTriVerts[0].u * mMaxTotalU), (GLfloat) (aTriVerts[0].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[0].x + tx, aTriVerts[0].u + ty, 0.0f};

            col = GetColorFromTriVertex(aTriVerts[1], theColor);

            D3DTLVERTEX vertex2 = {(GLfloat) (aTriVerts[1].u * mMaxTotalU), (GLfloat) (aTriVerts[1].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[1].x + tx, aTriVerts[1].u + ty, 0.0f};
            col = GetColorFromTriVertex(aTriVerts[2], theColor);

            D3DTLVERTEX vertex3 = {(GLfloat) (aTriVerts[2].u * mMaxTotalU), (GLfloat) (aTriVerts[2].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[2].x + tx, aTriVerts[2].u + ty, 0.0f};

            aVertex[0] = vertex1;
            aVertex[1] = vertex2;
            aVertex[2] = vertex3;

            float aMinU = mMaxTotalU, aMinV = mMaxTotalV;
            float aMaxU = 0, aMaxV = 0;

            int i, j, k;
            for (i = 0; i < 3; i++) {
                if (aVertex[i].tu < aMinU)
                    aMinU = aVertex[i].tu;

                if (aVertex[i].tv < aMinV)
                    aMinV = aVertex[i].tv;

                if (aVertex[i].tu > aMaxU)
                    aMaxU = aVertex[i].tu;

                if (aVertex[i].tv > aMaxV)
                    aMaxV = aVertex[i].tv;
            }

            VertexList aMasterList;
            aMasterList.push_back(aVertex[0]);
            aMasterList.push_back(aVertex[1]);
            aMasterList.push_back(aVertex[2]);

            VertexList aList;

            int aLeft = (int) floorf(aMinU);
            int aTop = (int) floorf(aMinV);
            int aRight = (int) ceilf(aMaxU);
            int aBottom = (int) ceilf(aMaxV);
            if (aLeft < 0)
                aLeft = 0;
            if (aTop < 0)
                aTop = 0;
            if (aRight > mTexVecWidth)
                aRight = mTexVecWidth;
            if (aBottom > mTexVecHeight)
                aBottom = mTexVecHeight;

            TextureDataPiece &aStandardPiece = mTextures[0];
            for (i = aTop; i < aBottom; i++) {
                for (j = aLeft; j < aRight; j++) {
                    TextureDataPiece &aPiece = mTextures[i * mTexVecWidth + j];


                    VertexList aList = aMasterList;
                    for (k = 0; k < 3; k++) {
                        aList[k].tu -= j;
                        aList[k].tv -= i;
                        if (i == mTexVecHeight - 1)
                            aList[k].tv *= (float) aStandardPiece.mHeight / aPiece.mHeight;
                        if (j == mTexVecWidth - 1)
                            aList[k].tu *= (float) aStandardPiece.mWidth / aPiece.mWidth;
                    }

                    DoPolyTextureClip(aList);
                    if (aList.size() >= 3) {
                        glBindTexture(GL_TEXTURE_2D, aPiece.mTexture);
                        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
                        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].sx));
                        glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].tu));
                        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::CreateImageTexture(MemoryImage *theImage)
{
    bool wantPurge = false;

    if (theImage->mD3DData == NULL) {
        theImage->mD3DData = new TextureData();

        // The actual purging was deferred
        wantPurge = theImage->mPurgeBits;

#if 0
        AutoCrit aCrit(gSexyAppBase->mDDInterface->mCritSect); // Make images thread safe
#endif
        mImageSet.insert(theImage);
    }

    TextureData *aData = (TextureData*) theImage->mD3DData;
    aData->CheckCreateTextures(theImage);

    if (wantPurge)
        theImage->PurgeBits();

    //FIXME
    return true; //aData->mPixelFormat != PixelFormat_Unknown;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::RecoverBits(MemoryImage* theImage)
{
#if 0
    if (theImage->mD3DData == NULL)
        return false;

    TextureData* aData = (TextureData*) theImage->mD3DData;
    if (aData->mBitsChangedCount != theImage->mBitsChangedCount) // bits have changed since texture was created
        return false;

    for (int aPieceRow = 0; aPieceRow < aData->mTexVecHeight; aPieceRow++) {
        for (int aPieceCol = 0; aPieceCol < aData->mTexVecWidth; aPieceCol++) {
            TextureDataPiece* aPiece = &aData->mTextures[aPieceRow * aData->mTexVecWidth + aPieceCol];

            int offx = aPieceCol * aData->mTexPieceWidth;
            int offy = aPieceRow * aData->mTexPieceHeight;
            int aWidth = std::min(theImage->mWidth - offx, aPiece->mWidth);
            int aHeight = std::min(theImage->mHeight - offy, aPiece->mHeight);

            CopySurface8888ToImage(aPiece->mTexture, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight);
            break;
        case PixelFormat_A4R4G4B4:
            CopyTexture4444ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight);
            break;
        case PixelFormat_R5G6B5:
            CopyTexture565ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight);
            break;
        case PixelFormat_Palette8:
            CopyTexturePalette8ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight, aData->mPalette);
            break;
        }


    }

#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::SetCurTexture(MemoryImage *theImage)
{
    if (theImage == NULL) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    if (!CreateImageTexture(theImage))
        return;

    TextureData *aData = (TextureData*) theImage->mD3DData;

    glBindTexture(GL_TEXTURE_2D, aData->mTextures[0].mTexture);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::PushTransform(const SexyMatrix3 &theTransform, bool concatenate)
{
    if (mTransformStack.empty() || !concatenate)
        mTransformStack.push_back(theTransform);
    else {
        SexyMatrix3 &aTrans = mTransformStack.back();
        mTransformStack.push_back(theTransform * aTrans);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::PopTransform()
{
    if (!mTransformStack.empty())
        mTransformStack.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::RemoveMemoryImage(MemoryImage *theImage)
{
    if (theImage->mD3DData != NULL) {
        delete (TextureData*) theImage->mD3DData;
        theImage->mD3DData = NULL;
#if 0
        AutoCrit aCrit(gSexyAppBase->mDDInterface->mCritSect); // Make images thread safe
#endif
        mImageSet.erase(theImage);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::Cleanup()
{
    Flush();

    ImageSet::iterator anItr;
    for (anItr = mImageSet.begin(); anItr != mImageSet.end(); ++anItr) {
        MemoryImage *anImage = *anItr;
        delete (TextureData*) anImage->mD3DData;
        anImage->mD3DData = NULL;
    }

    mImageSet.clear();

#if 0
    if (mD3DDevice != NULL) {
        mD3DDevice->Release();
        mD3DDevice = NULL;
    }

    if (mD3D != NULL) {
        mD3D->Release();
        mD3D = NULL;
    }
#endif
    if (mDDSDrawSurface != NULL) {
        SDL_FreeSurface(mDDSDrawSurface);
        mDDSDrawSurface = NULL;
    }

    if (mZBuffer != NULL) {
        SDL_FreeSurface(mZBuffer);
        mZBuffer = NULL;
    }

    //if (glIsTexture(custom_cursor_texture) == GL_TRUE)
    //    glDeleteTextures(1, &custom_cursor_texture);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::SetupDrawMode(int theDrawMode, const Color &theColor, Image *theImage)
{
    if (lastDrawMode == theDrawMode)
        return;
    if (theDrawMode == Graphics::DRAWMODE_NORMAL) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else // Additive
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    lastDrawMode = theDrawMode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::Blt(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
    if (!mTransformStack.empty()) {
        BltClipF(theImage, theX, theY, theSrcRect, NULL, theColor, theDrawMode);
        return;
    }

    if (!PreDraw())
        return;

    MemoryImage* aSrcMemoryImage = (MemoryImage*) theImage;

    if (!CreateImageTexture(aSrcMemoryImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theImage);

    TextureData *aData = (TextureData*) aSrcMemoryImage->mD3DData;

    //SetLinearFilter(linearFilter);
    //SetLinearFilter(true);

    aData->Blt(theX, theY, theSrcRect, theColor);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
    SexyTransform2D aTransform;

    aTransform.Translate(-theSrcRect.mWidth, 0);
    aTransform.Scale(-1, 1);
    aTransform.Translate(theX, theY);

    BltTransformed(theImage, NULL, theColor, theDrawMode, theSrcRect, aTransform, linearFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode)
{
    SexyTransform2D aTransform;
    aTransform.Translate(theX, theY);

    BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, aTransform, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror)
{
    float xScale = (float) theDestRect.mWidth / theSrcRect.mWidth;
    float yScale = (float) theDestRect.mHeight / theSrcRect.mHeight;

    SexyTransform2D aTransform;
    if (mirror) {
        aTransform.Translate(-theSrcRect.mWidth, 0);
        aTransform.Scale(-xScale, yScale);
    } else
        aTransform.Scale(xScale, yScale);

    aTransform.Translate(theDestRect.mX, theDestRect.mY);
    BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, aTransform, !fastStretch);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltRotated(Image* theImage, float theX, float theY, const Rect* theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect &theSrcRect)
{
    SexyTransform2D aTransform;

    aTransform.Translate(-theRotCenterX, -theRotCenterY);
    aTransform.RotateRad(theRot);
    aTransform.Translate(theX + theRotCenterX, theY + theRotCenterY);

    BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, aTransform, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center)
{
    if (!PreDraw())
        return;

    MemoryImage* aSrcMemoryImage = (MemoryImage*) theImage;

    if (!CreateImageTexture(aSrcMemoryImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theImage);

    TextureData *aData = (TextureData*) aSrcMemoryImage->mD3DData;

    //SetLinearFilter(true); // force linear filtering in the case of a global transform

    if (!mTransformStack.empty()) {

        if (theX != 0 || theY != 0) {
            SexyTransform2D aTransform;
            if (center)
                aTransform.Translate(-theSrcRect.mWidth / 2.0f, -theSrcRect.mHeight / 2.0f);

            aTransform = theTransform * aTransform;
            aTransform.Translate(theX, theY);
            aTransform = mTransformStack.back() * aTransform;

            aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect);
        } else {
            SexyTransform2D aTransform = mTransformStack.back() * theTransform;
            aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
        }
    } else {
        //SetLinearFilter(linearFilter);

        aData->BltTransformed(theTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);

    float x1, y1, x2, y2;
    SexyRGBA aColor = theColor.ToRGBA();

    if (!mTransformStack.empty()) {
        SexyVector2 p1(theStartX, theStartY);
        SexyVector2 p2(theEndX, theEndY);
        p1 = mTransformStack.back() * p1;
        p2 = mTransformStack.back() * p2;

        x1 = p1.x;
        y1 = p1.y;
        x2 = p2.x;
        y2 = p2.y;
    } else {
        x1 = theStartX;
        y1 = theStartY;
        x2 = theEndX;
        y2 = theEndY;
    }

    D3DTLVERTEX aVertex[2] =
    {
        { 0, 0, aColor, x1, y1, 0},
        { 0, 0, aColor, x2, y2, 0}
    };

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_LINE_STRIP, 0, 2);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);

    SexyRGBA aColor = theColor.ToRGBA();

    float x = theRect.mX; // - 0.5f;
    float y = theRect.mY; // - 0.5f;
    float aWidth = theRect.mWidth;
    float aHeight = theRect.mHeight;

    D3DTLVERTEX aVertex[4] ={
        { 0, 0, aColor, x,          y,           0},
        { 0, 0, aColor, x,          y + aHeight, 0},
        { 0, 0, aColor, x + aWidth, y,           0},
        { 0, 0, aColor, x + aWidth, y + aHeight, 0}
    };

    if (!mTransformStack.empty()) {
        SexyVector2 p[4] = {SexyVector2(x, y), SexyVector2(x, y + aHeight), SexyVector2(x + aWidth, y), SexyVector2(x + aWidth, y + aHeight)};

        int i;
        for (i = 0; i < 4; i++) {
            p[i] = mTransformStack.back() * p[i];
            //      p[i].x -= 0.5f;
            //p[i].y -= 0.5f;
            aVertex[i].sx = p[i].x;
            aVertex[i].sy = p[i].y;
        }
    }

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode)
{
    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);
    Color aColor1 = GetColorFromTriVertex(p1, theColor);
    Color aColor2 = GetColorFromTriVertex(p2, theColor);
    Color aColor3 = GetColorFromTriVertex(p3, theColor);

    SexyRGBA aRGBA1 = aColor1.ToRGBA();
    SexyRGBA aRGBA2 = aColor2.ToRGBA();
    SexyRGBA aRGBA3 = aColor3.ToRGBA();

    D3DTLVERTEX aVertex[3] ={
        { 0, 0, aRGBA1, p1.x, p1.y, 0},
        { 0, 0, aRGBA2, p2.x, p2.y, 0},
        { 0, 0, aRGBA3, p3.x, p3.y, 0}
    };


    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
    if (theNumVertices < 3)
        return;

    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);
    SexyRGBA aColor = theColor.ToRGBA();

    VertexList aList;
    for (int i = 0; i < theNumVertices; i++) {
        D3DTLVERTEX vert = {0, 0, aColor, theVertices[i].mX + tx, theVertices[i].mY + ty, 0};
        if (!mTransformStack.empty()) {
            SexyVector2 v(vert.sx, vert.sy);
            v = mTransformStack.back() * v;
            vert.sx = v.x;
            vert.sy = v.y;
        }

        aList.push_back(vert);
    }

    if (theClipRect != NULL)
        DrawPolyClipped(theClipRect, aList);
    else {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].sx));
        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());

        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend)
{
    TriVertex aVertices[1][3] = {
        {p1, p2, p3}};
    DrawTrianglesTex(aVertices, 1, theColor, theDrawMode, theTexture, blend);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
    if (!PreDraw())
        return;

    MemoryImage* aSrcMemoryImage = (MemoryImage*) theTexture;

    if (!CreateImageTexture(aSrcMemoryImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theTexture);

    TextureData *aData = (TextureData*) aSrcMemoryImage->mD3DData;

    //SetLinearFilter(blend);
    //SetLinearFilter(true);

    aData->BltTriangles(theVertices, theNumTriangles, (Uint32) theColor.ToInt(), tx, ty);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
    TriVertex aList[100][3];
    int aTriNum = 0;
    while (aTriNum < theNumTriangles) {
        int aMaxTriangles = std::min(100, theNumTriangles - aTriNum);
        for (int i = 0; i < aMaxTriangles; i++) {
            aList[i][0] = theVertices[aTriNum];
            aList[i][1] = theVertices[aTriNum + 1];
            aList[i][2] = theVertices[aTriNum + 2];
            aTriNum++;
        }
        DrawTrianglesTex(aList, aMaxTriangles, theColor, theDrawMode, theTexture, tx, ty, blend);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::Flush()
{
    if (mSceneBegun) {
#if 0
        mD3DDevice->EndScene();
#endif
        mSceneBegun = false;
        mErrorString.erase();
    }
}
