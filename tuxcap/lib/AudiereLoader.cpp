#include "AudiereLoader.h"
#include "audiere.h"

audiere::AudioDevicePtr gAudiereDevice;
audiere::MIDIDevicePtr gAudiereMIDIDevice;

audiere::AudioDevicePtr getAudiereDevice(void) {
  if (!gAudiereDevice)
		gAudiereDevice = audiere::OpenDevice(NULL); 
	return gAudiereDevice;
}

audiere::MIDIDevicePtr getAudiereMIDIDevice(void) {
  if (!gAudiereMIDIDevice)
		gAudiereMIDIDevice = audiere::OpenMIDIDevice(NULL); 
	return gAudiereMIDIDevice;
}
 

