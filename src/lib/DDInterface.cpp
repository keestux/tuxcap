#define INITGUID

#include "DDInterface.h"
#include "DDImage.h"
#include "Common.h"
#include "D3DInterface.h"
#if 0

#include <assert.h>
#include "D3DTester.h"
#include "AutoCrit.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "DirectXErrorString.h"
#endif

#include "SexyAppBase.h"
#include "Graphics.h"
#include "MemoryImage.h"

using namespace Sexy;

#if 0
typedef HRESULT (WINAPI *DirectDrawCreateFunc)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
typedef HRESULT (WINAPI *DirectDrawCreateExFunc)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

extern HMODULE gDDrawDLL;
static DirectDrawCreateFunc gDirectDrawCreateFunc = NULL;
static DirectDrawCreateExFunc gDirectDrawCreateExFunc = NULL;
#endif

DDInterface::DDInterface(SexyAppBase* theApp)
{
    mApp = theApp;
#if 0
    mDD = NULL;
    mDD7 = NULL;
#endif
    mRedAddTable = NULL;
    mGreenAddTable = NULL;
    mBlueAddTable = NULL;
    mInitialized = false;
    mVideoOnlyDraw = false;
    mScanLineFailCount = 0;

    //TODO: Standards, anyone?
    mCursorX = 0;
    mCursorY = 0;
    mCursorWidth = 64;
    mCursorHeight = 64;
    mCursorImage = NULL;
    mOldCursorArea = NULL;
    mHasOldCursorArea = false;
    mOldCursorAreaImage = NULL;
    mInitCount = 0;
    mRefreshRate = 60;
    mMillisecondsPerFrame = 1000/mRefreshRate;

    mD3DInterface = new D3DInterface;

#if 0
    mD3DTester = NULL;

    gDirectDrawCreateFunc = (DirectDrawCreateFunc)GetProcAddress(gDDrawDLL,"DirectDrawCreate");
    gDirectDrawCreateExFunc = (DirectDrawCreateExFunc)GetProcAddress(gDDrawDLL,"DirectDrawCreateEx");
#endif

    mScreenImage = NULL;
    mPrimarySurface = NULL;
    mSecondarySurface = NULL;
    mDrawSurface = NULL;
}

DDInterface::~DDInterface()
{
    delete [] mRedAddTable;
    delete [] mGreenAddTable;
    delete [] mBlueAddTable;

    Cleanup();
    delete mD3DInterface;

#if 0
    delete mD3DTester;
#endif
}

std::string DDInterface::ResultToString(int theResult)
{
    switch (theResult)
    {
    case RESULT_OK:
        return "RESULT_OK";
    case RESULT_FAIL:
        return "RESULT_FAIL";
    case RESULT_DD_CREATE_FAIL:
        return "RESULT_DD_CREATE_FAIL";
    case RESULT_SURFACE_FAIL:
        return "RESULT_SURFACE_FAIL";
    case RESULT_EXCLUSIVE_FAIL:
        return "RESULT_EXCLUSIVE_FAIL";
    case RESULT_DISPCHANGE_FAIL:
        return "RESULT_DISPCHANGE_FAIL";
    case RESULT_INVALID_COLORDEPTH:
        return "RESULT_INVALID_COLORDEPTH";
    default:
        return "RESULT_UNKNOWN";
    }
}

#if 0
bool DDInterface::GotDXError(HRESULT theResult, const char *theContext)
{
    if (!SUCCEEDED(theResult))
    {
        std::string anError = GetDirectXErrorString(theResult);
        mErrorString = StrFormat("%s: %s",theContext, anError.c_str());

        return true;
    }
    else
        return false;
}
#endif

DDImage* DDInterface::GetScreenImage()
{
    return mScreenImage;
}

