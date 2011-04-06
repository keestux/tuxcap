//#define SEXY_TRACING_ENABLED
//#define SEXY_PERF_ENABLED
//#define SEXY_MEMTRACE

#include "SexyAppBase.h"
#ifdef DEBUG
#include <iostream>
#endif
#include <fstream>
#include <time.h>
#include <math.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL_keycode.h>

#include "Logging.h"
#include "MTRand.h"
#include "Rect.h"
#include "WidgetManager.h"
#include "Widget.h"
#include "MemoryImage.h"
#include "ImageLib.h"
#include "SDLCommon.h"
#include "SoundManager.h"
#include "SoundInstance.h"
#include "MusicInterface.h"
#if defined(USE_BASS)
#include "BassMusicInterface.h"
#include "BassSoundManager.h"
#elif defined(USE_AUDIERE)
#include "AudiereMusicInterface.h"
#include "AudiereSoundManager.h"
#elif defined(USE_SDLMIXER)
#include <SDL_mixer.h>
#include "SDLMixerMusicInterface.h"
#include "SDLMixerSoundManager.h"
#endif

#include "DDInterface.h"
#include "DDImage.h"
#include "ResourceManager.h"
#include "Dialog.h"
#include "D3DInterface.h"
#include "XMLWriter.h"
#include "XMLParser.h"
#include "PropertiesParser.h"
#include "SWTri.h"
#include "ImageFont.h"
#include "PakInterface.h"
#include "CommandLine.h"
#include "anyoption.h"
#include "Logging.h"
#include "Timer.h"

using namespace Sexy;

const int DEMO_FILE_ID = 0x42BEEF78;
const int DEMO_VERSION = 2;

SexyAppBase* Sexy::gSexyAppBase = NULL;

static bool gScreenSaverActive = false;

#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif


//HotSpot: 11 4
//Size: 32 32
unsigned char gFingerCursorData[] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xe7, 0xff, 0xff,
    0xff, 0xc3, 0xff, 0xff,
    0xff, 0xc3, 0xff, 0xff,
    0xff, 0xc3, 0xff, 0xff,
    0xff, 0xc3, 0xff, 0xff,
    0xff, 0xc0, 0xff, 0xff,
    0xff, 0xc0, 0x1f, 0xff,
    0xff, 0xc0, 0x07, 0xff,
    0xff, 0xc0, 0x03, 0xff,
    0xfc, 0x40, 0x01, 0xff,
    0xfc, 0x00, 0x01, 0xff,
    0xfc, 0x00, 0x01, 0xff,
    0xfc, 0x00, 0x01, 0xff,
    0xff, 0x00, 0x01, 0xff,
    0xff, 0x00, 0x01, 0xff,
    0xff, 0x80, 0x01, 0xff,
    0xff, 0x80, 0x03, 0xff,
    0xff, 0xc0, 0x03, 0xff,
    0xff, 0xc0, 0x03, 0xff,
    0xff, 0xe0, 0x07, 0xff,
    0xff, 0xe0, 0x07, 0xff,
    0xff, 0xe0, 0x07, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,

    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00,
    0x00, 0x1b, 0x00, 0x00,
    0x00, 0x1b, 0x60, 0x00,
    0x00, 0x1b, 0x68, 0x00,
    0x00, 0x1b, 0x6c, 0x00,
    0x01, 0x9f, 0xec, 0x00,
    0x01, 0xdf, 0xfc, 0x00,
    0x00, 0xdf, 0xfc, 0x00,
    0x00, 0x5f, 0xfc, 0x00,
    0x00, 0x7f, 0xfc, 0x00,
    0x00, 0x3f, 0xfc, 0x00,
    0x00, 0x3f, 0xf8, 0x00,
    0x00, 0x1f, 0xf8, 0x00,
    0x00, 0x1f, 0xf8, 0x00,
    0x00, 0x0f, 0xf0, 0x00,
    0x00, 0x0f, 0xf0, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

//HotSpot: 15 10
//Size: 32 32
unsigned char gDraggingCursorData[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0,
    0x01, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff,
    0xe0, 0x00, 0xff, 0xfe, 0x60, 0x00, 0xff, 0xfc, 0x20, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff,
    0xfe, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00,
    0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xf0,
    0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x0d, 0xb0, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00,
    0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00,
    0x01, 0x8d, 0xb6, 0x00, 0x01, 0xcf, 0xfe, 0x00, 0x00, 0xef, 0xfe, 0x00, 0x00, 0xff, 0xfe,
    0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x1f,
    0xfc, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00,
    0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};

static DDImage* gFPSImage = NULL;
static ImageFont* aFont = NULL;

//////////////////////////////////////////////////////////////////////////

SexyAppBase::SexyAppBase()
{
    mLogFacil = LoggerFacil::find("sexyappbase");
    Logger::tlog(mLogFacil, 1, "new SexyAppBase");

    // There should be only one. Should we check?
    gSexyAppBase = this;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::string msg = SDL_GetError();
        msg = std::string("Video initialization failed: ") + msg;
        fprintf(stderr, "%s\n", msg.c_str());
        throw new SDLException(msg);
    }

    if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
        std::string msg = SDL_GetError();
        msg = std::string("Timer initialization failed: ") + msg;
        fprintf(stderr, "%s\n", msg.c_str());
        throw new SDLException(msg);
    }

    // TODO. Move this code somewhere else.
#if defined(USE_SDLMIXER)
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
        std::string msg = SDL_GetError();
        msg = std::string("Audio initialization failed: ") + msg;
        fprintf(stderr, "%s\n", msg.c_str());
        throw new SDLException(msg);
    }
#endif

    mRegistry.clear();

    mScreenSurface = NULL;
    mGameSurface = NULL;

    mProdName = "Product";          // Used in GameApp
    mProductVersion = "";

    mCompanyName = "";
    mFullCompanyName   = "";

    mArgv0 = "";
    mAppDataFolder = "";
    mAppResourceFolder = "";
    mUserLanguage = "";

    mShutdown = false;

    mRegKey = "";
    mRegisterLink = "";

    mPreferredX = -1;
    mPreferredY = -1;
    mWidth = 640;
    mHeight = 480;

    mFullscreenBits = 0; //Not Used

    mMusicVolume = 0.85;
    mSfxVolume = 0.85;
    mDemoMusicVolume = 0.0;
    mDemoSfxVolume = 0.0;
    mNoSoundNeeded = false;

    mWantFMod = false;
    mCmdLineParsed = false;
    mSkipSignatureChecks = false;
    mStandardWordWrap = true;
    mbAllowExtendedChars = true;
#ifdef _DEBUG
    mOnlyAllowOneCopyToRun = false;
#else
    mOnlyAllowOneCopyToRun = true;
#endif
    mSEHOccured = false;
    mExitToTop = false;
    mIsWindowed = false;
    mIsPhysWindowed = true;
    mFullScreenWindow = false;
    mForceFullscreen = false;
    mForceWindowed = false;
    mInitialized = false;
    mProcessInTimer = false;
    mIsScreenSaver = false;
    mAllowMonitorPowersave = true;
    mNoDefer = false;
    mFullScreenPageFlip = true; // should we page flip in fullscreen?
    mTabletPC = false;
    mAlphaDisabled = false;

    mReadFromRegistry = false;
    mIsOpeningURL = false;

    mMuteCount = 0;
    mAutoMuteCount = 0;
    mDemoMute = false;
    mMuteOnLostFocus = true;

    mCleanupSharedImages = false;

    mNonDrawCount = 0;
    mFrameTime = 10;

    mIsDrawing = false;
    mLastDrawWasEmpty = false;
    mHasPendingDraw = true;
    mPendingUpdatesAcc = 0.0;
    mUpdateFTimeAcc = 0.0;
    mDrawCount = 0;
    mSleepCount = 0;
    mUpdateCount = 0;
    mUpdateAppState = 0;
    mUpdateAppDepth = 0;
    mUpdateMultiplier = 1;
    mPaused = false;
    mFastForwardToUpdateNum = 0;
    mFastForwardToMarker = false;
    mFastForwardStep = false;
    mStepMode = 0;

    mCursorNum = CURSOR_POINTER;
    mMouseIn = false;
    mRunning = false;
    mActive = true;
    mMinimized = false;
    mPhysMinimized = false;
    mIsDisabled = false;
    mHasFocus = true;
    mDrawTime = 0;

    mFPSFlipCount = 0;
    mFPSDirtyCount = 0;
    mFPSTime = 0;
    mFPSCount = 0;
    mShowFPS = false;
    mShowFPSMode = FPS_ShowFPS;

    mScreenBltTime = 0;
    mAutoStartLoadingThread = true;
    mLoadingThreadStarted = false;
    mLoadingThreadCompleted = false;
    mLoaded = false;
    mYieldMainThread = false;
    mLoadingFailed = false;
    mSysCursor = true;
    mCustomCursorsEnabled = false;
    mCustomCursorDirty = false;
    mLastShutdownWasGraceful = true;
    mIsWideWindow = false;

    mNumLoadingThreadTasks = 0;
    mCompletedLoadingThreadTasks = 0;

    mWindowIconBMP = "";

    mDemoLength = 0;
    mLastDemoMouseX = 0;
    mLastDemoMouseY = 0;
    mLastDemoUpdateCnt = 0;
    mDemoNeedsCommand = true;
    mDemoCmdNum = 0;
    mDemoCmdOrder = -1; // Means we haven't processed any demo commands yet
    mDemoCmdBitPos = 0;
    mDemoLoadingComplete = false;

    mCurHandleNum = 0;

    mDemoMarkerList.clear();

    mDebugKeysEnabled = false;
    mEnableMaximizeButton = false;
    mCtrlDown = false;
    mAltDown = false;

    mSyncRefreshRate = 100;
    mVSyncUpdates = false;
    mVSyncBroken = false;
    mVSyncBrokenCount = 0;
    mWaitForVSync = false;
    mSoftVSyncWait = true;
    mUserChanged3DSetting = false;
    mAutoEnable3D = false;
    mTest3D = true;
    mWidescreenAware = false;
    mEnableWindowAspect = false;
    mStringProperties.clear();
    mBoolProperties.clear();
    mIntProperties.clear();
    mDoubleProperties.clear();
    mStringVectorProperties.clear();
    mPrimaryThreadId = 0;

    mMutex = NULL;
    mHandCursor = NULL;
    mDraggingCursor = NULL;
    mArrowCursor = NULL;

    mLastTimeCheck = 0;
    mLastTime = 0;
    mLastUserInputTick = 0;
    mLastTimerTime = 0;
    mLastDrawTick = SDL_GetTicks();
    mNextDrawTick = SDL_GetTicks();
    mSafeDeleteList.clear();
    mVSyncBrokenTestStartTick = 0;
    mVSyncBrokenTestUpdates = 0;

    mDDInterface = NULL;
    for (int i = 0; i < 256; i++)
        mAdd8BitMaxTable[i] = i;
    for (int i = 256; i < 512; i++)
        mAdd8BitMaxTable[i] = 255;
    mWidgetManager = new WidgetManager(this);
    mTimeLoaded = SDL_GetTicks();
    mTitle = _S("SexyApp");
    for (int i = 0; i < NUM_CURSORS; i++)
        mCursorImages[i] = NULL;
    mFPSStartTick = SDL_GetTicks();
    mDemoBuffer.Clear();
    mMemoryImageSet.clear();
    mSharedImageMap.clear();

    mHWnd = 0;
    mInvisHWnd = 0;
    mNotifyGameMessage = 0;
    mSoundManager = NULL;
    mMusicInterface = NULL;
    mDialogMap.clear();
    mDialogList.clear();

    // Best initial guess is that the viewport has the exact the same dimension as the game
    mViewportx = 0;
    mViewporty = 0;
    mViewportWidth = mWidth;
    mViewportHeight = mHeight;
    mViewportToGameRatio  = 1.0f;
    mViewportIsRotated = false;

    mVideoModeWidth = -1;       // We don't know yet
    mVideoModeHeight = -1;

    // Commandline options. See ParseCommandLine()
    // TODO.
    // We could set the correct values already by using CmdLine->getOpt()
    mFullScreenMode = false;
    mWindowedMode = false;
    mUseOpenGL = false;
    mUseSoftwareRenderer = false;
    mDebug = false;

    mResourceManager = NULL;
}

SexyAppBase::~SexyAppBase()
{
    Logger::tlog(mLogFacil, 1, "shutting down");
    if (!mShutdown)
        Shutdown();

    // Check if we should write the current 3d setting
    if (mUserChanged3DSetting)
    {
        RegistryWriteBoolean("Is3D", mDDInterface->mIs3D);
    }
    delete gFPSImage;
    gFPSImage = NULL;
    delete aFont;
    aFont = NULL;

    WaitForLoadingThread();
    DialogMap::iterator aDialogItr = mDialogMap.begin();
    while (aDialogItr != mDialogMap.end())
    {
        mWidgetManager->RemoveWidget(aDialogItr->second);
        delete aDialogItr->second;
        ++aDialogItr;
    }
    mDialogMap.clear();
    mDialogList.clear();

    delete mWidgetManager;
    delete mResourceManager;

    SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
    while (aSharedImageItr != mSharedImageMap.end())
    {
        SharedImage* aSharedImage = &aSharedImageItr->second;
        assert(aSharedImage->mRefCount == 0);
        delete aSharedImage->mImage;
        mSharedImageMap.erase(aSharedImageItr++);
    }

    delete mDDInterface;
    mDDInterface = NULL;
    delete mSoundManager;
    mSoundManager = NULL;
    delete mMusicInterface;
    mMusicInterface = NULL;

    SDL_FreeCursor(mHandCursor);
    SDL_FreeCursor(mDraggingCursor);
    SDL_FreeCursor(mArrowCursor);
    if (mMutex != NULL) {
        SDL_DestroyMutex(mMutex);
        mMutex = NULL;
    }

    // FIXME. Some values (Is3DAccelerated) may not be valid anymore. (See "delete mDDInterface" above.)
    if (mReadFromRegistry) {
        WriteToRegistry();
        WriteRegistryToIni(BuildIniName(mRegKey, ".") + ".ini");
    }

    if (gSexyAppBase == this)
        gSexyAppBase = NULL;
    SDL_Quit();
    Logger::tlog(mLogFacil, 1, "at exit(0) in ~SexyAppBase()");
    exit(0);                // ???? FIXME. KB says: why exit here?
}

