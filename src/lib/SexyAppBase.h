#ifndef __SEXYAPPBASE_H__
#define __SEXYAPPBASE_H__

#include <string>
#include <vector>
#include <map>
#include <list>
#include <assert.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "ButtonListener.h"
#include "DialogListener.h"
#include "Common.h"
#include "Logging.h"
#include "Rect.h"
#include "Color.h"
#include "NativeDisplay.h"

#include "Buffer.h"
#if 0
#include "CritSect.h"
#endif
#include "Ratio.h"

#if __ANDROID__ == 0
#define ENABLE_EXCEPTION
#else
#undef ENABLE_EXCEPTION
#endif

#ifndef WIN32
#define HWND void*
enum {
    REG_SZ,
    REG_DWORD,
    REG_BINARY,
    HKEY_CURRENT_USER,
};

typedef int HKEY;
#endif

namespace Sexy
{

class MemoryImage;
class Image;
class DDImage;

class WidgetManager;
class Widget;

typedef std::set<Image*> ImageSet;

typedef std::map<std::string, SexyString> StringSexyStringMap;

class SoundManager;
class SoundInstance;
class MusicInterface;

class DDInterface;
class D3DInterface;

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
    int                     mUpdateAppDepth;
    Widget*                 mWidget;
};

typedef std::list<WidgetSafeDeleteInfo>     WidgetSafeDeleteList;
typedef std::vector<std::string>            StringVector;
typedef std::map<std::string, StringVector> StringStringVectorMap;
typedef std::map<std::string, std::string>  StringStringMap;
typedef std::map<std::string, std::wstring> StringWStringMap;
typedef std::map<std::string, bool>         StringBoolMap;
typedef std::map<std::string, int>          StringIntMap;
typedef std::map<std::string, double>       StringDoubleMap;

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
    DEMO_IDLE = 31                // ???? Must be last
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


#ifdef ENABLE_EXCEPTION
class Exception : std::exception
{
public:
    Exception(const std::string & msg)
        : std::exception(),
        mMessage(msg)
        {}
    virtual ~Exception() throw (){}

    virtual const std::string     getMessage() const    { return mMessage; }
private:
    std::string     mMessage;
};
#endif


class SexyAppBase : public ButtonListener, public DialogListener
{
    friend class D3DInterface;

protected:
    std::map<SexyString, SexyString> mRegistry;

    bool WriteRegistryToIni(const std::string& IniFile);
    bool ReadRegistryFromIni(const std::string& IniFile);

public:

    std::string             mProdName;                // Used in GameApp
    std::string             mProductVersion;

    std::string             mCompanyName;
    std::string             mFullCompanyName;

    std::string             mArgv0;
    std::string             mAppDataFolder;
    std::string             mAppResourceFolder;
    std::string             mUserLanguage;

    bool                    mShutdown;

    std::string             mRegKey;
    std::string             mRegisterLink;

    int                     mPreferredX;
    int                     mPreferredY;
    int                     mWidth;
    int                     mHeight;

    int                     mFullscreenBits;

    double                  mMusicVolume;
    double                  mSfxVolume;
    double                  mDemoMusicVolume;
    double                  mDemoSfxVolume;
    bool                    mNoSoundNeeded;

    bool                    mWantFMod;
    bool                    mCmdLineParsed;
    bool                    mSkipSignatureChecks;
    bool                    mStandardWordWrap;
    bool                    mbAllowExtendedChars;
    bool                    mbAllowSleep;
    bool                    mOnlyAllowOneCopyToRun;
    bool                    mSEHOccured;
    bool                    mExitToTop;
    bool                    mIsWindowed;
    bool                    mForceFullscreen;
    bool                    mInitialized;
    bool                    mProcessInTimer;
    bool                    mIsScreenSaver;
    bool                    mAllowMonitorPowersave;
    bool                    mNoDefer;
    bool                    mFullScreenPageFlip;
    bool                    mTabletPC;
    bool                    mAlphaDisabled;
    bool                    mLookForAlpha;

    bool                    mReadFromRegistry;
    bool                    mIsOpeningURL;

    int                     mMuteCount;
    int                     mAutoMuteCount;
    bool                    mDemoMute;
    bool                    mMuteOnLostFocus;

    int                     mNonDrawCount;

    bool                    mIsDrawing;
    bool                    mLastDrawWasEmpty;
    bool                    mHasPendingDraw;
    float                  mPendingUpdatesAcc;
    float                  mUpdateFTimeAcc;
    int                     mSleepCount;
    int                     mDrawCount;
    int                     mUpdateCount;
    int                     mUpdateAppState;
    int                     mUpdateAppDepth;
    bool                    mPaused;
    int                     mFastForwardToUpdateNum;
    bool                    mFastForwardToMarker;
    bool                    mFastForwardStep;
    int                     mStepMode;  // 0 = off, 1 = step, 2 = waiting for step

