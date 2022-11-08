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

/* To be able to use midi, install package freepats  which contains midi instruments */

#include <string>

#include "SDLMixerMusicInterface.h"
#include "SexyAppBase.h"
#include "PakInterface.h"

using namespace Sexy;
using namespace std;

SDLMixerMusicInfo::SDLMixerMusicInfo()
{
    music = NULL;

    mVolume = 1.0;
    mStopOnFade = false;
    mRepeats = false;
    mPosition = 0;
    mIsActive = false;
    mBuffer = NULL;
}

SDLMixerMusicInterface::SDLMixerMusicInterface(HWND theHWnd)
{
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        // TODO. Throw exception
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
    }
    mMasterVolume = 1.0;
    mCurrentMusic = -1;
}

SDLMixerMusicInterface::~SDLMixerMusicInterface()
{
    StopAllMusic();
    SDLMixerMusicMap::iterator anItr = mMusicMap.begin();
    while (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            Mix_FreeMusic(aMusicInfo->music);
            aMusicInfo->music = NULL;
        }

        if (aMusicInfo->mBuffer != NULL) {
            delete[] aMusicInfo->mBuffer;
            aMusicInfo->mBuffer = NULL;
        }

        ++anItr;
    }

    int numtimesopened, frequency, channels;
    Uint16 format;
    numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
    while (numtimesopened--)
        Mix_CloseAudio();
}

bool SDLMixerMusicInterface::LoadMusic(int theSongId, const string& theFileName)
{
    SDLMixerMusicInfo aMusicInfo;

    // If the resources are in a PAK file then we need to read in the
    // whole music file at once and then use SDL_RWFromMem() plus Mix_LoadMUS_RW

    // Otherwise we use Mix_LoadMUS

    // TODO. Split the logic of loading the music inti a lower level
    // function. And a higher level function with a loop trying all extensions.

    bool pak = GetPakPtr()->isLoaded();
    string myFileName;
    if (pak)
        myFileName = ReplaceBackSlashes(theFileName);
    else
        myFileName = gSexyAppBase->GetAppResourceFileName(theFileName);

    Mix_Music* m = NULL;

    int aLastDotPos = myFileName.rfind('.');
    int aLastSlashPos = myFileName.rfind('/');
    string try_exts[] = {
        ".ogg",
        ".mp3",
        ".mid",
        ".mod",
    };
    if (pak) {

        PFILE* file = NULL;
        if (aLastDotPos > aLastSlashPos) {
            // The filename has an extension
            file = p_fopen(myFileName.c_str(), "r");
        }
        else {
            // Try to append a bunch of extensions.
            // TODO. Perhaps remove the extension first?
            // TODO. On Linux try upper case too.
            for (size_t i = 0; file == NULL && i < (sizeof(try_exts)/sizeof(try_exts[0])); i++) {
                file = p_fopen((myFileName + try_exts[i]).c_str(), "r");
            }
        }
        if (file == NULL)
            return false;

        int size = GetPakPtr()->FSize(file);
        aMusicInfo.mBuffer = new Uint8[size];
        int res = p_fread((void*) aMusicInfo.mBuffer, sizeof (Uint8), size * sizeof (Uint8), file);
        if (size != res)
            // TODO. Throw exception
            return false;

        SDL_RWops* rw = SDL_RWFromMem(aMusicInfo.mBuffer, size);
        m = Mix_LoadMUS_RW(rw, SDL_TRUE);
        p_fclose(file);
    } else {
        if (aLastDotPos > aLastSlashPos) {
            // The filename has an extension
            m = Mix_LoadMUS(myFileName.c_str());
        } else {
            // There is no filename extension. Try a couple
            for (size_t i = 0; m == NULL && i < (sizeof(try_exts)/sizeof(try_exts[0])); i++) {
                m = Mix_LoadMUS((myFileName + try_exts[i]).c_str());
            }
        }
    }
    if (m == NULL) {
        return false;
    }

    aMusicInfo.music = m;
    mMusicMap.insert(SDLMixerMusicMap::value_type(theSongId, aMusicInfo));
    return true;
}

void SDLMixerMusicInterface::UnloadMusic(int theSongId)
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            Mix_FreeMusic(aMusicInfo->music);
            aMusicInfo->music = NULL;
        }

        if (aMusicInfo->mBuffer != NULL) {
            delete[] aMusicInfo->mBuffer;
            aMusicInfo->mBuffer = NULL;
        }

        mMusicMap.erase(anItr);
    }
}

void SDLMixerMusicInterface::UnloadAllMusic()
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.begin();
    while (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            Mix_FreeMusic(aMusicInfo->music);
            aMusicInfo->music = NULL;
        }

        if (aMusicInfo->mBuffer != NULL) {
            delete[] aMusicInfo->mBuffer;
            aMusicInfo->mBuffer = NULL;
        }

        ++anItr;
    }
    mMusicMap.clear();
}

void SDLMixerMusicInterface::PauseAllMusic()
{
    Mix_PauseMusic();
}

void SDLMixerMusicInterface::ResumeAllMusic()
{
    Mix_ResumeMusic();
}

void SDLMixerMusicInterface::DeactivateAllMusic()
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.begin();
    while (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL)
            aMusicInfo->mIsActive = false;
        ++anItr;
    }
}

void SDLMixerMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
    DeactivateAllMusic();

    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo != NULL) {
            aMusicInfo->mIsActive = true;

            Mix_PlayMusic(aMusicInfo->music, noLoop ? 1 : -1);
            if (theOffset != 0) {
                Mix_SetMusicPosition(theOffset);
            }
            Mix_VolumeMusic((int) ((float) mMasterVolume * aMusicInfo->mVolume * 128));

            mCurrentMusic = theSongId;
        }
    } else {
        //TODO Load Music from sound id
    }
}

void SDLMixerMusicInterface::PauseMusic(int theSongId)
{
    PauseAllMusic();
}

void SDLMixerMusicInterface::ResumeMusic(int theSongId)
{
    ResumeAllMusic();
}

void SDLMixerMusicInterface::StopMusic(int theSongId)
{
    StopAllMusic();
}

void SDLMixerMusicInterface::StopAllMusic()
{
    Mix_HaltMusic();
}

void SDLMixerMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
    DeactivateAllMusic();

    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            aMusicInfo->mIsActive = true;
            Mix_FadeInMusicPos(aMusicInfo->music, noLoop ? 1 : -1, (int) (1.0f / theSpeed), theOffset);
            mCurrentMusic = theSongId;
        }
    }
}

void SDLMixerMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
    FadeOutAll(true, theSpeed);
}

void SDLMixerMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
    Mix_FadeOutMusic((int) (1.0f / theSpeed));
}

void SDLMixerMusicInterface::SetVolume(double theVolume)
{
    mMasterVolume = (float) theVolume;

    SDLMixerMusicMap::iterator anItr = mMusicMap.find(mCurrentMusic);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->mIsActive && Mix_PlayingMusic())
            Mix_VolumeMusic((int) (mMasterVolume * aMusicInfo->mVolume * 128.0f));
    }
}

void SDLMixerMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        aMusicInfo->mVolume = (float) theVolume;
        if (aMusicInfo->mIsActive && Mix_PlayingMusic())
            Mix_VolumeMusic((int) (mMasterVolume * aMusicInfo->mVolume * 128.0f));
    }
}

bool SDLMixerMusicInterface::IsPlaying(int theSongId)
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        return aMusicInfo->mIsActive && Mix_PlayingMusic();
    }
    return false;
}

void SDLMixerMusicInterface::Update()
{
}
