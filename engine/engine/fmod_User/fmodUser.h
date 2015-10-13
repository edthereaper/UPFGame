#ifndef FMOD_USER_H_
#define FMOD_USER_H_

#include "fmod/inc/fmod.hpp"
#include "fmod/inc/fmod_errors.h"
//#include "fmod/inc/fmod_output.h"
#include <string>

#include "../app.h"
#include "../gameElements/player/playerMov.h"
using namespace gameElements;

namespace fmodUser {

	class fmodUserClass
	{
	private:
		
	public:
		static bool initFmod();
		static bool playMusic(char fileName[64], bool isLoop = false, float volume = 1.0f, float pan = 0.0f);
		static bool playSound(char fileName[64], float volume = 1.0f, float pan = 0.0f);
		static bool playAmbient3DSound(FMOD::Channel *&channel, char fileName[64], float volume = 1.0f, float pan = 0.0f, float radius = 50.0f);
		static bool updateAmbient3DSound(FMOD::Channel *channel, XMVECTOR posSource);
		static void stopAmbient3DSound(FMOD::Channel *channel);
		static bool play3DSingleSound(char fileName[64], XMVECTOR posSource, float volume = 1.0f, float pan = 0.0f, float radius = 50.0f);
		static void stopSounds();	
		static void changeReverbSFX(FMOD_REVERB_PROPERTIES p);
		static void changeReverbSFX3D(FMOD_REVERB_PROPERTIES p);
		static void updateFmod();
		static void shutdownFmod();
		static void err_descriptor(FMOD_RESULT result);
	};
}
#endif