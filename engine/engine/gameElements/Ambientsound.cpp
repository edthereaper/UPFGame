#include "mcv_platform.h"
#include "ambientsound.h"

namespace gameElements {
    
void CAmbientSound::update(float)
{
	Entity* e(Handle(this).getOwner());
    if (e != nullptr) {
	    CTransform* t = e->get<CTransform>();
	    fmodUser::fmodUserClass::updateAmbient3DSound(channelFmod, t->getPosition());
    }
}

void CAmbientSound::playSound()
{	
	playerEntity = App::get().getPlayer();
	fmodUser::fmodUserClass::playAmbient3DSound(channelFmod, (char*)filename.c_str(), volume, pan, radius);
}

void CAmbientSound::stopSound()
{	
	fmodUser::fmodUserClass::stopAmbient3DSound(channelFmod);
}

}