bool SexyAppBase::RegistryWrite(const std::string& theValueName, uint32_t theType, const uchar* theValue, uint32_t theLength)
{
    if (mRegKey.length() == 0)
        return false;

    if (!mReadFromRegistry) {
        ReadRegistryFromIni(BuildIniName(mRegKey, ".") + ".ini");
        mReadFromRegistry = true;
    }

    std::string aValueName;

    size_t aSlashPos = (int) theValueName.rfind('\\');
    if (aSlashPos != std::string::npos)
    {
        aValueName = theValueName.substr(aSlashPos + 1);
    }
    else
    {
        aValueName = theValueName;
    }

    std::ostringstream stream;
    if (theType == REG_DWORD) {
        int i ;
        memcpy(&i, theValue, sizeof(int));;
        stream << i;
    }
    else if (theType == REG_SZ) {
        stream << theValue;
    }
    else if (theType == REG_BINARY) {
        uchar* buf = new uchar[theLength+1];
        memcpy(buf, theValue, theLength);
        buf[theLength] = '\0';
        stream << buf;
        delete[] buf;
    }
    mRegistry[StringToSexyString(aValueName)] = StringToSexyString(stream.str());


    return true;
}

// Note. The ini file is in XML format.
bool SexyAppBase::WriteRegistryToIni(const std::string& IniFile)
{

    if (mRegKey.length() == 0)
        return false;

    XMLWriter writer;

    std::string path = mAppDataFolder;
    if (path.empty()) {
        // Oops. AppDataFolder not set.
        return false;
    }

    path += IniFile;

    if (!writer.OpenFile(path))
        return false;

    writer.StartElement("Registry");

    std::map<SexyString, SexyString>::const_iterator it = mRegistry.begin();
    while (it != mRegistry.end()) {
        writer.StartElement("Key");
        writer.WriteAttribute("ID", it->first);
        writer.StartElement("Value");
        writer.WriteAttribute("value", it->second);
        writer.StopElement();
        writer.StopElement();
        ++it;
    }

    writer.StopElement();

    return true;
}

bool SexyAppBase::ReadRegistryFromIni(const std::string& IniFile)
{

    XMLParser parser;

    std::string path = mAppDataFolder;
    if (path.empty()) {
        // Oops. AppDataFolder not set.
        return false;
    }

    path += IniFile;

    if (parser.OpenFile(path)) {

        XMLElement e;

        std::string key;
        std::string value;

        mRegistry.clear();

        while (parser.NextElement(&e)) {
            if (e.mType == XMLElement::TYPE_START) {
                if (e.mSection == "Registry") {
                    if (e.mValue == "Key") {
                        key = e.mAttributes["ID"];
                    }
                } else if (e.mSection == "Registry/Key") {
                    if (e.mValue == "Value") {
                        value = e.mAttributes["value"];
                    }
                }
            } else if (e.mType == XMLElement::TYPE_END) {
                if (e.mSection == "Registry/Key") {
                    mRegistry[key] = value;
                }
            }
        }
#ifdef DEBUG
        if (parser.HasFailed()) {
            std::cout << parser.GetErrorText() << std::endl;
        }
#endif
    }
    return true;
}

bool SexyAppBase::RegistryWriteString(const std::string& theValueName, const std::string& theString)
{
    return RegistryWrite(theValueName, REG_SZ, (uchar*) theString.c_str(), theString.length());
}

bool SexyAppBase::RegistryWriteInteger(const std::string& theValueName, int theValue)
{
    return RegistryWrite(theValueName, REG_DWORD, (uchar*) &theValue, sizeof(int));
}

bool SexyAppBase::RegistryWriteBoolean(const std::string& theValueName, bool theValue)
{
    int aValue = theValue ? 1 : 0;
    return RegistryWrite(theValueName, REG_DWORD, (uchar*) &aValue, sizeof(int));
}

bool SexyAppBase::RegistryWriteData(const std::string& theValueName, const uchar* theValue, uint32_t theLength)
{
    return RegistryWrite(theValueName, REG_BINARY, (uchar*) theValue, theLength);
}

void SexyAppBase::WriteToRegistry()
{
    RegistryWriteInteger("MusicVolume", (int) (mMusicVolume * 100));
    RegistryWriteInteger("SfxVolume", (int) (mSfxVolume * 100));
    RegistryWriteInteger("Muted", (mMuteCount - mAutoMuteCount > 0) ? 1 : 0);
    RegistryWriteInteger("ScreenMode", mIsWindowed ? 0 : 1);
    RegistryWriteInteger("PreferredX", mPreferredX);
    RegistryWriteInteger("PreferredY", mPreferredY);
    RegistryWriteInteger("CustomCursors", mCustomCursorsEnabled ? 1 : 0);
    RegistryWriteInteger("InProgress", 0);
    RegistryWriteBoolean("WaitForVSync", mWaitForVSync);
    RegistryWriteBoolean("VSyncUpdates", mVSyncUpdates);
    RegistryWriteBoolean("Is3D", Is3DAccelerated());            // FIXME. mDDInterface may already have been deleted
}

bool SexyAppBase::RegistryEraseKey(const SexyString& _theKeyName)
{
    if (mRegKey.length() == 0)
        return false;

    mRegistry.erase(_theKeyName);

    return true;
}

void SexyAppBase::RegistryEraseValue(const SexyString& _theValueName)
{
    if (mRegKey.length() == 0)
        return;

    std::map<SexyString, SexyString>::iterator it = mRegistry.begin();
    while (it != mRegistry.end()) {
        if (it->second == _theValueName)
            it->second = "";
        ++it;
    }
}

bool SexyAppBase::RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
    //FIXME TODO
    return false;
}

bool SexyAppBase::RegistryRead(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength)
{
    return RegistryReadKey(theValueName, theType, theValue, theLength, HKEY_CURRENT_USER);
}

bool SexyAppBase::RegistryReadKey(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength, HKEY theKey)
{
    if (mRegKey.length() == 0)
        return false;

#if 0
    if (mPlayingDemoBuffer) {
        ...
    } else
#endif
    {
        if (!mReadFromRegistry) {
            ReadRegistryFromIni(BuildIniName(mRegKey, ".") + ".ini");
            mReadFromRegistry = true;
        }

        std::string aValueName;

        size_t aSlashPos = theValueName.rfind('\\');
        if (aSlashPos != std::string::npos) {
            aValueName = theValueName.substr(aSlashPos + 1);
        } else {
            aValueName = theValueName;
        }

        SexyString s = mRegistry[aValueName];
        if (s == "")
            return false;

        uchar* c_str = (uchar*) (SexyStringToString(s)).c_str();
        if (*theType == REG_SZ) {
            memcpy(theValue, c_str, 1023);
        } else if (*theType == REG_DWORD) {
            int i = atoi((const char*) c_str);
            memcpy(theValue, &i, sizeof (int));
        } else if (*theType == REG_BINARY) {
            memcpy(theValue, c_str, *theLength);
        }
    }

    return true;
}

bool SexyAppBase::RegistryReadString(const std::string& theKey, std::string* theString)
{
    char aStr[1024];

    uint32_t aType = REG_SZ;
    uint32_t aLen = sizeof(aStr) - 1;
    if (!RegistryRead(theKey, &aType, (uchar*) aStr, &aLen))
        return false;

    aStr[aLen] = 0;

    *theString = aStr;
    return true;
}

bool SexyAppBase::RegistryReadInteger(const std::string& theKey, int* theValue)
{
    uint32_t aType = REG_DWORD;
    uint32_t aLong;
    uint32_t aLen = 4;
    if (!RegistryRead(theKey, &aType, (uchar*) &aLong, &aLen))
        return false;

    *theValue = aLong;
    return true;
}

bool SexyAppBase::RegistryReadBoolean(const std::string& theKey, bool* theValue)
{
    int aValue;
    if (!RegistryReadInteger(theKey, &aValue))
        return false;

    *theValue = aValue != 0;
    return true;
}

bool SexyAppBase::RegistryReadData(const std::string& theKey, uchar* theValue, uint32_t* theLength)
{
    uint32_t aType = REG_BINARY;
    if (!RegistryRead(theKey, &aType, (uchar*) theValue, theLength))
        return false;

    return true;
}

void SexyAppBase::ReadFromRegistry()
{
    mRegKey = SexyStringToString(GetString("RegistryKey", StringToSexyString(mRegKey)));

    if (mRegKey.length() == 0)
        return;

    int anInt = 0;
    if (RegistryReadInteger("MusicVolume", &anInt))
        mMusicVolume = anInt / 100.0;

    if (RegistryReadInteger("SfxVolume", &anInt))
        mSfxVolume = anInt / 100.0;

    if (RegistryReadInteger("Muted", &anInt))
        mMuteCount = anInt;

    if (RegistryReadInteger("ScreenMode", &anInt))
        mIsWindowed = anInt == 0;

    RegistryReadInteger("PreferredX", &mPreferredX);
    RegistryReadInteger("PreferredY", &mPreferredY);

    if (RegistryReadInteger("CustomCursors", &anInt))
        EnableCustomCursors(anInt != 0);

    // Huh? Why is this "write"?
    RegistryWriteBoolean("WaitForVSync", mWaitForVSync);
    RegistryWriteBoolean("VSyncUpdates", mVSyncUpdates);

    if (RegistryReadInteger("InProgress", &anInt))
        mLastShutdownWasGraceful = anInt == 0;
}

static void CalculateFPS()
{
    static bool gForceDisplay = true;
    static int gFPSDisplay;
    static int gFrameCount;
    static Uint32 fps_oldtime;

    gFrameCount++;

#if 0
    //workaround to force texture reloading
    if (gSexyAppBase->Is3DAccelerated()) {
        delete gFPSImage;
        gFPSImage = NULL;
    }

    if (aFont == NULL)
        aFont = new ImageFont(gSexyAppBase, "fonts/Kiloton9.txt");

    if (gFPSImage == NULL) {

        gFPSImage = new DDImage(gSexyAppBase->mDDInterface);
        gFPSImage->Create(80, aFont->GetHeight() + 4);
        gFPSImage->SetImageMode(false, false);
        gFPSImage->SetVolatile(true);
        gFPSImage->mPurgeBits = false;
        gFPSImage->mWantDDSurface = true;
        gFPSImage->PurgeBits();
    }
#endif

    Uint32 fps_newtime = SDL_GetTicks();

    if (fps_newtime - fps_oldtime >= 1000 || gForceDisplay) {
        if (!gForceDisplay)
            gFPSDisplay = (int) ((gFrameCount * 1000 / (fps_newtime - fps_oldtime)) + 0.5f);
        else {
            gForceDisplay = false;
            gFPSDisplay = 0;
        }

        gFrameCount = 0;
        fps_oldtime = SDL_GetTicks();

#if 0
        Graphics aDrawG(gFPSImage);
        aDrawG.SetFont(aFont);
        SexyString aFPS = StrFormat(_S("FPS: %d"), gFPSDisplay);
        aDrawG.SetColor(Color(0, 0, 0));
        aDrawG.FillRect(0, 0, gFPSImage->GetWidth(), gFPSImage->GetHeight());
        aDrawG.SetColor(Color(0xFF, 0xFF, 0xFF));
        aDrawG.DrawString(aFPS, 2, aFont->GetAscent());
        gFPSImage->mBitsChangedCount++;
#else
        fprintf(stdout, "%d\n", gFPSDisplay);
#endif
    }
}

///////////////////////////// FPS Stuff to draw mouse coords
static void FPSDrawCoords(int theX, int theY)
{
    //workaround to force texture reloading
    if (gSexyAppBase->Is3DAccelerated()) {
        delete gFPSImage;
        gFPSImage = NULL;
    }

    if (gFPSImage == NULL) {
        gFPSImage = new DDImage(gSexyAppBase->mDDInterface);
        gFPSImage->Create(80, aFont->GetHeight() + 4);
        gFPSImage->SetImageMode(false, false);
        gFPSImage->SetVolatile(true);
        gFPSImage->mPurgeBits = false;
        gFPSImage->mWantDDSurface = true;
        gFPSImage->PurgeBits();
    }

    Graphics aDrawG(gFPSImage);
    aDrawG.SetFont(aFont);
    SexyString aFPS = StrFormat(_S("%d,%d"), theX, theY);
    aDrawG.SetColor(Color(0, 0, 0));
    aDrawG.FillRect(0, 0, gFPSImage->GetWidth(), gFPSImage->GetHeight());
    aDrawG.SetColor(Color(0xFF, 0xFF, 0xFF));
    aDrawG.DrawString(aFPS, 2, aFont->GetAscent());
    gFPSImage->mBitsChangedCount++;
}

void SexyAppBase::Set3DAcclerated(bool is3D, bool reinit)
{
    if (mDDInterface->mIs3D == is3D)
        return;

    mUserChanged3DSetting = true;               // Huh?
    mDDInterface->mIs3D = is3D;

    if (reinit)
    {
        int aResult = InitDDInterface();

        if (is3D && aResult != DDInterface::RESULT_OK)
        {
            Set3DAcclerated(false, reinit);
            return;
        }
        else if (aResult != DDInterface::RESULT_OK)
        {
#if 0
            Popup(GetString("FAILED_INIT_DIRECTDRAW", _S("Failed to initialize DirectDraw: ")) + StringToSexyString(DDInterface::ResultToString(aResult) + " " + mDDInterface->mErrorString));
#endif
            DoExit(1);
        }

        ReInitImages();

        // FIXME. We're using ScreenImage which is initialized using mSurface (created by SDL_SetVideoMode).
        // Using mSurface is not good when we expect to be doing OpenGL scaling game-to-screen.
        mWidgetManager->SetImage(mDDInterface->GetScreenImage());
        mWidgetManager->MarkAllDirty();
    }
}

