#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <vector>
#include "Common.h"
#include "TextureData.h"
#include "Logging.h"
#include "Color.h"
#include "Rect.h"
#include "Point.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

namespace Sexy
{

struct Span
{
    int                     mY;
    int                     mX;
    int                     mWidth;
};

enum AnimType
{
    AnimType_None,
    AnimType_Once,
    AnimType_PingPong,
    AnimType_Loop
};

struct AnimInfo
{
    AnimType                mAnimType;
    int                     mFrameDelay; // 1/100s
    int                     mNumCels;
    std::vector<int>        mPerFrameDelay;
    std::vector<int>        mFrameMap;
    int                     mTotalAnimTime;

    AnimInfo();
    void SetPerFrameDelay(int theFrame, int theTime);
    void Compute(int theNumCels, int theBeginFrameTime = 0, int theEndFrameTime = 0);

    int GetPerFrameCel(int theTime);
    int GetCel(int theTime);
};

class Graphics;
class SexyMatrix3;
#if 0
class SysFont;
#endif
class TriVertex;

class Image
{
#if 0
    friend class            Sexy::SysFont;
#endif

public:
    bool                    mDrawn;
    std::string             mFilePath;

    // for animations
    AnimInfo                *mAnimInfo;

protected:
    int                     mWidth;
    int                     mHeight;
    int                     mX0;                // Use when this image is part of some other image
    int                     mY0;
    int                     mX1;
    int                     mY1;

    // for image strips
    int                     mNumRows;
    int                     mNumCols;

    LoggerFacil *           mLogFacil;

private:
    uint32_t                mD3DFlags;  // see D3DInterface.h for possible values, set in ResourceManager::DoLoadImage
    struct TextureData*     mD3DData;
    std::vector<Image*>     mSubImages;

public:
    Image();
    Image(const Image& theImage);
    virtual ~Image();

    int                     GetWidth() { return mWidth; }
    int                     GetHeight() { return mHeight; }
    int                     GetNumCols() { return mNumCols; }
    int                     GetNumRows() { return mNumRows; }
    void                    SetWidth(int w) { mWidth = w; }
    void                    SetHeight(int h) { mHeight = h; }
    void                    SetX0Y0(int x, int y) { mX0 = x; mY0 = y; }
    int                     GetX0() const { return mX0; }
    int                     GetY0() const { return mY0; }
    void                    SetX1Y1(int x, int y) { mX1 = x; mY1 = y; }
    int                     GetCelWidth() { return mWidth / mNumCols; }      // returns the width of just 1 cel in a strip of images
    int                     GetCelHeight() { return mHeight / mNumRows; } // like above but for vertical strips
    void                    SetNumRowsCols(int r, int c) { mNumRows = r; mNumCols = c; }
    int                     GetAnimCel(int theTime); // use animinfo to return appropriate cel to draw at the time
    Rect                    GetAnimCelRect(int theTime);
    Rect                    GetCelRect(int theCel);             // Gets the rectangle for the given cel at the specified row/col
    Rect                    GetCelRect(int theCol, int theRow); // Same as above, but for an image with both multiple rows and cols
    void                    CopyAttributes(Image *from);
    Graphics*               GetGraphics();

    virtual bool            PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex);
    virtual void            FillRect(const Rect& theRect, const Color& theColor, int theDrawMode);
    virtual void            DrawRect(const Rect& theRect, const Color& theColor, int theDrawMode);
    virtual void            ClearRect(const Rect& theRect);
    virtual void            DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
    virtual void            DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
    virtual void            FillScanLines(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode);
    virtual void            FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const unsigned char* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight);
    virtual void            Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
    virtual void            BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode);
    virtual void            BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY);
    virtual void            StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);
    virtual void            BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend);
    virtual void            BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend);

    virtual void            BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
    virtual void            StretchBltMirror(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);

    virtual void            DeleteSWBuffers();
    virtual void            Delete3DBuffers();
    virtual void            DeleteExtraBuffers();
    virtual void            DeleteNativeData();
    virtual void            ReInit();

    virtual GLuint          CreateTexture(int x, int y, int w, int h);

    uint32_t                GetD3DFlags() const { return mD3DFlags; }
    bool                    HasTextureData() const { return mD3DData != NULL; }
    void                    CreateTextureData();
    void                    DeleteTextureData();
    TextureData *           GetTextureData() const { return mD3DData; }
    void                    CheckCreateTextures();

    int                     GetNumberOfSubImages() const { return mSubImages.size(); }
    Image *                 GetNthSubImage(int nth) const { return mSubImages[nth]; }
    void                    AddSubImage(Image * subimage) { mSubImages.push_back(subimage); }

    virtual int             GetBitsChangedCount() const { return 0; }
    virtual void            DoPurgeBits() {/*dummy*/}
    virtual bool            GetPurgeBits() { return false; }
    virtual void            CommitBits() {/*dummy*/}
};

}

#endif //__IMAGE_H__