#if 0
void DDInterface::ClearSurface(LPDIRECTDRAWSURFACE theSurface)
{
    if (theSurface)
    {
        DDSURFACEDESC desc;
        memset(&desc, 0, sizeof desc);
        desc.dwSize = sizeof desc;
        HRESULT hr = theSurface->Lock(NULL, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
        if ( DD_OK == hr )
        {
            DWORD pixelSize = desc.ddpfPixelFormat.dwRGBBitCount / 8;
            unsigned char* p = (unsigned char*)desc.lpSurface;
            for ( DWORD row = 0; row < desc.dwHeight; ++row )
            {
                memset(p, 0, pixelSize*desc.dwWidth);
                p += desc.lPitch;
            }
            theSurface->Unlock(NULL);
        }
    }
}

bool DDInterface::Do3DTest(HWND theHWND)
{
    if (mD3DTester == NULL)
    {
        if (mApp->mTest3D || mApp->mAutoEnable3D)
        {
            mD3DTester = new D3DTester;
            mD3DTester->SetVidMemoryConstraints(mApp->mMinVidMemory3D, mApp->mRecommendedVidMemory3D);
            mD3DTester->TestD3D(theHWND, mDD7);

            if (mApp->mAutoEnable3D && mD3DTester->Is3DRecommended())
                mIs3D = true;

            if (!mD3DTester->Is3DSupported())
                mIs3D = false;

            if (mD3DTester->ResultsChanged() && !mD3DTester->Is3DRecommended())
                mIs3D = false;

            return true;
        }
    }

    return false;
}
#endif

static inline int count_bits(Uint32 x)
{
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if (((x >> i) & 1)) {
            count++;
        }
    }
    return count;
}

int DDInterface::Init(HWND theWindow)
{
#if 0
    AutoCrit anAutoCrit(mCritSect);
#endif
    mInitialized = false;

    Cleanup();

    mWidth = mApp->mWidth;
    mHeight = mApp->mHeight;
    mAspect.Set(mWidth, mHeight);
#if 0
    mDesktopWidth = GetSystemMetrics(SM_CXSCREEN);
    mDesktopHeight = GetSystemMetrics(SM_CYSCREEN);
    mDesktopAspect.Set(mDesktopWidth, mDesktopHeight);
#endif
    mDisplayWidth = mWidth;
    mDisplayHeight = mHeight;
    mDisplayAspect = mAspect;
    mPresentationRect = Rect( 0, 0, mWidth, mHeight );
    // ???? FIXME. Why was this needed? mApp->mScreenBounds = mPresentationRect;
    mFullscreenBits = mApp->mFullscreenBits;
    mHasOldCursorArea = false;
#if SDL_VERSION_ATLEAST(2,0,0)
    // ???? Why do we need to create a surface for OldCursorArea?
#else
    CreateSurface(&mOldCursorArea, mCursorWidth, mCursorHeight, true);
#endif

#if SDL_VERSION_ATLEAST(2,0,0)
    SDL_SetSurfaceAlphaMod(mOldCursorArea, 0);
#else
    SDL_SetAlpha(mOldCursorArea,0,0);
#endif

    mOldCursorAreaImage = new DDImage(this);
    mOldCursorAreaImage->SetSurface(mOldCursorArea);
    mOldCursorAreaImage->SetImageMode(false, false);

#if 0
    CreateSurface(&mPrimarySurface, mWidth,mHeight,true);
    CreateSurface(&mDrawSurface, mWidth,mHeight,true);
#endif

    SetVideoOnlyDraw(mVideoOnlyDraw);

    // Get data from the primary surface
    if (mScreenImage->mSurface != NULL)
    {

        //FIXME SDL stores this for us in the surface, so use it!

        if ((mScreenImage->mSurface->format->BitsPerPixel != 16) &&
            (mScreenImage->mSurface->format->BitsPerPixel != 32)) {
            return RESULT_FAIL;
        }

        mRGBBits = mScreenImage->mSurface->format->BitsPerPixel;
        mRedMask   = mScreenImage->mSurface->format->Rmask;
        mGreenMask = mScreenImage->mSurface->format->Gmask;
        mBlueMask  = mScreenImage->mSurface->format->Bmask;

        mRedShift   = mScreenImage->mSurface->format->Rshift;
        mGreenShift = mScreenImage->mSurface->format->Gshift;
        mBlueShift  = mScreenImage->mSurface->format->Bshift;

        mRedBits   = count_bits(mRedMask);
        mGreenBits = count_bits(mGreenMask);
        mBlueBits  = count_bits(mBlueMask);

        delete [] mRedAddTable;
        delete [] mGreenAddTable;
        delete [] mBlueAddTable;

        int aMaxR = (1<<mRedBits) - 1;
        int aMaxG = (1<<mGreenBits) - 1;
        int aMaxB = (1<<mBlueBits) - 1;

        mRedAddTable = new int[aMaxR*2+1];
        mGreenAddTable = new int[aMaxG*2+1];
        mBlueAddTable = new int[aMaxB*2+1];

        for (int i = 0; i < aMaxR*2+1; i++)
            mRedAddTable[i] = std::min(i, aMaxR);
        for (int i = 0; i < aMaxG*2+1; i++)
            mGreenAddTable[i] = std::min(i, aMaxG);
        for (int i = 0; i < aMaxB*2+1; i++)
            mBlueAddTable[i] = std::min(i, aMaxB);

        // Create the tables that we will use to convert from
        // internal color representation to surface representation
        for (int i = 0; i < 256; i++)
        {
            mRedConvTable[i] = ((i * mRedMask) / 255) & mRedMask;
            mGreenConvTable[i] = ((i * mGreenMask) / 255) & mGreenMask;
            mBlueConvTable[i] = ((i * mBlueMask) / 255) & mBlueMask;
        }
    }

    if (mIs3D)
    {
        if(!mD3DInterface->InitFromDDInterface(this))
        {
            mErrorString = "3D init error: ";
            mErrorString += D3DInterface::mErrorString;
            return RESULT_3D_FAIL;
        }
    }

    mInitCount++;
    mInitialized = true;

    return RESULT_OK;
}