//convert from win32 cursors to sdl cursor

static void ConvertCursorToSDL(unsigned char* data)
{

    unsigned char temp_cursor[256];
    for (int i = 0; i < 128; ++i) {
        unsigned char and_mask = data[i];
        unsigned char xor_mask = data[i + 128];
        unsigned char new_and_mask = 0;
        unsigned char new_xor_mask = 0;

        for (int j = 0; j < 8; ++j) {
            if ((and_mask & (1 << j)) && (xor_mask & (1 << j))) {
                new_and_mask |= 1 << j;
            } else if (!(and_mask & (1 << j)) && (xor_mask & (1 << j))) {
                new_xor_mask |= 1 << j;
            } else if (!(and_mask & (1 << j)) && !(xor_mask & (1 << j))) {
                new_and_mask |= 1 << j;
                new_xor_mask |= 1 << j;
            }
        }

        temp_cursor[i] = new_and_mask;
        temp_cursor[i + 128] = new_xor_mask;
    }

    memcpy(data, temp_cursor, 256);
}

static std::string findResourceFolder(const std::string & dir)
{
    // Look for file main.pak
    if (FileExists(dir + "main.pak")) {
        return dir;
    }
    // Look for dir res, Resources
    if (IsDir(dir + "res")) {
        return dir + "res/";
    }
    else if (IsDir(dir + "Resources")) {
        return dir + "Resources/";
    }
    else if (IsDir(dir + "resources")) {
        // Linux is case sensitive. We may want to use lower case directory names
        return dir + "resources/";
    }
    return "";
}

static std::string stripCurrentDir(std::string dir)
{
    while (dir.find("./") == 0) {
        dir = dir.substr(2);
    }
    return dir;
}

static std::string determineResourceFolder(std::string bindir)
{
    bindir = stripCurrentDir(bindir);
    std::string rscDir;

    // Look in <bindir>/..
    rscDir = findResourceFolder(bindir + "../");
    if (rscDir != "") {
        return rscDir;
    }
    // Look in <bindir>/../lib
    rscDir = findResourceFolder(bindir + "../lib/");
    if (rscDir != "") {
        return rscDir;
    }
    // Look in <bindir>
    rscDir = findResourceFolder(bindir);
    if (rscDir != "") {
        return rscDir;
    }
    // Elsewhere?
    return "";
}

void SexyAppBase::Init()
{
    mPrimaryThreadId = SDL_ThreadID();

    if (mShutdown)
        return;

    if (mResourceManager == NULL)
        mResourceManager = new ResourceManager(this);

#ifdef __APPLE__
    SetAppDataFolder(std::string(getenv("HOME")) + "/Library/Preferences/" + mRegKey + "/");
#else
    // The rest is Linux/UNIX, right?
    // Create a new directory in the HOME directory of the user.
    SetAppDataFolder(std::string(getenv("HOME")) + "/." + mRegKey + "/");
#endif

    if (GetAppResourceFolder() == "" && mArgv0 != "") {
        // ResourceFolder not set.
        // Use the directory of the program instead.
        std::string bindir = GetFileDir(std::string(mArgv0), true);
        std::string rscDir = determineResourceFolder(bindir);
        Logger::log(mLogFacil, 2, Logger::format("determineResourceFolder = '%s'", rscDir.c_str()));
        if (rscDir != "") {
            Logger::log(mLogFacil, 1, Logger::format("Setting AppResourceFolder to '%s'\n", rscDir.c_str()));
            SetAppResourceFolder(rscDir);
        }
    } else {
        Logger::log(mLogFacil, 1, Logger::format("AppResourceFolder = '%s'", GetAppResourceFolder().c_str()));
    }

    InitPropertiesHook();

    ReadFromRegistry();

    if (GetPakPtr() == NULL) {
        new PakInterface();
    }
    if (!GetPakPtr()->isLoaded()) {
        if (!GetPakPtr()->AddPakFile(GetAppResourceFileName("main.pak"))) {
            // TODO. Throw exception.
            // Don't return; because it would leave SexyAppBase in a bad state.
            // Just hope that we can find the resource files anyway.
        }
    }

    if (mMutex != NULL)
        HandleGameAlreadyRunning();
    mMutex = SDL_CreateMutex();

    SRand(SDL_GetTicks());
    srand(SDL_GetTicks());

    mArrowCursor = SDL_GetCursor();

    ConvertCursorToSDL(gFingerCursorData);
    ConvertCursorToSDL(gDraggingCursorData);

    mHandCursor = SDL_CreateCursor(gFingerCursorData, gFingerCursorData+sizeof(gFingerCursorData)/2, 32, 32, 11, 4);
    mDraggingCursor = SDL_CreateCursor(gDraggingCursorData, gDraggingCursorData+sizeof(gDraggingCursorData)/2, 32, 32, 15,10);

    // Let app do something before showing window, or switching to fullscreen mode
    // NOTE: Moved call to PreDisplayHook above mIsWindowed and GetSystemsMetrics
    // checks because the checks below use values that could change in PreDisplayHook.
    // PreDisplayHook must call mWidgetManager->Resize if it changes mWidth or mHeight.
    PreDisplayHook();

    mWidgetManager->Resize(Rect(0, 0, mWidth, mHeight), Rect(0, 0, mWidth, mHeight));

    // Check to see if we CAN run windowed or not...
    assert(!mFullScreenWindow);                 // Please report. We want to know when mFullScreenWindow is true.
    if (mIsWindowed && !mFullScreenWindow)
    {
        //FIXME check OpenGL
        SDL_Rect **modes;
        modes = SDL_ListModes(NULL, SDL_DOUBLEBUF);

        /* Check if there are any modes available */
        if (modes == (SDL_Rect **)0) {
            // TODO. Raise exception
          printf("No modes available!\n");
          exit(-1);
        }

        /* Check if our resolution is restricted */
        if (modes == (SDL_Rect **)-1) {
            //printf("All resolutions available.\n");
        }
        else {
            // How can we be windowed if our screen isn't even big enough?
            //
            if ((mWidth > modes[0]->w) ||
                (mHeight > modes[0]->h))
            {
                // We're assuming that the first mode is the biggest.
                // Our game doesn't fit inthere. Try to switch non-windowed.
                Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::Init: video mode too small w=%d, h=%d", modes[0]->w, modes[0]->h));
                mIsWindowed = false;
                mForceFullscreen = true;
            }
        }
    }
    else if (mFullScreenWindow)
    {
        SDL_Rect **modes;
        modes = SDL_ListModes(NULL, SDL_DOUBLEBUF | SDL_FULLSCREEN);

        /* Check if there are any modes available */
        if (modes == (SDL_Rect **)0) {
            assert(0);                  // Need to verify this. Please report.
            mFullScreenWindow = false;
            mIsWindowed = false;        // Huh? Shouldn't this be false. There are no modes for full screen.
        }
    }

    SetUserLanguage("");

#if __APPLE__
    bool testedLanguage;
    RegistryReadBoolean("TestedLanguage", &testedLanguage);

    if (!testedLanguage) {
        //detect language
        FILE* info = popen("defaults read .GlobalPreferences AppleLanguages | tr -d [:space:] | cut -c2-3", "r");
        std::string s;

        if (info != NULL) {
        int c;

        while ((c = fgetc(info)) != EOF)
            s += (unsigned char)c;
        }

        pclose(info);

        SetUserLanguage(s);
        RegistryWriteBoolean("TestedLanguage", true);
    }
#endif

#if 0
        if (mShowFPS)
          aFont = new ImageFont(gSexyAppBase,"fonts/Kiloton9.txt");
#endif

    if (!mNoSoundNeeded) {
        mSoundManager = CreateSoundManager();
        if (mSoundManager == NULL)
            mNoSoundNeeded = true;
        else
            SetSfxVolume(mSfxVolume);
    }

    mMusicInterface = CreateMusicInterface();

    SetMusicVolume(mMusicVolume);

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    if (mDDInterface == NULL) {
        mDDInterface = new DDInterface(this);

        // Enable 3d setting

        bool is3D = mAutoEnable3D;
        bool tested3D = false;

        if (mAutoEnable3D) {
            tested3D = true;
        }
        else {
            RegistryReadBoolean("Is3D", &is3D);
            RegistryReadBoolean("Tested3D", &tested3D);
        }

#ifndef __APPLE__
        if (mTest3D && !tested3D) {
            //run glxinfo to get direct rendering info from driver
            //FIXME. First detect if glxinfo is present.

            is3D = false;
            const char * glxinfo = "/usr/bin/glxinfo";
            if (access(glxinfo, R_OK|X_OK) == 0) {
                std::string cmd = glxinfo;
                cmd += "|grep rendering";
                FILE* info = popen(cmd.c_str(), "r");
                std::string s;

                if (info != NULL) {
                    int c;

                    while ((c = fgetc(info)) != EOF)
                        s += (unsigned char)c;
                }

                pclose(info);

                if (s.find("Yes", 0) != std::string::npos) {
                    is3D = true;
                }
            }
            RegistryWriteBoolean("Tested3D", true);
        }
#else
        is3D = true;
#endif
        mDDInterface->mIs3D = is3D;
    }

    // Set Windowing mode based on commandline parameters, if present
    // Should we let some commandline parameters override the registry?
    if (mFullScreenMode) {
        mIsWindowed = false;
        if (mUseOpenGL) {
            Logger::log(mLogFacil, 1, "Running full screen with OpenGL hardware acceleration");
            SwitchScreenMode(mIsWindowed, true);
        } else if (mUseSoftwareRenderer) {
            Logger::log(mLogFacil, 1, "Running full screen with Software Renderer");
            SWTri_AddAllDrawTriFuncs();
            SwitchScreenMode(mIsWindowed, false);
        } else {
            Logger::log(mLogFacil, 1, "Running in fullscreen mode");
            SwitchScreenMode(mIsWindowed, mDDInterface->mIs3D);
        }
    } else if (mWindowedMode) {
        mIsWindowed = true;
        if (mUseOpenGL) {
            SwitchScreenMode(mIsWindowed, true);
            Logger::log(mLogFacil, 1, "Running with OpenGL hardware acceleration");
        } else if (mUseSoftwareRenderer) {
            SwitchScreenMode(mIsWindowed, false);
            Logger::log(mLogFacil, 1, "Running with Software Renderer");
            SWTri_AddAllDrawTriFuncs();
        } else {
            SwitchScreenMode(mIsWindowed, mDDInterface->mIs3D);
            Logger::log(mLogFacil, 1, "Running in windowed mode");
        }
    } else {
        // Use the flags from the registry.
        MakeWindow();
    }

    mInitialized = true;
}


void SexyAppBase::DeleteExtraImageData()
{
    MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
    while (anItr != mMemoryImageSet.end())
    {
        MemoryImage* aMemoryImage = *anItr;
        aMemoryImage->DeleteExtraBuffers();
        ++anItr;
    }
}

void SexyAppBase::ReInitImages()
{
    MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
    while (anItr != mMemoryImageSet.end())
    {
        MemoryImage* aMemoryImage = *anItr;
        aMemoryImage->ReInit();
        ++anItr;
    }
}


void SexyAppBase::Shutdown()
{
    if ((mPrimaryThreadId != 0) && (SDL_ThreadID() != mPrimaryThreadId))
    {
        mLoadingFailed = true;
    }
    else if (!mShutdown)
    {
        mExitToTop = true;
        mShutdown = true;
        ShutdownHook();
    }
}
void SexyAppBase::Start()
{
    if (mShutdown)
        return;

    if (mAutoStartLoadingThread)
        StartLoadingThread();

    Uint32 aStartTime = SDL_GetTicks();

    mRunning = true;

    mLastTime = aStartTime;
    mLastUserInputTick = aStartTime;
    mLastTimerTime = aStartTime;

    DoMainLoop();

    ProcessSafeDeleteList();

    mRunning = false;

    WaitForLoadingThread();

//  PreTerminate();

    WriteToRegistry();
    WriteRegistryToIni(BuildIniName(mRegKey, ".") + ".ini");
}

void SexyAppBase::DoMainLoop()
{
    while (!mShutdown)
    {
        if (mExitToTop)
            mExitToTop = false;
        UpdateApp();
    }
}

bool SexyAppBase::UpdateApp()
{
    bool updated;
    for (;;)
    {
        if (!UpdateAppStep(&updated))
            return false;
        if (updated)
            return true;
    }
}

int SexyAppBase::ViewportToGameX(int x, int y)
{
    x = (x - mViewportx) * mViewportToGameRatio;
    y = (y - mViewporty) * mViewportToGameRatio;
    if (mViewportIsRotated)
        return y;
    return x;
}

int SexyAppBase::ViewportToGameY(int x, int y)
{
    x = (x - mViewportx) * mViewportToGameRatio;
    y = (y - mViewporty) * mViewportToGameRatio;
    if (mViewportIsRotated)
        return mHeight - x;
    return y;
}

