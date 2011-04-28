#ifndef __MEMORYIMAGE_H__
#define __MEMORYIMAGE_H__

#include <string>
#include "Common.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif
#include "Image.h"

#define OPTIMIZE_SOFTWARE_DRAWING

namespace Sexy
{

const uint32_t MEMORYCHECK_ID = 0x4BEEFADE;

class NativeDisplay;
class SexyAppBase;

class MemoryImage : public Image
{
public:
    SexyAppBase*            mApp;

    uint32_t*               mBits;

    uint32_t*               mColorTable;
    uchar*                  mColorIndices;

    bool                    mForcedMode;
    bool                    mHasTrans;
    bool                    mHasAlpha;
    bool                    mIsVolatile;
    bool                    mBitsChanged;
    bool                    mWantPal;

    uint32_t*               mNativeAlphaData;
    uchar*                  mRLAlphaData;
    uchar*                  mRLAdditiveData;

protected:
    bool                    mOptimizeSoftwareDrawing;

private:
    bool                    mPurgeBits;
    int                     mBitsChangedCount;
    void                    Init();

public:
    virtual void*           GetNativeAlphaData(NativeDisplay *theNative);
    virtual uchar*          GetRLAlphaData();
    virtual uchar*          GetRLAdditiveData(NativeDisplay *theNative);
    virtual void            DoPurgeBits();
    virtual void            SetPurgeBits(bool x) { mPurgeBits = x; }
    virtual bool            GetPurgeBits() const { return mPurgeBits; }
    virtual void            DeleteSWBuffers();
    virtual void            DeleteNativeData();
    virtual void            ReInit();

    void                    NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
    void                    AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);

    void                    NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
    void                    AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);

    void                    NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
    void                    AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);

    void                    SlowStretchBlt(Image* theImage, const Rect& theDestRect, const FRect& theSrcRect, const Color& theColor, int theDrawMode);
    void                    FastStretchBlt(Image* theImage, const Rect& theDestRect, const FRect& theSrcRect, const Color& theColor, int theDrawMode);
    bool                    BltRotatedClipHelper(float &theX, float &theY, const Rect &theSrcRect, const Rect &theClipRect, double theRot, FRect &theDestRect, float theRotCenterX, float theRotCenterY);
    bool                    StretchBltClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect, FRect &theSrcRectOut, Rect &theDestRectOut);
    bool                    StretchBltMirrorClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect, FRect &theSrcRectOut, Rect &theDestRectOut);
    void                    BltMatrixHelper(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, void *theSurface, int theBytePitch, int thePixelFormat, bool blend);
    void                    BltTrianglesTexHelper(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect &theClipRect, const Color &theColor, int theDrawMode, void *theSurface, int theBytePitch, int thePixelFormat, float tx, float ty, bool blend);

    void                    FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode,
                                                      const unsigned char* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight);


public:
    MemoryImage();
    MemoryImage(SexyAppBase* theApp);
    MemoryImage(const MemoryImage& theMemoryImage);
    virtual ~MemoryImage();

    virtual void            Clear();
    virtual void            SetBits(uint32_t* theBits, int theWidth, int theHeight, bool commitBits = true);
    virtual void            Create(int theWidth, int theHeight);
    virtual uint32_t*       GetBits();
    virtual int             GetBitsChangedCount() const { return mBitsChangedCount; }
    virtual void            BumpBitsChangedCount() { mBitsChangedCount++; }
    virtual void            BitsChanged();
    bool                    RecoverBits();
    virtual void            CommitBits();

    virtual void            FillRect(const Rect& theRect, const Color& theColor, int theDrawMode);
    virtual void            ClearRect(const Rect& theRect);
    virtual void            DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
    virtual void            DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);

    virtual void            Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
    virtual void            BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode);
    virtual void            BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY);
    virtual void            StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);
    virtual void            BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend);
    virtual void            BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend);

    virtual void            SetImageMode(bool hasTrans, bool hasAlpha);
    virtual void            SetVolatile(bool isVolatile);

    virtual bool            Palletize();

    virtual MemoryImage*    CreateImageFrom(int offx, int offy, int theWidth, int theHeight);
    virtual void            CopyImageToSurface(SDL_Surface* surface, int offx, int offy, int theWidth, int theHeight);
    virtual void            CopyImageToSurface8888(void *theDest, Uint32 theDestPitch, int offx, int offy, int theWidth, int theHeight, bool rightPad);
    virtual void            SaveImageToBMP(const std::string& filename, const std::string& path);
    virtual void            SaveImageToPNG(const std::string& filename, const std::string& path);

    virtual GLuint          CreateTexture(int x, int y, int w, int h);
};

}

#endif //__MEMORYIMAGE_H__
