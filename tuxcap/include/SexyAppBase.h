#ifndef __SEXYAPPBASE_H__
#define __SEXYAPPBASE_H__

#include <string>
#include <vector>
#include <map>
#include <list>
#include <assert.h>
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"

#include "ButtonListener.h"
#include "DialogListener.h"
#include "Common.h"
#include "Rect.h"
#include "Color.h"
#include "NativeDisplay.h"
#include "SharedImage.h"

#include "Buffer.h"
#if 0
#include "CritSect.h"
#endif
#include "Ratio.h"

#ifndef WIN32
#define HWND void*
 enum {
   REG_SZ, 
   REG_DWORD, 
   REG_BINARY,
   HKEY_CURRENT_USER
 };

 typedef int HKEY;
#endif

namespace ImageLib
{
	class Image;
};


namespace Sexy
{
  
class WidgetManager;
class Image;
class Widget;
class MemoryImage;

typedef std::set<MemoryImage*> MemoryImageSet;
typedef std::map<std::string, SexyString> StringSexyStringMap;

class SoundManager;
class MusicInterface;


class DDInterface;

class DDImage;
class ResourceManager;
class Dialog;
typedef std::map<int, Dialog*> DialogMap;
typedef std::list<Dialog*> DialogList;
#if 0

class HTTPTransfer;

typedef std::list<MSG> WindowsMessageList;
typedef std::map<HANDLE, int> HandleToIntMap;

#endif

class WidgetSafeDeleteInfo
{
public:
	int						mUpdateAppDepth;
	Widget*					mWidget;
};


typedef std::list<WidgetSafeDeleteInfo> WidgetSafeDeleteList;
typedef std::vector<std::string> StringVector;
typedef std::map<std::string, StringVector> StringStringVectorMap;
typedef std::map<std::string, std::string> StringStringMap;
typedef std::map<std::string, std::wstring> StringWStringMap;
typedef std::map<std::string, bool> StringBoolMap;
typedef std::map<std::string, int> StringIntMap;
typedef std::map<std::string, double> StringDoubleMap;

enum
{
	CURSOR_POINTER,
	CURSOR_HAND,
	CURSOR_DRAGGING,
        CURSOR_TEXT,
	CURSOR_CIRCLE_SLASH,
	CURSOR_SIZEALL,
	CURSOR_SIZENESW,
	CURSOR_SIZENS,
	CURSOR_SIZENWSE,
	CURSOR_SIZEWE,	
	CURSOR_WAIT,
	CURSOR_NONE,
	CURSOR_CUSTOM,
	NUM_CURSORS
};

enum
{
	DEMO_MOUSE_POSITION,	
	DEMO_ACTIVATE_APP,
	DEMO_SIZE,
	DEMO_KEY_DOWN,
	DEMO_KEY_UP,
	DEMO_KEY_CHAR,
	DEMO_CLOSE,
	DEMO_MOUSE_ENTER,
	DEMO_MOUSE_EXIT,
	DEMO_LOADING_COMPLETE,
	DEMO_REGISTRY_GETSUBKEYS,
	DEMO_REGISTRY_READ,
	DEMO_REGISTRY_WRITE,
	DEMO_REGISTRY_ERASE,	
	DEMO_FILE_EXISTS,
	DEMO_FILE_READ,
	DEMO_FILE_WRITE,
	DEMO_HTTP_RESULT,
	DEMO_SYNC,
	DEMO_ASSERT_STRING_EQUAL,
	DEMO_ASSERT_INT_EQUAL,
	DEMO_MOUSE_WHEEL,
	DEMO_HANDLE_COMPLETE,
	DEMO_VIDEO_DATA,
	DEMO_IDLE = 31
};

enum {
	FPS_ShowFPS,
	FPS_ShowCoords,
	Num_FPS_Types
};

enum
{
	UPDATESTATE_MESSAGES,
	UPDATESTATE_PROCESS_1,
	UPDATESTATE_PROCESS_2,
	UPDATESTATE_PROCESS_DONE
};


class SexyAppBase : public ButtonListener, public DialogListener
{
 protected:
  std::map<SexyString, SexyString> mRegistry;

  bool WriteRegistryToIni(const std::string& IniFile, const std::string& IniDir);
  bool ReadRegistryFromIni(const std::string& IniFile, const std::string& IniDir);

public:
	
	Uint32					mRandSeed;
        SDL_Surface* surface;
		
	std::string				mCompanyName;
	std::string				mFullCompanyName;
	std::string				mProdName;	

	std::string				mRegKey;
	std::string				mChangeDirTo;
	
