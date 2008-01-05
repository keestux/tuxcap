#ifndef __DDINTERFACE_H__
#define __DDINTERFACE_H__

#include "Common.h"

#include "NativeDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#include "SDL.h"


#ifndef WIN32
#define HWND void*
#endif

#if 0
#include "CritSect.h"
#endif

namespace Sexy
{

class SexyAppBase;
class DDImage;
class Image;
class MemoryImage;
class D3DInterface;

#if 0

class D3DTester;
#endif

typedef std::set<DDImage*> DDImageSet;

class DDInterface : public NativeDisplay
{
public:
	enum
	{
		RESULT_OK					= 0,
		RESULT_FAIL					= 1,
		RESULT_DD_CREATE_FAIL		= 2,
		RESULT_SURFACE_FAIL			= 3,
		RESULT_EXCLUSIVE_FAIL		= 4,
		RESULT_DISPCHANGE_FAIL		= 5,
		RESULT_INVALID_COLORDEPTH	= 6,
		RESULT_3D_FAIL				= 7
	};

	SexyAppBase*			mApp;
	D3DInterface*			mD3DInterface;
#if 0

	D3DTester*				mD3DTester;
 	CritSect				mCritSect;

	LPDIRECTDRAW			mDD;
	LPDIRECTDRAW7			mDD7;
#endif
        //FIXME eliminate
	SDL_Surface*		mPrimarySurface;
	SDL_Surface*		mSecondarySurface;
	SDL_Surface*		mDrawSurface;


	bool					mIs3D;

	int						mWidth;
	int						mHeight;
	Ratio					mAspect;
	int						mDesktopWidth;
	int						mDesktopHeight;
	Ratio					mDesktopAspect;
	bool					mIsWidescreen;
	int						mDisplayWidth;
	int						mDisplayHeight;
	Ratio					mDisplayAspect;

	Rect					mPresentationRect;
	int						mFullscreenBits;
	ulong					mRefreshRate;
        ulong					mMillisecondsPerFrame;
	int						mScanLineFailCount;

	int*					mRedAddTable;
	int*					mGreenAddTable;
	int*					mBlueAddTable;

	ulong					mRedConvTable[256];
	ulong					mGreenConvTable[256];
	ulong					mBlueConvTable[256];

	bool					mInitialized;
	HWND					mHWnd;

	SDL_Surface*		mOldCursorArea;

	bool					mIsWindowed;
	DDImage*				mScreenImage;
	DDImageSet				mDDImageSet;
	bool					mVideoOnlyDraw;
	ulong					mInitCount;

	int						mCursorWidth;
	int						mCursorHeight;
	int						mCursorX;
	int						mCursorY;
	Image*					mCursorImage;
	bool					mHasOldCursorArea;	
	DDImage*				mOldCursorAreaImage;

	std::string				mErrorString;

public:
	int					CreateSurface(SDL_Surface** theSurface, int width, int height, bool mVideoMemory);
#if 0
	bool					CopyBitmap(LPDIRECTDRAWSURFACE theSurface, HBITMAP TheBitmap, int theX, int theY, int theWidth, int theHeight);
#endif
	ulong					GetColorRef(ulong theRGB);
	void					AddDDImage(DDImage* theDDImage);
	void					RemoveDDImage(DDImage* theDDImage);
	void					Remove3DData(MemoryImage* theImage); // for 3d texture cleanup

	void					Cleanup();
	void					SetVideoOnlyDraw(bool videoOnly);
	bool					Redraw(Rect* theClipRect = NULL);	
	void					RestoreOldCursorArea();
	void					DrawCursor();
#if 0
	bool					GotDXError(HRESULT theResult, const char *theContext = "");





	void					ClearSurface(LPDIRECTDRAWSURFACE theSurface);
	bool					Do3DTest(HWND theHWND);

#endif
public:
	DDInterface(SexyAppBase* theApp);
	virtual ~DDInterface();

	static std::string		ResultToString(int theResult);

	DDImage*				GetScreenImage();
	int						Init(HWND theWindow, bool IsWindowed);	


	void					RemapMouse(int& theX, int& theY);

	bool					SetCursorImage(Image* theImage);
};

}

#endif //__DDINTERFACE_H__