bool SexyAppBase::UpdateAppStep(bool* updated)
{
    if (updated != NULL)
        *updated = false;

    if (mExitToTop)
        return false;
    if (mUpdateAppState == UPDATESTATE_PROCESS_DONE)
        mUpdateAppState = UPDATESTATE_MESSAGES;

    mUpdateAppDepth++;

    // We update in two stages to avoid doing a Process if our loop termination
    //  condition has already been met by processing windows messages
    if (mUpdateAppState == UPDATESTATE_MESSAGES)
    {
        SDL_Event test_event;

        if (SDL_PollEvent(&test_event)) {
            switch(test_event.type) {

            case SDL_MOUSEMOTION:
                //FIXME
                if (/*(!gInAssert) &&*/ (!mSEHOccured))
                {
                    mDDInterface->mCursorX = ViewportToGameX(test_event.motion.x, test_event.motion.y);
                    mDDInterface->mCursorY = ViewportToGameY(test_event.motion.x, test_event.motion.y);
                    mWidgetManager->RemapMouse(mDDInterface->mCursorX, mDDInterface->mCursorY);

                    mLastUserInputTick = mLastTimerTime;

                    mWidgetManager->MouseMove(mDDInterface->mCursorX,mDDInterface->mCursorY);

                    if (!mMouseIn)
                    {
                        mMouseIn = true;

                        EnforceCursor();
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
            {
                int     x = ViewportToGameX(test_event.button.x, test_event.button.y);
                int     y = ViewportToGameY(test_event.button.x, test_event.button.y);
                if (test_event.button.button == SDL_BUTTON_LEFT && test_event.button.state == SDL_RELEASED)
                    mWidgetManager->MouseUp(x, y, 1);
                else if (test_event.button.button == SDL_BUTTON_RIGHT && test_event.button.state == SDL_RELEASED)
                    mWidgetManager->MouseUp(x, y, -1);
                else if (test_event.button.button == SDL_BUTTON_MIDDLE && test_event.button.state == SDL_RELEASED)
                    mWidgetManager->MouseUp(x, y, 3);
                else if (test_event.button.button == SDL_BUTTON_WHEELUP && test_event.button.state == SDL_RELEASED)
                    mWidgetManager->MouseWheel(1);
                else if (test_event.button.button == SDL_BUTTON_WHEELDOWN && test_event.button.state == SDL_RELEASED)
                    mWidgetManager->MouseWheel(-1);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                int     x = ViewportToGameX(test_event.button.x, test_event.button.y);
                int     y = ViewportToGameY(test_event.button.x, test_event.button.y);
                Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::UpdateAppStep: button event: x=%d, y=%d", test_event.button.x, test_event.button.y));
                Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::UpdateAppStep: x=%d, y=%d", x, y));
                if (test_event.button.button == SDL_BUTTON_LEFT && test_event.button.state == SDL_PRESSED)
                    mWidgetManager->MouseDown(x, y, 1);
                else if (test_event.button.button == SDL_BUTTON_RIGHT && test_event.button.state == SDL_PRESSED)
                    mWidgetManager->MouseDown(x, y, -1);
                else if (test_event.button.button == SDL_BUTTON_MIDDLE && test_event.button.state == SDL_PRESSED)
                    mWidgetManager->MouseDown(x, y, 3);
                break;
            }

            case SDL_KEYDOWN:
                mLastUserInputTick = mLastTimerTime;
                if (test_event.key.type == SDL_KEYDOWN) {

                    SDLKey k = test_event.key.keysym.sym;
                    mWidgetManager->KeyDown(GetKeyCodeFromSDLKey(k));
                    if (k >= SDLK_a && k <= SDLK_z)
                        mWidgetManager->KeyChar((SexyChar)*SDL_GetKeyName(k));
                }

                break;

            case SDL_KEYUP:
                mLastUserInputTick = mLastTimerTime;
                if (test_event.key.type == SDL_KEYUP) {

                    SDLKey k = test_event.key.keysym.sym;
                    mWidgetManager->KeyUp(GetKeyCodeFromSDLKey(k));
                }
                break;

            case SDL_ACTIVEEVENT:

                if (test_event.active.gain == 1) {

                    mHasFocus = true;
                    GotFocus();

                    if (mMuteOnLostFocus)
                        Unmute(true);

                    mWidgetManager->MouseMove(mDDInterface->mCursorX, mDDInterface->mCursorY);

                }
                else if (mHasFocus){

                    mHasFocus = false;
                    LostFocus();

                    mWidgetManager->MouseExit(mDDInterface->mCursorX, mDDInterface->mCursorY);

                    if (mMuteOnLostFocus)
                        Mute(true);
                }
                break;

            case SDL_QUIT:
                Shutdown();
                break;

            }

            if (SDL_PeepEvents(&test_event, 1, SDL_PEEKEVENT, SDL_ALLEVENTS) == 0)
                mUpdateAppState = UPDATESTATE_PROCESS_1;

        }
        else
            mUpdateAppState = UPDATESTATE_PROCESS_1;

    }
    else
    {
        // Process changes state by itself
        if (mStepMode)
        {
            if (mStepMode==2)
            {
                struct timespec timeOut,remains;

                timeOut.tv_sec = 0;
                timeOut.tv_nsec = mFrameTime * 1000000;

                nanosleep(&timeOut, &remains);

                mUpdateAppState = UPDATESTATE_PROCESS_DONE; // skip actual update until next step
            }
            else
            {
                mStepMode = 2;
                DoUpdateFrames();
                DoUpdateFramesF(1.0f);
                DrawDirtyStuff();
            }
        }
        else
        {
            int anOldUpdateCnt = mUpdateCount;
            Process();
            if (updated != NULL)
                *updated = mUpdateCount != anOldUpdateCnt;
        }
    }

    mUpdateAppDepth--;
    return true;
}


void SexyAppBase::DoUpdateFramesF(float theFrac)
{
    if ((mVSyncUpdates) && (!mMinimized))
        mWidgetManager->UpdateFrameF(theFrac);
}

bool SexyAppBase::DoUpdateFrames()
{
    if (mLoadingThreadCompleted && !mLoaded)
    {
        mLoaded = true;
        mYieldMainThread = false;
        LoadingThreadCompleted();
    }

    UpdateFrames();
    return true;
}

void SexyAppBase::UpdateFrames()
{
    mUpdateCount++;
    if (!mMinimized)
    {
        if (mWidgetManager->UpdateFrame())
            ++mFPSDirtyCount;
    }

    if (mMusicInterface)
        mMusicInterface->Update();
    CleanSharedImages();
}

bool gIsFailing = false;

bool SexyAppBase::DrawDirtyStuff()
{
    MTAutoDisallowRand aDisallowRand;
    if (gIsFailing) // just try to reinit
    {
        Redraw(NULL);
        mHasPendingDraw = false;
        mLastDrawWasEmpty = true;
        return false;
    }


    if (mShowFPS)
    {
        switch (mShowFPSMode)
        {
        case FPS_ShowFPS : CalculateFPS(); break;
        case FPS_ShowCoords:
            if (mWidgetManager!=NULL)
                FPSDrawCoords(mWidgetManager->GetLastMouseX(), mWidgetManager->GetLastMouseY());
            break;
        }
    }

    Uint32 aStartTime = SDL_GetTicks();

    // Update user input and screen saver info
    static Uint32 aPeriodicTick = 0;
    if (aStartTime-aPeriodicTick > 1000)
    {
        aPeriodicTick = aStartTime;
        //UpdateScreenSaverInfo(aStartTime);
    }
    mIsDrawing = true;

    bool drewScreen = mWidgetManager->DrawScreen();
    mIsDrawing = false;

        //custom mouse pointers need page flipping
    if ((drewScreen || mCustomCursorsEnabled || (aStartTime - mLastDrawTick >= 1000) || (mCustomCursorDirty)) &&
        ((int) (aStartTime - mNextDrawTick) >= 0))
    {
        mLastDrawWasEmpty = false;

        mDrawCount++;

        Uint32 aMidTime = SDL_GetTicks();

        mFPSCount++;
        mFPSTime += aMidTime - aStartTime;

        mDrawTime += aMidTime - aStartTime;

#if 0
        if (mShowFPS)
        {
            Graphics g(mDDInterface->GetScreenImage());
            g.DrawImage(gFPSImage, mWidth-gFPSImage->GetWidth()-10, mHeight-gFPSImage->GetHeight()-10);
        }
#endif

        if (mWaitForVSync && mIsPhysWindowed && mSoftVSyncWait) {
            Uint32 aTick = SDL_GetTicks();
            if (aTick - mLastDrawTick < mDDInterface->mMillisecondsPerFrame) {
                struct timespec timeOut, remains;

                timeOut.tv_sec = 0;
                timeOut.tv_nsec = (mDDInterface->mMillisecondsPerFrame - (aTick - mLastDrawTick)) * 1000000;

                nanosleep(&timeOut, &remains);
            }
        }

        Uint32 aPreScreenBltTime = SDL_GetTicks();
        mLastDrawTick = aPreScreenBltTime;

        Redraw(NULL);

        // This is our one UpdateFTimeAcc if we are vsynched
        UpdateFTimeAcc();

        Uint32 aEndTime = SDL_GetTicks();

        mScreenBltTime = aEndTime - aPreScreenBltTime;

        if ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
        {
            int aTotalTime = aEndTime - aStartTime;

            mNextDrawTick += 35 + std::max(aTotalTime, 15);

            if ((int) (aEndTime - mNextDrawTick) >= 0)
                mNextDrawTick = aEndTime;
        }
        else
            mNextDrawTick = aEndTime;

        mHasPendingDraw = false;
        mCustomCursorDirty = false;
        return true;
    }
    else
    {
        mHasPendingDraw = false;
        mLastDrawWasEmpty = true;
        return false;
    }



    return false;
}

void SexyAppBase::LoadingThreadCompleted()
{
}

void SexyAppBase::UpdateFTimeAcc()
{
    Uint32 aCurTime = SDL_GetTicks();

    if (mLastTimeCheck != 0)
    {
        // Number of milliseconds since last time
        Uint32 aDeltaTime = aCurTime - mLastTimeCheck;

        // Accumulate ms since last time. Minimum 200. Type float!
        mUpdateFTimeAcc = std::min(mUpdateFTimeAcc + aDeltaTime, 200.0);
    }

    mLastTimeCheck = aCurTime;
}

bool SexyAppBase::Process(bool allowSleep)
{
    if (mLoadingFailed)
        Shutdown();

    bool isVSynched =
          (mVSyncUpdates) && (!mLastDrawWasEmpty) && (!mVSyncBroken) &&
        ((!mIsPhysWindowed) || (mIsPhysWindowed && mWaitForVSync && !mSoftVSyncWait));
//FIXME why use doubles??
    double aFrameFTime;
    double anUpdatesPerUpdateF;

    if (mVSyncUpdates)
    {
        aFrameFTime = (1000.0 / mSyncRefreshRate) / mUpdateMultiplier;
        anUpdatesPerUpdateF = (float) (1000.0 / (mFrameTime * mSyncRefreshRate));
    }
    else
    {
        aFrameFTime = mFrameTime / mUpdateMultiplier;
        anUpdatesPerUpdateF = 1.0;
    }
    // Make sure we're not paused
    if ((!mPaused) && (mUpdateMultiplier > 0))
    {
        Uint32 aStartTime = SDL_GetTicks();

        int aCumSleepTime = 0;

        // When we are VSynching, only calculate this FTimeAcc right after drawing

        if (!isVSynched)
            UpdateFTimeAcc();

        // mNonDrawCount is used to make sure we draw the screen at least
        // 10 times per second, even if it means we have to slow down
        // the updates to make it draw 10 times per second in "game time"

        bool didUpdate = false;

        if (mUpdateAppState == UPDATESTATE_PROCESS_1)
        {
            if ((++mNonDrawCount < (int) ceil(10*mUpdateMultiplier)) || (!mLoaded))
            {
                bool doUpdate = false;

                if (isVSynched)
                {
                    // Synch'ed to vertical refresh, so update as soon as possible after draw
                    doUpdate = (!mHasPendingDraw) || (mUpdateFTimeAcc >= (int) (aFrameFTime * 0.75));
                }
                else if (mUpdateFTimeAcc >= aFrameFTime)
                {
                    doUpdate = true;
                }

                if (doUpdate)
                {
                    // Do VSyncBroken test.  This test fails if we're in fullscreen and
                    // "don't vsync" has been forced in Advanced settings up Display Properties
                    if (
                            (mUpdateMultiplier == 1.0))
                    {
                        mVSyncBrokenTestUpdates++;
                        if (mVSyncBrokenTestUpdates >= (Uint32) ((1000+mFrameTime-1)/mFrameTime))
                        {
                            // It has to be running 33% fast to be "broken" (25% = 1/0.800)
                            if (aStartTime - mVSyncBrokenTestStartTick <= 800)
                            {
                                // The test has to fail 3 times in a row before we decide that
                                //  vsync is broken overall
                                mVSyncBrokenCount++;
                                if (mVSyncBrokenCount >= 3)
                                    mVSyncBroken = true;
                            }
                            else
                                mVSyncBrokenCount = 0;

                            mVSyncBrokenTestStartTick = aStartTime;
                            mVSyncBrokenTestUpdates = 0;
                        }
                    }

                    bool hadRealUpdate = DoUpdateFrames();
                    if (hadRealUpdate)
                        mUpdateAppState = UPDATESTATE_PROCESS_2;

                    mHasPendingDraw = true;
                    didUpdate = true;
                }
            }
        }
        else if (mUpdateAppState == UPDATESTATE_PROCESS_2)
        {
            mUpdateAppState = UPDATESTATE_PROCESS_DONE;

            mPendingUpdatesAcc += anUpdatesPerUpdateF;
            mPendingUpdatesAcc -= 1.0;
            ProcessSafeDeleteList();

            // Process any extra updates
            while (mPendingUpdatesAcc >= 1.0)
            {
                // These should just be IDLE commands we have to clear out

                ++mNonDrawCount;
                bool hasRealUpdate = DoUpdateFrames();
                assert(hasRealUpdate);

                if (!hasRealUpdate)
                    break;

                ProcessSafeDeleteList();
                mPendingUpdatesAcc -= 1.0;
            }

            //aNumCalls++;
            DoUpdateFramesF((float) anUpdatesPerUpdateF);
            ProcessSafeDeleteList();

            if (isVSynched) {
                // Don't let mUpdateFTimeAcc dip below 0
                //  Subtract an extra 0.2ms, because sometimes refresh rates have some
                //  fractional component that gets truncated, and it's better to take off
                //  too much to keep our timing tending toward occuring right after
                //  redraws
                mUpdateFTimeAcc = std::max(mUpdateFTimeAcc - aFrameFTime - 0.2f, 0.0);
            }
            else
                mUpdateFTimeAcc -= aFrameFTime;

            didUpdate = true;
        }

        if (!didUpdate)
        {
            mUpdateAppState = UPDATESTATE_PROCESS_DONE;

            mNonDrawCount = 0;

            if (mHasPendingDraw)
            {
                DrawDirtyStuff();
            }
            else
            {
                // Let us take into account the time it took to draw dirty stuff
                int aTimeToNextFrame = (int) (aFrameFTime - mUpdateFTimeAcc);
                if (aTimeToNextFrame > 0)
                {
                    if (!allowSleep)
                        return false;

                    // Wait till next processing cycle
                    ++mSleepCount;
                    struct timespec timeOut,remains;

                    timeOut.tv_sec = 0;
                    timeOut.tv_nsec = aTimeToNextFrame * 1000000;

                    nanosleep(&timeOut, &remains);

                    aCumSleepTime += aTimeToNextFrame;
                }
            }
        }

        if (mYieldMainThread)
        {
            // This is to make sure that the title screen doesn't take up any more than
            // 1/3 of the processor time
        }
    }


    ProcessSafeDeleteList();
    return true;
}

void SexyAppBase::ProcessSafeDeleteList()
{
    MTAutoDisallowRand aDisallowRand;

    WidgetSafeDeleteList::iterator anItr = mSafeDeleteList.begin();
    while (anItr != mSafeDeleteList.end())
    {
        WidgetSafeDeleteInfo* aWidgetSafeDeleteInfo = &(*anItr);
        if (mUpdateAppDepth <= aWidgetSafeDeleteInfo->mUpdateAppDepth)
        {
            delete aWidgetSafeDeleteInfo->mWidget;
            anItr = mSafeDeleteList.erase(anItr);
        }
        else
            ++anItr;
    }
}

void SexyAppBase::Redraw(Rect* theClipRect)
{

    // Do mIsDrawing check because we could enter here at a bad time if any windows messages
    //  are processed during WidgetManager->Draw
    if ((mIsDrawing) || (mShutdown))
        return;

    if (gScreenSaverActive)
        return;

    static Uint32 aRetryTick = 0;       // FIXME. Not used

    if (!mDDInterface) {
        // Bad things happened.
        return;
    }
    if (!mDDInterface->Redraw(theClipRect))
    {
    }

    else
    {
        if (gIsFailing)
        {
            gIsFailing = false;
            aRetryTick = 0;
        }
    }

    mFPSFlipCount++;
}

void SexyAppBase::ShutdownHook()
{
}


void SexyAppBase::SetCursor(int theCursorNum)
{
    mCursorNum = theCursorNum;
    EnforceCursor();
}

void SexyAppBase::SafeDeleteWidget(Widget* theWidget)
{
    if (theWidget != NULL) {
        WidgetSafeDeleteInfo aWidgetSafeDeleteInfo;
        aWidgetSafeDeleteInfo.mUpdateAppDepth = mUpdateAppDepth;
        aWidgetSafeDeleteInfo.mWidget = theWidget;
        mSafeDeleteList.push_back(aWidgetSafeDeleteInfo);
    }
}

void SexyAppBase::AddMemoryImage(MemoryImage* theMemoryImage)
{
#if 0
    AutoCrit anAutoCrit(mDDInterface->mCritSect);
#endif
    mMemoryImageSet.insert(theMemoryImage);
}

void SexyAppBase::RemoveMemoryImage(MemoryImage* theMemoryImage)
{
#if 0
    AutoCrit anAutoCrit(mDDInterface->mCritSect);
#endif
    MemoryImageSet::iterator anItr = mMemoryImageSet.find(theMemoryImage);
    if (anItr != mMemoryImageSet.end())
        mMemoryImageSet.erase(anItr);

    Remove3DData(theMemoryImage);
}

void SexyAppBase::WaitForLoadingThread()
{
    while ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
    {
        SDL_Delay(20);      // sleep/wait 20 milliseconds
    }
}

SharedImageRef SexyAppBase::GetSharedImage(const std::string& theFileName, bool* isNew, bool lookForAlpha)
{
    std::string anUpperFileName = StringToUpper(theFileName);

    std::pair<SharedImageMap::iterator, bool> aResultPair;
    SharedImageRef aSharedImageRef;

    {
#if 0
        AutoCrit anAutoCrit(mDDInterface->mCritSect);
#endif
        aResultPair = mSharedImageMap.insert(SharedImageMap::value_type(anUpperFileName, SharedImage()));
        aSharedImageRef = &aResultPair.first->second;
    }

    if (isNew != NULL)
        *isNew = aResultPair.second;

    if (aResultPair.second)
    {
        // Pass in a '!' as the first char of the file name to create a new image
        if ((theFileName.length() > 0) && (theFileName[0] == '!'))
            aSharedImageRef.mSharedImage->mImage = new DDImage(mDDInterface);
        else
            aSharedImageRef.mSharedImage->mImage = GetImage(theFileName, /*commitBits*/false, lookForAlpha);
    }

    return aSharedImageRef;
}

double SexyAppBase::GetLoadingThreadProgress()
{
    if (mLoaded)
        return 1.0;
    if (!mLoadingThreadStarted)
        return 0.0;
    if (mNumLoadingThreadTasks == 0)
        return 0.0;

    return std::min((double)mCompletedLoadingThreadTasks / mNumLoadingThreadTasks, 1.0);

}

void SexyAppBase::LoadingThreadProc()
{
    Logger::log(mLogFacil, 1, "LoadingThreadProc");
}

int SexyAppBase::LoadingThreadProcStub(void *theArg)
{
    SexyAppBase* aSexyApp = (SexyAppBase*) theArg;

    aSexyApp->LoadingThreadProc();
    aSexyApp->mLoadingThreadCompleted = true;       // See DoUpdateFrames which will call LoadingThreadCompleted()

    return 0;
}

void SexyAppBase::StartLoadingThread()
{
    if (!mLoadingThreadStarted)
    {
        Logger::log(mLogFacil, 1, "StartLoadingThread");
        mYieldMainThread = true;
        mLoadingThreadStarted = true;
        SDL_CreateThread(&LoadingThreadProcStub, this);
    }
}

SoundInstance* SexyAppBase::PlaySample(int theSoundNum, bool original, double volume, bool loop)
{
    if (!mSoundManager)
        return NULL;

    SoundInstance* aSoundInstance ;

    if (original)
        aSoundInstance = mSoundManager->GetOriginalSoundInstance(theSoundNum);
    else
        aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);

    if (aSoundInstance != NULL)
    {
        aSoundInstance->SetVolume(volume);
        aSoundInstance->Play(loop, true);
    }

    return aSoundInstance;
}

SoundInstance* SexyAppBase::PlaySample(int theSoundNum, int thePan, bool original, double volume, bool loop, float pitch)
{
    if (!mSoundManager)
        return NULL;

    SoundInstance* aSoundInstance ;

    if (original)
        aSoundInstance = mSoundManager->GetOriginalSoundInstance(theSoundNum);
    else
        aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);

    if (aSoundInstance != NULL)
    {
        aSoundInstance->SetVolume(volume);
        aSoundInstance->SetPan(thePan);
        if (pitch > 0.0f)
            aSoundInstance->AdjustPitch(pitch);
        aSoundInstance->Play(loop, true);
    }

    return aSoundInstance;
}