void DDInterface::SetVideoOnlyDraw(bool videoOnlyDraw)
{
#if 0
    AutoCrit anAutoCrit(mCritSect);
#endif
    mVideoOnlyDraw = videoOnlyDraw;

#if 0
    if (mSecondarySurface == NULL)
    {

        if (CreateSurface(&mSecondarySurface, mWidth,mHeight,true) == RESULT_FAIL)
            mVideoOnlyDraw = false;
    }
#endif
    //bool useSecondary = mVideoOnlyDraw;
    delete mScreenImage;
    mScreenImage = new DDImage(this);
    mScreenImage->SetSurface(gSexyAppBase->GetGameSurface());/*useSecondary ? mSecondarySurface : mDrawSurface);*/
    mScreenImage->mNoLock = mVideoOnlyDraw;
    mScreenImage->mVideoMemory = mVideoOnlyDraw;
    mScreenImage->SetImageMode(false, false);
}


void DDInterface::RemapMouse(int& theX, int& theY)
{
    if (mInitialized)
    {
        theX = ( theX - mPresentationRect.mX ) * mWidth / mPresentationRect.mWidth;
        theY = ( theY - mPresentationRect.mY ) * mHeight / mPresentationRect.mHeight;
    }
}

uint32_t DDInterface::GetColorRef(uint32_t theRGB)
{
#if 0
    DDSURFACEDESC aDesc;

    ZeroMemory(&aDesc, sizeof(aDesc));
    aDesc.dwSize = sizeof(aDesc);
    aDesc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    HRESULT aResult = mPrimarySurface->GetSurfaceDesc(&aDesc);

    BYTE bRed   = (BYTE) ((theRGB >> 16) & 0xFF);
    BYTE bGreen = (BYTE) ((theRGB >>  8) & 0xFF);
    BYTE bBlue  = (BYTE) ((theRGB      ) & 0xFF);

    uint32_t aColor;
    aColor = ((DWORD(((LONGLONG)bRed * (LONGLONG)aDesc.ddpfPixelFormat.dwRBitMask) / 255) & aDesc.ddpfPixelFormat.dwRBitMask) |
            (DWORD(((LONGLONG)bGreen * (LONGLONG)aDesc.ddpfPixelFormat.dwGBitMask) / 255) & aDesc.ddpfPixelFormat.dwGBitMask) |
            (DWORD(((LONGLONG)bBlue * (LONGLONG)aDesc.ddpfPixelFormat.dwBBitMask) / 255) & aDesc.ddpfPixelFormat.dwBBitMask));

    return aColor;
#else
    return 0x00000000;
#endif
}

void DDInterface::AddDDImage(DDImage* theDDImage)
{

#if 0
    AutoCrit anAutoCrit(mCritSect);
#endif

    mDDImageSet.insert(theDDImage);
}

void DDInterface::RemoveDDImage(DDImage* theDDImage)
{

#if 0
    AutoCrit anAutoCrit(mCritSect);
#endif

    DDImageSet::iterator anItr = mDDImageSet.find(theDDImage);
    if (anItr != mDDImageSet.end())
        mDDImageSet.erase(anItr);
}


