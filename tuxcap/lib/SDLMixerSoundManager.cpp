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
#include "SDLMixerSoundManager.h"
#include "SDLMixerSoundInstance.h"

using namespace Sexy;

SDLMixerSoundManager::SDLMixerSoundManager()
{
  if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2 , 1024)==-1) {
    printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
  }
        Mix_AllocateChannels(MAX_CHANNELS);

	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mSourceSounds[i] = NULL;
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
		mBasePitches[i] = 1;
	}

	for (int i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = NULL;

	mMasterVolume = 1.0;
        mLastReleaseTick = SDL_GetTicks();
}

SDLMixerSoundManager::~SDLMixerSoundManager()
{
  StopAllSounds();
	ReleaseChannels();
	ReleaseSounds();

        int numtimesopened, frequency, channels;
        Uint16 format;
        numtimesopened=Mix_QuerySpec(&frequency, &format, &channels);
        while(numtimesopened--) 
          Mix_CloseAudio();
}

int SDLMixerSoundManager::FindFreeChannel()
{
  Uint32 aTick = SDL_GetTicks();
  if (aTick - mLastReleaseTick > 1000)
	{
		ReleaseFreeChannels();
		mLastReleaseTick = aTick;
	}

  for (int i = 0; i < MAX_CHANNELS; i++)
    {		
      if (mPlayingSounds[i] == NULL)
        return i;
		
      if (mPlayingSounds[i]->IsReleased())
        {
          delete mPlayingSounds[i];
          mPlayingSounds[i] = NULL;
          return i;
        }
    }
	
  return -1;
}

bool SDLMixerSoundManager::Initialized()
{
        int numtimesopened, frequency, channels;
        Uint16 format;

        numtimesopened=Mix_QuerySpec(&frequency, &format, &channels);

        return numtimesopened > 0;
}

void SDLMixerSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = (float)theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

int SDLMixerSoundManager::LoadSound(const std::string& theFilename) {
	int id = GetFreeSoundId();

	if (id != -1) {
		if (LoadSound(id, theFilename)) {
			return id;
		}
	}
	return -1;
}

bool SDLMixerSoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

	if (!Initialized())
		return true; // sounds just	won't play, but this is not treated as a failure condition

	std::string aFilename = ReplaceBackSlashes(theFilename);

	mSourceSounds[theSfxID] = Mix_LoadWAV(aFilename.c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = Mix_LoadWAV((aFilename + ".wav").c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = Mix_LoadWAV((aFilename + ".ogg").c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = Mix_LoadWAV((aFilename + ".mp3").c_str());

	return mSourceSounds[theSfxID] != NULL;
}

void SDLMixerSoundManager::ReleaseSound(unsigned int theSfxID)
{
  if (mSourceSounds[theSfxID] != NULL)  {

    for (int i = 0; i < MAX_CHANNELS; i++)
      if (mPlayingSounds[i] != NULL && mPlayingSounds[i]->mSample == mSourceSounds[theSfxID])
        {
          mPlayingSounds[i]->Release();
          break;
        }
    Mix_FreeChunk(mSourceSounds[theSfxID]);
    mSourceSounds[theSfxID] = NULL;
  }
}

bool SDLMixerSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = (float)theBaseVolume;
	return true;
}

bool SDLMixerSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

bool SDLMixerSoundManager::SetBasePitch(unsigned int theSfxID, float theBasePitch)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePitches[theSfxID] = theBasePitch;
	return true;
}

SoundInstance* SDLMixerSoundManager::GetSoundInstance(unsigned int theSfxID)
{
	if (theSfxID > MAX_SOURCE_SOUNDS)
		return NULL;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return NULL;

	if (!Initialized())
	{
          mPlayingSounds[aFreeChannel] = new SDLMixerSoundInstance(this, aFreeChannel, NULL);
	}
	else
	{
		if (!mSourceSounds[theSfxID])
			return NULL;
		mPlayingSounds[aFreeChannel] = new SDLMixerSoundInstance(this, aFreeChannel, mSourceSounds[theSfxID]);
	}

	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);
	mPlayingSounds[aFreeChannel]->AdjustBasePitch(mBasePitches[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void SDLMixerSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
          ReleaseSound(i);
}

void SDLMixerSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i])
		{
			mPlayingSounds[i]->Release();
			mPlayingSounds[i] = NULL;
		}
}

void SDLMixerSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] && mPlayingSounds[i]->IsReleased())
			mPlayingSounds[i] = NULL;
}

void SDLMixerSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
}

double SDLMixerSoundManager::GetMasterVolume()
{
        return 0.0;
}

void SDLMixerSoundManager::SetMasterVolume(double theVolume)
{
}

void SDLMixerSoundManager::Flush()
{
}

void SDLMixerSoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed)
{
}

int	SDLMixerSoundManager::GetFreeSoundId() {
	for (int i = 0; i < MAX_SOURCE_SOUNDS; ++i) {
		if (!mSourceSounds[i]) {
			return i;
		}
	}
	return -1;
}

int SDLMixerSoundManager::GetNumSounds() {
	int nr_sounds = 0;

	for (int i = 0; i < MAX_SOURCE_SOUNDS; ++i) {
		if (mSourceSounds[i]) {
			++nr_sounds;
		}
	}
	return nr_sounds;
}


#undef SOUND_FLAGS

