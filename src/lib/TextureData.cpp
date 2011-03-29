#include "TextureData.h"
#include "VertexList.h"
#include "Common.h"
#include "D3DInterface.h"
#include "TriVertex.h"
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
    mTextures.clear();
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
    mLast_color = Color::White;
    mColors[0] = mColors[1] = mColors[2] = mColors[3] = Color::White.ToRGBA();
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
    /**/

    if (theImage->GetNumberOfSubImages() == 0) {
        // Calculate inner piece sizes
        // The size of the "inner piece" is used to compute
        // the TexturePiece when given the X,Y coordinates
        mTexPieceWidth = aWidth;
        mTexPieceHeight = aHeight;
        bool usePow2 = true; //gTextureSizeMustBePow2 || mPixelFormat==PixelFormat_Palette8;
        GetBestTextureDimensions(mTexPieceWidth, mTexPieceHeight, false, usePow2, mImageFlags);

        // Allocate texture array for the pieces
        mTexVecWidth = (aWidth + mTexPieceWidth - 1) / mTexPieceWidth;
        mTexVecHeight = (aHeight + mTexPieceHeight - 1) / mTexPieceHeight;
        mTextures.resize(mTexVecWidth * mTexVecHeight);

        // Assign sizes to all the pieces, all being equal in size
        int y0 = 0;
        for (int j = 0; j < mTexVecHeight; j++) {
            int x0 = 0;
            for (int i = 0; i < mTexVecWidth; i++) {
                TextureDataPiece &aPiece = mTextures[j * mTexVecWidth + i];
                aPiece.mTexture = 0;
                aPiece.mWidth = mTexPieceWidth;
                aPiece.mHeight = mTexPieceHeight;
                aPiece.mX0 = x0;
                aPiece.mY0 = y0;
                aPiece.mX1 = x0 + mTexPieceWidth;
                aPiece.mY1 = y0 + mTexPieceWidth;
                x0 += mTexPieceWidth;
            }
            y0 += mTexPieceWidth;
        }
    }
    else {
        // Use the info from the sub-images
        // Use the size of the first sub image for the texture pieces
        MemoryImage * subimg = theImage->GetNthSubImage(0);
        mTexPieceWidth = subimg->GetWidth();
        mTexPieceHeight = subimg->GetHeight();

        // Allocate texture array for the pieces
        mTexVecWidth = (aWidth + mTexPieceWidth - 1) / mTexPieceWidth;
        mTexVecHeight = (aHeight + mTexPieceHeight - 1) / mTexPieceHeight;
        mTextures.resize(mTexVecWidth * mTexVecHeight);

        for (int i = 0; i < theImage->GetNumberOfSubImages(); i++) {
            subimg = theImage->GetNthSubImage(i);
            int x0 = subimg->GetX0();
            int y0 = subimg->GetY0();
            int tx = x0 / mTexPieceWidth;
            int ty = y0 / mTexPieceHeight;
            TextureDataPiece &aPiece = mTextures[ty * mTexVecWidth + tx];
            aPiece.mX0 = x0;
            aPiece.mY0 = y0;
            aPiece.mX1 = x0 + mTexPieceWidth;
            aPiece.mY1 = y0 + mTexPieceWidth;
            aPiece.mWidth = mTexPieceWidth;
            aPiece.mHeight = mTexPieceWidth;
        }
    }

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

