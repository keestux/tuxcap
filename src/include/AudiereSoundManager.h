/* Original Audiere Sound Manager  by Rheenen 2005 */

#ifndef __AudiereSoundMANAGER_H__
#define __AudiereSoundMANAGER_H__

#include "SoundManager.h"
#include "audiere.h"
#include "SDL.h"

#ifndef WIN32
#define HWND void*
#endif

using namespace audiere;

namespace Sexy
{

class AudiereSoundInstance;

class AudiereSoundManager : public SoundManager
{
    friend class AudiereSoundInstance;

protected:
    SampleSourcePtr         mSourceSounds[MAX_SOURCE_SOUNDS];
    float                   mBaseVolumes[MAX_SOURCE_SOUNDS];
    int                     mBasePans[MAX_SOURCE_SOUNDS];
    float                   mBasePitches[MAX_SOURCE_SOUNDS];
    AudiereSoundInstance*   mPlayingSounds[MAX_CHANNELS];   
    float                   mMasterVolume;
       Uint32                   mLastReleaseTick;

protected:
    int                     FindFreeChannel();
    void                    ReleaseFreeChannels();

public:
    AudioDevicePtr          mDevice;

public:
    AudiereSoundManager();
    virtual ~AudiereSoundManager();

    virtual bool            Initialized();
    
    virtual bool            LoadSound(unsigned int theSfxID, const std::string& theFilename);       
    virtual int             LoadSound(const std::string& theFilename);      
    virtual void            ReleaseSound(unsigned int theSfxID);
    virtual int             GetFreeSoundId();
    virtual int             GetNumSounds();

    virtual void            SetVolume(double theVolume);
    virtual bool            SetBaseVolume(unsigned int theSfxID, double theBaseVolume);
    virtual bool            SetBasePan(unsigned int theSfxID, int theBasePan);
    virtual bool            SetBasePitch(unsigned int theSfxID, float theBasePitch);

    virtual SoundInstance*  GetSoundInstance(unsigned int theSfxID);

    virtual void            ReleaseSounds();
    virtual void            ReleaseChannels();

    virtual double          GetMasterVolume();
    virtual void            SetMasterVolume(double theVolume);

    virtual void            Flush();

    virtual void            SetCooperativeWindow(HWND theHWnd, bool isWindowed);
    virtual void            StopAllSounds();
};

}

#endif //__AudiereSoundMANAGER_H__

