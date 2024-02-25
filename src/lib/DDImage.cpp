#include "DDImage.h"
#include <math.h>
#include "DDInterface.h"
#include "Rect.h"
#include "Graphics.h"
#include "SexyAppBase.h"
#include "D3DInterface.h"

#if 0
#include "Debug.h"
#include "PerfTimer.h"
#endif

#include <assert.h>

#if SDL_VERSION_ATLEAST(2,0,0)
// SDL_SRCCOLORKEY is not present in SDL2.
// It used to be a bit flag for SDL_SetColorKey, but it isn't used anymore, except for SDL_RLEACCEL
#ifndef SDL_SRCCOLORKEY
#define SDL_SRCCOLORKEY SDL_TRUE
#endif
#endif

using namespace Sexy;

DDImage::DDImage(DDInterface* theDDInterface) :
    MemoryImage(theDDInterface->mApp)
{
    TLOG(mLogFacil, 1, "new DDImage(DDInterface* theDDInterface)");
    mDDInterface = theDDInterface;
    Init();
}

DDImage::DDImage() :
    MemoryImage(gSexyAppBase)
{
    TLOG(mLogFacil, 1, "new DDImage()");
    mDDInterface = gSexyAppBase->mDDInterface;
    Init();
}

DDImage::~DDImage()
{
    if (mSurface != NULL && mSurface != gSexyAppBase->GetGameSurface()) {
        // TODO. When SDL is fixed we can do this.
        SDL_FreeSurface(mSurface);
    }

    mDDInterface->RemoveDDImage(this);

    assert(mLockCount == 0);
}

void DDImage::Init()
{

    mSurface = NULL;
    mDDInterface->AddDDImage(this);

    mNoLock = false;
    mVideoMemory = false;
    mFirstPixelTrans = false;
    mWantDDSurface = false;
    mDrawToBits = false;
    mSurfaceSet = false;

    mLockCount = 0;
}

bool DDImage::Check3D(Image *theImage)
{
    DDImage *anImage = dynamic_cast<DDImage*> (theImage);
    if (anImage != NULL)
        return Check3D(anImage);
    else
        return false;
}

bool DDImage::Check3D(DDImage *theImage)
{
    return theImage->mDDInterface->mIs3D && theImage->mSurface == gSexyAppBase->GetGameSurface();
}

bool DDImage::LockSurface()
{
    if (Check3D(this))
        return false;

    if (mLockCount == 0) {

        if (SDL_MUSTLOCK(mSurface)) {
            if (SDL_LockSurface(mSurface) < 0) {
                return false;
            }
        }
    }
    mLockCount++;

    assert(mLockCount < 8);

    return true;

}

bool DDImage::UnlockSurface()
{
    if (Check3D(this))
        return false;

    --mLockCount;

    if (mLockCount == 0) {

        if (SDL_MUSTLOCK(mSurface)) {
            SDL_UnlockSurface(mSurface);
        }
    }

    assert(mLockCount >= 0);

    return true;
}

void DDImage::SetSurface(SDL_Surface* theSurface)
{
    if (!theSurface) {
        // FIXME. What to do in this case?
        return;
    }

    mSurfaceSet = true;
    mSurface = theSurface;

    mWidth = mSurface->w;
    mHeight = mSurface->h;
    if (mSurface->format->Bmask < mSurface->format->Gmask && mSurface->format->Gmask < mSurface->format->Rmask) {
        mOptimizeSoftwareDrawing = true;
    }

    mNoLock = false;
}

bool DDImage::GenerateDDSurface()
{
    if (mSurface != NULL)
        return true;

    CommitBits();

    if (mHasAlpha)
        return false;

    mWantDDSurface = true;

    // Force into non-palletized mode for this
    if (mColorTable != NULL)
        GetBits();


    bool aResult;

#if 0
    AutoCrit aCrit(mDDInterface->mCritSect); // prevent mSurface from being released while we're in this code
#endif

    // TODO. Determine how this affects SDL_FreeSurface
    aResult = mDDInterface->CreateSurface(&mSurface, mWidth, mHeight, mVideoMemory);
    if (!aResult)
        return false;

    if (!LockSurface())
        return false;

    const int rRightShift = 16 + (8 - mDDInterface->mRedBits);
    const int gRightShift = 8 + (8 - mDDInterface->mGreenBits);
    const int bRightShift = 0 + (8 - mDDInterface->mBlueBits);

    const int rLeftShift = mDDInterface->mRedShift;
    const int gLeftShift = mDDInterface->mGreenShift;
    const int bLeftShift = mDDInterface->mBlueShift;

    const int rMask = mDDInterface->mRedMask;
    const int gMask = mDDInterface->mGreenMask;
    const int bMask = mDDInterface->mBlueMask;
    int aNumBits = 32;                    // FIXME. What's a good default value?
    if (gSexyAppBase->GetGameSurface()) {
        aNumBits = gSexyAppBase->GetGameSurface()->format->BitsPerPixel;
    }
#if 0
    const int rMask = mSurface->format->Rmask;
    const int gMask = mSurface->format->Gmask;
    const int bMask = mSurface->format->Bmask;
    int aNumBits = mSurface->format->BitsPerPixel;
#endif


    if (aNumBits == 16) {
        Uint16* mSurfaceBits = (Uint16*) mSurface->pixels;

        if (mSurfaceBits != NULL) {
            int i;
            bool firstTrans = true;

            Uint16* a16Bits = NULL;
            Uint16 aTransColor = 0;

            if (mBits != NULL) {
                a16Bits = new Uint16[mWidth * mHeight];
                Uint32* aSrcPtr = (Uint32*) mBits;
                Uint16* a16SrcPtr = a16Bits;

                for (i = 0; i < mWidth * mHeight; i++) {
                    Uint32 val = *(aSrcPtr++);

                    *a16SrcPtr = (((val >> rRightShift) << rLeftShift) & rMask) |
                            (((val >> gRightShift) << gLeftShift) & gMask) |
                            (((val >> bRightShift) << bLeftShift) & bMask);

                    int anAlpha = val >> 24;
                    if ((firstTrans) && (anAlpha < 255)) {
                        firstTrans = false;
                        aTransColor = *a16SrcPtr;
                    }

                    ++a16SrcPtr;
                }
            }

            if ((mHasTrans) && (mBits != NULL)) {
                if (mFirstPixelTrans) {
                    aTransColor = *a16Bits;

                    if (a16Bits != NULL) {
                        Uint16* aDestPtr = mSurfaceBits;
                        Uint16* a16SrcPtr = a16Bits;
                        for (int aRow = 0; aRow < mHeight; aRow++) {
                            for (int aCol = 0; aCol < mWidth; aCol++) {
                                *(aDestPtr++) = *a16SrcPtr;
                                ++a16SrcPtr;
                            }

                            aDestPtr += mSurface->pitch / 2 - mWidth;
                        }
                    }
                } else {
                    bool needNewColor = false;
                    Uint32* aSrcPtr = (Uint32*) mBits;
                    Uint16* a16SrcPtr = a16Bits;
                    for (i = 0; i < mWidth * mHeight; i++) {
                        Uint32 val = *(aSrcPtr++);
                        Uint16 a16Val = *(a16SrcPtr++);

                        int anAlpha = val >> 24;
                        if ((anAlpha == 255) && (aTransColor == a16Val)) {
                            needNewColor = true;
                            break;
                        }
                    }

                    if (needNewColor) {
                        int* aUsedColorArray = new int[2048];

                        memset(aUsedColorArray, 0, 2048 * sizeof (int));

                        aSrcPtr = (Uint32*) mBits;
                        a16SrcPtr = a16Bits;
                        for (i = 0; i < mWidth * mHeight; i++) {
                            Uint32 val = *(aSrcPtr++);
                            Uint16 a16Val = *(a16SrcPtr++);

                            int anAlpha = val >> 24;
                            if (anAlpha > 0)
                                aUsedColorArray[a16Val / 32] |= (1 << (a16Val % 32));
                        }

                        bool done = false;
                        for (int i = 0; i < 2048; i++) {
                            if (aUsedColorArray[i] != 0xFFFF) {
                                for (int aBit = 0; aBit < 32; aBit++) {
                                    if ((aUsedColorArray[i] & (1 << aBit)) == 0) {
                                        aTransColor = i * 32 + aBit;
                                        break;
                                    }
                                }
                            }

                            if (done)
                                break;
                        }

                        delete aUsedColorArray;
                    }

                    if (mBits != NULL) {
                        Uint16* aDestPtr = mSurfaceBits;
                        Uint32* aSrcPtr = (Uint32*) mBits;
                        Uint16* a16SrcPtr = a16Bits;
                        for (int aRow = 0; aRow < mHeight; aRow++) {
                            for (int aCol = 0; aCol < mWidth; aCol++) {
                                Uint32 val = *(aSrcPtr++);

                                int anAlpha = val >> 24;
                                if (anAlpha < 255)
                                    *(aDestPtr++) = aTransColor;
                                else
                                    *(aDestPtr++) = *a16SrcPtr;

                                ++a16SrcPtr;
                            }

                            aDestPtr += mSurface->pitch / 2 - mWidth;
                        }
                    }
                }
            } else {
                if (a16Bits != NULL) {
                    Uint16* aDestPtr = mSurfaceBits;
                    Uint16* a16SrcPtr = a16Bits;
                    for (int aRow = 0; aRow < mHeight; aRow++) {
                        for (int aCol = 0; aCol < mWidth; aCol++) {
                            *(aDestPtr++) = *a16SrcPtr;
                            ++a16SrcPtr;
                        }

                        aDestPtr += mSurface->pitch / 2 - mWidth;
                    }
                }
            }

            delete a16Bits;

            if (mHasTrans) {
                SDL_SetColorKey(mSurface, SDL_SRCCOLORKEY /*SDL_RLEACCEL*/, aTransColor);
            }
        }
    } else if (aNumBits == 32) {
        Uint32* mSurfaceBits = (Uint32*) mSurface->pixels;

        if (mSurfaceBits != NULL) {
            int i;
            bool firstTrans = true;

            Uint32 aTransColor = 0;

            if ((mHasTrans) && (mBits != NULL)) {
                if (mFirstPixelTrans) {
                    Uint32 val = *mBits;

                    aTransColor = (((val >> rRightShift) << rLeftShift) & rMask) |
                            (((val >> gRightShift) << gLeftShift) & gMask) |
                            (((val >> bRightShift) << bLeftShift) & bMask);

                    if (mBits != NULL) {
                        Uint32* aDestPtr = mSurfaceBits;
                        Uint32* aSrcPtr = (Uint32*) mBits;
                        for (int aRow = 0; aRow < mHeight; aRow++) {
                            for (int aCol = 0; aCol < mWidth; aCol++) {
                                Uint32 val = *(aSrcPtr++);

                                *(aDestPtr++) =
                                        (((val >> rRightShift) << rLeftShift) & rMask) |
                                        (((val >> gRightShift) << gLeftShift) & gMask) |
                                        (((val >> bRightShift) << bLeftShift) & bMask);
                            }

                            aDestPtr += mSurface->pitch / 4 - mWidth;
                        }
                    }
                } else {
                    Uint32* aSrcPtr = (Uint32*) mBits;
                    for (i = 0; i < mWidth * mHeight; i++) {
                        Uint32 val = *(aSrcPtr++);

                        int anAlpha = val >> 24;
                        if ((firstTrans) && (anAlpha < 255)) {
                            firstTrans = false;
                            aTransColor = val;
                        }
                    }

                    bool needNewColor = false;
                    aSrcPtr = (Uint32*) mBits;
                    for (i = 0; i < mWidth * mHeight; i++) {
                        Uint32 val = *(aSrcPtr++);

                        int anAlpha = val >> 24;
                        if ((anAlpha == 255) && (aTransColor == (val & 0x00FFFFFF))) {
                            needNewColor = true;
                            break;
                        }
                    }

                    if (needNewColor) {
                        int* aUsedColorArray = new int[0x80000];

                        memset(aUsedColorArray, 0, 0x80000 * sizeof (int));

                        aSrcPtr = (Uint32*) mBits;
                        for (i = 0; i < mWidth * mHeight; i++) {
                            Uint32 val = *(aSrcPtr++);

                            int anAlpha = val >> 24;

                            if (anAlpha > 0)
                                aUsedColorArray[(val & 0xFFFFFF) / 32] |= (1 << (val % 32));
                        }

                        bool done = false;
                        for (int i = 0; i < 0x80000; i++) {
                            if (aUsedColorArray[i] != 0xFFFF) {
                                for (int aBit = 0; aBit < 32; aBit++) {
                                    if ((aUsedColorArray[i] & (1 << aBit)) == 0) {
                                        aTransColor = i * 32 + aBit;
                                        done = true;
                                        break;
                                    }
                                }
                            }

                            if (done)
                                break;
                        }

                        delete aUsedColorArray;
                    }

                    if (mBits != NULL) {
                        Uint32* aDestPtr = mSurfaceBits;
                        Uint32* aSrcPtr = (Uint32*) mBits;
                        for (int aRow = 0; aRow < mHeight; aRow++) {
                            for (int aCol = 0; aCol < mWidth; aCol++) {
                                Uint32 val = *(aSrcPtr++);

                                int anAlpha = val >> 24;
                                if (anAlpha < 255)
                                    *(aDestPtr++) = aTransColor;
                                else {
                                    *(aDestPtr++) =
                                            (((val >> rRightShift) << rLeftShift) & rMask) |
                                            (((val >> gRightShift) << gLeftShift) & gMask) |
                                            (((val >> bRightShift) << bLeftShift) & bMask);
                                }
                            }

                            aDestPtr += mSurface->pitch / 4 - mWidth;
                        }
                    }
                }

                SDL_SetColorKey(mSurface, SDL_SRCCOLORKEY/* | SDL_RLEACCEL*/, aTransColor);
            } else {
                if (mBits != NULL) {
                    Uint32* aDestPtr = mSurfaceBits;
                    Uint32* aSrcPtr = (Uint32*) mBits;
                    for (int aRow = 0; aRow < mHeight; aRow++) {
                        for (int aCol = 0; aCol < mWidth; aCol++) {
                            Uint32 val = *(aSrcPtr++);

                            *(aDestPtr++) =
                                    (((val >> rRightShift) << rLeftShift) & rMask) |
                                    (((val >> gRightShift) << gLeftShift) & gMask) |
                                    (((val >> bRightShift) << bLeftShift) & bMask);
                        }

                        aDestPtr += mSurface->pitch / 4 - mWidth;
                    }
                }
            }
        }
    } else {
        return false;
    }

    UnlockSurface();

    return true;
}

