#ifndef __DDIMAGE_H__
#define __DDIMAGE_H__

#include "MemoryImage.h"
#include <SDL.h>



namespace Sexy
{

class DDInterface;
#if 0
class SysFont;
#endif

class DDImage : public MemoryImage
{
protected:
#if 0
    friend class            SysFont;
#endif
    void                    DeleteAllNonSurfaceData();

public:
    DDInterface*            mDDInterface;
    SDL_Surface*            mSurface;
#if 0

    DDSURFACEDESC           mLockedSurfaceDesc;
#endif
    bool                    mSurfaceSet;
    bool                    mNoLock;
    bool                    mVideoMemory;
    bool                    mFirstPixelTrans;
    bool                    mWantDDSurface;
    bool                    mDrawToBits;

    int                     mLockCount;


private:
    void                    Init();

public:
    virtual void            ReInit();
    virtual void            SetVideoMemory(bool wantVideoMemory);

    SDL_Surface*        GetSurface();
    bool                    GenerateDDSurface();
    void                    DeleteDDSurface();

#if 0
    virtual void            RehupFirstPixelTrans();


#endif
    virtual void            BitsChanged();
    virtual void            CommitBits();

    virtual void            NormalFillRect(const Rect& theRect, const Color& theColor);
    virtual void            AdditiveFillRect(const Rect& theRect, const Color& theColor);
    virtual void            NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
    virtual void            AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
    virtual void            NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
    virtual void            AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
    virtual void            NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
    virtual void            AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);


    virtual void            NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
    virtual void            AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);

    virtual void            FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const unsigned char* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight);

    static bool             Check3D(DDImage *theImage);
    static bool             Check3D(Image *theImage);

public:
    DDImage();
    DDImage(DDInterface* theDDInterface);
    virtual ~DDImage();

    virtual bool            LockSurface();
    virtual bool            UnlockSurface();

    virtual void            SetSurface(SDL_Surface* theSurface);

    virtual void            Create(int theWidth, int theHeight);
    virtual uint32_t*           GetBits();

    virtual bool            PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool comvex);
    virtual void            FillRect(const Rect& theRect, const Color& theColor, int theDrawMode);
    virtual void            DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
    virtual void            DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
    virtual void            Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
    virtual void            BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode);
    virtual void            BltRotated(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY);
    virtual void            StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);
    virtual void            BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend);
    virtual void            BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend);

    virtual void            BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
    virtual void            StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);

    virtual bool            Palletize();
    virtual void            DoPurgeBits();
    virtual void            DeleteNativeData();
    virtual void            DeleteExtraBuffers();
};

}

#endif //__DDIMAGE_H__