SoundInstance* SexyAppBase::StopSample(int theSoundNum)
{
    if (!mSoundManager)
        return NULL;

    mSoundManager->StopSound(theSoundNum);

#if 0
    SoundInstance* aSoundInstance = mSoundManager->GetOriginalSoundInstance(theSoundNum);

    if (aSoundInstance != NULL)
    {
        SoundInstance()->Stop();
    }

    return aSoundInstance;
#endif
    return NULL;
}

bool SexyAppBase::IsSamplePlaying(int theSoundNum)
{
    if (!mSoundManager)
        return false;

    return mSoundManager->IsSoundPlaying(theSoundNum);

#if 0
    SoundInstance* aSoundInstance = mSoundManager->GetOriginalSoundInstance(theSoundNum);
    if (aSoundInstance != NULL)
    {
        return aSoundInstance->IsPlaying();
    }

    return false;
#endif
}

bool SexyAppBase::IsMuted()
{
    return mMuteCount > 0;
}

void SexyAppBase::Mute(bool autoMute)
{
    mMuteCount++;
    if (autoMute)
        mAutoMuteCount++;

    SetMusicVolume(mMusicVolume);
    SetSfxVolume(mSfxVolume);
}

void SexyAppBase::Unmute(bool autoMute)
{
    if (mMuteCount > 0)
    {
        mMuteCount--;
        if (autoMute)
            mAutoMuteCount--;
    }

    SetMusicVolume(mMusicVolume);
    SetSfxVolume(mSfxVolume);
}


double SexyAppBase::GetMusicVolume()
{
    return mMusicVolume;
}

void SexyAppBase::SetMusicVolume(double theVolume)
{
    mMusicVolume = theVolume;

    if (mMusicInterface != NULL)
        mMusicInterface->SetVolume((mMuteCount > 0) ? 0.0 : mMusicVolume);
}

double SexyAppBase::GetSfxVolume()
{
    return mSfxVolume;
}

void SexyAppBase::SetSfxVolume(double theVolume)
{
    mSfxVolume = theVolume;

    if (mSoundManager != NULL)
        mSoundManager->SetVolume((mMuteCount > 0) ? 0.0 : mSfxVolume);
}

double SexyAppBase::GetMasterVolume()
{
    return mSoundManager->GetMasterVolume();
}

void SexyAppBase::SetMasterVolume(double theMasterVolume)
{
    mSfxVolume = theMasterVolume;
    mSoundManager->SetMasterVolume(mSfxVolume);
}

SoundManager* SexyAppBase::CreateSoundManager()
{
    // In the derived class you can select another sound manager if you wish, such as Bass Sound Manager

#if defined(USE_BASS)
    return new BassSoundManager;
#elif defined(USE_AUDIERE)
    return new AudiereSoundManager;
#elif defined(USE_SDLMIXER)
    return new SDLMixerSoundManager;
#endif
    return NULL;
}

MusicInterface* SexyAppBase::CreateMusicInterface()
{
    if (mNoSoundNeeded)
        // Huh? Maybe it is because want at least _some_ MusicInterface.
        return new MusicInterface;

    // In the derived class you can select another sound manager if you wish, such as Bass Sound Manager
#if defined(USE_BASS)
    return new BassMusicInterface(NULL);
#elif defined(USE_AUDIERE)
    return new AudiereMusicInterface(mInvisHWnd);
#elif defined(USE_SDLMIXER)
    return new SDLMixerMusicInterface(NULL);
#endif
    return NULL;
}

DDImage* SexyAppBase::CopyImage(Image* theImage, const Rect& theRect)
{
    DDImage* anImage = new DDImage(mDDInterface);
    anImage->Create(theRect.mWidth, theRect.mHeight);

    Graphics g(anImage);
    g.DrawImage(theImage, -theRect.mX, -theRect.mY);

    anImage->CopyAttributes(theImage);

    return anImage;
}

DDImage* SexyAppBase::CopyImage(Image* theImage)
{
    return CopyImage(theImage, Rect(0, 0, theImage->GetWidth(), theImage->GetHeight()));
}

Sexy::DDImage* SexyAppBase::GetImage(const std::string& theFileName, bool commitBits, bool lookForAlpha)
{
    ImageLib::Image* aLoadedImage;

    std::string myFileName = GetAppResourceFileName(theFileName);
#ifdef DEBUG
    static Timer * timer = new Timer();
    timer->start();
    double start_time = timer->getElapsedTimeInSec();
    Logger::tlog(mLogFacil, 1, Logger::format("SexyAppBase::GetImage: '%s'", myFileName.c_str()));
#endif
    aLoadedImage = ImageLib::GetImage(myFileName, lookForAlpha);
#ifdef DEBUG
    timer->stop();
    Logger::tlog(mLogFacil, 1, Logger::format("SexyAppBase::GetImage: - done in %8.3f", timer->getElapsedTimeInSec() - start_time));
#endif

    if (aLoadedImage == NULL)
        return NULL;

    DDImage* anImage = new DDImage(mDDInterface);
    anImage->SetBits(aLoadedImage->GetBits(), aLoadedImage->GetWidth(), aLoadedImage->GetHeight(), commitBits);
    anImage->mFilePath = theFileName;
    delete aLoadedImage;

    return anImage;
}