	int						mRelaxUpdateBacklogCount; // app doesn't try to catch up for this many frames
	int						mPreferredX;
	int						mPreferredY;
	int						mWidth;
	int						mHeight;
	int						mFullscreenBits;

	double					mMusicVolume;
	double					mSfxVolume;
	double					mDemoMusicVolume;
	double					mDemoSfxVolume;
	bool					mNoSoundNeeded;

	bool					mWantFMod;
	bool					mCmdLineParsed;
	bool					mSkipSignatureChecks;
	bool					mStandardWordWrap;
	bool					mbAllowExtendedChars;
	bool					mOnlyAllowOneCopyToRun;
	bool					mBetaValidate;
	bool					mSEHOccured;
	bool					mShutdown;
	bool					mExitToTop;
	bool					mIsWindowed;
	bool					mIsPhysWindowed;
	bool					mFullScreenWindow; // uses ChangeDisplaySettings to run fullscreen with mIsWindowed true
	bool					mForceFullscreen;
	bool					mForceWindowed;	
	bool					mInitialized;	
	bool					mProcessInTimer;
	bool					mIsScreenSaver;
	bool					mAllowMonitorPowersave;
	bool					mNoDefer;	
	bool					mFullScreenPageFlip;	
	bool					mTabletPC;
	bool					mAlphaDisabled;
	bool					mReadFromRegistry;
	bool					mIsOpeningURL;
	bool					mShutdownOnURLOpen;
	std::string				mOpeningURL;

	std::string				mRegisterLink;
	std::string				mProductVersion;	
	double					mUnmutedMusicVolume;
	double					mUnmutedSfxVolume;	
	int						mMuteCount;
	int						mAutoMuteCount;
	bool					mDemoMute;
	bool					mMuteOnLostFocus;
	bool					mCleanupSharedImages;
	
	int						mNonDrawCount;
	int						mFrameTime;

	bool					mIsDrawing;
	bool					mLastDrawWasEmpty;
	bool					mHasPendingDraw;
	double					mPendingUpdatesAcc;
	double					mUpdateFTimeAcc;
	int						mSleepCount;
	int						mDrawCount;
	int						mUpdateCount;
	int						mUpdateAppState;
	int						mUpdateAppDepth;
	double					mUpdateMultiplier;		
	bool					mPaused;
	int						mFastForwardToUpdateNum;
	bool					mFastForwardToMarker;
	bool					mFastForwardStep;
	int						mStepMode;  // 0 = off, 1 = step, 2 = waiting for step

	int						mCursorNum;
	bool					mMouseIn;	
	bool					mRunning;
	bool					mActive;
	bool					mMinimized;
	bool					mPhysMinimized;
	bool					mIsDisabled;
	bool					mHasFocus;
	int						mDrawTime;
	int						mFPSFlipCount;
	int						mFPSDirtyCount;
	int						mFPSTime;
	int						mFPSCount;
	bool					mShowFPS;
	int						mShowFPSMode;
	int						mScreenBltTime;
	bool					mAutoStartLoadingThread;
	bool					mLoadingThreadStarted;
	bool					mLoadingThreadCompleted;
	bool					mLoaded;
	bool					mYieldMainThread;
	bool					mLoadingFailed;
	bool					mSysCursor;	
	bool					mCustomCursorsEnabled;
	bool					mCustomCursorDirty;	
	bool					mLastShutdownWasGraceful;
	bool					mIsWideWindow;

	int						mNumLoadingThreadTasks;
	int						mCompletedLoadingThreadTasks;

	// For recording/playback of program control
	bool					mRecordingDemoBuffer;
	bool					mPlayingDemoBuffer;
	bool					mManualShutdown;
	std::string			mDemoPrefix;
	std::string			mDemoFileName;
	int						mDemoLength;
	int						mLastDemoMouseX;
	int						mLastDemoMouseY;
	int						mLastDemoUpdateCnt;
	bool					mDemoNeedsCommand;
	bool					mDemoIsShortCmd;
	int						mDemoCmdNum;
	int						mDemoCmdOrder;
	int						mDemoCmdBitPos;
	bool					mDemoLoadingComplete;
	int						mCurHandleNum;

	typedef std::pair<std::string, int> DemoMarker;
	typedef std::list<DemoMarker> DemoMarkerList;
	DemoMarkerList			mDemoMarkerList;

	bool					mDebugKeysEnabled;
	bool					mEnableMaximizeButton;
	bool					mCtrlDown;
	bool					mAltDown;
	
