/* Original Audiere Sound Manager  by Rheenen 2005 */

#include "AudiereMusicInterface.h"
#include "AudiereSoundManager.h"
#include "AudiereSoundInstance.h"
#include "AudiereLoader.h"
#include "audiere.h"

using namespace Sexy;
using namespace audiere;

AudiereSoundManager::AudiereSoundManager()
{
	mDevice = getAudiereDevice();

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
}

AudiereSoundManager::~AudiereSoundManager()
{
	ReleaseChannels();
	ReleaseSounds();
}

int	AudiereSoundManager::FindFreeChannel()
{
  Uint32 aTick = SDL_GetTicks();
	if (aTick-mLastReleaseTick > 1000)
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
			mPlayingSounds[i] = NULL;
			return i;
		}
	}
	
	return -1;
}

bool AudiereSoundManager::Initialized()
{
  return (mDevice.get() != NULL);
}

void AudiereSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = (float)theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

int AudiereSoundManager::LoadSound(const std::string& theFilename) {
	int id = GetFreeSoundId();

	if (id != -1) {
		if (LoadSound(id, theFilename)) {
			return id;
		}
	}
	return -1;
}

bool AudiereSoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

	if (!mDevice)
		return true; // sounds just	won't play, but this is not treated as a failure condition

	std::string aFilename = theFilename;

	int aLastDotPos = aFilename.rfind('.');
	int aLastSlashPos = std::max((int) aFilename.rfind('\\'), (int) aFilename.rfind('/'));	
	if (aLastSlashPos < 0)
		aLastSlashPos = 0;

	mSourceSounds[theSfxID] = OpenSampleSource(aFilename.c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = OpenSampleSource((aFilename + ".wav").c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = OpenSampleSource((aFilename + ".ogg").c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = OpenSampleSource((aFilename + ".mp3").c_str());

	return (mSourceSounds[theSfxID]);
}

void AudiereSoundManager::ReleaseSound(unsigned int theSfxID)
{
  mSourceSounds[theSfxID] = NULL;
}

bool AudiereSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = (float)theBaseVolume;
	return true;
}

bool AudiereSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

bool AudiereSoundManager::SetBasePitch(unsigned int theSfxID, float theBasePitch)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePitches[theSfxID] = theBasePitch;
	return true;
}

SoundInstance* AudiereSoundManager::GetSoundInstance(unsigned int theSfxID)
{
	if (theSfxID > MAX_SOURCE_SOUNDS)
		return NULL;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return NULL;


	if (!mDevice)
	{
		mPlayingSounds[aFreeChannel] = new AudiereSoundInstance(this, NULL);
	}
	else
	{
		if (!mSourceSounds[theSfxID])
			return NULL;
		mPlayingSounds[aFreeChannel] = new AudiereSoundInstance(this, mSourceSounds[theSfxID]);
	}

	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);
	mPlayingSounds[aFreeChannel]->AdjustBasePitch(mBasePitches[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void AudiereSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceSounds[i])
			mSourceSounds[i] = NULL;
}

void AudiereSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i])
		{
			mPlayingSounds[i]->Release();
			mPlayingSounds[i] = NULL;
		}
}

void AudiereSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] && mPlayingSounds[i]->IsReleased())
			mPlayingSounds[i] = NULL;
}

void AudiereSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
}


double AudiereSoundManager::GetMasterVolume()
{
#if 0
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u;
	MIXERLINECONTROLS mxlc;
	MIXERCONTROL mlct;
	MIXERLINE mixerLine;
	HMIXER hmx;
	MIXERCAPS pmxcaps;	

	mixerOpen((HMIXER*) &hmx, 0, 0, 0, MIXER_OBJECTF_MIXER);
	mixerGetDevCaps(0, &pmxcaps, sizeof(pmxcaps));

	mxlc.cbStruct = sizeof(mxlc);	
	mxlc.cbmxctrl = sizeof(mlct);
	mxlc.pamxctrl = &mlct;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mixerLine.cbStruct = sizeof(mixerLine);
	mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
	mixerGetLineInfo((HMIXEROBJ) hmx, &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE);
	mxlc.dwLineID = mixerLine.dwLineID;
	mixerGetLineControls((HMIXEROBJ) hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);	

	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = mlct.dwControlID;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(mxcd_u);
	mcd.paDetails = &mxcd_u;
		
	mixerGetControlDetails((HMIXEROBJ) hmx, &mcd, 0L);	

	mixerClose(hmx);

	return mxcd_u.dwValue / (double) 0xFFFF;
#else
        return 0.0;
#endif
        
}

void AudiereSoundManager::SetMasterVolume(double theVolume)
{
#if 0
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u;
	MIXERLINECONTROLS mxlc;
	MIXERCONTROL mlct;
	MIXERLINE mixerLine;
	HMIXER hmx;
	MIXERCAPS pmxcaps;	

	mixerOpen((HMIXER*) &hmx, 0, 0, 0, MIXER_OBJECTF_MIXER);
	mixerGetDevCaps(0, &pmxcaps, sizeof(pmxcaps));

	mxlc.cbStruct = sizeof(mxlc);	
	mxlc.cbmxctrl = sizeof(mlct);
	mxlc.pamxctrl = &mlct;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mixerLine.cbStruct = sizeof(mixerLine);
	mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
	mixerGetLineInfo((HMIXEROBJ) hmx, &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE);
	mxlc.dwLineID = mixerLine.dwLineID;
	mixerGetLineControls((HMIXEROBJ) hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);	

	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = mlct.dwControlID;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(mxcd_u);
	mcd.paDetails = &mxcd_u;
	
	mxcd_u.dwValue = (int) (0xFFFF * theVolume);
	mixerSetControlDetails((HMIXEROBJ) hmx, &mcd, 0L);

	mixerClose(hmx);
#endif
}

void AudiereSoundManager::Flush()
{
}

void AudiereSoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed)
{
}

int	AudiereSoundManager::GetFreeSoundId() {
	for (int i = 0; i < MAX_SOURCE_SOUNDS; ++i) {
		if (!mSourceSounds[i]) {
			return i;
		}
	}
	return -1;
}

int AudiereSoundManager::GetNumSounds() {
	int nr_sounds = 0;

	for (int i = 0; i < MAX_SOURCE_SOUNDS; ++i) {
		if (mSourceSounds[i]) {
			++nr_sounds;
		}
	}
	return nr_sounds;
}


#undef SOUND_FLAGS

