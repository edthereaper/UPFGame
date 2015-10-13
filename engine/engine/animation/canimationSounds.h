#ifndef ANIMATION_CSOUNDS_H_
#define ANIMATION_CSOUNDS_H_

#include "mcv_platform.h"
#include "utils/itemsByName.h"

using namespace utils;

namespace animation {

	class CAnimationSounds {
	public:
		struct sounds_t {
			std::string anim = "";
			std::string sound = "";
			float time = -1.0f;
			float volume = 1.0f;
			int numsounds = 0;
			sounds_t() = default;
		};

		void loadFromProperties(const std::string& elem, MKeyValue &atts);
		void update(float elapsed);
		inline void init() {}
	private:
		std::vector<sounds_t> sounds;
		std::vector<sounds_t> actualSounds;
		int size = 0;
		int prevId = -1;
		bool plugisCycle = false;

		void soundActioner();
	};

}

#endif