	int						mSyncRefreshRate;
	bool					mVSyncUpdates;
	bool					mVSyncBroken;
	int						mVSyncBrokenCount;
	bool					mWaitForVSync;
	bool					mSoftVSyncWait;
	bool					mUserChanged3DSetting;
	bool					mAutoEnable3D;
	bool					mTest3D;
	bool					mWidescreenAware;
	bool					mEnableWindowAspect;
	StringWStringMap		mStringProperties;
	StringBoolMap			mBoolProperties;
	StringIntMap			mIntProperties;
	StringDoubleMap			mDoubleProperties;
	StringStringVectorMap	mStringVectorProperties;
	Uint32				mPrimaryThreadId;
	SDL_mutex*			mMutex;
	SDL_Cursor*					mHandCursor;
	SDL_Cursor*				mDraggingCursor;
        SDL_Cursor* mArrowCursor;
	Uint32					mLastTimeCheck;
	Uint32					mLastTime;
	Uint32					mLastUserInputTick;
	Uint32					mLastTimerTime;
	Uint32					mLastDrawTick;
	Uint32					mNextDrawTick;
	WidgetSafeDeleteList	mSafeDeleteList;
	Uint32					mVSyncBrokenTestStartTick;
	Uint32					mVSyncBrokenTestUpdates;
        DDInterface*			mDDInterface;	
        uchar					mAdd8BitMaxTable[512];
	WidgetManager*			mWidgetManager;
	Uint32					mTimeLoaded;
	SexyString				mTitle;	
	Image*					mCursorImages[NUM_CURSORS];
        ulong mFPSStartTick;
	Buffer					mDemoBuffer;
	MemoryImageSet			mMemoryImageSet;
	SharedImageMap			mSharedImageMap;

	HWND					mHWnd;
	HWND					mInvisHWnd;
	uint					mNotifyGameMessage;
	Rect					mScreenBounds;
	SoundManager*			mSoundManager;
	MusicInterface*			mMusicInterface;	
	DialogMap				mDialogMap;
	DialogList				mDialogList;
#if 0

	CritSect				mCritSect;	


	HCURSOR					mOverrideCursor;
	DWORD					mOpeningURLTime;
	DWORD					mLastBigDelayTime;	





	HandleToIntMap			mHandleToIntMap; // For waiting on handles
	DWORD					mMinVidMemory3D;
	DWORD					mRecommendedVidMemory3D;






	LONG					mOldWndProc;
#endif
	Ratio					mWindowAspect;
	ResourceManager*		mResourceManager;
	virtual void			Init();	
	SexyAppBase();
	virtual ~SexyAppBase();
	virtual void			Shutdown();	
	virtual void			Start();	
	void					SetCursor(int theCursorNum);
	virtual void			SafeDeleteWidget(Widget* theWidget);	
	void					AddMemoryImage(MemoryImage* theMemoryImage);
	void					RemoveMemoryImage(MemoryImage* theMemoryImage);
	void					WaitForLoadingThread();				
	virtual void			LoadingThreadProc();
	virtual SharedImageRef	GetSharedImage(const std::string& theFileName, const std::string& theVariant = "", bool* isNew = NULL);

	virtual void			PlaySample(int theSoundNum);
	virtual void			PlaySample(int theSoundNum, int thePan);

	virtual double			GetMasterVolume();
	virtual double			GetMusicVolume();
	virtual double			GetSfxVolume();
	virtual bool			IsMuted();

	virtual void			SetMasterVolume(double theVolume);
	virtual void			SetMusicVolume(double theVolume);
	virtual void			SetSfxVolume(double theVolume);	
	virtual void			Mute(bool autoMute = false);
	virtual void			Unmute(bool autoMute = false);
	virtual MusicInterface*	CreateMusicInterface();
        DDImage*				CopyImage(Image* theImage, const Rect& theRect);
	DDImage*				CopyImage(Image* theImage);
	virtual DDImage*		GetImage(const std::string& theFileName, bool commitBits = true);	

