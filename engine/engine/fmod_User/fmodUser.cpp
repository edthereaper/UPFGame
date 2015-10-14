#include "mcv_platform.h"
#include "fmodUser.h"

using namespace DirectX;

#define TIME_FADE_IN_MUSIC 5
#define TIME_FADE_OUT_MUSIC 5

namespace fmodUser {

static FMOD::System			*musicsystemFmod = NULL;		
static FMOD::System			*sfxsystemFmod = NULL;	
static FMOD::System			*sfx3DsystemFmod = NULL;	
static FMOD::Channel		*musicChannelFmod = NULL;
static FMOD::Sound			*musicSoundFmod = NULL;
static void					*extradriverdata = 0;
static FMOD_RESULT			resultFmod;

bool fmodUserClass::initFmod()

{
    resultFmod = FMOD::System_Create(&musicsystemFmod);
	resultFmod = FMOD::System_Create(&sfxsystemFmod);
	resultFmod = FMOD::System_Create(&sfx3DsystemFmod);
	resultFmod = musicsystemFmod->init(32, FMOD_INIT_NORMAL, extradriverdata);		//32 channels maximum
	resultFmod = sfxsystemFmod->init(32, FMOD_INIT_NORMAL, extradriverdata);		//32 channels maximum
	resultFmod = sfx3DsystemFmod->init(32, FMOD_INIT_NORMAL, extradriverdata);		//32 channels maximum
	resultFmod = sfx3DsystemFmod->set3DSettings(1.0, 1.0f, 1.0f);
	//Initial reverb set for the sfx
	FMOD_REVERB_PROPERTIES propon = FMOD_PRESET_AUDITORIUM;
	resultFmod = sfxsystemFmod->setReverbProperties(0, &propon);
	//Initial reverb set for the sfx3D
	propon = FMOD_PRESET_OFF;
	resultFmod = sfx3DsystemFmod->setReverbProperties(0, &propon);

	if (resultFmod == FMOD_OK){
		return true;
	}
	return false;
}

bool fmodUserClass::playMusic(char fileName[64], bool isLoop, float volume, float pan)
{
	char fileRoute[128] = "data/music/";
	strcat(fileRoute, fileName);
	//Necessary values for fadeIn & fadeOut
	unsigned long long parentclock;
	resultFmod = musicChannelFmod->getDSPClock(NULL, &parentclock);
	int rate;
	musicsystemFmod->getSoftwareFormat(&rate, 0, 0);
	//FadeOut
	musicChannelFmod->addFadePoint(parentclock, 1.0f);
	musicChannelFmod->addFadePoint(parentclock + (rate * TIME_FADE_IN_MUSIC), 0.0f);
	if (isLoop){
		resultFmod = musicsystemFmod->createSound(fileRoute, FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &musicSoundFmod);
	}
	else{
		resultFmod = musicsystemFmod->createSound(fileRoute, FMOD_DEFAULT | FMOD_LOOP_OFF, 0, &musicSoundFmod);
	}
	resultFmod = musicsystemFmod->playSound(musicSoundFmod, 0, false, &musicChannelFmod);
	resultFmod = musicChannelFmod->setVolume(volume);
	resultFmod = musicChannelFmod->setPan(pan);
	//FadeIn
	musicChannelFmod->addFadePoint(parentclock, 0.0f);
	musicChannelFmod->addFadePoint(parentclock + (rate * 10), 1.0f);

	if (resultFmod == FMOD_OK){
		return true;
	}
	return false;
}

bool fmodUserClass::playSound(char fileName[64], float volume, float pan)
{
	FMOD::Channel *channelFmod = NULL;
	FMOD::Sound	*musicFmod = NULL;
	char fileRoute[128] = "data/sounds/";
	strcat(fileRoute, fileName);
	strcat(fileRoute, ".wav");
	resultFmod = sfxsystemFmod->createStream(fileRoute, FMOD_DEFAULT | FMOD_LOOP_OFF, 0, &musicFmod);
	resultFmod = sfxsystemFmod->playSound(musicFmod, 0, false, &channelFmod);
	resultFmod = channelFmod->setVolume(volume);
	resultFmod = channelFmod->setPan(pan);

	if (resultFmod == FMOD_OK){
		return true;
	}
	else{
		err_descriptor(resultFmod);
	}
	return false;
}

bool fmodUserClass::playAmbient3DSound(FMOD::Channel *&channel, char fileName[64], float volume, float pan, float radius)
{
	FMOD::Sound	*sfxFmod = NULL;
	char fileRoute[128] = "data/sounds/";
	strcat(fileRoute, fileName);
	strcat(fileRoute, ".wav");
	resultFmod = sfx3DsystemFmod->createStream(fileRoute, FMOD_3D_LINEARSQUAREROLLOFF | FMOD_LOOP_NORMAL, 0, &sfxFmod);
	resultFmod = sfxFmod->set3DMinMaxDistance(0.5f, radius);
	resultFmod = sfx3DsystemFmod->playSound(sfxFmod, 0, false, &channel);
	FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
	resultFmod = channel->set3DAttributes(&pos, &vel);
	resultFmod = channel->setVolume(volume);
	resultFmod = channel->setPan(pan);
	//To abool lisening 3d sounds in the 1st frame
	FMOD_VECTOR listenerpos	   = { 1000000000.0f, 1000000000.0f, 1000000000.0f };
	FMOD_VECTOR forward        = { 0.0f, 1.0f, 0.0f };
    FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };
	resultFmod = sfx3DsystemFmod->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);
	resultFmod = sfx3DsystemFmod->update();	

	if (resultFmod == FMOD_OK){
		return true;
	}
	return false;
}

