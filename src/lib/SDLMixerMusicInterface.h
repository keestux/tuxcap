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

#ifndef __SDLMIXERMUSICINTERFACE_H__
#define __SDLMIXERMUSICINTERFACE_H__

#include "MusicInterface.h"
#include <SDL_mixer.h>

#ifndef WIN32
#define HWND void*
#endif

namespace Sexy {

class SexyAppBase;

class SDLMixerMusicInfo {
public:
    Mix_Music*              music;
    float                   mVolume;
    bool                    mStopOnFade;
    bool                    mRepeats;
    int                     mPosition;
    bool                    mIsActive;
    Uint8*                  mBuffer; //needed because ogg and mp3 are streamed from buffer and not read in at once

public:
    SDLMixerMusicInfo();
};

typedef std::map<int, SDLMixerMusicInfo> SDLMixerMusicMap;

class SDLMixerMusicInterface : public MusicInterface {
public:
    SDLMixerMusicMap        mMusicMap;
    float                   mMasterVolume;
    int                     mCurrentMusic;

public:
    SDLMixerMusicInterface(HWND theHWnd);
    virtual ~SDLMixerMusicInterface();

    virtual bool            LoadMusic(int theSongId, const std::string& theFileName);

    virtual void            PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false);
    virtual void            StopMusic(int theSongId);
    virtual void            PauseMusic(int theSongId);
    virtual void            ResumeMusic(int theSongId);
    virtual void            StopAllMusic();

    virtual void            UnloadMusic(int theSongId);
    virtual void            UnloadAllMusic();
    virtual void            PauseAllMusic();
    virtual void            ResumeAllMusic();

    virtual void            FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false);
    virtual void            FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004);
    virtual void            FadeOutAll(bool stopSong = true, double theSpeed = 0.004);
    virtual void            SetSongVolume(int theSongId, double theVolume);
    virtual bool            IsPlaying(int theSongId);

    virtual void            SetVolume(double theVolume);
    virtual void            Update();

    void                    DeactivateAllMusic();
};

}

#endif //__SDLMIXERMUSICINTERFACE_H__
