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

#include "SDLMixerMusicInterface.h"
#include "SDLMixerSoundInstance.h"
#include "SDLMixerSoundManager.h"

using namespace Sexy;

SDLMixerSoundInstance::SDLMixerSoundInstance(SDLMixerSoundManager* theSoundManager, int channel, Mix_Chunk* theSourceSound)
{
    mSDLMixerSoundManagerP = theSoundManager;

    if (mSDLMixerSoundManagerP) {
        mSample = theSourceSound;
        mChannel = channel;
    } else {
        mSample = NULL;
        mChannel = -1;
    }

    mReleased = false;
    mAutoRelease = false;
    mHasPlayed = false;

    mBaseVolume = 1.0;
    mBasePan = 0.0;

    mVolume = 1.0;
    mPan = 0.0;
    mPitch = 1.0;

    mHasPlayed = false;

    RehupVolume();
}

SDLMixerSoundInstance::~SDLMixerSoundInstance()
{
    Release();
    mSample = NULL;
    mChannel = -1;
}

void SDLMixerSoundInstance::RehupVolume()
{
    if (mSample)
        Mix_VolumeChunk(mSample, (int) (mBaseVolume * mVolume * mSDLMixerSoundManagerP->mMasterVolume * (float) MIX_MAX_VOLUME));
}

void SDLMixerSoundInstance::RehupPan()
{
    if (mSample) {
        float aPan = /*mBasePan/100.0f * */mPan / 100.0f;
        if (aPan > 1.0f)
            aPan = 1.0f;
        else if (aPan < -1.0f)
            aPan = -1.0f;

        int mLeft, mRight;
        if (aPan < 0.0f) {
            mRight = 128 - (int) (128.0f * aPan);
            mLeft = 255 - mRight;
        } else {
            mLeft = 128 - (int) (128.0f * aPan);
            mRight = 255 - mLeft;
        }
        Mix_SetPanning(mChannel, mLeft, mRight);
    }
}

void SDLMixerSoundInstance::RehupPitch()
{
}

void SDLMixerSoundInstance::Release()
{
    Stop();
    mSample = NULL;
    mReleased = true;
}

void SDLMixerSoundInstance::SetVolume(double theVolume) // 0.0 to 1.0
{
    mVolume = (float) theVolume;
    RehupVolume();
}

void SDLMixerSoundInstance::SetPan(int thePosition) //-100 to +100 = left to right
{
    mPan = float(thePosition / 100);
    RehupPan();
}

void SDLMixerSoundInstance::AdjustPitch(double theNumSteps) //+0.5 to +2.0 = lower to higher
{
    mPitch = (float) theNumSteps;
    RehupPitch();
}

void SDLMixerSoundInstance::SetBaseVolume(double theBaseVolume)
{
    mBaseVolume = (float) theBaseVolume;
    RehupVolume();
}

void SDLMixerSoundInstance::SetBasePan(int theBasePan)
{
    mBasePan = float(theBasePan / 100);
    RehupPan();
}

void SDLMixerSoundInstance::AdjustBasePitch(float thePitch)
{
    mBasePitch = thePitch;
    RehupPitch();
}

bool SDLMixerSoundInstance::Play(bool looping, bool autoRelease)
{
    Stop();

    mAutoRelease = autoRelease;

    if (!mSample)
        return false;

    Mix_PlayChannel(mChannel, mSample, looping ? -1 : 0);

    mHasPlayed = true;
    return true;
}

void SDLMixerSoundInstance::Stop()
{
    if (mSample) {
        Mix_HaltChannel(mChannel);
        mAutoRelease = false;
    }
}

bool SDLMixerSoundInstance::IsPlaying()
{
    return Mix_Playing(mChannel);
}

bool SDLMixerSoundInstance::IsReleased()
{
    if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying()))
        Release();

    return mReleased;
}

double SDLMixerSoundInstance::GetVolume()
{
    return mVolume;
}
