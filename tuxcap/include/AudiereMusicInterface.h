#ifndef __AudiereMUSICINTERFACE_H__
#define __AudiereMUSICINTERFACE_H__

#include "MusicInterface.h"
#include "audiere.h"

#ifndef WIN32
#define HWND void*
#endif

using namespace audiere;

namespace Sexy
{

class SexyAppBase;

class AudiereMusicInfo
{
public:
	OutputStreamPtr			mStream;
	MIDIStreamPtr			mMIDIStream;
	float					mVolume;
	float					mVolumeAdd;		
	float					mVolumeCap;		
	bool					mStopOnFade;
	bool					mRepeats;
	int						mPosition;

 public:
	AudiereMusicInfo();
};

 typedef std::map<int, AudiereMusicInfo> AudiereMusicMap;

 class AudiereMusicInterface : public MusicInterface
 {
 public:	
   AudioDevicePtr			mDevice;
   MIDIDevicePtr			mMIDIDevice;
   AudiereMusicMap			mMusicMap;
   float					mMasterVolume;

public:
	AudiereMusicInterface(HWND theHWnd);
	virtual ~AudiereMusicInterface();
		
	virtual bool			LoadMusic(int theSongId, const std::string& theFileName);

	virtual void			PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false);	
	virtual void			StopMusic(int theSongId);
	virtual void			PauseMusic(int theSongId);
	virtual void			ResumeMusic(int theSongId);
	virtual void			StopAllMusic();	

	virtual void			UnloadMusic(int theSongId);
	virtual void			UnloadAllMusic();
	virtual void			PauseAllMusic();
	virtual void			ResumeAllMusic();

	virtual void			FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false);
	virtual void			FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004);
	virtual void			FadeOutAll(bool stopSong = true, double theSpeed = 0.004);
	virtual void			SetSongVolume(int theSongId, double theVolume);
	virtual bool			IsPlaying(int theSongId);

	virtual void			SetVolume(double theVolume);
	virtual void			Update();
};

}

#endif //__AudiereMUSICINTERFACE_H__