void DDImage::DeleteDDSurface()
{
    if (mSurface != NULL) {
        if ((mColorTable == NULL) && (mBits == NULL) && !HasTextureData()) {
            // No colortable, no textures, no pixelsbits
            // ???? Why do we do this? GetBits allocates pixel memory? Is this some sort of switching back to software renderer?
            GetBits();
        }

        if (mSurface != NULL && mSurface != gSexyAppBase->GetGameSurface()) {
            SDL_FreeSurface(mSurface);
        }
        mSurface = NULL;
    }
}

void DDImage::ReInit()
{
    MemoryImage::ReInit();

    if (mWantDDSurface)
        GenerateDDSurface();
}

void DDImage::DoPurgeBits()
{
    if (mSurfaceSet)
        return;

    SetPurgeBits(true);

    CommitBits();

    //FIXME
    if (true)//!mApp->Is3DAccelerated())
    {
        if ((mWantDDSurface) && true)//GenerateDDSurface())
        {
            delete [] mBits;
            mBits = NULL;

            delete [] mColorIndices;
            mColorIndices = NULL;

            delete [] mColorTable;
            mColorTable = NULL;

            return;
        }
    } else // Accelerated
    {
        if (mSurface != NULL) {
            GetBits();
            DeleteDDSurface();
        }
    }

    MemoryImage::DoPurgeBits();
}

void DDImage::DeleteAllNonSurfaceData()
{
    delete [] mBits;
    mBits = NULL;

    delete [] mNativeAlphaData;
    mNativeAlphaData = NULL;

    delete [] mRLAdditiveData;
    mRLAdditiveData = NULL;

    delete [] mRLAlphaData;
    mRLAlphaData = NULL;

    delete [] mColorTable;
    mColorTable = NULL;

    delete [] mColorIndices;
    mColorIndices = NULL;
}

void DDImage::DeleteNativeData()
{
    if (mSurfaceSet)
        return;

    MemoryImage::DeleteNativeData();
    DeleteDDSurface();
}

void DDImage::DeleteExtraBuffers()
{
    if (mSurfaceSet)
        return;

    MemoryImage::DeleteExtraBuffers();
    DeleteDDSurface();
}

void DDImage::SetVideoMemory(bool wantVideoMemory)
{
    if (wantVideoMemory != mVideoMemory) {
        mVideoMemory = wantVideoMemory;

        // Make sure that we have the bits
        GetBits();
        DeleteDDSurface();
    }
}

SDL_Surface* DDImage::GetSurface()
{
    //TODO: Log if generate surface fails

    if (mSurface == NULL)
        GenerateDDSurface();

    return mSurface;
}

bool DDImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
{
    if (Check3D(this)) {
        mDDInterface->mD3DInterface->FillPoly(theVertices, theNumVertices, theClipRect, theColor, theDrawMode, tx, ty);
        return true;
    } else
        return false;
}

void DDImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    if (Check3D(this)) {
        mDDInterface->mD3DInterface->FillRect(theRect, theColor, theDrawMode);
        return;
    }

    // It makes no sense that we get here with a DDImage and no 3D!
    //assert(0);

    CommitBits();
    if ((mDrawToBits) || (mHasAlpha) || ((mHasTrans) && (!mFirstPixelTrans)) || (mDDInterface->mIs3D)) {
        MemoryImage::FillRect(theRect, theColor, theDrawMode);
        return;
    }

    switch (theDrawMode) {
    case Graphics::DRAWMODE_NORMAL:
        NormalFillRect(theRect, theColor);
        break;
    case Graphics::DRAWMODE_ADDITIVE:
        AdditiveFillRect(theRect, theColor);
        break;
    }

    DeleteAllNonSurfaceData();
}

void DDImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
    if (mNoLock)
        return;

    double aMinX = std::min(theStartX, theEndX);
    double aMinY = std::min(theStartY, theEndY);
    double aMaxX = std::max(theStartX, theEndX);
    double aMaxY = std::max(theStartY, theEndY);

    if (!LockSurface())
        return;


    uint32_t aRMask = mSurface->format->Rmask;
    uint32_t aGMask = mSurface->format->Gmask;
    uint32_t aBMask = mSurface->format->Bmask;


    uint32_t aRRoundAdd = aRMask >> 1;
    uint32_t aGRoundAdd = aGMask >> 1;
    uint32_t aBRoundAdd = aBMask >> 1;

    if (mSurface->format->BitsPerPixel == 16) {
        if (theColor.mAlpha == 255) {
            ushort aColor = (ushort)
                    (((((theColor.mRed * aRMask) + aRRoundAdd) >> 8) & aRMask) |
                    ((((theColor.mGreen * aGMask) + aGRoundAdd) >> 8) & aGMask) |
                    ((((theColor.mBlue * aBMask) + aBRoundAdd) >> 8) & aBMask));

            double dv = theEndY - theStartY;
            double dh = theEndX - theStartX;
            int minG, maxG, G, DeltaG1, DeltaG2;
            double swap;
            int inc = 1;
            int aCurX;
            int aCurY;
            int aRowWidth = mSurface->pitch / 2;
            int aRowAdd = aRowWidth;

            if (fabs(dv) < fabs(dh)) {
                // Mostly horizontal
                if (dh < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }
                if (dv < 0) {
                    dv = -dv;
                    inc = -1;
                    aRowAdd = -aRowAdd;
                }

                ushort* aDestPixels = ((ushort*) mSurface->pixels) + ((int) theStartY * aRowWidth) + (int) theStartX;
                *aDestPixels = aColor;
                aDestPixels++;

                aCurY = (int) theStartY;
                aCurX = (int) theStartX + 1;

                G = (int) (2 * dv - dh);
                DeltaG1 = (int) (2 * (dv - dh));
                DeltaG2 = (int) (2 * dv);

                G += (int) (DeltaG2 * (theStartY - (int) theStartY));

                while (aCurX <= theEndX) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurY += inc;
                        aDestPixels += aRowAdd;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    *aDestPixels = aColor;

                    aCurX++;
                    aDestPixels++;
                }
            } else {
                // Mostly vertical
                if (dv < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }

                if (dh < 0) {
                    dh = -dh;
                    inc = -1;
                }

                ushort* aDestPixels = ((ushort*) mSurface->pixels) + ((int) theStartY * mSurface->pitch / 2) + (int) theStartX;
                *aDestPixels = aColor;
                aDestPixels += aRowAdd;

                aCurX = (int) (theStartX);
                aCurY = (int) (theStartY + 1);

                G = (int) (2 * dh - dv);
                minG = maxG = G;
                DeltaG1 = (int) (2 * (dh - dv));
                DeltaG2 = (int) (2 * dh);

                G += (int) (DeltaG2 * (theStartX - (int) theStartX));

                while (aCurY <= theEndY) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurX += inc;
                        aDestPixels += inc;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    *aDestPixels = aColor;

                    aCurY++;
                    aDestPixels += aRowAdd;
                }
            }
        } else {
            ushort src =
                    ((((((theColor.mRed * theColor.mAlpha + 0x80) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask) +
                    ((((((theColor.mGreen * theColor.mAlpha + 0x80) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) +
                    ((((((theColor.mBlue * theColor.mAlpha + 0x80) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask);
            int oma = 256 - theColor.mAlpha;

            double dv = theEndY - theStartY;
            double dh = theEndX - theStartX;
            int minG, maxG, G, DeltaG1, DeltaG2;
            double swap;
            int inc = 1;
            int aCurX;
            int aCurY;
            int aRowWidth = mSurface->pitch / 2;
            int aRowAdd = aRowWidth;

            if (fabs(dv) < fabs(dh)) {
                // Mostly horizontal
                if (dh < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }
                if (dv < 0) {
                    dv = -dv;
                    inc = -1;
                    aRowAdd = -aRowAdd;
                }

                ushort* aDestPixels = ((ushort*) mSurface->pixels) + ((int) theStartY * aRowWidth) + (int) theStartX;
                ushort dest = *aDestPixels;
                *(aDestPixels++) = src +
                        (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                        (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                        (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

                aCurY = (int) (theStartY);
                aCurX = (int) (theStartX + 1);

                G = (int) (2 * dv - dh);
                DeltaG1 = (int) (2 * (dv - dh));
                DeltaG2 = (int) (2 * dv);

                G += (int) (DeltaG2 * (theStartY - (int) theStartY));

                while (aCurX <= theEndX) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurY += inc;
                        aDestPixels += aRowAdd;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    dest = *aDestPixels;
                    *(aDestPixels++) = src +
                            (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                            (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                            (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

                    aCurX++;
                }
            } else {
                // Mostly vertical
                if (dv < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }

                if (dh < 0) {
                    dh = -dh;
                    inc = -1;
                }

                ushort* aDestPixels = ((ushort*) mSurface->pixels) + ((int) theStartY * mSurface->pitch / 2) + (int) theStartX;
                ushort dest = *aDestPixels;
                *aDestPixels = src +
                        (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                        (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                        (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);
                aDestPixels += aRowAdd;

                aCurX = (int) (theStartX);
                aCurY = (int) (theStartY + 1);

                G = (int) (2 * dh - dv);
                minG = maxG = G;
                DeltaG1 = (int) (2 * (dh - dv));
                DeltaG2 = (int) (2 * dh);

                G += (int) (DeltaG2 * (theStartX - (int) theStartX));

                while (aCurY <= theEndY) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurX += inc;
                        aDestPixels += inc;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    dest = *aDestPixels;
                    *aDestPixels = src +
                            (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                            (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                            (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

                    aCurY++;
                    aDestPixels += aRowAdd;
                }
            }
        }
    } else if (mSurface->format->BitsPerPixel == 32) {
        if (theColor.mAlpha == 255) {
            uint32_t aColor =
                    ((((theColor.mRed * aRMask) + aRRoundAdd) >> 8) & aRMask) |
                    ((((theColor.mGreen * aGMask) + aGRoundAdd) >> 8) & aGMask) |
                    ((((theColor.mBlue * aBMask) + aBRoundAdd) >> 8) & aBMask);

            double dv = theEndY - theStartY;
            double dh = theEndX - theStartX;
            int minG, maxG, G, DeltaG1, DeltaG2;
            double swap;
            int inc = 1;
            int aCurX;
            int aCurY;
            int aRowWidth = mSurface->pitch / 4;
            int aRowAdd = aRowWidth;
            ;

            if (fabs(dv) < fabs(dh)) {
                // Mostly horizontal
                if (dh < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }
                if (dv < 0) {
                    dv = -dv;
                    inc = -1;
                    aRowAdd = -aRowAdd;
                }

                uint32_t* aDestPixels = ((uint32_t*) mSurface->pixels) + ((int) theStartY * aRowWidth) + (int) theStartX;
                *aDestPixels = aColor;
                aDestPixels++;

                aCurY = (int) (theStartY);
                aCurX = (int) (theStartX + 1);

                G = (int) (2 * dv - dh);
                DeltaG1 = (int) (2 * (dv - dh));
                DeltaG2 = (int) (2 * dv);

                G += (int) (DeltaG2 * (theStartY - (int) theStartY));

                while (aCurX <= theEndX) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurY += inc;
                        aDestPixels += aRowAdd;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    *aDestPixels = aColor;

                    aCurX++;
                    aDestPixels++;
                }
            } else {
                // Mostly vertical
                if (dv < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }

                if (dh < 0) {
                    dh = -dh;
                    inc = -1;
                }

                uint32_t* aDestPixels = ((uint32_t*) mSurface->pixels) + ((int) theStartY * aRowWidth) + (int) theStartX;
                *aDestPixels = aColor;
                aDestPixels += aRowAdd;

                aCurX = (int) (theStartX);
                aCurY = (int) (theStartY + 1);

                G = (int) (2 * dh - dv);
                minG = maxG = G;
                DeltaG1 = (int) (2 * (dh - dv));
                DeltaG2 = (int) (2 * dh);

                G += (int) (DeltaG2 * (theStartX - (int) theStartX));

                while (aCurY <= theEndY) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurX += inc;
                        aDestPixels += inc;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    *aDestPixels = aColor;

                    aCurY++;
                    aDestPixels += aRowAdd;
                }
            }
        } else {
            uint32_t src =
                    ((((((theColor.mRed * theColor.mAlpha + 0x80) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask) +
                    ((((((theColor.mGreen * theColor.mAlpha + 0x80) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) +
                    ((((((theColor.mBlue * theColor.mAlpha + 0x80) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask);
            int oma = 256 - theColor.mAlpha;

            double dv = theEndY - theStartY;
            double dh = theEndX - theStartX;
            int minG, maxG, G, DeltaG1, DeltaG2;
            double swap;
            int inc = 1;
            int aCurX;
            int aCurY;
            int aRowWidth = mSurface->pitch / 4;
            int aRowAdd = aRowWidth;

            if (fabs(dv) < fabs(dh)) {
                // Mostly horizontal
                if (dh < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }
                if (dv < 0) {
                    dv = -dv;
                    inc = -1;
                    aRowAdd = -aRowAdd;
                }

                uint32_t* aDestPixels = ((uint32_t*) mSurface->pixels) + ((int) theStartY * aRowWidth) + (int) theStartX;
                uint32_t dest = *aDestPixels;
                *(aDestPixels++) = src +
                        (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                        (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                        (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

                aCurY = (int) (theStartY);
                aCurX = (int) (theStartX + 1);

                G = (int) (2 * dv - dh);
                DeltaG1 = (int) (2 * (dv - dh));
                DeltaG2 = (int) (2 * dv);

                G += (int) (DeltaG2 * (theStartX - (int) theStartX));

                while (aCurX <= theEndX) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurY += inc;
                        aDestPixels += aRowAdd;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    dest = *aDestPixels;
                    *(aDestPixels++) = src +
                            (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                            (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                            (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

                    aCurX++;
                }
            } else {
                // Mostly vertical
                if (dv < 0) {
                    dh = -dh;
                    dv = -dv;
                    swap = theEndY;
                    theEndY = theStartY;
                    theStartY = swap;
                    swap = theEndX;
                    theEndX = theStartX;
                    theStartX = swap;
                }

                if (dh < 0) {
                    dh = -dh;
                    inc = -1;
                }

                uint32_t* aDestPixels = ((uint32_t*) mSurface->pixels) + ((int) theStartY * aRowWidth) + (int) theStartX;
                uint32_t dest = *aDestPixels;
                *aDestPixels = src +
                        (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                        (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                        (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);
                aDestPixels += aRowAdd;

                aCurX = (int) (theStartX);
                aCurY = (int) (theStartY + 1);

                G = (int) (2 * dh - dv);
                minG = maxG = G;
                DeltaG1 = (int) (2 * (dh - dv));
                DeltaG2 = (int) (2 * dh);

                G += (int) (DeltaG2 * (theStartX - (int) theStartX));

                while (aCurY <= theEndY) {
                    if (G > 0) {
                        G += DeltaG1;
                        aCurX += inc;
                        aDestPixels += inc;

                        if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                            break;
                    } else
                        G += DeltaG2;

                    dest = *aDestPixels;
                    *aDestPixels = src +
                            (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                            (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                            (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

                    aCurY++;
                    aDestPixels += aRowAdd;
                }
            }
        }
    }

    UnlockSurface();
}

void DDImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
    if (mNoLock)
        return;

    //NOT IMPLEMENTED YET
    assert(false);

#if 0
    double aMinX = std::min(theStartX, theEndX);
    double aMinY = std::min(theStartY, theEndY);
    double aMaxX = std::max(theStartX, theEndX);
    double aMaxY = std::max(theStartY, theEndY);

    LPDIRECTDRAWSURFACE aSurface = GetSurface();

    if (!LockSurface())
        return;

    uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
    uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
    uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

    uint32_t aRRoundAdd = aRMask >> 1;
    uint32_t aGRoundAdd = aGMask >> 1;
    uint32_t aBRoundAdd = aBMask >> 1;

    int aRedShift = mDDInterface->mRedShift;
    int aGreenShift = mDDInterface->mGreenShift;
    int aBlueShift = mDDInterface->mBlueShift;

    int* aMaxRedTable = mDDInterface->mRedAddTable;
    int* aMaxGreenTable = mDDInterface->mGreenAddTable;
    int* aMaxBlueTable = mDDInterface->mBlueAddTable;

    if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16) {
        ushort rc = ((theColor.mRed * theColor.mAlpha) / 255) >> (8 - mDDInterface->mRedBits);
        ushort gc = ((theColor.mGreen * theColor.mAlpha) / 255) >> (8 - mDDInterface->mGreenBits);
        ushort bc = ((theColor.mBlue * theColor.mAlpha) / 255) >> (8 - mDDInterface->mBlueBits);

        double dv = theEndY - theStartY;
        double dh = theEndX - theStartX;
        int minG, maxG, G, DeltaG1, DeltaG2;
        double swap;
        int inc = 1;
        int aCurX;
        int aCurY;
        int aRowWidth = mLockedSurfaceDesc.lPitch / 2;
        int aRowAdd = aRowWidth;

        if (abs(dv) < abs(dh)) {
            // Mostly horizontal
            if (dh < 0) {
                dh = -dh;
                dv = -dv;
                swap = theEndY;
                theEndY = theStartY;
                theStartY = swap;
                swap = theEndX;
                theEndX = theStartX;
                theStartX = swap;
            }
            if (dv < 0) {
                dv = -dv;
                inc = -1;
                aRowAdd = -aRowAdd;
            }

            ushort* aDestPixels = ((ushort*) mLockedSurfaceDesc.lpSurface) + ((int) theStartY * aRowWidth) + (int) theStartX;
            ushort dest = *aDestPixels;

            int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
            int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
            int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

            *(aDestPixels++) =
                    (r << aRedShift) |
                    (g << aGreenShift) |
                    (b << aBlueShift);

            aCurY = theStartY;
            aCurX = theStartX + 1;

            G = 2 * dv - dh;
            DeltaG1 = 2 * (dv - dh);
            DeltaG2 = 2 * dv;

            G += DeltaG2 * (theStartY - (int) theStartY);

            while (aCurX <= theEndX) {
                if (G > 0) {
                    G += DeltaG1;
                    aCurY += inc;
                    aDestPixels += aRowAdd;

                    if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                        break;
                } else
                    G += DeltaG2;

                dest = *aDestPixels;

                r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
                g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
                b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

                *(aDestPixels++) =
                        (r << aRedShift) |
                        (g << aGreenShift) |
                        (b << aBlueShift);

                aCurX++;
            }
        } else {
            // Mostly vertical
            if (dv < 0) {
                dh = -dh;
                dv = -dv;
                swap = theEndY;
                theEndY = theStartY;
                theStartY = swap;
                swap = theEndX;
                theEndX = theStartX;
                theStartX = swap;
            }

            if (dh < 0) {
                dh = -dh;
                inc = -1;
            }

            ushort* aDestPixels = ((ushort*) mLockedSurfaceDesc.lpSurface) + ((int) theStartY * mLockedSurfaceDesc.lPitch / 2) + (int) theStartX;

            ushort dest = *aDestPixels;

            int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
            int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
            int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

            *aDestPixels =
                    (r << aRedShift) |
                    (g << aGreenShift) |
                    (b << aBlueShift);

            aDestPixels += aRowAdd;

            aCurX = theStartX;
            aCurY = theStartY + 1;

            G = 2 * dh - dv;
            minG = maxG = G;
            DeltaG1 = 2 * (dh - dv);
            DeltaG2 = 2 * dh;

            G += DeltaG2 * (theStartX - (int) theStartX);

            while (aCurY <= theEndY) {
                if (G > 0) {
                    G += DeltaG1;
                    aCurX += inc;
                    aDestPixels += inc;

                    if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                        break;
                } else
                    G += DeltaG2;

                dest = *aDestPixels;

                r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
                g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
                b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

                *aDestPixels =
                        (r << aRedShift) |
                        (g << aGreenShift) |
                        (b << aBlueShift);

                aCurY++;
                aDestPixels += aRowAdd;
            }
        }
    } else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32) {
        uint32_t rc = ((theColor.mRed * theColor.mAlpha) / 255) >> (8 - mDDInterface->mRedBits);
        uint32_t gc = ((theColor.mGreen * theColor.mAlpha) / 255) >> (8 - mDDInterface->mGreenBits);
        uint32_t bc = ((theColor.mBlue * theColor.mAlpha) / 255) >> (8 - mDDInterface->mBlueBits);

        double dv = theEndY - theStartY;
        double dh = theEndX - theStartX;
        int minG, maxG, G, DeltaG1, DeltaG2;
        double swap;
        int inc = 1;
        int aCurX;
        int aCurY;
        int aRowWidth = mLockedSurfaceDesc.lPitch / 4;
        int aRowAdd = aRowWidth;

        if (abs(dv) < abs(dh)) {
            // Mostly horizontal
            if (dh < 0) {
                dh = -dh;
                dv = -dv;
                swap = theEndY;
                theEndY = theStartY;
                theStartY = swap;
                swap = theEndX;
                theEndX = theStartX;
                theStartX = swap;
            }

            if (dv < 0) {
                dv = -dv;
                inc = -1;
                aRowAdd = -aRowAdd;
            }

            uint32_t* aDestPixels = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + ((int) theStartY * aRowWidth) + (int) theStartX;
            uint32_t dest = *aDestPixels;

            int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
            int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
            int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

            *(aDestPixels++) =
                    (r << aRedShift) |
                    (g << aGreenShift) |
                    (b << aBlueShift);

            aCurY = theStartY;
            aCurX = theStartX + 1;

            G = 2 * dv - dh;
            DeltaG1 = 2 * (dv - dh);
            DeltaG2 = 2 * dv;

            while (aCurX <= theEndX) {
                if (G > 0) {
                    G += DeltaG1;
                    aCurY += inc;
                    aDestPixels += aRowAdd;

                    if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                        break;
                } else
                    G += DeltaG2;

                dest = *aDestPixels;

                r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
                g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
                b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

                *(aDestPixels++) =
                        (r << aRedShift) |
                        (g << aGreenShift) |
                        (b << aBlueShift);

                aCurX++;
            }
        } else {
            // Mostly vertical
            if (dv < 0) {
                dh = -dh;
                dv = -dv;
                swap = theEndY;
                theEndY = theStartY;
                theStartY = swap;
                swap = theEndX;
                theEndX = theStartX;
                theStartX = swap;
            }

            if (dh < 0) {
                dh = -dh;
                inc = -1;
            }

            uint32_t* aDestPixels = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + ((int) theStartY * mLockedSurfaceDesc.lPitch / 4) + (int) theStartX;

            uint32_t dest = *aDestPixels;

            int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
            int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
            int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

            *aDestPixels =
                    (r << aRedShift) |
                    (g << aGreenShift) |
                    (b << aBlueShift);

            aDestPixels += aRowAdd;

            aCurX = theStartX;
            aCurY = theStartY + 1;

            G = 2 * dh - dv;
            minG = maxG = G;
            DeltaG1 = 2 * (dh - dv);
            DeltaG2 = 2 * dh;
            while (aCurY <= theEndY) {
                if (G > 0) {
                    G += DeltaG1;
                    aCurX += inc;
                    aDestPixels += inc;

                    if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
                        break;
                } else
                    G += DeltaG2;

                dest = *aDestPixels;

                r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
                g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
                b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

                *aDestPixels =
                        (r << aRedShift) |
                        (g << aGreenShift) |
                        (b << aBlueShift);

                aCurY++;
                aDestPixels += aRowAdd;
            }
        }
    }

    UnlockSurface();
#endif
}

void DDImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
    if (Check3D(this)) {
        mDDInterface->mD3DInterface->DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
        return;
    }

    if ((mDrawToBits) || (mHasAlpha) || (mHasTrans) || (mDDInterface->mIs3D)) {
        MemoryImage::DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
        return;
    }

    if (theStartY == theEndY) {
        int aStartX = (int) std::min(theStartX, theEndX);
        int aEndX = (int) std::max(theStartX, theEndX);

        FillRect(Rect(aStartX, (int) theStartY, aEndX - aStartX + 1, (int) (theEndY - theStartY + 1)), theColor, theDrawMode);
        return;
    } else if (theStartX == theEndX) {
        int aStartY = (int) std::min(theStartY, theEndY);
        int aEndY = (int) std::max(theStartY, theEndY);

        FillRect(Rect((int) theStartX, aStartY, (int) (theEndX - theStartX + 1), aEndY - aStartY + 1), theColor, theDrawMode);
        return;
    }

    CommitBits();

    switch (theDrawMode) {
    case Graphics::DRAWMODE_NORMAL:
        NormalDrawLine(theStartX, theStartY, theEndX, theEndY, theColor);
        break;
    case Graphics::DRAWMODE_ADDITIVE:
        AdditiveDrawLine(theStartX, theStartY, theEndX, theEndY, theColor);
        break;
    }

    DeleteAllNonSurfaceData();
}

// AA => AntiAliasing
void DDImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
    SDL_Surface* aSurface = GetSurface();

    if (!LockSurface())
        return;

    uint32_t aRMask = aSurface->format->Rmask;
    uint32_t aGMask = aSurface->format->Gmask;
    uint32_t aBMask = aSurface->format->Bmask;

    uint32_t color = (((theColor.mRed * aRMask) >> 8) & aRMask) |
            (((theColor.mGreen * aGMask) >> 8) & aGMask) |
            (((theColor.mBlue * aBMask) >> 8) & aBMask);

    int aX0 = (int) theStartX, aX1 = (int) theEndX;
    int aY0 = (int) theStartY, aY1 = (int) theEndY;
    int aXinc = 1;
    if (aY0 > aY1) {
        int aTempX = aX0, aTempY = aY0;
        aX0 = aX1;
        aY0 = aY1;
        aX1 = aTempX;
        aY1 = aTempY;
        double aTempXd = theStartX, aTempYd = theStartY;
        theStartX = theEndX;
        theStartY = theEndY;
        theEndX = aTempXd;
        theEndY = aTempYd;
    }

    int dx = aX1 - aX0;
    int dy = aY1 - aY0;
    double dxd = theEndX - theStartX;
    double dyd = theEndY - theStartY;
    if (dx < 0) {
        dx = -dx;
        aXinc = -1;
        dxd = -dxd;
    }

    if (aSurface->format->BitsPerPixel == 32) {
        uint32_t* aBits = (uint32_t*) aSurface->pixels;
#ifdef OPTIMIZE_SOFTWARE_DRAWING
        if (theColor.mAlpha != 255) {
#           define PIXEL_TYPE          uint32_t
#           define CALC_WEIGHT_A(w)    (((w) * (theColor.mAlpha+1)) >> 8)
#define BLEND_PIXEL(p)                                                  \
               *(p) =                                                   \
                    ((((color & 0xFF00FF) * a + (dest & 0xFF00FF) * oma) >> 8) & 0xFF00FF) | \
                    ((((color & 0x00FF00) * a + (dest & 0x00FF00) * oma) >> 8) & 0x00FF00);
            const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);

#           include "GENERIC_DrawLineAA.inc"

#           undef PIXEL_TYPE
#           undef CALC_WEIGHT_A
#undef BLEND_PIXEL
        } else {
#           define PIXEL_TYPE          uint32_t
#           define CALC_WEIGHT_A(w)    (w)
#define BLEND_PIXEL(p)                                                  \
               *(p) =                                                   \
                    ((((color & 0xFF00FF) * a + (dest & 0xFF00FF) * oma) >> 8) & 0xFF00FF) | \
                    ((((color & 0x00FF00) * a + (dest & 0x00FF00) * oma) >> 8) & 0x00FF00);
            const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);

#           include "GENERIC_DrawLineAA.inc"

#           undef PIXEL_TYPE
#           undef CALC_WEIGHT_A
#undef BLEND_PIXEL
        }
#else
        if (theColor.mAlpha != 255) {
#           define PIXEL_TYPE          uint32_t
#           define CALC_WEIGHT_A(w)    (((w) * (theColor.mAlpha+1)) >> 8)
#define BLEND_PIXEL(p)                                                  \
               *(p) =                                                   \
                    ((((color & aRMask) * a + (dest & aRMask) * oma) >> 8) & aRMask) | \
                    ((((color & aGMask) * a + (dest & aGMask) * oma) >> 8) & aGMask) | \
                    ((((color & aBMask) * a + (dest & aBMask) * oma) >> 8) & aBMask);
            const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);

#           include "GENERIC_DrawLineAA.inc"

#           undef PIXEL_TYPE
#           undef CALC_WEIGHT_A
#undef BLEND_PIXEL
        } else {
#           define PIXEL_TYPE          uint32_t
#           define CALC_WEIGHT_A(w)    (w)
#define BLEND_PIXEL(p)                                                  \
               *(p) =                                                   \
                    ((((color & aRMask) * a + (dest & aRMask) * oma) >> 8) & aRMask) | \
                    ((((color & aGMask) * a + (dest & aGMask) * oma) >> 8) & aGMask) | \
                    ((((color & aBMask) * a + (dest & aBMask) * oma) >> 8) & aBMask);
            const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);

#           include "GENERIC_DrawLineAA.inc"

#           undef PIXEL_TYPE
#           undef CALC_WEIGHT_A
#undef BLEND_PIXEL
        }
#endif
    } else if (aSurface->format->BitsPerPixel == 16) {
        ushort* aBits = (ushort*) aSurface->pixels;
#ifdef OPTIMIZE_SOFTWARE_DRAWING
        if (aGMask == 0x3E0) // 5-5-5
        {
#           define PIXEL_TYPE          ushort
#define BLEND_PIXEL(p)                                                  \
               {                                                        \
                    a >>= 3;                                            \
                    oma >>= 3;                                          \
                    uint32_t _src = (((color | (color << 16)) & 0x3E07C1F) * a >> 5) & 0x3E07C1F; \
                    uint32_t _dest = (((dest | (dest << 16)) & 0x3E07C1F) * oma >> 5) & 0x3E07C1F; \
                    *(p) = (_src | (_src>>16)) + (_dest | (_dest>>16)); \
               }
            const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);
            if (theColor.mAlpha != 255) {
#               define CALC_WEIGHT_A(w)    (((w) * (theColor.mAlpha+1)) >> 8)
#               include "GENERIC_DrawLineAA.inc"
#               undef CALC_WEIGHT_A
            } else {
#               define CALC_WEIGHT_A(w)    (w)
#               include "GENERIC_DrawLineAA.inc"
#               undef CALC_WEIGHT_A
            }
#           undef PIXEL_TYPE
#undef BLEND_PIXEL
        } else if (aGMask == 0x7E0) // 5-6-5
        {
#           define PIXEL_TYPE          ushort
#define BLEND_PIXEL(p)                                                  \
               {                                                        \
                    a >>= 3;                                            \
                    oma >>= 3;                                          \
                    uint32_t _src = (((color | (color << 16)) & 0x7E0F81F) * a >> 5) & 0x7E0F81F; \
                    uint32_t _dest = (((dest | (dest << 16)) & 0x7E0F81F) * oma >> 5) & 0x7E0F81F; \
                    *(p) = (_src | (_src>>16)) + (_dest | (_dest>>16)); \
               }
            const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);
            if (theColor.mAlpha != 255) {
#               define CALC_WEIGHT_A(w)    (((w) * (theColor.mAlpha+1)) >> 8)
#               include "GENERIC_DrawLineAA.inc"
#               undef CALC_WEIGHT_A
            } else {
#               define CALC_WEIGHT_A(w)    (w)
#               include "GENERIC_DrawLineAA.inc"
#               undef CALC_WEIGHT_A
            }
#           undef PIXEL_TYPE
#undef BLEND_PIXEL
        } else {
#endif
            if (theColor.mAlpha != 255) {
#               define PIXEL_TYPE          ushort
#               define CALC_WEIGHT_A(w)    (((w) * (theColor.mAlpha+1)) >> 8)
#define BLEND_PIXEL(p)                                                  \
                    *(p) =                                              \
                         ((((color & aRMask) * a + (dest & aRMask) * oma) >> 8) & aRMask) | \
                         ((((color & aGMask) * a + (dest & aGMask) * oma) >> 8) & aGMask) | \
                         ((((color & aBMask) * a + (dest & aBMask) * oma) >> 8) & aBMask);
                const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);

#               include "GENERIC_DrawLineAA.inc"

#               undef PIXEL_TYPE
#               undef CALC_WEIGHT_A
#undef BLEND_PIXEL
            } else {
#               define PIXEL_TYPE          ushort
#               define CALC_WEIGHT_A(w)    (w)
#define BLEND_PIXEL(p)                                                  \
                    *(p) =                                              \
                         ((((color & aRMask) * a + (dest & aRMask) * oma) >> 8) & aRMask) | \
                         ((((color & aGMask) * a + (dest & aGMask) * oma) >> 8) & aGMask) | \
                         ((((color & aBMask) * a + (dest & aBMask) * oma) >> 8) & aBMask);
                const int STRIDE = aSurface->pitch / sizeof (PIXEL_TYPE);

#               include "GENERIC_DrawLineAA.inc"

#               undef PIXEL_TYPE
#               undef CALC_WEIGHT_A
#undef BLEND_PIXEL
            }
#ifdef OPTIMIZE_SOFTWARE_DRAWING
        }
#endif
    }

    UnlockSurface();
}

void DDImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
}

void DDImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
    if (Check3D(this)) {
        mDDInterface->mD3DInterface->DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
        return;
    }

    if ((mDrawToBits) || (mHasAlpha) || (mHasTrans) || (mDDInterface->mIs3D)) {
        MemoryImage::DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
        return;
    }

    if (theStartY == theEndY) {
        int aStartX = (int) (std::min(theStartX, theEndX));
        int aEndX = (int) (std::max(theStartX, theEndX));

        FillRect(Rect(aStartX, (int) theStartY, aEndX - aStartX + 1, (int) (theEndY - theStartY + 1)), theColor, theDrawMode);
        return;
    } else if (theStartX == theEndX) {
        int aStartY = (int) (std::min(theStartY, theEndY));
        int aEndY = (int) (std::max(theStartY, theEndY));

        FillRect(Rect((int) theStartX, aStartY, (int) (theEndX - theStartX + 1), aEndY - aStartY + 1), theColor, theDrawMode);
        return;
    }

    CommitBits();

    switch (theDrawMode) {
    case Graphics::DRAWMODE_NORMAL:
        NormalDrawLineAA(theStartX, theStartY, theEndX, theEndY, theColor);
        break;
    case Graphics::DRAWMODE_ADDITIVE:
        AdditiveDrawLineAA(theStartX, theStartY, theEndX, theEndY, theColor);
        break;
    }

    DeleteAllNonSurfaceData();
}

void DDImage::CommitBits()
{
    if (mSurface == NULL) {
        MemoryImage::CommitBits();
        return;
    }
}

void DDImage::Create(int theWidth, int theHeight)
{
    delete [] mBits;

    mWidth = theWidth;
    mHeight = theHeight;

    mBits = NULL;

    BitsChanged();
}

void DDImage::BitsChanged()
{
    MemoryImage::BitsChanged();

    if (mSurface != NULL && mSurface != gSexyAppBase->GetGameSurface()) {
        SDL_FreeSurface(mSurface);
    }
    mSurface = NULL;
}

uint32_t* DDImage::GetBits()
{
    if (mBits == NULL) {
        if (mSurface == NULL)
            return MemoryImage::GetBits();

        if (mNoLock)
            return NULL;

        if (!LockSurface())
            return NULL;

        mBits = new uint32_t[mWidth * mHeight + 1];
        mBits[mWidth * mHeight] = MEMORYCHECK_ID;

        //int aRRound = (1 << (7 - mDDInterface->mRedBits));
        //int aGRound = (1 << (7 - mDDInterface->mGreenBits));
        //int aBRound = (1 << (7 - mDDInterface->mBlueBits));

        if (mSurface->format->BitsPerPixel == 16) {
            ushort* aSrcPixelsRow = (ushort*) mSurface->pixels;
            uint32_t* aDest = mBits;

            for (int y = 0; y < mHeight; y++) {
                ushort* aSrcPixels = aSrcPixelsRow;

                for (int x = 0; x < mWidth; x++) {
                    uint32_t src = *(aSrcPixels++);

                    int r = ((src >> mSurface->format->Rshift << (8 - mDDInterface->mRedBits)) & 0xFF);
                    int g = ((src >> mSurface->format->Gshift << (8 - mDDInterface->mGreenBits)) & 0xFF);
                    int b = ((src >> mSurface->format->Bshift << (8 - mDDInterface->mBlueBits)) & 0xFF);

                    *aDest++ = 0xFF000000 | (r << 16) | (g << 8) | (b);
                }

                aSrcPixelsRow += mSurface->pitch / 2;
            }
        } else if (mSurface->format->BitsPerPixel == 32) {
            uint32_t* aSrcPixelsRow = (uint32_t*) mSurface->pixels;
            uint32_t* aDest = mBits;

            for (int y = 0; y < mHeight; y++) {
                uint32_t* aSrcPixels = aSrcPixelsRow;

                for (int x = 0; x < mWidth; x++) {
                    uint32_t src = *(aSrcPixels++);

                    int r = (src >> mSurface->format->Rshift << (8 - mDDInterface->mRedBits)) & 0xFF;
                    int g = (src >> mSurface->format->Gshift << (8 - mDDInterface->mGreenBits)) & 0xFF;
                    int b = (src >> mSurface->format->Bshift << (8 - mDDInterface->mBlueBits)) & 0xFF;

                    *aDest++ = 0xFF000000 | (r << 16) | (g << 8) | (b);
                }

                aSrcPixelsRow += mSurface->pitch / 4;
            }
        }

        UnlockSurface();
    }

    return mBits;

}

void DDImage::NormalFillRect(const Rect& theRect, const Color& theColor)
{
    if (mNoLock)
        return;

    SDL_Surface* aSurface = GetSurface();

    if (!LockSurface())
        return;

    //FIXME change to Uint32 and Uint16 respectively
    uint32_t aRMask = aSurface->format->Rmask;
    uint32_t aGMask = aSurface->format->Gmask;
    uint32_t aBMask = aSurface->format->Bmask;

    uint32_t aRRoundAdd = aRMask;
    uint32_t aGRoundAdd = aGMask;
    uint32_t aBRoundAdd = aBMask;

    if (aSurface->format->BitsPerPixel == 16) {
        if (theColor.mAlpha == 255) {
            ushort aColor =
                    ((((theColor.mRed * aRMask) + aRRoundAdd) >> 8) & aRMask) |
                    ((((theColor.mGreen * aGMask) + aGRoundAdd) >> 8) & aGMask) |
                    ((((theColor.mBlue * aBMask) + aBRoundAdd) >> 8) & aBMask);

            ushort* aDestPixelsRow = ((ushort*) aSurface->pixels) + (theRect.mY * aSurface->pitch / 2) + theRect.mX;

            for (int y = 0; y < theRect.mHeight; y++) {
                ushort* aDestPixels = aDestPixelsRow;

                for (int x = 0; x < theRect.mWidth; x++)
                    *(aDestPixels++) = aColor;

                aDestPixelsRow += aSurface->pitch / 2;
            }
        } else {
            ushort src =
                    ((((((theColor.mRed * theColor.mAlpha + 0x80) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask) +
                    ((((((theColor.mGreen * theColor.mAlpha + 0x80) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) +
                    ((((((theColor.mBlue * theColor.mAlpha + 0x80) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask);
            int oma = 256 - theColor.mAlpha;

            ushort* aDestPixelsRow = ((ushort*) aSurface->pixels) + (theRect.mY * aSurface->pitch / 2) + theRect.mX;

            for (int y = 0; y < theRect.mHeight; y++) {
                ushort* aDestPixels = aDestPixelsRow;

                for (int x = 0; x < theRect.mWidth; x++) {
                    ushort dest = *aDestPixels;

                    *(aDestPixels++) = src +
                            (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
                            (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
                            (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);
                }

                aDestPixelsRow += aSurface->pitch / 2;
            }
        }
    } else if (aSurface->format->BitsPerPixel == 32) {
        if (theColor.mAlpha == 255) {
            uint32_t aColor =
                    ((((theColor.mRed * aRMask)) >> 8) & aRMask) |
                    ((((theColor.mGreen * aGMask)) >> 8) & aGMask) |
                    ((((theColor.mBlue * aBMask)) >> 8) & aBMask);

            uint32_t* aDestPixelsRow = ((uint32_t*) aSurface->pixels) + (theRect.mY * aSurface->pitch / 4) + theRect.mX;

            for (int y = 0; y < theRect.mHeight; y++) {
                uint32_t* aDestPixels = aDestPixelsRow;

                for (int x = 0; x < theRect.mWidth; x++)
                    *(aDestPixels++) = aColor;

                aDestPixelsRow += aSurface->pitch / 4;
            }
        } else {
            uint32_t src =
                    ((((((theColor.mRed * theColor.mAlpha + 0x7F) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask) +
                    ((((((theColor.mGreen * theColor.mAlpha + 0x7F) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) +
                    ((((((theColor.mBlue * theColor.mAlpha + 0x7F) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask);
            int oma = 256 - theColor.mAlpha;

            uint32_t* aDestPixelsRow = ((uint32_t*) aSurface->pixels) + (theRect.mY * aSurface->pitch / 4) + theRect.mX;

            for (int y = 0; y < theRect.mHeight; y++) {
                uint32_t* aDestPixels = aDestPixelsRow;

                for (int x = 0; x < theRect.mWidth; x++) {
                    uint32_t dest = *aDestPixels;

                    *(aDestPixels++) = src +
                            (((((dest & aRMask) * oma)) >> 8) & aRMask) +
                            (((((dest & aGMask) * oma)) >> 8) & aGMask) +
                            (((((dest & aBMask) * oma)) >> 8) & aBMask);
                }

                aDestPixelsRow += aSurface->pitch / 4;
            }
        }
    }


    UnlockSurface();
}

void DDImage::AdditiveFillRect(const Rect& theRect, const Color& theColor)
{
    if (mNoLock)
        return;

    //    SDL_Surface* aSurface = GetSurface();

    if (!LockSurface())
        return;

    uint32_t aRMask = mSurface->format->Rmask;
    uint32_t aGMask = mSurface->format->Gmask;
    uint32_t aBMask = mSurface->format->Bmask;

    //uint32_t aRRoundAdd = aRMask >> 1;
    //uint32_t aGRoundAdd = aGMask >> 1;
    //uint32_t aBRoundAdd = aBMask >> 1;

    int aRedShift = mSurface->format->Rshift;
    int aGreenShift = mSurface->format->Gshift;
    int aBlueShift = mSurface->format->Bshift;

    int* aMaxRedTable = mDDInterface->mRedAddTable;
    int* aMaxGreenTable = mDDInterface->mGreenAddTable;
    int* aMaxBlueTable = mDDInterface->mBlueAddTable;

    if (mSurface->format->BitsPerPixel == 16) {
        ushort rc = ((theColor.mRed * theColor.mAlpha) / 255) >> (8 - mDDInterface->mRedBits);
        ushort gc = ((theColor.mGreen * theColor.mAlpha) / 255) >> (8 - mDDInterface->mGreenBits);
        ushort bc = ((theColor.mBlue * theColor.mAlpha) / 255) >> (8 - mDDInterface->mBlueBits);

        ushort* aDestPixelsRow = ((ushort*) mSurface->pixels) + (theRect.mY * mSurface->pitch / 2) + theRect.mX;

        for (int y = 0; y < theRect.mHeight; y++) {
            ushort* aDestPixels = aDestPixelsRow;

            for (int x = 0; x < theRect.mWidth; x++) {
                ushort dest = *aDestPixels;

                int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
                int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
                int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

                *(aDestPixels++) =
                        (r << aRedShift) |
                        (g << aGreenShift) |
                        (b << aBlueShift);
            }

            aDestPixelsRow += mSurface->pitch / 2;
        }
    } else if (mSurface->format->BitsPerPixel == 32) {
        uint32_t rc = ((theColor.mRed * theColor.mAlpha) / 255) >> (8 - mDDInterface->mRedBits);
        uint32_t gc = ((theColor.mGreen * theColor.mAlpha) / 255) >> (8 - mDDInterface->mGreenBits);
        uint32_t bc = ((theColor.mBlue * theColor.mAlpha) / 255) >> (8 - mDDInterface->mBlueBits);

        uint32_t* aDestPixelsRow = ((uint32_t*) mSurface->pixels) + (theRect.mY * mSurface->pitch / 4) + theRect.mX;

        for (int y = 0; y < theRect.mHeight; y++) {
            uint32_t* aDestPixels = aDestPixelsRow;

            for (int x = 0; x < theRect.mWidth; x++) {
                uint32_t dest = *aDestPixels;

                int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + rc];
                int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + gc];
                int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + bc];

                *(aDestPixels++) =
                        (r << aRedShift) |
                        (g << aGreenShift) |
                        (b << aBlueShift);
            }

            aDestPixelsRow += mSurface->pitch / 4;
        }
    }

    UnlockSurface();
}

void DDImage::NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
    theImage->mDrawn = true;

    MemoryImage* aMemoryImage = dynamic_cast<MemoryImage*> (theImage);
    DDImage* aDDImage = dynamic_cast<DDImage*> (theImage);

    if (aMemoryImage != NULL) {
        aMemoryImage->CommitBits();

        Rect aDestRect(theX, theY, theX + theSrcRect.mWidth, theY + theSrcRect.mHeight);
        SDL_Rect aSDLDestRect = {theX, theY, theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};

        Rect aSrcRect(theSrcRect.mX, theSrcRect.mY, theSrcRect.mX + theSrcRect.mWidth, theSrcRect.mY + theSrcRect.mHeight);
        SDL_Rect aSDLSrcRect = {theSrcRect.mX, theSrcRect.mY, theSrcRect.mX + theSrcRect.mWidth, theSrcRect.mY + theSrcRect.mHeight};

        //TODO:
        if ((aMemoryImage->mIsVolatile) && ((aDDImage == NULL) || (aDDImage->mSurface == NULL)) &&
                (!mNoLock) && (theColor == Color::White)) {
            if (aMemoryImage->mColorTable == NULL) {
                uint32_t* aSrcBits = aMemoryImage->GetBits();

#               define SRC_TYPE uint32_t
#               define NEXT_SRC_COLOR (*(aSrcPixels++))

#               include "DDI_NormalBlt_Volatile.inc"

#               undef  SRC_TYPE
#               undef NEXT_SRC_COLOR
            } else {
                uint32_t* aColorTable = aMemoryImage->mColorTable;
                uchar* aSrcBits = aMemoryImage->mColorIndices;

#               define SRC_TYPE uchar
#               define NEXT_SRC_COLOR (aColorTable[*(aSrcPixels++)])

#               include "DDI_NormalBlt_Volatile.inc"

#               undef SRC_TYPE
#               undef NEXT_SRC_COLOR
            }
        } else if (((aMemoryImage->GetHasAlpha()) || (theColor != Color::White))) {
            if (mNoLock)
                return;

            if (!LockSurface())
                return;

            // Ensure NativeAlphaData is calculated
            void *aNativeData = aMemoryImage->GetNativeAlphaData(mDDInterface);

            // Ensure RunLength data is calculated
            uchar* aSrcRLAlphaData = aMemoryImage->GetRLAlphaData();

#define _PLUSPLUS ++
#define _PLUSEQUALS +=
            if (aMemoryImage->mColorTable == NULL) {
                uint32_t* aSrcPixelsRow = ((uint32_t*) aNativeData) + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
                uint32_t* aSrcPixels;

#               define NEXT_SRC_COLOR (*(aSrcPixels++))
#               define PEEK_SRC_COLOR (*aSrcPixels)

#               include "DDI_AlphaBlt.inc"

#               undef NEXT_SRC_COLOR
#               undef PEEK_SRC_COLOR
            } else {
                uint32_t* aNativeColorTable = (uint32_t*) aNativeData;

                uchar* aSrcPixelsRow = aMemoryImage->mColorIndices + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
                uchar* aSrcPixels;

#               define NEXT_SRC_COLOR (aNativeColorTable[*(aSrcPixels++)])
#               define PEEK_SRC_COLOR (aNativeColorTable[*aSrcPixels])

#               include "DDI_AlphaBlt.inc"

#               undef NEXT_SRC_COLOR
#               undef PEEK_SRC_COLOR
            }

#undef _PLUSPLUS
#undef _PLUSEQUALS
            UnlockSurface();
        } else if ((aDDImage == NULL) || (aDDImage->mSurface == NULL) || ((!mVideoMemory) && (aDDImage->mVideoMemory))) {
            if (mNoLock)
                return;

            //TODO: Have some sort of cool thing here

            if (!LockSurface())
                return;

            void* aNativeAlphaData = aMemoryImage->GetNativeAlphaData(mDDInterface);

            if (aMemoryImage->mColorTable == NULL) {
                uint32_t* aSrcPixelsRow = ((uint32_t*) aNativeAlphaData) + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
                uint32_t* aSrcPixels;

#               define NEXT_SRC_COLOR (*(aSrcPixels++))
#               define PEEK_SRC_COLOR (*aSrcPixels)

#               include "DDI_FastBlt_NoAlpha.inc"

#               undef NEXT_SRC_COLOR
#               undef PEEK_SRC_COLOR
            } else {
                uint32_t* aNativeAlphaColorTable = (uint32_t*) aNativeAlphaData;

                uchar* aSrcPixelsRow = aMemoryImage->mColorIndices + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
                uchar* aSrcPixels;

#               define NEXT_SRC_COLOR (aNativeAlphaColorTable[*(aSrcPixels++)])
#               define PEEK_SRC_COLOR (aNativeAlphaColorTable[*aSrcPixels])

#               include "DDI_FastBlt_NoAlpha.inc"

#               undef NEXT_SRC_COLOR
#               undef PEEK_SRC_COLOR
            }
        } else {

            if (mLockCount > 0)
                SDL_UnlockSurface(mSurface);

            if (aDDImage->mHasTrans) {
                SDL_SetColorKey(mSurface, SDL_SRCCOLORKEY, 0);
            }

            SDL_BlitSurface(aDDImage->GetSurface(), &aSDLSrcRect, mSurface, &aSDLDestRect);

            if (mLockCount > 0)
                SDL_LockSurface(mSurface);

        }
    }
}

void DDImage::NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
    theImage->mDrawn = true;

    Rect theSrcRect = theSrcRectOrig;
    theX += theSrcRect.mWidth - 1;

    MemoryImage* aMemoryImage = dynamic_cast<MemoryImage*> (theImage);
    //DDImage* aDDImage = dynamic_cast<DDImage*> (theImage);

    if (aMemoryImage != NULL) {
        aMemoryImage->CommitBits();

        if (mNoLock)
            return;

        if (!LockSurface())
            return;

        // Ensure NativeAlphaData is calculated
        void *aNativeData = aMemoryImage->GetNativeAlphaData(mDDInterface);

        // Ensure RunLength data is calculated
        uchar* aSrcRLAlphaData = aMemoryImage->GetRLAlphaData();

#define _PLUSPLUS --
#define _PLUSEQUALS -=
        if (aMemoryImage->mColorTable == NULL) {
            uint32_t* aSrcPixelsRow = ((uint32_t*) aNativeData) + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
            uint32_t* aSrcPixels;

#           define NEXT_SRC_COLOR (*(aSrcPixels++))
#           define PEEK_SRC_COLOR (*aSrcPixels)

#           include "DDI_AlphaBlt.inc"

#           undef NEXT_SRC_COLOR
#           undef PEEK_SRC_COLOR
        } else {
            uint32_t* aNativeColorTable = (uint32_t*) aNativeData;

            uchar* aSrcPixelsRow = aMemoryImage->mColorIndices + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
            uchar* aSrcPixels;

#           define NEXT_SRC_COLOR (aNativeColorTable[*(aSrcPixels++)])
#           define PEEK_SRC_COLOR (aNativeColorTable[*aSrcPixels])

#           include "DDI_AlphaBlt.inc"

#           undef NEXT_SRC_COLOR
#           undef PEEK_SRC_COLOR
        }

#undef _PLUSPLUS
#undef _PLUSEQUALS
        UnlockSurface();
    }
}

void DDImage::AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
    theImage->mDrawn = true;

    if (mNoLock)
        return;

    MemoryImage* aMemoryImage = dynamic_cast<MemoryImage*> (theImage);
    //DDImage* aDDImage = dynamic_cast<DDImage*> (theImage);

    if (aMemoryImage != NULL) {
        if (!LockSurface())
            return;

        // Ensure NativeAlphaData is calculated
        void* aNativeAlphaData = aMemoryImage->GetNativeAlphaData(mDDInterface);

#define _PLUSPLUS ++
#define _PLUSEQUALS +=
        if (aMemoryImage->mColorTable == NULL) {
            uint32_t* aSrcPixelsRow = ((uint32_t*) aNativeAlphaData) + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
            uint32_t* aSrcPixels;

#           define NEXT_SRC_COLOR (*(aSrcPixels++))
#           define PEEK_SRC_COLOR (*aSrcPixels)

#           include "DDI_Additive.inc"

#           undef NEXT_SRC_COLOR
#           undef PEEK_SRC_COLOR
        } else {
            uint32_t* aNativeAlphaColorTable = (uint32_t*) aNativeAlphaData;

            uchar* aSrcPixelsRow = aMemoryImage->mColorIndices + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
            uchar* aSrcPixels;

#           define NEXT_SRC_COLOR (aNativeAlphaColorTable[*(aSrcPixels++)])
#           define PEEK_SRC_COLOR (aNativeAlphaColorTable[*aSrcPixels])

#           include "DDI_Additive.inc"

#           undef NEXT_SRC_COLOR
#           undef PEEK_SRC_COLOR
        }
#undef _PLUSPLUS
#undef _PLUSEQUALS

        UnlockSurface();
    }
}

void DDImage::AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
    theImage->mDrawn = true;

    if (mNoLock)
        return;

    Rect theSrcRect = theSrcRectOrig;
    theX += theSrcRect.mWidth - 1;

    MemoryImage* aMemoryImage = dynamic_cast<MemoryImage*> (theImage);
    //DDImage* aDDImage = dynamic_cast<DDImage*> (theImage);

    if (aMemoryImage != NULL) {

        if (!LockSurface())
            return;

        // Ensure NativeAlphaData is calculated
        void* aNativeAlphaData = aMemoryImage->GetNativeAlphaData(mDDInterface);

#define _PLUSPLUS --
#define _PLUSEQUALS -=
        if (aMemoryImage->mColorTable == NULL) {
            uint32_t* aSrcPixelsRow = ((uint32_t*) aNativeAlphaData) + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
            uint32_t* aSrcPixels;

#           define NEXT_SRC_COLOR (*(aSrcPixels++))
#           define PEEK_SRC_COLOR (*aSrcPixels)

#           include "DDI_Additive.inc"

#           undef NEXT_SRC_COLOR
#           undef PEEK_SRC_COLOR
        } else {
            uint32_t* aNativeAlphaColorTable = (uint32_t*) aNativeAlphaData;

            uchar* aSrcPixelsRow = aMemoryImage->mColorIndices + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;
            uchar* aSrcPixels;

#           define NEXT_SRC_COLOR (aNativeAlphaColorTable[*(aSrcPixels++)])
#           define PEEK_SRC_COLOR (aNativeAlphaColorTable[*aSrcPixels])

#           include "DDI_Additive.inc"

#           undef NEXT_SRC_COLOR
#           undef PEEK_SRC_COLOR
        }
#undef _PLUSPLUS
#undef _PLUSEQUALS

        UnlockSurface();

    }
}

void DDImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
    theImage->mDrawn = true;

    assert((theColor.mRed >= 0) && (theColor.mRed <= 255));
    assert((theColor.mGreen >= 0) && (theColor.mGreen <= 255));
    assert((theColor.mBlue >= 0) && (theColor.mBlue <= 255));
    assert((theColor.mAlpha >= 0) && (theColor.mAlpha <= 255));

    CommitBits();

    if (Check3D(this)) {
        DDImage* aDDImage = dynamic_cast<DDImage*> (theImage);

        // Special short-circuit
        if ((aDDImage != NULL) && (aDDImage->mSurface != NULL) &&
                (mDDInterface->mD3DInterface->mTransformStack.empty()) &&
                (theDrawMode == Graphics::DRAWMODE_NORMAL) &&
                (theColor == Color::White) && (!aDDImage->mHasAlpha)) {

            if (mLockCount > 0)
                SDL_UnlockSurface(mSurface);

            SDL_Rect aSDLDestRect = {theX, theY, theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
            SDL_Rect aSDLSrcRect = {theSrcRect.mX, theSrcRect.mY, theSrcRect.mX + theSrcRect.mWidth, theSrcRect.mY + theSrcRect.mHeight};

            if (aDDImage->mHasTrans) {
                SDL_SetColorKey(mSurface, SDL_SRCCOLORKEY, 0);
            }
            SDL_BlitSurface(aDDImage->GetSurface(), &aSDLSrcRect, mSurface, &aSDLDestRect);

            if (mLockCount > 0)
                SDL_LockSurface(mSurface);

            return;
        }

        mDDInterface->mD3DInterface->Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
        return;
    }
    if ((mDrawToBits) || (mHasAlpha) || ((mHasTrans) && (!mFirstPixelTrans)) || (mDDInterface->mIs3D && this != mDDInterface->mOldCursorAreaImage)) {
        MemoryImage::Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
        return;
    }

    switch (theDrawMode) {
    case Graphics::DRAWMODE_NORMAL:
        NormalBlt(theImage, theX, theY, theSrcRect, theColor);
        break;
    case Graphics::DRAWMODE_ADDITIVE:
        AdditiveBlt(theImage, theX, theY, theSrcRect, theColor);
        break;
    }

    DeleteAllNonSurfaceData();
}

void DDImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
    assert((theColor.mRed >= 0) && (theColor.mRed <= 255));
    assert((theColor.mGreen >= 0) && (theColor.mGreen <= 255));
    assert((theColor.mBlue >= 0) && (theColor.mBlue <= 255));
    assert((theColor.mAlpha >= 0) && (theColor.mAlpha <= 255));

    CommitBits();

    if (Check3D(this)) {
        mDDInterface->mD3DInterface->BltMirror(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
        return;
    }

    switch (theDrawMode) {
    case Graphics::DRAWMODE_NORMAL:
        NormalBltMirror(theImage, theX, theY, theSrcRect, theColor);
        break;
    case Graphics::DRAWMODE_ADDITIVE:
        AdditiveBltMirror(theImage, theX, theY, theSrcRect, theColor);
        break;
    }

    DeleteAllNonSurfaceData();
}

void DDImage::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode)
{
    theImage->mDrawn = true;

    if (Check3D(this)) {
        FRect aClipRect(theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight);
        FRect aDestRect(theX, theY, theSrcRect.mWidth, theSrcRect.mHeight);

        FRect anIntersect = aDestRect.Intersection(aClipRect);
        if (anIntersect.mWidth != aDestRect.mWidth || anIntersect.mHeight != aDestRect.mHeight) {
            if (anIntersect.mWidth != 0 && anIntersect.mHeight != 0)
                mDDInterface->mD3DInterface->BltClipF(theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode);
        } else
            mDDInterface->mD3DInterface->Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode, true);

        return;
    } else

        BltRotated(theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode, 0, 0, 0);
}

void DDImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
    theImage->mDrawn = true;

    if (mNoLock)
        return;

    CommitBits();

    if (Check3D(this)) {
        mDDInterface->mD3DInterface->BltRotated(theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode, theRot, theRotCenterX, theRotCenterY);
        return;
    }

    if ((mDrawToBits) || (mHasAlpha) || ((mHasTrans) && (!mFirstPixelTrans)) || (mDDInterface->mIs3D)) {
        MemoryImage::BltRotated(theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode, theRot, theRotCenterX, theRotCenterY);
        return;
    }

    // This BltRotatedClipHelper clipping used to happen in Graphics::DrawImageRotated
    FRect aDestRect;
    if (!BltRotatedClipHelper(theX, theY, theSrcRect, theClipRect, theRot, aDestRect, theRotCenterX, theRotCenterY))
        return;

    MemoryImage* aMemoryImage = dynamic_cast<MemoryImage*> (theImage);
    //DDImage* aDDImage = dynamic_cast<DDImage*> (theImage);

    if (aMemoryImage != NULL) {
        aMemoryImage->CommitBits();

        if (theDrawMode == Graphics::DRAWMODE_NORMAL) {
            if (aMemoryImage->mColorTable == NULL) {
                uint32_t* aSrcBits = aMemoryImage->GetBits() + theSrcRect.mX + theSrcRect.mY * aMemoryImage->GetWidth();

#               define SRC_TYPE uint32_t
#               define READ_COLOR(ptr) (*(ptr))

#               include "DDI_BltRotated.inc"

#               undef SRC_TYPE
#               undef READ_COLOR

            } else {
                uint32_t* aColorTable = aMemoryImage->mColorTable;
                uchar* aSrcBits = aMemoryImage->mColorIndices + theSrcRect.mX + theSrcRect.mY * aMemoryImage->GetWidth();

#               define SRC_TYPE uchar
#               define READ_COLOR(ptr) (aColorTable[*(ptr)])

#               include "DDI_BltRotated.inc"

#               undef SRC_TYPE
#               undef READ_COLOR
            }
        } else {
            if (aMemoryImage->mColorTable == NULL) {
                uint32_t* aSrcBits = aMemoryImage->GetBits() + theSrcRect.mX + theSrcRect.mY * aMemoryImage->GetWidth();

#               define SRC_TYPE uint32_t
#               define READ_COLOR(ptr) (*(ptr))

#               include "DDI_BltRotated_Additive.inc"

#               undef SRC_TYPE
#               undef READ_COLOR

            } else {
                uint32_t* aColorTable = aMemoryImage->mColorTable;
                uchar* aSrcBits = aMemoryImage->mColorIndices + theSrcRect.mX + theSrcRect.mY * aMemoryImage->GetWidth();

#               define SRC_TYPE uchar
#               define READ_COLOR(ptr) (aColorTable[*(ptr)])

#               include "DDI_BltRotated_Additive.inc"

#               undef SRC_TYPE
#               undef READ_COLOR
            }
        }
    }
    DeleteAllNonSurfaceData();
}

void DDImage::StretchBlt(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
    theImage->mDrawn = true;

    DDImage* aSrcDDImage = dynamic_cast<DDImage*> (theImage);
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*> (theImage);

    CommitBits();

    if (Check3D(this)) {
        mDDInterface->mD3DInterface->StretchBlt(theImage, theDestRectOrig, theSrcRectOrig, theClipRect, theColor, theDrawMode, fastStretch);
        return;
    }

    Rect theDestRect;
    FRect theSrcRect;
    if (!StretchBltClipHelper(theSrcRectOrig, theClipRect, theDestRectOrig, theSrcRect, theDestRect))
        return;

    //TODO investigate why StretchBltClipHelpder doesn't catch this
    if (theDestRect.mWidth == 0 || theDestRect.mHeight == 0 || theSrcRect.mWidth == 0 || theSrcRect.mHeight == 0)
        return;

    if (fastStretch) {

        if ((aSrcDDImage != NULL) && (theColor == Color::White) && (theDrawMode == Graphics::DRAWMODE_NORMAL) &&
                (!aSrcDDImage->mHasAlpha) && (aSrcDDImage->GetSurface() != NULL)) {

            //SDL_Surface* aSrcSurface = aSrcDDImage->GetSurface();

            SDL_Rect aDestRect = {(Sint16) theDestRect.mX, (Sint16) theDestRect.mY, (Uint16) (theDestRect.mX + theDestRect.mWidth),
                (Uint16) (theDestRect.mY + theDestRect.mHeight)};
            SDL_Rect aSrcRect = {(Sint16) theSrcRect.mX, (Sint16) theSrcRect.mY, (Uint16) (theSrcRect.mX + theSrcRect.mWidth),
                (Uint16) (theSrcRect.mY + theSrcRect.mHeight)};

            if (mLockCount > 0)
                SDL_UnlockSurface(mSurface);

            if (aSrcDDImage->mHasTrans) {
                SDL_SetColorKey(mSurface, SDL_SRCCOLORKEY, 0);
            }

            SDL_BlitSurface(mSurface, &aSrcRect, mSurface, &aDestRect);

            if (mLockCount > 0)
                SDL_LockSurface(mSurface);

        } else {
            if (aSrcMemoryImage != NULL) {
                aSrcMemoryImage->CommitBits();

                // Ensure NativeAlphaData is calculated
                void *aNativeAlphaData = aSrcMemoryImage->GetNativeAlphaData(mDDInterface);

#define _PLUSPLUS ++
                if (theDrawMode == Graphics::DRAWMODE_NORMAL) {
                    if (aSrcMemoryImage->mColorTable == NULL) {
                        uint32_t* aSrcBits = ((uint32_t*) aNativeAlphaData);

#                       define SRC_TYPE uint32_t
#                       define READ_COLOR(ptr) (*(ptr))

#                       include "DDI_FastStretch.inc"

#                       undef SRC_TYPE
#                       undef READ_COLOR
                    } else {
                        uint32_t* aColorTable = (uint32_t*) aNativeAlphaData;
                        uchar* aSrcBits = aSrcMemoryImage->mColorIndices;

#                       define SRC_TYPE uchar
#                       define READ_COLOR(ptr) (aColorTable[*(ptr)])

#                       include "DDI_FastStretch.inc"

#                       undef SRC_TYPE
#                       undef READ_COLOR
                    }
                } else {
                    if (aSrcMemoryImage->mColorTable == NULL) {
                        uint32_t* aSrcBits = ((uint32_t*) aNativeAlphaData);

#                       define SRC_TYPE uint32_t
#                       define READ_COLOR(ptr) (*(ptr))

#                       include "DDI_FastStretch_Additive.inc"

#                       undef SRC_TYPE
#                       undef READ_COLOR
                    } else {
                        uint32_t* aColorTable = (uint32_t*) aNativeAlphaData;
                        uchar* aSrcBits = aSrcMemoryImage->mColorIndices;

#                       define SRC_TYPE uchar
#                       define READ_COLOR(ptr) (aColorTable[*(ptr)])

#                       include "DDI_FastStretch_Additive.inc"

#                       undef SRC_TYPE
#                       undef READ_COLOR
                    }
                }

#undef _PLUSPLUS
            }

        }

    } else {
        if ((mDrawToBits) || (mHasAlpha) || (mHasTrans) || (mDDInterface->mIs3D)) {
            MemoryImage::StretchBlt(theImage, theDestRectOrig, theSrcRectOrig, theClipRect, theColor, theDrawMode, fastStretch);
            return;
        }

        // Stretch it to a temporary image
        MemoryImage aTempImage(mApp);
        Rect aTempRect(0, 0, theDestRect.mWidth, theDestRect.mHeight);

        aTempImage.Create(theDestRect.mWidth, theDestRect.mHeight);
        if (fastStretch)
            aTempImage.FastStretchBlt(theImage, aTempRect, theSrcRect, theColor, 0);
        else
            aTempImage.SlowStretchBlt(theImage, aTempRect, theSrcRect, theColor, 0);

        Blt(&aTempImage, theDestRect.mX, theDestRect.mY, aTempRect, theColor, theDrawMode);
    }

    DeleteAllNonSurfaceData();
}

void DDImage::StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
    theImage->mDrawn = true;

    //DDImage* aSrcDDImage = dynamic_cast<DDImage*> (theImage);
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*> (theImage);

    CommitBits();

    if (Check3D(this)) {
        mDDInterface->mD3DInterface->StretchBlt(theImage, theDestRectOrig, theSrcRectOrig, theClipRect, theColor, theDrawMode, fastStretch, true);
        return;
    }

    FRect theSrcRect;
    Rect theDestRect;

    if (!StretchBltMirrorClipHelper(theSrcRectOrig, theClipRect, theDestRectOrig, theSrcRect, theDestRect))
        return;

    theDestRect.mX += theDestRect.mWidth - 1;

    if (aSrcMemoryImage != NULL) {
        aSrcMemoryImage->CommitBits();


        // Ensure NativeAlphaData is calculated
        void *aNativeAlphaData = aSrcMemoryImage->GetNativeAlphaData(mDDInterface);

#define _PLUSPLUS --
        if (theDrawMode == Graphics::DRAWMODE_NORMAL) {
            if (aSrcMemoryImage->mColorTable == NULL) {
                uint32_t* aSrcBits = ((uint32_t*) aNativeAlphaData);

#               define SRC_TYPE uint32_t
#               define READ_COLOR(ptr) (*(ptr))

#               include "DDI_FastStretch.inc"

#               undef SRC_TYPE
#               undef READ_COLOR
            } else {
                uint32_t* aColorTable = (uint32_t*) aNativeAlphaData;
                uchar* aSrcBits = aSrcMemoryImage->mColorIndices;

#               define SRC_TYPE uchar
#               define READ_COLOR(ptr) (aColorTable[*(ptr)])

#               include "DDI_FastStretch.inc"

#               undef SRC_TYPE
#               undef READ_COLOR
            }
        } else {
            //NOT IMPLEMENTED YET
            assert(false);

#if 0

            if (aSrcMemoryImage->mColorTable == NULL) {
                //uint32_t* aSrcBits = aSrcMemoryImage->GetBits();
                uint32_t* aSrcBits = ((uint32_t*) aNativeAlphaData);

#               define SRC_TYPE uint32_t
#               define READ_COLOR(ptr) (*(ptr))

#               include "DDI_FastStretch_Additive.inc"

#               undef SRC_TYPE
#               undef READ_COLOR
            } else {
                uint32_t* aColorTable = (uint32_t*) aNativeAlphaData;
                uchar* aSrcBits = aSrcMemoryImage->mColorIndices;

#               define SRC_TYPE uchar
#               define READ_COLOR(ptr) (aColorTable[*(ptr)])

#               include "DDI_FastStretch_Additive.inc"

#               undef SRC_TYPE
#               undef READ_COLOR
            }
#endif
        }

#undef _PLUSPLUS
    }

    DeleteAllNonSurfaceData();
}

void DDImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
    theImage->mDrawn = true;

    if (Check3D(this)) {
        mDDInterface->mD3DInterface->BltTransformed(theImage, &theClipRect, theColor, theDrawMode, theSrcRect, theMatrix, blend, x, y, true);
        return;
    }

    if (!LockSurface())
        return;

    int aPixelFormat;
    if (mSurface->format->BitsPerPixel == 32)
        aPixelFormat = 0x888;
    else if (mDDInterface->mRedMask == 0xf800 && mDDInterface->mGreenMask == 0x07e0 && mDDInterface->mBlueMask == 0x001f)
        aPixelFormat = 0x565;
    else if (mDDInterface->mRedMask == 0x7c00 && mDDInterface->mGreenMask == 0x03e0 && mDDInterface->mBlueMask == 0x001f)
        aPixelFormat = 0x555;
    else
        assert(false);

    BltMatrixHelper(theImage, x, y, theMatrix, theClipRect, theColor, theDrawMode, theSrcRect, mSurface, mSurface->pitch, aPixelFormat, blend);

    UnlockSurface();
    DeleteAllNonSurfaceData();
}

void DDImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
    theTexture->mDrawn = true;

    if (Check3D(this)) {
        mDDInterface->mD3DInterface->DrawTrianglesTex(theVertices, theNumTriangles, theColor, theDrawMode, theTexture, tx, ty, blend);
        return;
    }
    //NOT IMPLEMENTED YET
    assert(false);

#if 0

    LPDIRECTDRAWSURFACE aSurface = GetSurface();
    if (!LockSurface())
        return;

    int aPixelFormat;
    if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
        aPixelFormat = 0x888;
    else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask == 0xf800 && mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask == 0x07e0 && mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask == 0x001f)
        aPixelFormat = 0x565;
    else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask == 0x7c00 && mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask == 0x03e0 && mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask == 0x001f)
        aPixelFormat = 0x555;
    else
        assert(FALSE);

    BltTrianglesTexHelper(theTexture, theVertices, theNumTriangles, theClipRect, theColor, theDrawMode, mLockedSurfaceDesc.lpSurface, mLockedSurfaceDesc.lPitch, aPixelFormat, tx, ty, blend);
    UnlockSurface();
#endif
    DeleteAllNonSurfaceData();
}

bool DDImage::Palletize()
{
    if (MemoryImage::Palletize()) {
        // Don't keep around the DDSurface if we palletize the image, that
        // would be a waste of memory
        DeleteDDSurface();
        return true;
    } else {
        return false;
    }
}

void DDImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const unsigned char* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
    if (theSpanCount == 0) return;
    //NOT IMPLEMENTED YET
    assert(false);

#if 0

    if (Check3D(this)) { // ugh!#@$
        int l = theSpans[0].mX, t = theSpans[0].mY;
        int r = l + theSpans[0].mWidth, b = t;
        for (int i = 1; i < theSpanCount; ++i) {
            l = min(theSpans[i].mX, l);
            r = max(theSpans[i].mX + theSpans[i].mWidth - 1, r);
            t = min(theSpans[i].mY, t);
            b = max(theSpans[i].mY, b);
        }
        for (int i = 0; i < theSpanCount; ++i) {
            theSpans[i].mX -= l;
            theSpans[i].mY -= t;
        }

        MemoryImage aTempImage;
        aTempImage.Create(r - l + 1, b - t + 1);
        aTempImage.FillScanLinesWithCoverage(theSpans, theSpanCount, theColor, theDrawMode, theCoverage, theCoverX - l, theCoverY - t, theCoverWidth, theCoverHeight);
        Blt(&aTempImage, l, t, Rect(0, 0, r - l + 1, b - t + 1), Color::White, theDrawMode);
        return;
    }

    LPDIRECTDRAWSURFACE aSurface = GetSurface();

    if (!LockSurface())
        return;

    uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
    uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
    uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

    uint32_t aRRoundAdd = aRMask >> 1;
    uint32_t aGRoundAdd = aGMask >> 1;
    uint32_t aBRoundAdd = aBMask >> 1;

    if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16) {
        //ushort src_red        = (((theColor.mRed * (theColor.mAlpha+1)) >> 8) * aRMask) & aRMask;
        //ushort src_green  = (((theColor.mGreen * (theColor.mAlpha+1)) >> 8) * aGMask) & aGMask;
        //ushort src_blue       = (((theColor.mBlue * (theColor.mAlpha+1)) >> 8) * aBMask) & aBMask;
        ushort src =
                (((theColor.mRed * aRMask) >> 8) & aRMask) |
                (((theColor.mGreen * aGMask) >> 8) & aGMask) |
                (((theColor.mBlue * aBMask) >> 8) & aBMask);
        ushort* theBits = (ushort*) mLockedSurfaceDesc.lpSurface;

        for (int i = 0; i < theSpanCount; ++i) {
            Span* aSpan = &theSpans[i];
            int x = aSpan->mX - theCoverX;
            int y = aSpan->mY - theCoverY;

            ushort* aDestPixels = &theBits[aSpan->mY * mWidth + aSpan->mX];
            const BYTE* aCoverBits = &theCoverage[y * theCoverWidth + x];
            for (int w = 0; w < aSpan->mWidth; ++w) {
                int cover = *aCoverBits++;
                int a = ((cover + 1) * theColor.mAlpha) >> 8;
                int oma = 256 - a;
                ushort dest = *aDestPixels;

                *(aDestPixels++) =
                        ((((dest & aRMask) * oma + (src & aRMask) * a) >> 8) & aRMask) |
                        ((((dest & aGMask) * oma + (src & aGMask) * a) >> 8) & aGMask) |
                        ((((dest & aBMask) * oma + (src & aBMask) * a) >> 8) & aBMask);
            }
        }
    } else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32) {
        //uint32_t src_red      = (((theColor.mRed * (theColor.mAlpha+1)) >> 8) * aRMask) & aRMask;
        //uint32_t src_green        = (((theColor.mGreen * (theColor.mAlpha+1)) >> 8) * aGMask) & aGMask;
        //uint32_t src_blue     = (((theColor.mBlue * (theColor.mAlpha+1)) >> 8) * aBMask) & aBMask;
        uint32_t src =
                (((theColor.mRed * aRMask) >> 8) & aRMask) |
                (((theColor.mGreen * aGMask) >> 8) & aGMask) |
                (((theColor.mBlue * aBMask) >> 8) & aBMask);
        uint32_t* theBits = (uint32_t*) mLockedSurfaceDesc.lpSurface;

        for (int i = 0; i < theSpanCount; ++i) {
            Span* aSpan = &theSpans[i];
            int x = aSpan->mX - theCoverX;
            int y = aSpan->mY - theCoverY;

            uint32_t* aDestPixels = &theBits[aSpan->mY * mWidth + aSpan->mX];
            const BYTE* aCoverBits = &theCoverage[y * theCoverWidth + x];
            for (int w = 0; w < aSpan->mWidth; ++w) {
                int cover = *aCoverBits++;
                int a = ((cover + 1) * theColor.mAlpha) >> 8;
                int oma = 256 - a;
                uint32_t dest = *aDestPixels;

                *(aDestPixels++) =
                        ((((dest & aRMask) * oma + (src & aRMask) * a) >> 8) & aRMask) |
                        ((((dest & aGMask) * oma + (src & aGMask) * a) >> 8) & aGMask) |
                        ((((dest & aBMask) * oma + (src & aBMask) * a) >> 8) & aBMask);
            }
        }
    }

    UnlockSurface();
#endif
    DeleteAllNonSurfaceData();
}