    int                     mCursorNum;
    bool                    mMouseIn;
    bool                    mRunning;
    bool                    mActive;
    bool                    mMinimized;
    bool                    mPhysMinimized;
    bool                    mIsDisabled;
    bool                    mHasFocus;
    int                     mDrawTime;

    int                     mFPSFlipCount;
    int                     mFPSDirtyCount;
    int                     mFPSTime;
    int                     mFPSCount;
    bool                    mShowFPS;
    int                     mShowFPSMode;

    int                     mScreenBltTime;
    bool                    mAutoStartLoadingThread;
    bool                    mLoadingThreadStarted;
    bool                    mLoadingThreadCompleted;
    bool                    mLoaded;
    bool                    mYieldMainThread;
    bool                    mLoadingFailed;
    bool                    mSysCursor;
    bool                    mCustomCursorsEnabled;
    bool                    mCustomCursorDirty;
    bool                    mLastShutdownWasGraceful;
    bool                    mIsWideWindow;

    int                     mNumLoadingThreadTasks;
    int                     mCompletedLoadingThreadTasks;
    void                  TakeScreenshot(const std::string& filename, const std::string& path) const;


    std::string             mWindowIconBMP;

    int                     mDemoLength;
    int                     mLastDemoMouseX;
    int                     mLastDemoMouseY;
    int                     mLastDemoUpdateCnt;
    bool                    mDemoNeedsCommand;
    int                     mDemoCmdNum;
    int                     mDemoCmdOrder;
    int                     mDemoCmdBitPos;
    bool                    mDemoLoadingComplete;

    int                     mCurHandleNum;

    typedef std::pair<std::string, int> DemoMarker;
    typedef std::list<DemoMarker> DemoMarkerList;
    DemoMarkerList          mDemoMarkerList;

    bool                    mDebugKeysEnabled;
    bool                    mEnableMaximizeButton;
    bool                    mCtrlDown;
    bool                    mAltDown;

    bool                    mUserChanged3DSetting;
    bool                    mAutoEnable3D;
    bool                    mTest3D;
    bool                    mWidescreenAware;
    bool                    mEnableWindowAspect;
    StringWStringMap        mStringProperties;
    StringBoolMap           mBoolProperties;
    StringIntMap            mIntProperties;
    StringDoubleMap         mDoubleProperties;
    StringStringVectorMap   mStringVectorProperties;
    SDL_threadID            mPrimaryThreadId;

    SDL_mutex*              mMutex;
    SDL_Cursor*             mHandCursor;
    SDL_Cursor*             mDraggingCursor;
    SDL_Cursor*             mArrowCursor;

    Uint32                  mLastTimeCheck;
    Uint32                  mLastTime;
    Uint32                  mLastUserInputTick;
    Uint32                  mLastTimerTime;
    Uint32                  mLastDrawTick;
    Uint32                  mNextDrawTick;
    WidgetSafeDeleteList    mSafeDeleteList;

    DDInterface*            mDDInterface;
    uchar                   mAdd8BitMaxTable[512];
    WidgetManager*          mWidgetManager;
    Uint32                  mTimeLoaded;
    SexyString              mTitle;
    Image*                  mCursorImages[NUM_CURSORS];
    uint32_t                mFPSStartTick;
    Buffer                  mDemoBuffer;
    ImageSet                mImageSet;

    HWND                    mHWnd;                  // Useless for TuxCap
    HWND                    mInvisHWnd;             // Useless for TuxCap
    uint                    mNotifyGameMessage;
    SoundManager*           mSoundManager;
    MusicInterface*         mMusicInterface;
    DialogMap               mDialogMap;
    DialogList              mDialogList;

    ResourceManager*        mResourceManager;

    LoggerFacil *           mLogFacil;

protected:

    // We need al these things to remember how game world is translated to real hardware
    int                     mVideoModeWidth;            // The width used in the last SetVideoMode
    int                     mVideoModeHeight;           // The height used in the last SetVideoMode
    int                     mViewportx;
    int                     mViewporty;
    int                     mViewportWidth;
    int                     mViewportHeight;
    float                   mViewportToGameRatio;
    bool                    mViewportIsRotated;

    // Set from the commandline
    bool                    mFullScreenMode;        // as oposed to windowed
    bool                    mWindowedMode;          // as oposed to full screen
    bool                    mUseOpenGL;             // as oposed to using software renderer
    bool                    mUseSoftwareRenderer;   // as oposed to using OpenGL
    bool                    mDebug;

private:
    int                     mSyncRefreshRate;
    int                     mFrameTime;                 // In milliseconds (SDL ticks)
    double                  mUpdateMultiplier;