void TextureData::CreateTextures(MemoryImage *theImage)
{
    theImage->DeleteSWBuffers(); // don't need these buffers for 3d drawing

    theImage->CommitBits();

    // Release texture if image size has changed
    // Why don't we check mBitsChangedCount?
    bool createNewTextures = false;
    if (mWidth != theImage->GetWidth() || mHeight != theImage->GetHeight()
            || theImage->mD3DFlags != mImageFlags) {
        ReleaseTextures();
        mImageFlags = theImage->mD3DFlags;
        CreateTextureDimensions(theImage);
        createNewTextures = true;
    }

    int i;

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

    (*GLExtensions::glGenBuffers_ptr)(1, &mVBO_colors);
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_colors);

    //allocate memory for vbo and fill it
    (*GLExtensions::glBufferData_ptr)(GL_ARRAY_BUFFER, sizeof(mColors), mColors, GL_DYNAMIC_DRAW);

    (*GLExtensions::glGenBuffers_ptr)(1, &mVBO_static);
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_static);

    //allocate memory for vbo
    (*GLExtensions::glBufferData_ptr)(GL_ARRAY_BUFFER, (int)mTextures.size()*sizeof(D3DTLVERTEX)*4, NULL, GL_STATIC_DRAW);

    SexyRGBA rgba = Color::White.ToRGBA();
    assert((int)mTextures.size() == mTexVecWidth * mTexVecHeight);
    i = 0;
    for (int y = 0; y < aHeight; y += mTexPieceHeight) {
        for (int x = 0; x < aWidth; x += mTexPieceWidth) {
            int tx = x / mTexPieceWidth;
            int ty = y / mTexPieceHeight;
            TextureDataPiece &aPiece = mTextures[i++];
            if (createNewTextures) {

                aPiece.mTexture = theImage->CreateTexture(x, y, aPiece.mWidth, aPiece.mHeight);
                if (aPiece.mTexture == 0) // create texture failure
                {
                    // TODO. This dies silently. Maybe assert(0) is better.
                    assert(0);
                    return;
                }

                D3DTLVERTEX aVertex[4] =
                    {
                        {0.0f, 0.0f, rgba, x,y           },
                        {0.0f, 1.0f, rgba, x,y + mTexPieceHeight },
                        {1.0f, 0.0f, rgba, x + mTexPieceWidth,y           },
                        {1.0f, 1.0f, rgba, x + mTexPieceWidth,y + mTexPieceHeight },
                    };
                (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_static);
                (GLExtensions::glBufferSubData_ptr)(GL_ARRAY_BUFFER, (ty * mTexVecWidth + tx) * sizeof(D3DTLVERTEX) * 4 , sizeof(aVertex), aVertex);
                aPiece.texture_offset = (ty * mTexVecWidth + tx) * sizeof(D3DTLVERTEX) * 4;
                aPiece.color_offset = aPiece.texture_offset + 2*sizeof(GLfloat);
                aPiece.vertex_offset = aPiece.color_offset + 4*sizeof(GLubyte);

#if 0                                           
                if (mPalette != NULL)
                    aPiece.mTexture->SetPalette(mPalette);
#endif

                // For diagnostic?
                mTexMemSize += aPiece.mWidth * aPiece.mHeight*aFormatSize;
            }
        }
    }

    //unbind buffer
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, 0);

    mWidth = theImage->GetWidth();
    mHeight = theImage->GetHeight();
    mBitsChangedCount = theImage->mBitsChangedCount;
}

void TextureData::CreateTexturesFromSubs(MemoryImage *theImage)
{
    if (mTextures.size() == 0) {
        CreateTextureDimensions(theImage);
    }
    mWidth = theImage->GetWidth();
    mHeight = theImage->GetHeight();

    (*GLExtensions::glGenBuffers_ptr)(1, &mVBO_colors);
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_colors);

    //allocate memory for vbo and fill it
    (*GLExtensions::glBufferData_ptr)(GL_ARRAY_BUFFER, sizeof(mColors), mColors, GL_DYNAMIC_DRAW);

    (*GLExtensions::glGenBuffers_ptr)(1, &mVBO_static);
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_static);

    //allocate memory for vbo
    (*GLExtensions::glBufferData_ptr)(GL_ARRAY_BUFFER, (int)mTextures.size()*sizeof(D3DTLVERTEX)*4, NULL, GL_STATIC_DRAW);

    SexyRGBA rgba = Color::White.ToRGBA();
    for (int i = 0; i < theImage->GetNumberOfSubImages(); i++) {
        MemoryImage * subimg = theImage->GetNthSubImage(i);
        int x = subimg->GetX0();
        int y = subimg->GetY0();

        int tx = x / mTexPieceWidth;
        int ty = y / mTexPieceHeight;
        TextureDataPiece &aPiece = mTextures[ty * mTexVecWidth + tx];
        aPiece.mTexture = subimg->CreateTexture(0, 0, aPiece.mWidth, aPiece.mHeight);
        if (aPiece.mTexture == 0) // create texture failure
        {
            // TODO. This dies silently. Maybe assert(0) is better.
            assert(0);
            return;
        }
        // TODO.
        // We can drop the mBits

        D3DTLVERTEX aVertex[4] =
            {
                {0.0f, 0.0f, rgba, x,y           },
                {0.0f, 1.0f, rgba, x,y + mTexPieceHeight },
                {1.0f, 0.0f, rgba, x + mTexPieceWidth,y           },
                {1.0f, 1.0f, rgba, x + mTexPieceWidth,y + mTexPieceHeight },
            };
        (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_static);
        (GLExtensions::glBufferSubData_ptr)(GL_ARRAY_BUFFER, (ty * mTexVecWidth + tx) * sizeof(D3DTLVERTEX) * 4 , sizeof(aVertex), aVertex);
        aPiece.texture_offset = (ty * mTexVecWidth + tx) * sizeof(D3DTLVERTEX) * 4;
        aPiece.color_offset = aPiece.texture_offset + 2*sizeof(GLfloat);
        aPiece.vertex_offset = aPiece.color_offset + 4*sizeof(GLubyte);
    }
    mBitsChangedCount = theImage->mBitsChangedCount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::CheckCreateTextures(MemoryImage *theImage)
{
    if (theImage->GetWidth() != mWidth || theImage->GetHeight() != mHeight
            || theImage->mBitsChangedCount != mBitsChangedCount
            || theImage->mD3DFlags != mImageFlags) {
        if (theImage->GetNumberOfSubImages() > 0) {
            CreateTexturesFromSubs(theImage);
        }
        else {
            CreateTextures(theImage);
        }
    }
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

void TextureData::Blt(float theX, float theY, const Color& theColor)
{
    bool update_color = mLast_color != theColor;
        
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(theX, theY, 0);

    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_colors);

    if (update_color) {

        SexyRGBA rgba = theColor.ToRGBA();

        mColors[0] = mColors[1] = mColors[2] = mColors[3] = rgba;

        mLast_color = theColor;

        (GLExtensions::glBufferSubData_ptr)(GL_ARRAY_BUFFER, 0, sizeof(mColors), (GLvoid*)mColors);
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SexyRGBA), BUFFER_OFFSET(0));
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_static);

    for (unsigned int i = 0; i < mTextures.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i].mTexture);
        glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), BUFFER_OFFSET(mTextures[i].texture_offset));
        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), BUFFER_OFFSET(mTextures[i].vertex_offset));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    //unbind buffer
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, 0);

    glPopMatrix();
}

