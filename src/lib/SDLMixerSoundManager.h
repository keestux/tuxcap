/* Copyright (c) 2007 W.P. van Paassen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SDLMixerSoundMANAGER_H__
#define __SDLMixerSoundMANAGER_H__

#include "SoundManager.h"
#include "SDL_mixer.h"

#ifndef WIN32
#define HWND void*
#endif

namespace Sexy
{

class SDLMixerSoundInstance;

class SDLMixerSoundManager : public SoundManager
{
    friend class SDLMixerSoundInstance;

protected:
    Mix_Chunk*              mSourceSounds[MAX_SOURCE_SOUNDS];
    float                   mBaseVolumes[MAX_SOURCE_SOUNDS];
    int                     mBasePans[MAX_SOURCE_SOUNDS];
    float                   mBasePitches[MAX_SOURCE_SOUNDS];
    SDLMixerSoundInstance*  mPlayingSounds[MAX_CHANNELS];   
    float                   mMasterVolume;
    Uint32                  mLastReleaseTick;

protected:
    int                     FindFreeChannel();
    void                    ReleaseFreeChannels();

public:
    SDLMixerSoundManager();
    virtual ~SDLMixerSoundManager();

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
    virtual SoundInstance*  GetOriginalSoundInstance(unsigned int theSfxID);

    virtual void            ReleaseSounds();
    virtual void            ReleaseChannels();

    virtual double          GetMasterVolume();
    virtual void            SetMasterVolume(double theVolume);

    virtual void            Flush();

    virtual void            SetCooperativeWindow(HWND theHWnd, bool isWindowed);
    virtual void            StopAllSounds();
    virtual void            StopSound(int theSfxID); //FIXME, TODO
    virtual bool            IsSoundPlaying(int theSfxID){ return false; } //FIXME, TODO
};

}

#endif //__SDLMIXERSoundMANAGER_H__
