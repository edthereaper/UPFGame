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

#include "fmod_User/fmodStudio.h"
using namespace fmodUser;

using namespace utils;
using namespace DirectX;
using namespace component;

namespace gameElements {

	class CAmbientSound {
	private:
		std::string filename = "";
		FMOD::Studio::EventInstance* instanceFmod = NULL;
		Handle playerEntity;

	public:
		CAmbientSound() {}

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
			filename = atts.getString("file", "");
		}

		void init(){}
		void update(float);
		void playSound();
		void stopSound();
		void pauseSound();
		void resumeSound();
	};

}
#endif