	DDImage*				CreateCrossfadeImage(Image* theImage1, const Rect& theRect1, Image* theImage2, const Rect& theRect2, double theFadeFactor);
	void					ColorizeImage(Image* theImage, const Color& theColor);
	DDImage*				CreateColorizedImage(Image* theImage, const Color& theColor);
	void					MirrorImage(Image* theImage);
	void					FlipImage(Image* theImage);
	void					RotateImageHue(Sexy::MemoryImage *theImage, int theDelta);
	ulong					HSLToRGB(int h, int s, int l);
	ulong					RGBToHSL(int r, int g, int b);
	void					HSLToRGB(const ulong* theSource, ulong* theDest, int theSize);
	void					RGBToHSL(const ulong* theSource, ulong* theDest, int theSize);
	void					PrecacheAdditive(MemoryImage* theImage);
	void					PrecacheAlpha(MemoryImage* theImage);
	void					PrecacheNative(MemoryImage* theImage);
	virtual void			MakeWindow();
	bool					Is3DAccelerated();
	virtual double			GetLoadingThreadProgress();	
	bool					FileExists(const std::string& theFileName);
	bool					ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo = false);//UNICODE
	bool					WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer);

	virtual Dialog*			NewDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode);		
	virtual Dialog*			DoDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode);
	virtual Dialog*			GetDialog(int theDialogId);
	virtual void			AddDialog(int theDialogId, Dialog* theDialog);
	virtual void			AddDialog(Dialog* theDialog);
	virtual bool			KillDialog(Dialog* theDialog);

	virtual bool			KillDialog(int theDialogId, bool removeWidget, bool deleteWidget);
	virtual bool			KillDialog(int theDialogId);
	virtual int				GetDialogCount();
	virtual void			DialogButtonPress(int theDialogId, int theButtonId);
	virtual void			DialogButtonDepress(int theDialogId, int theButtonId);
	virtual void			ModalOpen();
	virtual void			ModalClose();	
	virtual void			GotFocus();
	virtual void			LostFocus();	
	virtual bool			UpdateAppStep(bool* updated);
	virtual void			SetAlphaDisabled(bool isDisabled);
	void					Set3DAcclerated(bool is3D, bool reinit = true);
	virtual void			HandleGameAlreadyRunning(); 
	virtual void			SwitchScreenMode();
	virtual void			SwitchScreenMode(bool wantWindowed);
	virtual void			SwitchScreenMode(bool wantWindowed, bool is3d, bool force = false);
	bool					Is3DAccelerationSupported();
	bool					Is3DAccelerationRecommended();
	void					SetCursorImage(int theCursorNum, Image* theImage);
	int						GetCursor();
	void					EnableCustomCursors(bool enabled);	

	// Registry access methods
	bool					RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys);
	bool					RegistryReadString(const std::string& theValueName, std::string* theString);
	bool					RegistryReadInteger(const std::string& theValueName, int* theValue);
	bool					RegistryReadBoolean(const std::string& theValueName, bool* theValue);
	bool					RegistryReadData(const std::string& theValueName, uchar* theValue, ulong* theLength);
	bool					RegistryEraseKey(const SexyString& theKeyName);
	void					RegistryEraseValue(const SexyString& theValueName);
	bool					RegistryWriteString(const std::string& theValueName, const std::string& theString);
	bool					RegistryWriteInteger(const std::string& theValueName, int theValue);
	bool					RegistryWriteBoolean(const std::string& theValueName, bool theValue);
	bool					RegistryWriteData(const std::string& theValueName, const uchar* theValue, ulong theLength);	
	virtual void			WriteToRegistry();
	virtual void			ReadFromRegistry();
	SexyString				GetString(const std::string& theId);
	SexyString				GetString(const std::string& theId, const SexyString& theDefault);


	virtual void			Done3dTesting();
	virtual std::string		NotifyCrashHook(); // return file name that you want to upload
	

	// Properties access methods
	bool					LoadProperties(const std::string& theFileName, bool required, bool checkSig);
	bool					LoadProperties();


	
	bool					GetBoolean(const std::string& theId);
	bool					GetBoolean(const std::string& theId, bool theDefault);	
	int						GetInteger(const std::string& theId);
	int						GetInteger(const std::string& theId, int theDefault);
	double					GetDouble(const std::string& theId);
	double					GetDouble(const std::string& theId, double theDefault);
	StringVector			GetStringVector(const std::string& theId);

	void					SetBoolean(const std::string& theId, bool theValue);
	void					SetInteger(const std::string& theId, int theValue);
	void					SetDouble(const std::string& theId, double theValue);
	void					SetString(const std::string& theId, const std::wstring& theValue);
	virtual bool			CheckSignature(const Buffer& theBuffer, const std::string& theFileName);	

