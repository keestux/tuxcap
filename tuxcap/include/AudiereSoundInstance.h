#ifndef __AudiereSoundINSTANCE_H__
#define __AudiereSoundINSTANCE_H__

#include "SoundInstance.h"
#include "audiere.h"

using namespace audiere;

namespace Sexy
{

class AudiereSoundManager;

class AudiereSoundInstance : public SoundInstance
{
	friend class AudiereSoundManager;

protected:
	AudiereSoundManager*	mAudiereSoundManagerP;

	OutputStreamPtr			mStream;
	bool					mAutoRelease;
	bool					mReleased;

	float					mBasePan;
	float					mBaseVolume;
	float					mBasePitch;

	float					mPan;
	float					mVolume;
	float					mPitch;

	bool					mHasPlayed;

protected:
	void					RehupVolume();
	void					RehupPan();
	void					RehupPitch();

public:
	AudiereSoundInstance(AudiereSoundManager* theSoundManager, SampleSourcePtr theSourceSound);
	virtual ~AudiereSoundInstance();	
	
	virtual void			Release();
	
	virtual void			SetBaseVolume(double theBaseVolume); //0.0 to 1.0
	virtual void			SetBasePan(int theBasePan); //-100 to +100
	virtual void			AdjustBasePitch(float thePitch); //+0.5 to +2.0 relative to normal playing speed

	virtual void			SetVolume(double theVolume); //0.0 to 1.0
	virtual void			SetPan(int thePosition); //-100 to +100 = left to right	
	virtual void			AdjustPitch(double thePitch); //+0.5 to +2.0 relative to normal playing speed

	virtual bool			Play(bool looping, bool autoRelease);
	virtual void			Stop();
	virtual bool			IsPlaying();
	virtual bool			IsReleased();
	virtual double			GetVolume();

};

}

#endif //__AudiereSoundINSTANCE_H__

