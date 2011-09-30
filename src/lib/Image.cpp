#include <assert.h>
#include "Common.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

#include "SexyAppBase.h"
#include "Image.h"
#include "Graphics.h"

using namespace Sexy;

Image::Image()
{
    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("image");
    TLOG(mLogFacil, 1, "new Image()");
#endif

    mFilePath = "";
    mApp = gSexyAppBase;            // We need this just to be able to remove this from mImageSet

    mWidth = 0;
    mHeight = 0;

    mX0 = 0;
    mY0 = 0;
    mX1 = 0;
    mY1 = 0;

    mNumRows = 1;
    mNumCols = 1;

    mHasTrans = false;
    mHasAlpha = false;

    mAnimInfo = NULL;
    mDrawn = false;

    mD3DFlags = 0;
    mD3DData = NULL;
    mSubImages.clear();

    mPow2 = true;
    mSquare = true;
}

Image::Image(const Image& theImage) :
    mFilePath(theImage.mFilePath),
    mApp(theImage.mApp),
    mWidth(theImage.mWidth),
    mHeight(theImage.mHeight),
    mNumRows(theImage.mNumRows),
    mNumCols(theImage.mNumCols),
    mHasTrans(theImage.mHasTrans),
    mHasAlpha(theImage.mHasAlpha)
{
    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("image");
    TLOG(mLogFacil, 1, "new Image(const Image& theImage)");
    TLOG(mLogFacil, 1, Logger::format("theImage: w=%d h=%d", theImage.mWidth, theImage.mHeight));
#endif

    mX0 = 0;
    mY0 = 0;
    mX1 = 0;
    mY1 = 0;

    mDrawn = false;
    if (theImage.mAnimInfo != NULL)
        mAnimInfo = new AnimInfo(*theImage.mAnimInfo);
    else
        mAnimInfo = NULL;

    mD3DFlags = theImage.mD3DFlags;
    mD3DData = NULL;
    mSubImages.clear();
}

Image::~Image()
{
    TLOG(mLogFacil, 1, Logger::format("~Image() %p filepath='%s'", this, mFilePath.c_str()));
    if (mApp)
        mApp->RemoveImage(this);

    delete mAnimInfo;
}

Rect Image::GetCelRect(int theCel)
{
    int h = GetCelHeight();
    int w = GetCelWidth();
    int x = (theCel % mNumCols) * w;
    int y = (theCel / mNumCols) * h;

    return Rect(x, y, w, h);
}

Rect Image::GetCelRect(int theCol, int theRow)
{
    int h = GetCelHeight();
    int w = GetCelWidth();
    int x = theCol * w;
    int y = theRow * h;

    return Rect(x, y, w, h);
}

AnimInfo::AnimInfo()
{
    mAnimType = AnimType_None;
    mFrameDelay = 1;
    mNumCels = 1;
}

void AnimInfo::SetPerFrameDelay(int theFrame, int theTime)
{
    if ((int)mPerFrameDelay.size()<=theFrame)
        mPerFrameDelay.resize(theFrame+1);

    mPerFrameDelay[theFrame] = theTime;
}

void AnimInfo::Compute(int theNumCels, int theBeginFrameTime, int theEndFrameTime)
{
    int i;

    mNumCels = theNumCels;
    if (mNumCels<=0)
        mNumCels = 1;

    if (mFrameDelay<=0)
        mFrameDelay = 1;

    if (mAnimType==AnimType_PingPong && mNumCels>1)
    {
        mFrameMap.resize(theNumCels*2-2);
        int index = 0;
        for (i=0; i<theNumCels; i++)
            mFrameMap[index++] = i;
        for (i=theNumCels-2; i>=1; i--)
            mFrameMap[index++] = i;
    }

    if (!mFrameMap.empty())
        mNumCels = (int)mFrameMap.size();

    if (theBeginFrameTime>0)
        SetPerFrameDelay(0,theBeginFrameTime);

    if (theEndFrameTime>0)
        SetPerFrameDelay(mNumCels-1,theEndFrameTime);

    if (!mPerFrameDelay.empty())
    {
        mTotalAnimTime = 0;
        mPerFrameDelay.resize(mNumCels);

        for (i=0; i<mNumCels; i++)
        {
            if (mPerFrameDelay[i]<=0)
                mPerFrameDelay[i] = mFrameDelay;

            mTotalAnimTime += mPerFrameDelay[i];
        }
    }
    else
        mTotalAnimTime = mFrameDelay*mNumCels;

    if (!mFrameMap.empty())
        mFrameMap.resize(mNumCels);
}