void DDInterface::Remove3DData(Image* theImage) // for 3d texture cleanup
{
    mD3DInterface->RemoveImage(theImage);
}


void DDInterface::Cleanup()
{

#if 0
    AutoCrit anAutoCrit(mCritSect);
#endif

    mInitialized = false;
    mD3DInterface->Cleanup();

    if (mScreenImage != NULL)
    {
        delete mScreenImage;
        mScreenImage = NULL;
    }

    if (mDrawSurface != NULL)
    {
        SDL_FreeSurface(mDrawSurface);
        mDrawSurface = NULL;
    }

    if (mSecondarySurface != NULL)
    {
        SDL_FreeSurface(mSecondarySurface);
        mSecondarySurface = NULL;
    }

    if (mPrimarySurface != NULL)
    {
        SDL_FreeSurface(mPrimarySurface);
        mPrimarySurface = NULL;
    }

    if (mOldCursorAreaImage != NULL)
    {
        delete mOldCursorAreaImage;
        mOldCursorAreaImage = NULL;
    }

#if 0
    if (mDD != NULL)
    {
        mDD->SetCooperativeLevel(mHWnd, DDSCL_NORMAL);
        mDD->Release();
        mDD = NULL;
    }

    if (mDD7 != NULL)
    {
        mDD7->Release();
        mDD7 = NULL;
    }
#endif
}

#if 0
bool DDInterface::CopyBitmap(LPDIRECTDRAWSURFACE theSurface, HBITMAP theBitmap, int theX, int theY, int theWidth, int theHeight)
{
    AutoCrit anAutoCrit(mCritSect);

    HRESULT hr;

    if (theBitmap == NULL || theSurface == NULL) return false;

    // Make sure this surface is restored.
    theSurface->Restore();

    // Get size of the bitmap
    BITMAP bmBitmap;
    GetObject(theBitmap, sizeof(bmBitmap), &bmBitmap);
    theWidth  = (theWidth  == 0) ? bmBitmap.bmWidth : theWidth;
    theHeight = (theHeight == 0) ? bmBitmap.bmHeight : theHeight;

    DDSURFACEDESC aDesc;
    ZeroMemory(&aDesc, sizeof(aDesc));
    aDesc.dwSize = sizeof(aDesc);
    aDesc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    hr = theSurface->GetSurfaceDesc(&aDesc);
    if (FAILED(hr)) return false;

    // Create memory DC
    HDC hdcImage = CreateCompatibleDC(NULL);
    if (hdcImage != NULL)
    {
        // Select bitmap into memory DC
        HBITMAP anOldBitmap = (HBITMAP)SelectObject(hdcImage, theBitmap);

        // Get surface DC
        HDC hdc;
        hr = theSurface->GetDC(&hdc);
        if (SUCCEEDED(hr))
        {
            // Copy the bitmap. Use BitBlt, if possible, otherwise use
            // StretchBlt
            if (theWidth == aDesc.dwWidth && theHeight == aDesc.dwHeight)
            {
                BitBlt(hdc, 0, 0, theWidth, theHeight, hdcImage, theX, theY, SRCCOPY);
            }
            else
            {
                StretchBlt(hdc, 0, 0, aDesc.dwWidth, aDesc.dwHeight, hdcImage,
                           theX, theY, theWidth, theHeight, SRCCOPY);
            }

            // Release surface DC
            theSurface->ReleaseDC(hdc);
        }

        // Select old bitmap into the memory DC and delete the DC
        SelectObject(hdcImage, anOldBitmap);
        DeleteDC(hdcImage);
    }
    else return false;

    return true;
}

extern std::fstream gStreamThing;

#endif