    SDL_Surface*            mScreenSurface;
    SDL_Surface*            mGameSurface;
    SDL_Window*             mMainWindow;
    SDL_GLContext           mMainGLContext;

public:
    virtual void            Init();
    SexyAppBase();
    virtual ~SexyAppBase();
    virtual void            Shutdown();
    virtual void            Start();
    void                    SetCursor(int theCursorNum);
    virtual void            SafeDeleteWidget(Widget* theWidget);

    void                    ReInitImages();

    void                    AddImage(Image* theImage);
    void                    RemoveImage(Image* theImage);
    void                    DeleteExtraImageData();
    void                    Remove3DData(Image* theImage);

    void                    WaitForLoadingThread();
    virtual void            LoadingThreadProc();

    virtual SoundInstance*  PlaySample(int theSoundNum, bool original= false, double volume=1.0, bool loop=false);
    virtual SoundInstance*  PlaySample(int theSoundNum, int thePan, bool original = false, double volume=1.0, bool loop=false, float pitch=0.0f);
    virtual SoundInstance*  StopSample(int theSoundNum);
    virtual bool            IsSamplePlaying(int theSoundNum);

    virtual double          GetMasterVolume();
    virtual double          GetMusicVolume();
    virtual double          GetSfxVolume();
    virtual bool            IsMuted();

    virtual void            SetMasterVolume(double theVolume);
    virtual void            SetMusicVolume(double theVolume);
    virtual void            SetSfxVolume(double theVolume);
    virtual void            Mute(bool autoMute = false);
    virtual void            Unmute(bool autoMute = false);
    Image*                  CopyImage(Image* theImage, const Rect& theRect);
    Image*                  CopyImage(Image* theImage);
    virtual Image*          GetImage(const std::string& theFileName);
    virtual Image*          GetImage(const std::string& theFileName, bool commitBits);
    virtual Image*          GetImage(const std::string& theFileName, bool commitBits, bool lookForAlpha);
    SDL_Surface*            GetScreenSurface() { return mScreenSurface; }
    SDL_Surface*            GetGameSurface() { return mGameSurface; }
    SDL_Window*             GetMainWindow() { return mMainWindow; }

    DDImage*                CreateCrossfadeImage(Image* theImage1, const Rect& theRect1, Image* theImage2, const Rect& theRect2, double theFadeFactor);
    void                    ColorizeImage(Image* theImage, const Color& theColor);
    DDImage*                CreateColorizedImage(Image* theImage, const Color& theColor);
    void                    MirrorImage(Image* theImage);
    void                    FlipImage(Image* theImage);
    void                    RotateImageHue(Sexy::MemoryImage *theImage, int theDelta);
    uint32_t                HSLToRGB(int h, int s, int l);
    uint32_t                RGBToHSL(int r, int g, int b);
    void                    HSLToRGB(const uint32_t* theSource, uint32_t* theDest, int theSize);
    void                    RGBToHSL(const uint32_t* theSource, uint32_t* theDest, int theSize);
    void                    PrecacheAdditive(MemoryImage* theImage);
    void                    PrecacheAlpha(MemoryImage* theImage);
    void                    PrecacheNative(MemoryImage* theImage);
    bool                    Is3DAccelerated();
    virtual double          GetLoadingThreadProgress();
    bool                    FileExists(const std::string& theFileName);
    bool                    ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo = false);//UNICODE
    bool                    WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer);

    virtual Dialog*         NewDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode);
    virtual Dialog*         DoDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode);
    virtual Dialog*         GetDialog(int theDialogId);
    virtual void            AddDialog(int theDialogId, Dialog* theDialog);
    virtual void            AddDialog(Dialog* theDialog);
    virtual bool            KillDialog(Dialog* theDialog);

    virtual bool            KillDialog(int theDialogId, bool removeWidget, bool deleteWidget);
    virtual bool            KillDialog(int theDialogId);
    virtual int             GetDialogCount();
    virtual void            DialogButtonPress(int theDialogId, int theButtonId);
    virtual void            DialogButtonDepress(int theDialogId, int theButtonId);
    virtual void            ModalOpen();
    virtual void            ModalClose();
    virtual void            GotFocus();
    virtual void            LostFocus();
    void                    UpdateAppStep(bool* updated);                     // ???? Why is this needed in Dialog::WaitForResult()
    virtual void            SetAlphaDisabled(bool isDisabled);
    virtual void            HandleGameAlreadyRunning();
    virtual void            SwitchScreenMode();
    virtual void            SwitchScreenMode(bool wantWindowed);
    virtual void            SwitchScreenMode(bool wantWindowed, bool is3d, bool force = false);
    bool                    Is3DAccelerationSupported();
    bool                    Is3DAccelerationRecommended();
    void                    SetCursorImage(int theCursorNum, Image* theImage);
    int                     GetCursor();
    void                    EnableCustomCursors(bool enabled);

    // Registry access methods
    bool                    RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys);
    bool                    RegistryReadString(const std::string& theValueName, std::string* theString);
    bool                    RegistryReadInteger(const std::string& theValueName, int* theValue);
    bool                    RegistryReadBoolean(const std::string& theValueName, bool* theValue);
    bool                    RegistryReadData(const std::string& theValueName, uchar* theValue, uint32_t* theLength);
    bool                    RegistryEraseKey(const SexyString& theKeyName);
    void                    RegistryEraseValue(const SexyString& theValueName);
    bool                    RegistryWriteString(const std::string& theValueName, const std::string& theString);
    bool                    RegistryWriteInteger(const std::string& theValueName, int theValue);
    bool                    RegistryWriteBoolean(const std::string& theValueName, bool theValue);
    bool                    RegistryWriteData(const std::string& theValueName, const uchar* theValue, uint32_t theLength);
    virtual void            WriteToRegistry();
    virtual void            ReadFromRegistry();
    SexyString              GetString(const std::string& theId);
    SexyString              GetString(const std::string& theId, const SexyString& theDefault);

    void                    SetWindowIconBMP(const std::string& icon);

    virtual void            Done3dTesting();
    virtual std::string     NotifyCrashHook(); // return file name that you want to upload


    // Properties access methods
    bool                    LoadProperties(const std::string& theFileName, bool required, bool checkSig);
    bool                    LoadProperties();



    bool                    GetBoolean(const std::string& theId, bool theDefault=false);
    int                     GetInteger(const std::string& theId, int theDefault=0);
    double                  GetDouble(const std::string& theId, double theDefault=0.0);
    StringVector            GetStringVector(const std::string& theId);

    void                    SetBoolean(const std::string& theId, bool theValue);
    void                    SetInteger(const std::string& theId, int theValue);
    void                    SetDouble(const std::string& theId, double theValue);
    void                    SetString(const std::string& theId, const std::wstring& theValue);
    virtual bool            CheckSignature(const Buffer& theBuffer, const std::string& theFileName);

    int                     ParseCommandLine(int argc, char** argv);

    std::string             GetAppDataFolder() const              { return mAppDataFolder; }
    void                    SetAppDataFolder(const std::string & thePath);
    std::string             GetAppResourceFolder() const          { return mAppResourceFolder; }
    void                    SetAppResourceFolder(const std::string & thePath);
    std::string             GetAppResourceFileName(const std::string& fileName) const;
    std::string             GetUserLanguage() const               { return mUserLanguage; }
    void                    SetUserLanguage(const std::string& l);