bool fmodUserClass::updateAmbient3DSound(FMOD::Channel *channel, XMVECTOR posSource)
{
	CTransform* playerT = App::get().getPlayer().getSon<CTransform>();
	XMVECTOR posTarget = playerT->getPosition();
	XMVECTOR frontTarget = playerT->getFront();
	FMOD_VECTOR pos			   = { XMVectorGetX(posSource), XMVectorGetY(posSource), XMVectorGetZ(posSource) };
	FMOD_VECTOR listenerpos	   = { XMVectorGetX(posTarget), XMVectorGetY(posTarget), XMVectorGetZ(posTarget) };
	FMOD_VECTOR forward        = { XMVectorGetX(frontTarget), XMVectorGetY(frontTarget), XMVectorGetZ(frontTarget) };
    FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };
	FMOD_VECTOR vel            = { 0.0f, 0.0f, 0.0f };
	resultFmod = channel->set3DAttributes(&pos, &vel);
	resultFmod = sfx3DsystemFmod->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);
	resultFmod = sfx3DsystemFmod->update();

	if (resultFmod == FMOD_OK){
		return true;
	}
	return false;
}

void fmodUserClass::stopAmbient3DSound(FMOD::Channel *channel){
	channel->setVolume(0.0f);
	channel->stop();
	channel = NULL;
}

bool fmodUserClass::play3DSingleSound(char fileName[64], XMVECTOR posSource, float volume, float pan, float radius){
	FMOD::Sound	*sfxFmod = NULL;
	FMOD::Channel *channelFmod = NULL;
	CTransform* playerT = App::get().getPlayer().getSon<CTransform>();
	XMVECTOR posTarget = playerT->getPosition();
	XMVECTOR frontTarget = playerT->getFront();
	char fileRoute[128] = "data/sounds/";
	strcat(fileRoute, fileName);
	strcat(fileRoute, ".wav");
	resultFmod = sfx3DsystemFmod->createStream(fileRoute, FMOD_3D_LINEARSQUAREROLLOFF, 0, &sfxFmod);
	resultFmod = sfxFmod->set3DMinMaxDistance(0.5f, radius);
	resultFmod = sfx3DsystemFmod->playSound(sfxFmod, 0, false, &channelFmod);
	FMOD_VECTOR pos = { XMVectorGetX(posSource), XMVectorGetY(posSource), XMVectorGetZ(posSource) };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
	resultFmod = channelFmod->set3DAttributes(&pos, &vel);
	resultFmod = channelFmod->setVolume(volume);
	resultFmod = channelFmod->setPan(pan);
	FMOD_VECTOR listenerpos	   = { XMVectorGetX(posTarget), XMVectorGetY(posTarget), XMVectorGetZ(posTarget) };
	FMOD_VECTOR forward        = { XMVectorGetX(frontTarget), XMVectorGetY(frontTarget), XMVectorGetZ(frontTarget) };
    FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };
	resultFmod = sfx3DsystemFmod->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);
	resultFmod = sfx3DsystemFmod->update();	
	if (resultFmod == FMOD_OK){
		return true;
	}
	return false;
}

void fmodUserClass::stopSounds(){
	resultFmod = sfx3DsystemFmod->close();
	resultFmod = sfxsystemFmod->close();
	resultFmod = sfx3DsystemFmod->init(32, FMOD_INIT_NORMAL, extradriverdata);		//32 channels maximum
	resultFmod = sfxsystemFmod->init(32, FMOD_INIT_NORMAL, extradriverdata);		//32 channels maximum
	FMOD_REVERB_PROPERTIES propon = FMOD_PRESET_AUDITORIUM;
	resultFmod = sfxsystemFmod->setReverbProperties(0, &propon);
	propon = FMOD_PRESET_OFF;
	resultFmod = sfx3DsystemFmod->setReverbProperties(0, &propon);
}

void fmodUserClass::changeReverbSFX(FMOD_REVERB_PROPERTIES p){
	resultFmod = sfxsystemFmod->setReverbProperties(0, &p);
	sfxsystemFmod->update();
}

void fmodUserClass::changeReverbSFX3D(FMOD_REVERB_PROPERTIES p){
	resultFmod = sfx3DsystemFmod->setReverbProperties(0, &p);
	sfx3DsystemFmod->update();
}

void fmodUserClass::updateFmod()
{
	musicsystemFmod->update();
	sfxsystemFmod->update();
	sfx3DsystemFmod->update();
}

void fmodUserClass::shutdownFmod()
{
	resultFmod = sfxsystemFmod->close();
	resultFmod = sfxsystemFmod->release();
	resultFmod = sfx3DsystemFmod->close();
	resultFmod = sfx3DsystemFmod->release();
	resultFmod = musicsystemFmod->close();
	resultFmod = musicsystemFmod->release();
}

void fmodUserClass::err_descriptor(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		utils::dbg("FMOD error!(%d) % s\n", result, FMOD_ErrorString(result));
	}
}

}