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

#ifndef __SDLMixerSoundINSTANCE_H__
#define __SDLMixerSoundINSTANCE_H__


#include "SoundInstance.h"

namespace Sexy {

class SDLMixerSoundManager;

class SDLMixerSoundInstance : public SoundInstance {
    friend class SDLMixerSoundManager;

protected:
    SDLMixerSoundManager*   mSDLMixerSoundManagerP;

    Mix_Chunk*              mSample;
    int                     mChannel;
    bool                    mAutoRelease;
    bool                    mReleased;

    float                   mBasePan;
    float                   mBaseVolume;
    float                   mBasePitch;

    float                   mPan;
    float                   mVolume;
    float                   mPitch;

    bool                    mHasPlayed;

protected:
    void                    RehupVolume();
    void                    RehupPan();
    void                    RehupPitch();

public:
    SDLMixerSoundInstance(SDLMixerSoundManager* theSoundManager, int channel, Mix_Chunk* theSourceSound);
    virtual ~SDLMixerSoundInstance();

    virtual void            Release();

    virtual void            SetBaseVolume(double theBaseVolume); //0.0 to 1.0
    virtual void            SetBasePan(int theBasePan); //-100 to +100
    virtual void            AdjustBasePitch(float thePitch); //+0.5 to +2.0 relative to normal playing speed

    virtual void            SetVolume(double theVolume); //0.0 to 1.0
    virtual void            SetPan(int thePosition); //-100 to +100 = left to right
    virtual void            AdjustPitch(double thePitch); //+0.5 to +2.0 relative to normal playing speed

    virtual bool            Play(bool looping, bool autoRelease);
    virtual void            Stop();
    virtual bool            IsPlaying();
    virtual bool            IsReleased();
    virtual double          GetVolume();

};

}

#endif //__SDLMixerSoundINSTANCE_H__