protected:	
	// Registry helpers
	bool					RegistryRead(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength);
	bool					RegistryReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theMainKey = HKEY_CURRENT_USER);
	bool					RegistryWrite(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength);

	virtual bool			DoUpdateFrames();
	virtual void			DoUpdateFramesF(float theFrac);
	virtual void			LoadingThreadCompleted();
	virtual void			UpdateFrames();
	virtual bool			DrawDirtyStuff();
	void					UpdateFTimeAcc();
	virtual bool			Process(bool allowSleep = true);		
	void					ProcessSafeDeleteList();	
	static int				LoadingThreadProcStub(void *theArg);	
	void					StartLoadingThread();
	virtual void			PreDDInterfaceInitHook();
	virtual void			DeleteNativeImageData();	
	virtual void			PostDDInterfaceInitHook();
	virtual void			PreDisplayHook();
	bool					WriteBytesToFile(const std::string& theFileName, const void *theData, unsigned long theDataLen);
	void					RestoreScreenResolution();
	void					DoExit(int theCode);
	virtual bool			ChangeDirHook(const char *theIntendedPath);
	virtual void			InitPropertiesHook();
	virtual void			InitHook();

	virtual void			DeleteExtraImageData();
	virtual void			ReInitImages();
	void					Remove3DData(MemoryImage* theMemoryImage);
	virtual void			EnforceCursor();
	virtual void			PreTerminate();

#if 0
	void					RehupFocus();
	void					ClearKeysDown();




	
	// Loading thread methods	



	



	void					TakeScreenshot();
	void					DumpProgramInfo();	
	void					ShowMemoryUsage();			





	// Demo recording helpers	
	void					ProcessDemo();
public:

	// Common overrides:





	virtual void			GetSEHWebParams(DefinesMap* theDefinesMap);








	// Demo access methods
	bool					PrepareDemoCommand(bool required);
	void					WriteDemoTimingBlock();
	void					WriteDemoBuffer();
	bool					ReadDemoBuffer(std::string &theError);//UNICODE
	void					DemoSyncBuffer(Buffer* theBuffer);
	void					DemoSyncString(std::string* theString);
	void					DemoSyncInt(int* theInt);
	void					DemoSyncBool(bool* theBool);
	void					DemoAssertStringEqual(const std::string& theString);
	void					DemoAssertIntEqual(int theInt);
	void					DemoAddMarker(const std::string& theString);
	void					DemoRegisterHandle(HANDLE theHandle);
	void					DemoWaitForHandle(HANDLE theHandle);
	bool					DemoCheckHandle(HANDLE theHandle);
	

	// File access methods




	bool					EraseFile(const std::string& theFileName);
	virtual bool			IsAltKeyUsed(WPARAM wParam);
	virtual int				MsgBox(const std::string &theText, const std::string &theTitle = "Message", int theFlags = MB_OK);
	virtual int				MsgBox(const std::wstring &theText, const std::wstring &theTitle = L"Message", int theFlags = MB_OK);



	// Public methods
	virtual void			BeginPopup();
	virtual void			EndPopup();
	virtual void			Popup(const std::string& theString);
	virtual void			Popup(const std::wstring& theString);
	virtual void			LogScreenSaverError(const std::string &theError);

	virtual void			URLOpenFailed(const std::string& theURL);
	virtual void			URLOpenSucceeded(const std::string& theURL);
	virtual bool			OpenURL(const std::string& theURL, bool shutdownOnOpen = false);	
	virtual std::string		GetProductVersion(const std::string& thePath);	

	virtual void			SEHOccured();
	virtual std::string		GetGameSEHInfo();

	virtual void			DoParseCmdLine();
	virtual void			ParseCmdLine(const std::string& theCmdLine);
	virtual void			HandleCmdLineParam(const std::string& theParamName, const std::string& theParamValue);
	virtual void			HandleNotifyGameMessage(int theType, int theParam); // for HWND_BROADCAST of mNotifyGameMessage (0-1000 are reserved for SexyAppBase for theType)








	void					CopyToClipboard(const std::string& theString);
	std::string				GetClipboard();




	

	virtual bool			DebugKeyDown(int theKey);	
	virtual bool			DebugKeyDownAsync(int theKey, bool ctrlDown, bool altDown);
	virtual void			CloseRequestAsync();

	void					DemoSyncRefreshRate();

	// Misc methods
	bool					IsScreenSaver();
	virtual bool			AppCanRestore();
#endif
	void					CleanSharedImages();
	// Resource access methods
	void					LoadResourceManifest();
	void					ShowResourceError(bool doExit = false);

	int						InitDDInterface();
	void					ClearUpdateBacklog(bool relaxForASecond = false);
	virtual void			DoMainLoop();

	virtual bool			UpdateApp();
	virtual void			Redraw(Rect* theClipRect);
	virtual void			ShutdownHook();	


};

extern SexyAppBase* gSexyAppBase;

};

#endif //__SEXYAPPBASE_H__