int AnimInfo::GetPerFrameCel(int theTime)
{
    for (int i=0; i<mNumCels; i++)
    {
        theTime -= mPerFrameDelay[i];
        if (theTime<0)
            return i;
    }

    return mNumCels-1;
}



int AnimInfo::GetCel(int theTime)
{
    if (mAnimType==AnimType_Once && theTime>=mTotalAnimTime)
    {
        if (!mFrameMap.empty())
            return mFrameMap[mFrameMap.size()-1];
        else
            return mNumCels-1;
    }

    theTime = theTime%mTotalAnimTime;

    int aFrame;
    if (!mPerFrameDelay.empty())
        aFrame = GetPerFrameCel(theTime);
    else
        aFrame = (theTime/mFrameDelay)%mNumCels;

    if (mFrameMap.empty())
        return aFrame;
    else
        return mFrameMap[aFrame];
}

int Image::GetAnimCel(int theTime)
{
    if (mAnimInfo==NULL)
        return 0;
    else
        return mAnimInfo->GetCel(theTime);
}

Rect Image::GetAnimCelRect(int theTime)
{
    Rect aRect;
    int aCel = GetAnimCel(theTime);
    int aCelWidth = GetCelWidth();
    int aCelHeight = GetCelHeight();
    if (mNumCols>1)
        return Rect(aCel*aCelWidth,0,aCelWidth,mHeight);
    else
        return Rect(0,aCel*aCelHeight,mWidth,aCelHeight);
}

void Image::CopyAttributes(Image *from)
{
    mNumCols = from->mNumCols;
    mNumRows = from->mNumRows;
    delete mAnimInfo;
    mAnimInfo = NULL;
    if (from->mAnimInfo != NULL)
        mAnimInfo = new AnimInfo(*from->mAnimInfo);
}

Graphics* Image::GetGraphics()
{
    Graphics* g = new Graphics(this);

    return g;
}

void Image::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
}

void Image::ClearRect(const Rect& theRect)
{
}

void Image::DrawRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    FillRect(Rect(theRect.mX, theRect.mY, theRect.mWidth + 1, 1), theColor, theDrawMode);
    FillRect(Rect(theRect.mX, theRect.mY + theRect.mHeight, theRect.mWidth + 1, 1), theColor, theDrawMode);
    FillRect(Rect(theRect.mX, theRect.mY + 1, 1, theRect.mHeight - 1), theColor, theDrawMode);
    FillRect(Rect(theRect.mX + theRect.mWidth, theRect.mY + 1, 1, theRect.mHeight - 1), theColor, theDrawMode);
}

void Image::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
}

void Image::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
}

void Image::FillScanLines(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode)
{
    for (int i = 0; i < theSpanCount; i++)
    {
        Span* aSpan = &theSpans[i];
        FillRect(Rect(aSpan->mX, aSpan->mY, aSpan->mWidth, 1), theColor, theDrawMode);
    }
}

void Image::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const unsigned char* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
}

bool Image::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
{
    return false;
}

void Image::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
}

void Image::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode)
{
}

void Image::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
}

void Image::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
}

void Image::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
}

void Image::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
}


void Image::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
}

void Image::StretchBltMirror(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
}

GLuint Image::CreateTexture(int x, int y, int w, int h)
{
    // This is a dummy function. It should never be called.
    assert(0);
    return 0;
}

void Image::CreateTextureData()
{
    if (mD3DData != NULL) {
        delete mD3DData;
    }
    mD3DData = new TextureData();
}

void Image::DeleteTextureData()
{
    if (mD3DData != NULL) {
        delete mD3DData;
        mD3DData = NULL;
    }
}

void Image::CheckCreateTextures()
{
    if (!HasTextureData()) {
        CreateTextureData();
    }
    mD3DData->CheckCreateTextures(this);
}

void Image::DeleteSWBuffers()
{
}

void Image::Delete3DBuffers()
{
#if 0
    mApp->Remove3DData(this);
#endif
}

void Image::DeleteExtraBuffers()
{
    DeleteSWBuffers();
    Delete3DBuffers();
}

void Image::DeleteNativeData()
{
}

void Image::ReInit()
{
}
