#include "mcv_platform.h"
#include "ambientsound.h"

namespace gameElements {

	void CAmbientSound::update(float)
	{
		Entity* e(Handle(this).getOwner());
		if (e != nullptr) {
			CTransform* t = e->get<CTransform>();
			fmodUser::FmodStudio::update3DAmbientEvent(instanceFmod, t->getPosition());
		}
	}

	void CAmbientSound::playSound()
	{
		Entity* e(Handle(this).getOwner());
		if (e != nullptr) {
			CTransform* t = e->get<CTransform>();
			instanceFmod = fmodUser::FmodStudio::getEventInstance("SFX/" + filename);
			fmodUser::FmodStudio::play3DAmbientEvent(instanceFmod, t->getPosition());
		}
	}

	void CAmbientSound::stopSound()
	{
		fmodUser::FmodStudio::stopEvent(instanceFmod);
	}

	void CAmbientSound::pauseSound()
	{
		fmodUser::FmodStudio::pauseEvent(instanceFmod);
	}

	void CAmbientSound::resumeSound()
	{
		fmodUser::FmodStudio::resumeEvent(instanceFmod);
	}


}