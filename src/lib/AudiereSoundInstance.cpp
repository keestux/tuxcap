/* Original Audiere Sound Instance class  by Rheenen 2005 */

#include "AudiereMusicInterface.h"
#include "AudiereSoundInstance.h"
#include "AudiereSoundManager.h"
#include "audiere.h"

using namespace Sexy;
using namespace audiere;

AudiereSoundInstance::AudiereSoundInstance(AudiereSoundManager* theSoundManager, SampleSourcePtr theSourceSound)
{
    mAudiereSoundManagerP = theSoundManager;
    
    mStream = mAudiereSoundManagerP->mDevice->openStream(theSourceSound.get());

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

AudiereSoundInstance::~AudiereSoundInstance()
{
    Release();
    mStream = NULL;
}

void AudiereSoundInstance::RehupVolume()
{
    if (mStream)
        mStream->setVolume(mBaseVolume * mVolume * mAudiereSoundManagerP->mMasterVolume);
}

void AudiereSoundInstance::RehupPan()
{
    if (mStream) {
          float aPan = /*float(mBasePan/100) * */float(mPan/100);
        if (aPan > 1.0)
            mStream->setPan(1.0);
        else if (aPan < -1.0)
            mStream->setPan(-1.0);
        else
            mStream->setPan(aPan);
    }
}

void AudiereSoundInstance::RehupPitch()
{
    if (mStream)
        mStream->setPitchShift(mBasePitch * mPitch);
}

void AudiereSoundInstance::Release()
{
    Stop();
    mStream = NULL;
    mReleased = true;           
}

void AudiereSoundInstance::SetVolume(double theVolume) // 0.0 to 1.0
{
    mVolume = (float)theVolume;
    RehupVolume();  
}

void AudiereSoundInstance::SetPan(int thePosition) //-100 to +100 = left to right
{
    mPan = float(thePosition/100);
    RehupPan(); 
}

void AudiereSoundInstance::AdjustPitch(double theNumSteps) //+0.5 to +2.0 = lower to higher
{
    mPitch = (float)theNumSteps;
    RehupPitch();
}

void AudiereSoundInstance::SetBaseVolume(double theBaseVolume)
{
    mBaseVolume = (float)theBaseVolume;
    RehupVolume();
}

void AudiereSoundInstance::SetBasePan(int theBasePan)
{
    mBasePan = float(theBasePan/100);
    RehupPan();
}

void AudiereSoundInstance::AdjustBasePitch(float thePitch)
{
    mBasePitch = thePitch;
    RehupPitch();
}

bool AudiereSoundInstance::Play(bool looping, bool autoRelease)
{
    Stop();

    mAutoRelease = autoRelease; 

    if (!mStream)
        return false;
    
    mStream->setRepeat(looping);
    mStream->play();
    mHasPlayed = true;
    return true;
}

void AudiereSoundInstance::Stop()
{
    if (mStream)
    {
        mStream->stop();
        mStream->setPosition(0);
        mAutoRelease = false;
    }
}

bool AudiereSoundInstance::IsPlaying()
{
  return (mStream.get() != NULL) && (mStream->isPlaying());
}

bool AudiereSoundInstance::IsReleased()
{
    if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying()))   
        Release();  

    return mReleased;
}

double AudiereSoundInstance::GetVolume()
{
    return mVolume; 
}

