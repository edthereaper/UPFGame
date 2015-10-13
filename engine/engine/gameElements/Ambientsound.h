#ifndef GAMELEMENTS_AMBIENT_SOUND
#define GAMELEMENTS_AMBIENT_SOUND

#include "mcv_platform.h"
#include "components/color.h"
#include "gameMsgs.h"
#include "app.h"
#include "handles/handle.h"
#include "components/transform.h"
#include "../render/camera/component.h"
#include "../render/render_utils.h"   

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "fmod_User/fmodUser.h"
using namespace fmodUser;

using namespace utils;
using namespace DirectX;
using namespace component;

namespace gameElements {

class CAmbientSound {
    private:
        std::string filename = ""; 
		float volume = 1.0f;
		float pan = 0.0f;
		float radius = 50.0f;
		FMOD::Channel *channelFmod = NULL;
		Handle playerEntity;

    public:
        CAmbientSound() {}
    
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            filename		= atts.getString("file", "");
            volume			= atts.getFloat("volume", 0.0f);
			pan				= atts.getFloat("pan", 0.0f);
			radius			= atts.getFloat("radius", 50.0f);
        }
    
        void drawVolume();

        void init(){}
        void update(float);
		void playSound();
		void stopSound();
};

}
#endif