int   DDInterface::CreateSurface(SDL_Surface** theSurface, int width, int height, bool mVideoMemory)
{
    //FIXME
    Uint32 flags;// = SDL_SRCCOLORKEY || SDL_DOUBLEBUF;
    if (mVideoMemory) {
#if SDL_VERSION_ATLEAST(2,0,0)
	// SDL2 does not have hardware SDL_Surface
	// If we hit this assert we have to figure out why we get here.
        fprintf(stderr, "Can't do SDL_CreateRGBSurface with SDL_HWSURFACE\n");
	assert(0);
#else
        flags |= SDL_HWSURFACE;
#endif
    }
    else
        flags |= SDL_SWSURFACE;

    //FIXME fixed depth

    // Keep the RGB fields the same as in the rest of TuxCap
    const Uint32 SDL_amask = 0xFF000000;
    const Uint32 SDL_rmask = 0x00FF0000;
    const Uint32 SDL_gmask = 0x0000FF00;
    const Uint32 SDL_bmask = 0x000000FF;
    SDL_Surface* mSurface;
    mSurface = SDL_CreateRGBSurface(flags, width, height, 32,
                               SDL_rmask, SDL_gmask, SDL_bmask, SDL_amask);
    if (mSurface == NULL)
        return RESULT_FAIL;

    *theSurface = mSurface;

    return RESULT_OK;
}

bool DDInterface::Redraw(Rect* theClipRect)
{
    if (!mInitialized)
        return false;
    if (mIs3D) {
        if (!mD3DInterface->mErrorString.empty()) {
            mIs3D = false;
            return false;
        }

        mD3DInterface->Flush();
    }

    Rect aDestRect;
    Rect aSrcRect;
    if (NULL == theClipRect || mIsWidescreen) {
        // ClipRect cannot be supported when the draw surface and
        // primary surface are not the same size in widescreen mode.
        aDestRect = mPresentationRect;
        aSrcRect = Rect(0, 0, mWidth, mHeight);
    } else {

        aDestRect = *theClipRect;
        aSrcRect = *theClipRect;

    }

    DrawCursor();

    if (mIs3D)
        SDL_GL_SwapWindow(gSexyAppBase->GetMainWindow());
    else {
#if SDL_VERSION_ATLEAST(2,0,0)
        SDL_UpdateWindowSurface(gSexyAppBase->GetMainWindow());
#else
        SDL_Flip(mScreenImage->mSurface);
#endif
    }

    //restore custom cursor background

    RestoreOldCursorArea();

    return true;
}


bool DDInterface::SetCursorImage(Image* theImage)
{
#if 0
    AutoCrit anAutoCrit(mCritSect);
#endif
    if (mCursorImage != theImage)
    {
        // Wait until next Redraw or cursor move to draw new cursor
        mCursorImage = theImage;
        return true;
    }
    else
        return false;
}

void DDInterface::RestoreOldCursorArea()
{
#if TARGET_OS_IPHONE != 0
    return;
#endif
    if (mIs3D)
        return;

    if ((mHasOldCursorArea)) {
        Rect aSexyScreenRect(
                           mCursorX - (mCursorWidth / 2),
                           mCursorY - (mCursorHeight / 2),
                           mCursorWidth,
                           mCursorHeight);

        if (!mIs3D) {
            SDL_Rect source = { 0,0,64,64 };
            SDL_Rect destination = {aSexyScreenRect.mX, aSexyScreenRect.mY, aSexyScreenRect.mWidth, aSexyScreenRect.mHeight};
            SDL_BlitSurface(mOldCursorArea, &source, gSexyAppBase->GetGameSurface(), &destination);
        }
        mHasOldCursorArea = false;
    }
}

void DDInterface::DrawCursor()
{
#if TARGET_OS_IPHONE != 0
    return;
#endif
    if (mCursorImage != NULL)
    {
        Rect aSexyScreenRect(
                           mCursorX - (mCursorWidth / 2),
                           mCursorY - (mCursorHeight / 2),
                           mCursorWidth,
                           mCursorHeight);

        int res = 0;
        if (!mIs3D) {
            SDL_Rect source = {aSexyScreenRect.mX, aSexyScreenRect.mY, aSexyScreenRect.mWidth, aSexyScreenRect.mHeight};
            SDL_Rect destination = { 0,0,64,64 };
            res = SDL_BlitSurface(gSexyAppBase->GetGameSurface(), &source, mOldCursorArea, &destination);
        }

        mHasOldCursorArea = (res == 0);

        // Draw the middle of the cursor at X,Y coordinates
        Graphics g(mScreenImage);
        g.DrawImage(mCursorImage,
                  mCursorX - mCursorImage->GetWidth() / 2,
                  mCursorY - mCursorImage->GetHeight() / 2);
    }
    else
        mHasOldCursorArea = false;
}