Sexy::DDImage* SexyAppBase::CreateCrossfadeImage(Sexy::Image* theImage1, const Rect& theRect1, Sexy::Image* theImage2, const Rect& theRect2, double theFadeFactor)
{
    MemoryImage* aMemoryImage1 = dynamic_cast<MemoryImage*>(theImage1);
    MemoryImage* aMemoryImage2 = dynamic_cast<MemoryImage*>(theImage2);

    if ((aMemoryImage1 == NULL) || (aMemoryImage2 == NULL))
        return NULL;

    if ((theRect1.mX < 0) || (theRect1.mY < 0) ||
        (theRect1.mX + theRect1.mWidth > theImage1->GetWidth()) ||
        (theRect1.mY + theRect1.mHeight > theImage1->GetHeight()))
    {
        // TODO. Raise exception.
        assert("Crossfade Rect1 out of bounds" == NULL);
        return NULL;
    }

    if ((theRect2.mX < 0) || (theRect2.mY < 0) ||
        (theRect2.mX + theRect2.mWidth > theImage2->GetWidth()) ||
        (theRect2.mY + theRect2.mHeight > theImage2->GetHeight()))
    {
        // TODO. Raise exception.
        assert("Crossfade Rect2 out of bounds" == NULL);
        return NULL;
    }

    int aWidth = theRect1.mWidth;
    int aHeight = theRect1.mHeight;

    DDImage* anImage = new DDImage(mDDInterface);
    anImage->Create(aWidth, aHeight);

    uint32_t* aDestBits = anImage->GetBits();
    uint32_t* aSrcBits1 = aMemoryImage1->GetBits();
    uint32_t* aSrcBits2 = aMemoryImage2->GetBits();

    int aSrc1Width = aMemoryImage1->GetWidth();
    int aSrc2Width = aMemoryImage2->GetWidth();
    uint32_t aMult = (int) (theFadeFactor*256);
    uint32_t aOMM = (256 - aMult);

    for (int y = 0; y < aHeight; y++)
    {
        uint32_t* s1 = &aSrcBits1[(y+theRect1.mY)*aSrc1Width+theRect1.mX];
        uint32_t* s2 = &aSrcBits2[(y+theRect2.mY)*aSrc2Width+theRect2.mX];
        uint32_t* d = &aDestBits[y*aWidth];

        for (int x = 0; x < aWidth; x++)
        {
            uint32_t p1 = *s1++;
            uint32_t p2 = *s2++;

            //p1 = 0;
            //p2 = 0xFFFFFFFF;

            *d++ =
                ((((p1 & 0x000000FF)*aOMM + (p2 & 0x000000FF)*aMult)>>8) & 0x000000FF) |
                ((((p1 & 0x0000FF00)*aOMM + (p2 & 0x0000FF00)*aMult)>>8) & 0x0000FF00) |
                ((((p1 & 0x00FF0000)*aOMM + (p2 & 0x00FF0000)*aMult)>>8) & 0x00FF0000) |
                ((((p1 >> 24)*aOMM + (p2 >> 24)*aMult)<<16) & 0xFF000000);
        }
    }

    anImage->BitsChanged();

    return anImage;
}