protected:
    // Registry helpers
    bool                    RegistryRead(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength);
    bool                    RegistryReadKey(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength, HKEY theMainKey = HKEY_CURRENT_USER);
    bool                    RegistryWrite(const std::string& theValueName, uint32_t theType, const uchar* theValue, uint32_t theLength);

    virtual bool            DoUpdateFrames();
    virtual void            DoUpdateFramesF(float theFrac);
    virtual void            LoadingThreadCompleted();
    virtual void            UpdateFrames();
    virtual bool            DrawDirtyStuff();
    void                    UpdateFTimeAcc();
    virtual bool            Process(bool allowSleep = true);
    void                    ProcessSafeDeleteList();
    static int              LoadingThreadProcStub(void *theArg);
    void                    StartLoadingThread();
    virtual void            PreDDInterfaceInitHook();
    virtual void            DeleteNativeImageData();
    virtual void            PostDDInterfaceInitHook();
    virtual void            PreDisplayHook();
    bool                    WriteBytesToFile(const std::string& theFileName, const void *theData, uint32_t theDataLen);
    void                    RestoreScreenResolution();
    void                    DoExit(int theCode);
    virtual bool            ChangeDirHook(const char *theIntendedPath);
    virtual void            InitPropertiesHook();
    virtual void            InitHook();

    virtual void            EnforceCursor();
    virtual void            PreTerminate();

    // Resource access methods
    void                    LoadResourceManifest();
    void                    ShowResourceError(bool doExit = false);

    int                     InitDDInterface();
    virtual void            DoMainLoop();

    virtual bool            UpdateApp();
    virtual void            Redraw(Rect* theClipRect);
    virtual void            ShutdownHook();

    int                     ViewportToGameX(int x, int y);
    int                     ViewportToGameY(int x, int y);

private:
    void                    MakeWindow(bool isWindowed, bool is3D);
    void                    MakeWindow_3D_FullScreen();
    void                    MakeWindow_3D_Windowed();
    void                    MakeWindow_SoftwareRendered(bool isWindowed, int bpp);
    void                    Set3DAcclerated(bool is3D, bool reinit);

    virtual MusicInterface* CreateMusicInterface();
    virtual SoundManager*   CreateSoundManager();
};

extern SexyAppBase* gSexyAppBase;

};

#endif //__SEXYAPPBASE_H__
