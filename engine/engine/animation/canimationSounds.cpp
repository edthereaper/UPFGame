#include "mcv_platform.h"
#include "canimationSounds.h"
#include "animationplugger.h"
#include "Cinematic/camera_manager.h"
#include "app.h"
#include "fmod_User/fmodUser.h"
#include "handles/entity.h"
#include "handles/handle.h"
#include "handles/prefab.h"
#include "../gameElements/player/playerMov.h"
using namespace gameElements;
using namespace component;
using namespace utils;
using namespace fmodUser;

namespace animation {

	void CAnimationSounds::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
	{
		if (elem == "animAudio") {
			sounds_t currentSound;
			if (atts.has("anim"))		{ currentSound.anim = atts.getString("anim", ""); }
			if (atts.has("sound"))		{ currentSound.sound = atts.getString("sound", ""); }
			if (atts.has("time"))		{ currentSound.time = atts.getFloat("time", 0.0f); }
			if (atts.has("volume"))		{ currentSound.volume = atts.getFloat("volume", 1.0f); }
			if (atts.has("numsounds"))	{ currentSound.numsounds = atts.getInt("numsounds", 0); }
			//dbg("loaded: %s\n", currentSound.sound.c_str());
			sounds.push_back(currentSound);
		}
	}

	void CAnimationSounds::soundActioner()
	{

		if (cinematic::CameraManager::get().isPlayerCam()){

			CAnimationPlugger* aniP = Handle(this).getBrother<CAnimationPlugger>();
			size = (int)actualSounds.size();
			for (auto i = 0; i < size; ++i) {
				if (actualSounds[i].sound != "" && aniP->getAnimationElapsed() > actualSounds[i].time){
					char* cstr = (char*)actualSounds[i].sound.c_str();
					if (actualSounds[i].numsounds > 0){
						int randomV = rand_uniform(actualSounds[i].numsounds, 1);
						char randomC[32] = "";
						sprintf(randomC, "%d", randomV);
						strcat(cstr, randomC);
					}
					//dbg("playing: %s at %f\n", cstr, actualSounds[i].time);
					if (Handle(this).hasBrother<CPlayerMov>()){
						fmodUser::fmodUserClass::playSound(cstr, actualSounds[i].volume, 0.0f);
					}
					else{
						CTransform* meT = Handle(this).getBrother<CTransform>();
						fmodUser::fmodUserClass::play3DSingleSound(cstr, meT->getPosition(), actualSounds[i].volume, 0.0f);
					}
					actualSounds[i] = sounds_t();
					size--;
				}
			}
		}
	}

	void CAnimationSounds::update(float elapsed)
	{
		if (Handle(this).hasBrother<CAnimationPlugger>()){
			CAnimationPlugger* aniP = Handle(this).getBrother<CAnimationPlugger>();
			aniP->setAnimationElapsed(elapsed);
			if (!actualSounds.empty()) soundActioner();
			//dbg("%f / %f\n", aniP->getAnimationElapsed(), aniP->getActualPlugDuration());
			if (!aniP->getActualPlug().unplug && aniP->getActualPlug().plugId != prevId ||
				(aniP->getActualPlug().type == AnimationArchetype::MAIN_CYCLE && aniP->getAnimationElapsed() > aniP->getActualPlugDuration()) ||
				(aniP->getActualPlug().fakeCycle && aniP->getAnimationElapsed() > aniP->getPreviousPlugDuration())){
				//dbg("anim: %i, time: %f\n", aniP->getPreviousPlug().plugId, aniP->getPreviousPlugDuration());
				//if (aniP->getActualPlug().plugId == 2) dbg("time: %f\n", aniP->getActualPlugDuration());
				actualSounds.clear();
				aniP->resetAnimationElapsed();
				prevId = aniP->getActualPlug().plugId;
				for (auto i = 0; i<sounds.size(); ++i) {
					if (sounds[i].anim == aniP->getActualPlug().name) {
						float backupTime = sounds[i].time;
						sounds[i].time = sounds[i].time / aniP->getActualPlug().factor;
						actualSounds.push_back(sounds[i]);
						sounds[i].time = backupTime;
						//dbg("loaded: %s at %f\n", sounds[i].sound.c_str(), sounds[i].time);
					}
				}
			}
		}
	}
}