void TextureData::Blt(float theX, float theY)
{
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, mVBO_static);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(theX, theY, 0);

    for (unsigned int i = 0; i < mTextures.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i].mTexture);

        glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), BUFFER_OFFSET(mTextures[i].texture_offset));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), BUFFER_OFFSET(mTextures[i].color_offset));
        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), BUFFER_OFFSET(mTextures[i].vertex_offset));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glPopMatrix();

    //unbind buffer
    (*GLExtensions::glBindBuffer_ptr)(GL_ARRAY_BUFFER, 0);
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
                    { u1,v1,rgba, tp[0].x,          tp[0].y},
                    { u1,v2,rgba, tp[1].x,          tp[1].y},
                    { u2,v1,rgba, tp[2].x,          tp[2].y},
                    { u2,v2,rgba, tp[3].x,          tp[3].y},
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

                aList.DrawPolyClipped(theClipRect);
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
#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color ? theVertex.color : theColor)

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
        glVertexPointer(2, GL_FLOAT, sizeof (D3DTLVERTEX), &(aVertexCache[0].sx));

        for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++) {
            Color col;
            TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];
            D3DTLVERTEX* aD3DVertex = &aVertexCache[aVertexCacheNum];
            aVertexCacheNum += 3;

            aD3DVertex[0].sx = aTriVerts[0].x + tx;
            aD3DVertex[0].sy = aTriVerts[0].y + ty;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[0].color = col.ToRGBA();
            aD3DVertex[0].tu = aTriVerts[0].u * mMaxTotalU;
            aD3DVertex[0].tv = aTriVerts[0].v * mMaxTotalV;

            aD3DVertex[1].sx = aTriVerts[1].x + tx;
            aD3DVertex[1].sy = aTriVerts[1].y + ty;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[1].color = col.ToRGBA();
            aD3DVertex[1].tu = aTriVerts[1].u * mMaxTotalU;
            aD3DVertex[1].tv = aTriVerts[1].v * mMaxTotalV;

            aD3DVertex[2].sx = aTriVerts[2].x + tx;
            aD3DVertex[2].sy = aTriVerts[2].y + ty;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[2].color = col.ToRGBA();
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
                aTriVerts[0].x + tx, aTriVerts[0].u + ty};

            col = GetColorFromTriVertex(aTriVerts[1], theColor);

            D3DTLVERTEX vertex2 = {(GLfloat) (aTriVerts[1].u * mMaxTotalU), (GLfloat) (aTriVerts[1].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[1].x + tx, aTriVerts[1].u + ty};
            col = GetColorFromTriVertex(aTriVerts[2], theColor);

            D3DTLVERTEX vertex3 = {(GLfloat) (aTriVerts[2].u * mMaxTotalU), (GLfloat) (aTriVerts[2].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[2].x + tx, aTriVerts[2].u + ty};

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

                    aList.DoPolyTextureClip();
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