// TODO. Document me.
void SexyAppBase::ColorizeImage(Image* theImage, const Color& theColor)
{
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

    if (aSrcMemoryImage == NULL)
        return;

    uint32_t* aBits;
    int aNumColors;         // number of pixels

    if (aSrcMemoryImage->mColorTable == NULL)
    {
        aBits = aSrcMemoryImage->GetBits();
        aNumColors = theImage->GetWidth() * theImage->GetHeight();
    }
    else
    {
        aBits = aSrcMemoryImage->mColorTable;
        aNumColors = 256;
    }

    // Notice that the 32 bits color values are assumed to be: ARGB.
    // In a byte buffer on a little endian system that would be BGRA.

    if ((theColor.mAlpha <= 255) &&
        (theColor.mRed   <= 255) &&
        (theColor.mGreen <= 255) &&
        (theColor.mBlue  <= 255))
    {
        for (int i = 0; i < aNumColors; i++)
        {
            uint32_t aColor = aBits[i];

            // Notice that the alpha is first shifted right to get some room in the 32 bits to multiply
            // the RGB parts are first multiplied and then shifted
            aBits[i] =
                ((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
                ((((aColor & 0x00FF0000) * theColor.mRed)   >> 8) & 0x00FF0000) |
                ((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00) |
                ((((aColor & 0x000000FF) * theColor.mBlue)  >> 8) & 0x000000FF);
        }
    }
    else
    {
        // White, opaque
        for (int i = 0; i < aNumColors; i++)
        {
            uint32_t aColor = aBits[i];

            // Notice that for alpha we shift 24 and then we don't need a 0xFF mask because of type uint32_t
            // Notice that the / 255 is not the same as >> 8
            int aAlpha = ((aColor  >> 24) * theColor.mAlpha) / 255;
            int aRed   = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
            int aGreen = (((aColor >> 8)  & 0xFF) * theColor.mGreen) / 255;
            int aBlue  = ((aColor & 0xFF) * theColor.mBlue) / 255;

            // saturated values at 255. (clipping)
            if (aAlpha > 255)
                aAlpha = 255;
            if (aRed > 255)
                aRed = 255;
            if (aGreen > 255)
                aGreen = 255;
            if (aBlue > 255)
                aBlue = 255;

            aBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
        }
    }

    aSrcMemoryImage->BitsChanged();
}

DDImage* SexyAppBase::CreateColorizedImage(Image* theImage, const Color& theColor)
{
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

    if (aSrcMemoryImage == NULL)
        return NULL;

    DDImage* anImage = new DDImage(mDDInterface);

    anImage->Create(theImage->GetWidth(), theImage->GetHeight());

    uint32_t* aSrcBits;
    uint32_t* aDestBits;
    int aNumColors;

    if (aSrcMemoryImage->mColorTable == NULL)
    {
        aSrcBits = aSrcMemoryImage->GetBits();
        aDestBits = anImage->GetBits();
        aNumColors = theImage->GetWidth()*theImage->GetHeight();
    }
    else
    {
        aSrcBits = aSrcMemoryImage->mColorTable;
        aDestBits = anImage->mColorTable = new uint32_t[256];
        aNumColors = 256;

        int nr_pixels = anImage->GetWidth() * theImage->GetHeight();
        anImage->mColorIndices = new uchar[nr_pixels];
        memcpy(anImage->mColorIndices, aSrcMemoryImage->mColorIndices, nr_pixels);
    }

    if ((theColor.mAlpha <= 255) &&
            (theColor.mRed <= 255) &&
            (theColor.mGreen <= 255) &&
            (theColor.mBlue <= 255))
    {
        for (int i = 0; i < aNumColors; i++)
        {
            uint32_t aColor = aSrcBits[i];

            aDestBits[i] =
                ((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
                ((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
                ((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
                ((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
        }
    }
    else
    {
        for (int i = 0; i < aNumColors; i++)
        {
            uint32_t aColor = aSrcBits[i];

            int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
            int aRed   = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
            int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
            int aBlue  = ((aColor & 0xFF) * theColor.mBlue) / 255;

            if (aAlpha > 255)
                aAlpha = 255;
            if (aRed > 255)
                aRed = 255;
            if (aGreen > 255)
                aGreen = 255;
            if (aBlue > 255)
                aBlue = 255;

            aDestBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
        }
    }

    anImage->BitsChanged();

    return anImage;
}


void SexyAppBase::MirrorImage(Image* theImage)
{
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

    uint32_t* aSrcBits = aSrcMemoryImage->GetBits();

    int aPhysSrcWidth = aSrcMemoryImage->GetWidth();
    for (int y = 0; y < aSrcMemoryImage->GetHeight(); y++)
    {
        uint32_t* aLeftBits = aSrcBits + (y * aPhysSrcWidth);
        uint32_t* aRightBits = aLeftBits + (aPhysSrcWidth - 1);

        for (int x = 0; x < (aPhysSrcWidth >> 1); x++)
        {
            uint32_t aSwap = *aLeftBits;

            *(aLeftBits++) = *aRightBits;
            *(aRightBits--) = aSwap;
        }
    }

    aSrcMemoryImage->BitsChanged();
}

void SexyAppBase::FlipImage(Image* theImage)
{
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

    uint32_t* aSrcBits = aSrcMemoryImage->GetBits();

    int aPhysSrcHeight = aSrcMemoryImage->GetHeight();
    int aPhysSrcWidth = aSrcMemoryImage->GetWidth();
    for (int x = 0; x < aPhysSrcWidth; x++)
    {
        uint32_t* aTopBits    = aSrcBits + x;
        uint32_t* aBottomBits = aTopBits + (aPhysSrcWidth * (aPhysSrcHeight - 1));

        for (int y = 0; y < (aPhysSrcHeight >> 1); y++)
        {
            uint32_t aSwap = *aTopBits;

            *aTopBits = *aBottomBits;
            aTopBits += aPhysSrcWidth;
            *aBottomBits = aSwap;
            aBottomBits -= aPhysSrcWidth;
        }
    }

    aSrcMemoryImage->BitsChanged();
}

void SexyAppBase::RotateImageHue(Sexy::MemoryImage *theImage, int theDelta)
{
    while (theDelta < 0)
        theDelta += 256;

    int aSize = theImage->GetWidth() * theImage->GetHeight();
    uint32_t *aPtr = theImage->GetBits();
    for (int i=0; i<aSize; i++)
    {
        uint32_t aPixel = *aPtr;
        int alpha = aPixel&0xff000000;
        int r = (aPixel>>16)&0xff;
        int g = (aPixel>>8) &0xff;
        int b = aPixel&0xff;

        int maxval = std::max(r, std::max(g, b));
        int minval = std::min(r, std::min(g, b));
        int h = 0;
        int s = 0;
        int l = (minval+maxval)/2;
        int delta = maxval - minval;

        if (delta != 0)
        {
            s = (delta * 256) / ((l <= 128) ? (minval + maxval) : (512 - maxval - minval));

            if (r == maxval)
                h = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
            else if (g == maxval)
                h = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
            else
                h = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));

            h /= 6;
        }

        h += theDelta;
        if (h >= 256)
            h -= 256;

        double v= (l < 128) ? (l * (255+s))/255 :
                (l+s-l*s/255);

        int y = (int) (2*l-v);

        int aColorDiv = (6 * h) / 256;
        int x = (int)(y+(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
        if (x > 255)
            x = 255;

        int z = (int) (v-(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
        if (z < 0)
            z = 0;

        switch (aColorDiv)
        {
            case 0: r = (int) v; g = x; b = y; break;
            case 1: r = z; g= (int) v; b = y; break;
            case 2: r = y; g= (int) v; b = x; break;
            case 3: r = y; g = z; b = (int) v; break;
            case 4: r = x; g = y; b = (int) v; break;
            case 5: r = (int) v; g = y; b = z; break;
            default: r = (int) v; g = x; b = y; break;
        }

        *aPtr++ = alpha | (r<<16) | (g << 8) | (b);

    }

    theImage->BitsChanged();
}

uint32_t SexyAppBase::HSLToRGB(int h, int s, int l)
{
    int r;
    int g;
    int b;

    double v= (l < 128) ? (l * (255+s))/255 :
            (l+s-l*s/255);

    int y = (int) (2*l-v);

    int aColorDiv = (6 * h) / 256;
    int x = (int)(y+(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
    if (x > 255)
        x = 255;

    int z = (int) (v-(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
    if (z < 0)
        z = 0;

    switch (aColorDiv)
    {
        case 0:  r = (int) v; g = x;       b = y; break;
        case 1:  r = z;       g = (int) v; b = y; break;
        case 2:  r = y;       g = (int) v; b = x; break;
        case 3:  r = y;       g = z;       b = (int) v; break;
        case 4:  r = x;       g = y;       b = (int) v; break;
        case 5:  r = (int) v; g = y;       b = z; break;
        default: r = (int) v; g = x;       b = y; break;
    }

    return 0xFF000000 | (r << 16) | (g << 8) | (b);
}

uint32_t SexyAppBase::RGBToHSL(int r, int g, int b)
{
    int maxval = std::max(r, std::max(g, b));
    int minval = std::min(r, std::min(g, b));
    int hue = 0;
    int saturation = 0;
    int luminosity = (minval + maxval) / 2;
    int delta = maxval - minval;

    if (delta != 0) {
        saturation = (delta * 256) / ((luminosity <= 128) ? (minval + maxval) : (512 - maxval - minval));

        if (r == maxval)
            hue = (g == minval ? 1280 + (((maxval - b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
        else if (g == maxval)
            hue = (b == minval ?  256 + (((maxval - r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
        else
            hue = (r == minval ?  768 + (((maxval - g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));

        hue /= 6;
    }

    return 0xFF000000 | (hue) | (saturation << 8) | (luminosity << 16);
}

void SexyAppBase::HSLToRGB(const uint32_t* theSource, uint32_t* theDest, int theSize)
{
    for (int i = 0; i < theSize; i++)
    {
        uint32_t src = theSource[i];
        theDest[i] = (src & 0xFF000000) | (HSLToRGB((src & 0xFF), (src >> 8) & 0xFF, (src >> 16) & 0xFF) & 0x00FFFFFF);
    }
}

void SexyAppBase::RGBToHSL(const uint32_t* theSource, uint32_t* theDest, int theSize)
{
    for (int i = 0; i < theSize; i++)
    {
        uint32_t src = theSource[i];
        theDest[i] = (src & 0xFF000000) | (RGBToHSL(((src >> 16) & 0xFF), (src >> 8) & 0xFF, (src & 0xFF)) & 0x00FFFFFF);
    }
}

void SexyAppBase::PrecacheAdditive(MemoryImage* theImage)
{
    theImage->GetRLAdditiveData(mDDInterface);
}

void SexyAppBase::PrecacheAlpha(MemoryImage* theImage)
{
    theImage->GetRLAlphaData();
}

void SexyAppBase::PrecacheNative(MemoryImage* theImage)
{
    theImage->GetNativeAlphaData(mDDInterface);
}

static inline int count_bits(Uint32 x)
{
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if (((x >> 1) & 1)) {
            count++;
        }
    }
    return count;
}

#if 0
// See below
static int find_mode_match(SDL_Rect **modes, int w, int h)
{
    for (int i = 0; modes[i]; i++) {
        if (modes[i]->w == w && modes[i]->h == h) {
           // Exact match. Use this one.
            return i;
        }
    }
    return -1;
}

static int find_mode_aspect(SDL_Rect **modes, int w, int h)
{
    float aspect_ratio = float(w) / h;
    for (int i = 0; modes[i]; i++) {
        if (aspect_ratio == ((float)modes[i]->w / modes[i]->h)) {
            // Aspect ratio matches. Use this one.
            return i;
        }
    }

    // Perhaps consider rotate
    for (int i = 0; modes[i]; i++) {
        if (aspect_ratio == ((float)modes[i]->h/ modes[i]->w)) {
            // Aspect ratio matches. Use this one.
            return i;
        }
    }

    return -1;
}

#ifdef DEBUG
static void dump_modes(LoggerFacil *mLogFacil, SDL_Rect **modes)
{
    for (int i = 0; modes[i]; i++) {
        Logger::log(mLogFacil, 1, Logger::format("mode[%d] w=%d, h=%d", i, modes[i]->w, modes[i]->h));
    }
}
#endif

#endif

void SexyAppBase::MakeWindow()
{
    Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::MakeWindow: game w=%d, h=%d", mWidth, mHeight));

    if (!mWindowIconBMP.empty()) {
        std::string fname = GetAppResourceFileName(mWindowIconBMP);
        SDL_WM_SetIcon(SDL_LoadBMP(fname.c_str()), NULL);
    }

    //Determine pixelformat of the video device
    SDL_PixelFormat* pf = SDL_GetVideoInfo()->vfmt;

    if (mDDInterface->mIs3D) {
        // Set OpenGL(ES) parameters same as SDL
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, count_bits(pf->Rmask));
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, count_bits(pf->Gmask));
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, count_bits(pf->Bmask));
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (mScreenSurface != NULL) {
            SDL_FreeSurface(mScreenSurface);
        }
        mScreenSurface = NULL;

        if (mIsWindowed) {
            // Always use the game dimensions
            Logger::log(mLogFacil, 1, "SexyAppBase::MakeWindow: mIs3D && mIsWindowed");
            mVideoModeWidth = mWidth;
            mVideoModeHeight = mHeight;
            //mVideoModeWidth = 768;          // testing
            //mVideoModeHeight = 1024;        // testing
            mScreenSurface = SDL_SetVideoMode(mVideoModeWidth, mVideoModeHeight, 32, SDL_OPENGL | SDL_HWSURFACE);
            if (mScreenSurface && (mScreenSurface->flags & SDL_FULLSCREEN) == SDL_FULLSCREEN) {
                if (SDL_WM_ToggleFullScreen(mScreenSurface) == -1) {
                    // FIXME. Should we panic and throw an exception?
                    mShutdown = true;
                }
            }
        }
        else {
            Logger::log(mLogFacil, 1, "SexyAppBase::MakeWindow: mIs3D && !mIsWindowed (full screen)");

#if 1
            SDL_DisplayMode mode;
            SDL_GetDisplayMode(0, 0, &mode);
#ifdef DEBUG
            Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::MakeWindow: SDL_GetDisplayMode mode w=%d, h=%d", mode.w, mode.h));
#endif
            mVideoModeWidth = mode.w;
            mVideoModeHeight = mode.h;
#else
            // Get available fullscreen/hardware modes
            // ???? Do we need the SDL_OPENGL flag too?
            SDL_Rect **modes;
            modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

            /* Check if there are any modes available */
            if (modes == (SDL_Rect **)0) {
                // TODO. Raise exception.
                printf("No modes available!\n");
                exit(-1);
            }

#ifdef DEBUG
            dump_modes(mLogFacil, modes);
#endif
            int use_mode_ix;
            if ((use_mode_ix = find_mode_match(modes, mWidth, mHeight)) < 0) {
                // Need to look further for a mode that has same aspect ratio
                if ((use_mode_ix = find_mode_aspect(modes, mWidth, mHeight)) < 0) {
                    // Give up. Use the first.
                    use_mode_ix = 0;
                }
            }
            use_mode_ix = 0;        // This one probably has actual screen dimensions
            Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::MakeWindow: mode[%d] w=%d, h=%d", use_mode_ix, modes[use_mode_ix]->w, modes[use_mode_ix]->h));
            mVideoModeWidth = modes[use_mode_ix]->w;
            mVideoModeHeight = modes[use_mode_ix]->h;
#endif

            // Note. A possible rotate is done in D3DInterface::UpdateViewport
            mScreenSurface = SDL_SetVideoMode(mVideoModeWidth, mVideoModeHeight, 32, SDL_OPENGL | SDL_FULLSCREEN | SDL_HWSURFACE);
            if (mScreenSurface && (mScreenSurface->flags & SDL_FULLSCREEN) != SDL_FULLSCREEN) {
                if (SDL_WM_ToggleFullScreen(mScreenSurface) == -1) {
                    // FIXME. Should we panic and throw an exception?
                    mShutdown = true;
                }
            }
        }
        // We need a new surface with game dimension so that DDImage and friends can
        // draw images.
        // Use all but the width and height from the ScreenSurface
        mGameSurface = SDL_CreateRGBSurface(
                mScreenSurface->flags,
                mWidth,
                mHeight,
                pf->BitsPerPixel,
                pf->Rmask,
                pf->Gmask,
                pf->Bmask,
                pf->Amask
                );
    }
    else {
        // Software renderer, not using OpenGL
        // ???? Is this always "windowed"?
        if (mScreenSurface != NULL) {
            SDL_FreeSurface(mScreenSurface);
        }
        Logger::log(mLogFacil, 1, "SexyAppBase::MakeWindow: !mIs3D");
        mVideoModeWidth = mWidth;
        mVideoModeHeight = mHeight;
        mScreenSurface = SDL_SetVideoMode(mWidth, mHeight, pf->BitsPerPixel, SDL_DOUBLEBUF | SDL_HWSURFACE);
        mGameSurface = mScreenSurface;
    }
#ifdef DEBUG
    Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::MakeWindow: mSurface: RMask=0x%08x", mScreenSurface->format->Rmask));
    Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::MakeWindow: mSurface: GMask=0x%08x", mScreenSurface->format->Gmask));
    Logger::log(mLogFacil, 1, Logger::format("SexyAppBase::MakeWindow: mSurface: BMask=0x%08x", mScreenSurface->format->Bmask));
#endif

    if (mScreenSurface == NULL) {
        // TODO. Throw an exception is probably better.
        mShutdown = true;
    }

    SDL_WM_SetCaption(mTitle.c_str(), NULL);

    (void) InitDDInterface();

    ReInitImages();
    // ???? See note in SexyAppBase::Set3DAcclerated()
    mWidgetManager->SetImage(mDDInterface->GetScreenImage());
    mWidgetManager->MarkAllDirty();
}

int SexyAppBase::InitDDInterface()
{
    PreDDInterfaceInitHook();
    DeleteNativeImageData();
    int aResult = mDDInterface->Init(NULL, mIsPhysWindowed);
    if ( DDInterface::RESULT_OK == aResult )
    {
        Rect mScreenBounds;
        mScreenBounds.mX = ( mWidth - mDDInterface->mWidth ) / 2;
        mScreenBounds.mY = ( mHeight - mDDInterface->mHeight ) / 2;
        mScreenBounds.mWidth = mDDInterface->mWidth;
        mScreenBounds.mHeight = mDDInterface->mHeight;
        mWidgetManager->Resize(mScreenBounds, mDDInterface->mPresentationRect);
        PostDDInterfaceInitHook();
    }
    return aResult;
}

void SexyAppBase::PreDisplayHook()
{
}

void SexyAppBase::DeleteNativeImageData()
{
    MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
    while (anItr != mMemoryImageSet.end())
    {
        MemoryImage* aMemoryImage = *anItr;
        aMemoryImage->DeleteNativeData();
        ++anItr;
    }
}

void SexyAppBase::PostDDInterfaceInitHook()
{
}

void SexyAppBase::PreDDInterfaceInitHook()
{
}

bool SexyAppBase::Is3DAccelerated()
{
    return mDDInterface && mDDInterface->mIs3D;
}

// TODO. Move to Common if we really want to keep this
bool SexyAppBase::FileExists(const std::string& theFileName)
{
    {
        FILE* aFP = fopen(theFileName.c_str(), "rb");

        if (aFP == NULL)
            return false;

        fclose(aFP);
        return true;
    }
}

bool SexyAppBase::ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo)
{
    {
        FILE* aFP = fopen(theFileName.c_str(), "rb");

        if (aFP == NULL)
        {
#ifdef DEBUG
            Logger::log(mLogFacil, 2, Logger::format("ReadBufferFromFile: File not found: %s\n", theFileName.c_str()));
#endif
            return false;
        }

        fseek(aFP, 0, SEEK_END);
        int aFileSize = ftell(aFP);
        fseek(aFP, 0, SEEK_SET);

        uchar* aData = new uchar[aFileSize];

        size_t read_bytes = fread(aData, sizeof(unsigned char), aFileSize, aFP);

        fclose(aFP);

        if (read_bytes != aFileSize * sizeof(unsigned char)) {
            delete[] aData;
            return false;
        }

        theBuffer->Clear();
        theBuffer->SetData(aData, aFileSize);
        delete[] aData;

        return true;
    }
}

bool SexyAppBase::WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer)
{
    return WriteBytesToFile(theFileName,theBuffer->GetDataPtr(),theBuffer->GetDataLen());
}

bool SexyAppBase::WriteBytesToFile(const std::string& theFileName, const void *theData, uint32_t theDataLen)
{
    MkDir(GetFileDir(theFileName));
    FILE* aFP = fopen(theFileName.c_str(), "w+b");

    if (aFP == NULL)
    {
        return false;
    }

    size_t written_bytes = fwrite(theData, sizeof(unsigned char), theDataLen, aFP);
    fclose(aFP);
    if (written_bytes != theDataLen * sizeof(unsigned char))
        return false;

    return true;
}

void SexyAppBase::LoadResourceManifest()
{
    if (!mResourceManager->ParseResourcesFile("properties/resources.xml"))
        ShowResourceError(true);
}

void SexyAppBase::ShowResourceError(bool doExit)
{
    if (doExit)
        DoExit(0);
}

void SexyAppBase::RestoreScreenResolution()
{
    if (mFullScreenWindow)
    {
        mFullScreenWindow = false;
    }
}

void SexyAppBase::DoExit(int theCode)
{
    RestoreScreenResolution();
    exit(theCode);
}

Dialog* SexyAppBase::NewDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode)
{
    Dialog* aDialog = new Dialog(NULL, NULL, theDialogId, isModal, theDialogHeader, theDialogLines, theDialogFooter, theButtonMode);
    return aDialog;
}

Dialog* SexyAppBase::DoDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode)
{
    KillDialog(theDialogId);

    Dialog* aDialog = NewDialog(theDialogId, isModal, theDialogHeader, theDialogLines, theDialogFooter, theButtonMode);

    AddDialog(theDialogId, aDialog);

    return aDialog;
}


Dialog* SexyAppBase::GetDialog(int theDialogId)
{
    DialogMap::iterator anItr = mDialogMap.find(theDialogId);

    if (anItr != mDialogMap.end())
        return anItr->second;

    return NULL;
}

bool SexyAppBase::KillDialog(int theDialogId, bool removeWidget, bool deleteWidget)
{
    DialogMap::iterator anItr = mDialogMap.find(theDialogId);

    if (anItr != mDialogMap.end())
    {
        Dialog* aDialog = anItr->second;

        // set the result to something else so DoMainLoop knows that the dialog is gone
        // in case nobody else sets mResult
        if (aDialog->mResult == -1)
            aDialog->mResult = 0;

        DialogList::iterator aListItr = std::find(mDialogList.begin(),mDialogList.end(),aDialog);
        if (aListItr != mDialogList.end())
            mDialogList.erase(aListItr);

        mDialogMap.erase(anItr);

        if (removeWidget || deleteWidget)
        mWidgetManager->RemoveWidget(aDialog);

        if (aDialog->IsModal())
        {
            ModalClose();
            mWidgetManager->RemoveBaseModal(aDialog);
        }

        if (deleteWidget)
        SafeDeleteWidget(aDialog);

        return true;
    }

    return false;
}

bool SexyAppBase::KillDialog(int theDialogId)
{
    return KillDialog(theDialogId,true,true);
}

bool SexyAppBase::KillDialog(Dialog* theDialog)
{
    return KillDialog(theDialog->mId);
}

int SexyAppBase::GetDialogCount()
{
    return mDialogMap.size();
}

void SexyAppBase::AddDialog(int theDialogId, Dialog* theDialog)
{
    KillDialog(theDialogId);

    if (theDialog->mWidth == 0)
    {
        // Set the dialog position ourselves
        int aWidth = mWidth/2;
        theDialog->Resize((mWidth - aWidth)/2, mHeight / 5, aWidth, theDialog->GetPreferredHeight(aWidth));
    }

    mDialogMap.insert(DialogMap::value_type(theDialogId, theDialog));
    mDialogList.push_back(theDialog);

    mWidgetManager->AddWidget(theDialog);
    if (theDialog->IsModal())
    {
        mWidgetManager->AddBaseModal(theDialog);
        ModalOpen();
    }
}

void SexyAppBase::AddDialog(Dialog* theDialog)
{
    AddDialog(theDialog->mId, theDialog);
}

void SexyAppBase::ModalOpen()
{
}

void SexyAppBase::ModalClose()
{
}

void SexyAppBase::DialogButtonPress(int theDialogId, int theButtonId)
{
    if (theButtonId == Dialog::ID_YES)
        ButtonPress(2000 + theDialogId);
    else if (theButtonId == Dialog::ID_NO)
        ButtonPress(3000 + theDialogId);
}

void SexyAppBase::DialogButtonDepress(int theDialogId, int theButtonId)
{
    if (theButtonId == Dialog::ID_YES)
        ButtonDepress(2000 + theDialogId);
    else if (theButtonId == Dialog::ID_NO)
        ButtonDepress(3000 + theDialogId);
}

void SexyAppBase::GotFocus()
{
}

void SexyAppBase::LostFocus()
{
}

void SexyAppBase::CleanSharedImages()
{
    if (mCleanupSharedImages)
    {
        // Delete shared images with reference counts of 0
        // This doesn't occur in ~SharedImageRef because sometimes we can not only access the image
        //  through the SharedImageRef returned by GetSharedImage, but also by calling GetSharedImage
        //  again with the same params -- so we can have instances where we do the 'final' deref on
        //  an image but immediately re-request it via GetSharedImage
        SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
        while (aSharedImageItr != mSharedImageMap.end())
        {
            SharedImage* aSharedImage = &aSharedImageItr->second;
            if (aSharedImage->mRefCount == 0)
            {
                delete aSharedImage->mImage;
                mSharedImageMap.erase(aSharedImageItr++);
            }
            else
                ++aSharedImageItr;
        }

        mCleanupSharedImages = false;
    }
}

bool SexyAppBase::ChangeDirHook(const char *theIntendedPath)
{
    return false;
}

void SexyAppBase::InitPropertiesHook()
{
}

void SexyAppBase::InitHook()
{
}

void SexyAppBase::SetAlphaDisabled(bool isDisabled)
{
    if (mAlphaDisabled != isDisabled)
    {
        mAlphaDisabled = isDisabled;
        mDDInterface->SetVideoOnlyDraw(mAlphaDisabled);
        // ???? See note in SexyAppBase::Set3DAcclerated
        mWidgetManager->SetImage(mDDInterface->GetScreenImage());
        mWidgetManager->MarkAllDirty();
    }
}



void SexyAppBase::HandleGameAlreadyRunning()
{
    if (mOnlyAllowOneCopyToRun)
    {

        DoExit(0);
    }
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed, bool is3d, bool force)
{
    if (mForceFullscreen)
        // We may have asked for windowed, but we can't, sorry.
        wantWindowed = false;


        //FIXME TODO should be enabled
#if 0
    if (!wantWindowed)
    {
        // full screen = smooth scrolling and vsyncing
        mVSyncUpdates = true;
        mWaitForVSync = true;
    }
    else
    {
        // windowed doesn't vsync and do smooth motion
        mVSyncUpdates = false;
        mWaitForVSync = false;
    }
#endif
    // Set 3d acceleration preference
    Set3DAcclerated(is3d, false);

    // Always make the app windowed when playing demos, in order to
    //  make it easier to track down bugs.  We place this after the
    //  sanity check just so things get re-initialized and stuff
    //if (mPlayingDemoBuffer)
    //  wantWindowed = true;

    mIsWindowed = wantWindowed;

    MakeWindow();

    if (mSoundManager!=NULL)
    {
        mSoundManager->SetCooperativeWindow(mHWnd,mIsWindowed);
    }

    mLastTime = SDL_GetTicks();
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed)
{
    SwitchScreenMode(wantWindowed, Is3DAccelerated());
}

void SexyAppBase::SwitchScreenMode()
{
    SwitchScreenMode(mIsWindowed, Is3DAccelerated(), true);
}

bool SexyAppBase::Is3DAccelerationSupported()
{

#if 0
    if (mDDInterface->mD3DTester)
        return mDDInterface->mD3DTester->Is3DSupported();
    else
        return false;
#else
    return true;
#endif
}

bool SexyAppBase::Is3DAccelerationRecommended()
{
#if 0
    if (mDDInterface->mD3DTester)
        return mDDInterface->mD3DTester->Is3DRecommended();
    else
        return false;
#else
    return true;
#endif
}

void SexyAppBase::Remove3DData(MemoryImage* theMemoryImage)
{
    if (mDDInterface)
        mDDInterface->Remove3DData(theMemoryImage);
}

void SexyAppBase::EnforceCursor()
{
    bool wantSysCursor = true;

    if (mDDInterface == NULL)
        return;

    if ((mSEHOccured) || (!mMouseIn))
    {
        SDL_SetCursor(mArrowCursor);
        SDL_ShowCursor(SDL_ENABLE);

        if (mDDInterface->SetCursorImage(NULL))
            mCustomCursorDirty = true;
    }
    else
    {
        if ((mCursorImages[mCursorNum] == NULL) ||
            (
            (!mCustomCursorsEnabled) && (mCursorNum != CURSOR_CUSTOM)))
        {

            switch(mCursorNum) {
            case CURSOR_POINTER:
                SDL_SetCursor(mArrowCursor);
                SDL_ShowCursor(SDL_ENABLE);
                break;
            case CURSOR_HAND:
                SDL_SetCursor(mHandCursor);
                SDL_ShowCursor(SDL_ENABLE);
                break;
            case CURSOR_DRAGGING:
                SDL_SetCursor(mDraggingCursor);
                SDL_ShowCursor(SDL_ENABLE);
                break;
            case CURSOR_NONE:
                SDL_ShowCursor(SDL_DISABLE);
                break;
            }

            if (mDDInterface->SetCursorImage(NULL))
                mCustomCursorDirty = true;
        }
        else
        {
            if (mDDInterface->SetCursorImage(mCursorImages[mCursorNum]))
                mCustomCursorDirty = true;

#if 0
            if (!mPlayingDemoBuffer)
            {
#endif
                SDL_ShowCursor(SDL_DISABLE);
#if 0
            }
            else {
                SDL_ShowCursor(SDL_ENABLE);
            }
#endif
            wantSysCursor = false;
        }
    }

    if (wantSysCursor != mSysCursor)
    {
        mSysCursor = wantSysCursor;
    }
}

void SexyAppBase::SetCursorImage(int theCursorNum, Image* theImage)
{
    if ((theCursorNum >= 0) && (theCursorNum < NUM_CURSORS))
    {
        mCursorImages[theCursorNum] = theImage;
        EnforceCursor();
    }
}

int SexyAppBase::GetCursor()
{
    return mCursorNum;
}

void SexyAppBase::EnableCustomCursors(bool enabled)
{
    mCustomCursorsEnabled = enabled;
    if (!mCustomCursorsEnabled) {
        SDL_ShowCursor(SDL_ENABLE);
    }
    EnforceCursor();
}

SexyString SexyAppBase::GetString(const std::string& theId)
{
    StringWStringMap::iterator anItr = mStringProperties.find(theId);
    assert(anItr != mStringProperties.end());

    if (anItr != mStringProperties.end())
        return WStringToSexyString(anItr->second);
    else
        return _S("");
}

SexyString SexyAppBase::GetString(const std::string& theId, const SexyString& theDefault)
{
    StringWStringMap::iterator anItr = mStringProperties.find(theId);

    if (anItr != mStringProperties.end())
        return WStringToSexyString(anItr->second);
    else
        return theDefault;
}

void SexyAppBase::Done3dTesting()
{
}

// return file name that you want to upload
std::string SexyAppBase::NotifyCrashHook()
{
    return "";
}

void SexyAppBase::PreTerminate()
{
}

bool SexyAppBase::CheckSignature(const Buffer& theBuffer, const std::string& theFileName)
{
    // Add your own signature checking code here
    return false;
}

bool SexyAppBase::LoadProperties(const std::string& theFileName, bool required, bool checkSig)
{
    Buffer aBuffer;

    if (!ReadBufferFromFile(GetAppResourceFileName(theFileName), &aBuffer))
    {
        if (!required)
            return true;
        else
            return false;
    }
    if (checkSig)
    {
        if (!CheckSignature(aBuffer, theFileName))
        {
            return false;
        }
    }

    PropertiesParser aPropertiesParser(this);

    // Load required language-file properties
    if (!aPropertiesParser.ParsePropertiesBuffer(aBuffer))
    {
        return false;
    }
    else
        return true;
}

bool SexyAppBase::LoadProperties()
{
    // Load required language-file properties
    return LoadProperties("properties/default.xml", true, false);
}

bool SexyAppBase::GetBoolean(const std::string& theId, bool theDefault)
{
    StringBoolMap::iterator anItr = mBoolProperties.find(theId);

    if (anItr != mBoolProperties.end())
        return anItr->second;
    else
        return theDefault;
}

int SexyAppBase::GetInteger(const std::string& theId, int theDefault)
{
    StringIntMap::iterator anItr = mIntProperties.find(theId);

    if (anItr != mIntProperties.end())
        return anItr->second;
    else
        return theDefault;
}

double SexyAppBase::GetDouble(const std::string& theId, double theDefault)
{
    StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);

    if (anItr != mDoubleProperties.end())
        return anItr->second;
    else
        return theDefault;
}

StringVector SexyAppBase::GetStringVector(const std::string& theId)
{
    StringStringVectorMap::iterator anItr = mStringVectorProperties.find(theId);
    assert(anItr != mStringVectorProperties.end());

    if (anItr != mStringVectorProperties.end())
        return anItr->second;
    else
        return StringVector();
}

void SexyAppBase::SetString(const std::string& theId, const std::wstring& theValue)
{
    std::pair<StringWStringMap::iterator, bool> aPair = mStringProperties.insert(StringWStringMap::value_type(theId, theValue));
    if (!aPair.second) // Found it, change value
        aPair.first->second = theValue;
}

void SexyAppBase::SetBoolean(const std::string& theId, bool theValue)
{
    std::pair<StringBoolMap::iterator, bool> aPair = mBoolProperties.insert(StringBoolMap::value_type(theId, theValue));
    if (!aPair.second) // Found it, change value
        aPair.first->second = theValue;
}

void SexyAppBase::SetInteger(const std::string& theId, int theValue)
{
    std::pair<StringIntMap::iterator, bool> aPair = mIntProperties.insert(StringIntMap::value_type(theId, theValue));
    if (!aPair.second) // Found it, change value
        aPair.first->second = theValue;
}

void SexyAppBase::SetDouble(const std::string& theId, double theValue)
{
    std::pair<StringDoubleMap::iterator, bool> aPair = mDoubleProperties.insert(StringDoubleMap::value_type(theId, theValue));
    if (!aPair.second) // Found it, change value
        aPair.first->second = theValue;
}

void SexyAppBase::SetWindowIconBMP(const std::string& icon)
{
    mWindowIconBMP = ReplaceBackSlashes(icon);
}

int SexyAppBase::ParseCommandLine(int argc, char** argv)
{
    if (CmdLine::getCmdLine() == NULL) {
        if (!CmdLine::ParseCommandLine(argc, argv)) {
            return -1;
        }
    }
    mArgv0 = CmdLine::getArgv0();
    AnyOption * opt = CmdLine::getOpt();

    if (!opt->hasOptions()) {
        return 0;
    }

    mDebug = opt->getFlag("debug");

    if (opt->getFlag("fullscreen") || opt->getFlag('f')) {
        mFullScreenMode = true;
        //SwitchScreenMode(false, mDDInterface->mIs3D);
        //puts("Running in fullscreen mode");
    }
    if (opt->getFlag("windowed") || opt->getFlag('w')) {
        mWindowedMode = true;
        //SwitchScreenMode(true, mDDInterface->mIs3D);
        //puts("Running in windowed mode");
    }
    if (opt->getFlag("fps") || opt->getFlag('p')) {
        mShowFPS = true;
        //printf("Displaying fps\n");
    }
    if (opt->getFlag("opengl") || opt->getFlag('o')) {
        mUseOpenGL = true;
        //SwitchScreenMode(mIsWindowed, true);
        //puts("Running with OpenGL hardware acceleration");
        //printf("Using OpenGL renderer\n");
    }
    if (opt->getFlag("software") || opt->getFlag('s')) {
        mUseSoftwareRenderer = true;
        //SwitchScreenMode(mIsWindowed, false);
        //puts("Running with Software Renderer");
        //printf("Using software renderer\n");
    }

    if (opt->getValue("resource-dir") != NULL) {
        std::string rsc_dir = opt->getValue("resource-dir");
        SetAppResourceFolder(rsc_dir);
    }

    /* 8. DONE */
    delete opt;
    return 0;
}

void SexyAppBase::SetAppDataFolder(const std::string& thePath)
{
    std::string aPath = thePath;
    if (!aPath.empty()) {
        // If last char is not a slash, add one
        // Use the UNIX slash, even Windows can handle it.
        if (aPath[aPath.length() - 1] != '\\' && aPath[aPath.length() - 1] != '/')
            aPath += '/';
    }
    mAppDataFolder = ReplaceBackSlashes(aPath);
    if (!IsDir(mAppDataFolder)) {
        MkDir(mAppDataFolder);
    }
}

void SexyAppBase::SetAppResourceFolder(const std::string& thePath)
{
    std::string aPath = thePath;
    if (!aPath.empty()) {
        // If last char is not a slash, add one
        // Use the UNIX slash, even Windows can handle it.
        if (aPath[aPath.length() - 1] != '\\' && aPath[aPath.length() - 1] != '/')
            aPath += '/';
    }
    mAppResourceFolder = ReplaceBackSlashes(aPath);
    // The directory must be present. Don't just create it.
}

std::string SexyAppBase::GetAppResourceFileName(const std::string & fileName) const
{
    // Convert the filename so that it will be located in the Recource folder.
    // If it is an absolute filename, just return that.
    if (fileName[0] == '/') {
        return fileName;
    }
    if (mAppResourceFolder.empty()) {
        return fileName;
    }
    // TODO. Get rid of ReplaceBackSlashes, we shouldn't have backslahes in the first place.
    // FIXME. Why not use this method for all places where we prefix with AppResourceFolder?
    if (fileName.find(mAppResourceFolder) == 0) {
        // The filename already starts with the resource folder name
        return fileName;
    }
    return ReplaceBackSlashes(mAppResourceFolder + fileName);
}

void SexyAppBase::SetUserLanguage(const std::string& l)
{
    mUserLanguage = l;
}
