#include "AudiereMusicInterface.h"
#include "AudiereLoader.h"
#include "SexyAppBase.h"

using namespace Sexy;
using namespace audiere;

AudiereMusicInfo::AudiereMusicInfo()
{
	mStream = NULL;
	mMIDIStream = NULL;
	mVolume = 0.0;
	mVolumeAdd = 0.0;
	mVolumeCap = 1.0;
	mStopOnFade = false;
	mRepeats = false;
	mPosition = 0;
}

AudiereMusicInterface::AudiereMusicInterface(HWND theHWnd)
{	
	mDevice = getAudiereDevice();
	mMIDIDevice = getAudiereMIDIDevice(); 
	mMasterVolume = 1.0;
}

AudiereMusicInterface::~AudiereMusicInterface()
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mStream = NULL;
		aMusicInfo->mMIDIStream = NULL;
		++anItr;
	}
}

bool AudiereMusicInterface::LoadMusic(int theSongId, const std::string& theFileName)
{
	AudiereMusicInfo aMusicInfo;

	if (theFileName.find(".mid") != std::string::npos) {

		aMusicInfo.mMIDIStream = mMIDIDevice->openStream(theFileName.c_str());
		if (aMusicInfo.mMIDIStream)
			return false;

	}
	else
	{
		aMusicInfo.mStream = OpenSound(mDevice, theFileName.c_str(), true);
		if (!aMusicInfo.mStream)
			return false;
	}

	mMusicMap.insert(AudiereMusicMap::value_type(theSongId, aMusicInfo));

	return true;
}

void AudiereMusicInterface::UnloadMusic(int theSongId)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->stop();
			aMusicInfo->mStream = NULL;
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->stop();
			aMusicInfo->mMIDIStream = NULL;
		}
		mMusicMap.erase(anItr);
	}
}

void AudiereMusicInterface::UnloadAllMusic()
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->stop();
			aMusicInfo->mStream = NULL;
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->stop();
			aMusicInfo->mMIDIStream = NULL;
		}
		++anItr;
	}
	mMusicMap.clear();
}

void AudiereMusicInterface::PauseAllMusic()
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mPosition = aMusicInfo->mStream->getPosition();
			aMusicInfo->mStream->stop();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mPosition = aMusicInfo->mMIDIStream->getPosition();
			aMusicInfo->mMIDIStream->pause();
		}
		++anItr;
	}
}

void AudiereMusicInterface::ResumeAllMusic()
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->play();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->setPosition(aMusicInfo->mPosition);
			aMusicInfo->mMIDIStream->play();
		}
		++anItr;
	}
}

void AudiereMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = aMusicInfo->mVolumeCap;
		aMusicInfo->mVolumeAdd = 0.0f;

		if (aMusicInfo->mStream) {
			OutputStreamPtr aStream = aMusicInfo->mStream;
			aStream->setVolume(float(mMasterVolume * aMusicInfo->mVolume));
			aStream->setRepeat(!noLoop);
			if (theOffset != 0)
				aStream->setPosition(theOffset);
			aStream->play();
		}
		else if (aMusicInfo->mMIDIStream) {
			MIDIStreamPtr aStream = aMusicInfo->mMIDIStream;
			aStream->setRepeat(!noLoop);
			if (theOffset != 0)
				aStream->setPosition(theOffset);
			aStream->play();		
		}
	}
}

void AudiereMusicInterface::PauseMusic(int theSongId)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mPosition = aMusicInfo->mStream->getPosition();
			aMusicInfo->mStream->stop();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mPosition = aMusicInfo->mMIDIStream->getPosition();
			aMusicInfo->mMIDIStream->pause();
		}
	}
}

void AudiereMusicInterface::ResumeMusic(int theSongId)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->play();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->setPosition(aMusicInfo->mPosition);
			aMusicInfo->mMIDIStream->play();
		}
	}
}

void AudiereMusicInterface::StopMusic(int theSongId)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->stop();
			aMusicInfo->mStream->reset();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->stop();
		}
		aMusicInfo->mPosition = 0;
	}
}

void AudiereMusicInterface::StopAllMusic()
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = 0.0;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->stop();
			aMusicInfo->mStream->reset();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->stop();
		}
		aMusicInfo->mPosition = 0;
		++anItr;
	}
}

void AudiereMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolumeAdd = (float)theSpeed * 10.0f;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->setVolume(float(mMasterVolume * aMusicInfo->mVolume));
			aMusicInfo->mStream->setRepeat(!noLoop);
			if (theOffset != 0)
				aMusicInfo->mStream->setPosition(theOffset);
			aMusicInfo->mStream->play();
		}
		else if (aMusicInfo->mMIDIStream) {
			aMusicInfo->mMIDIStream->setRepeat(!noLoop);
			if (theOffset != 0)
				aMusicInfo->mMIDIStream->setPosition(theOffset);
			aMusicInfo->mMIDIStream->play();
		}
	}
}

void AudiereMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolumeAdd = -(float)theSpeed * 10.0f;
		aMusicInfo->mStopOnFade = stopSong;
	}
}

void AudiereMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolumeAdd = -(float)theSpeed * 10.0f;
		aMusicInfo->mStopOnFade = stopSong;
		++anItr;
	}
}

void AudiereMusicInterface::SetVolume(double theVolume)
{
	mMasterVolume = (float)theVolume;

	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;						
		if (aMusicInfo->mStream) { 
                  //			aMusicInfo->mStream->stop();
                  //aMusicInfo->mStream->reset();
			aMusicInfo->mStream->setVolume(mMasterVolume * aMusicInfo->mVolume);
		}
		++anItr;
	}	
}

void AudiereMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{		
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = (float)theVolume;
		if (aMusicInfo->mStream) { 
			aMusicInfo->mStream->setVolume(mMasterVolume * aMusicInfo->mVolume);
		}
	}
}

bool AudiereMusicInterface::IsPlaying(int theSongId)
{
	AudiereMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;
		if (aMusicInfo->mStream) { 
			return aMusicInfo->mStream->isPlaying();
		}
		else if (aMusicInfo->mMIDIStream) {
			return aMusicInfo->mMIDIStream->isPlaying();
		}
	}
	return false;	
}

void AudiereMusicInterface::Update()
{
	AudiereMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		AudiereMusicInfo* aMusicInfo = &anItr->second;

		if (aMusicInfo->mVolumeAdd != 0.0)
		{
			aMusicInfo->mVolume += aMusicInfo->mVolumeAdd;
			
			if (aMusicInfo->mVolume > aMusicInfo->mVolumeCap)
			{
				aMusicInfo->mVolume = aMusicInfo->mVolumeCap;
				aMusicInfo->mVolumeAdd = 0.0;
			}
			else if (aMusicInfo->mVolume < 0.0)
			{
				aMusicInfo->mVolume = 0.0;
				aMusicInfo->mVolumeAdd = 0.0;

				if (aMusicInfo->mStopOnFade)
				{
					if (aMusicInfo->mStream) {
						if (aMusicInfo->mStream->isPlaying())
						{
							aMusicInfo->mStream->stop();
							aMusicInfo->mStream->reset();
						}
					}
					else if (aMusicInfo->mMIDIStream) {
						if (aMusicInfo->mMIDIStream->isPlaying())
						{
							aMusicInfo->mMIDIStream->stop();
						}
					}
				}
			}
				
			if (aMusicInfo->mStream) 
				aMusicInfo->mStream->setVolume(mMasterVolume * aMusicInfo->mVolume);
		}

		++anItr;
	}
	mDevice->update();
}

