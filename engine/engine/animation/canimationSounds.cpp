#include "mcv_platform.h"
#include "canimationSounds.h"
#include "animationplugger.h"
#include "Cinematic/camera_manager.h"
#include "app.h"
#include "fmod_User/fmodStudio.h"
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
					//dbg("playing: %s at %f\n", actualSounds[i].sound.c_str(), actualSounds[i].time);
					if (Handle(this).hasBrother<CPlayerMov>()){
						fmodUser::FmodStudio::playEvent(fmodUser::FmodStudio::getEventInstance("SFX/" + actualSounds[i].sound));
					} else {
						CTransform* meT = Handle(this).getBrother<CTransform>();
						fmodUser::FmodStudio::play3DSingleEvent(
							fmodUser::FmodStudio::getEventInstance("SFX/" + actualSounds[i].sound),
							meT->getPosition());
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