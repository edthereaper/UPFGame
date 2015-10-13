#ifndef GAME_ELEMENTS_CANNONPATH_H_
#define GAME_ELEMENTS_CANNONPATH_H_

#include "handles/entity.h"

namespace gameElements {

	class CCannonPath {
	public:
		static const float GRAVITY;
	private:
		utils::Counter<float> ttlTimer;
		XMVECTOR xzvel;
		XMVECTOR yvel;
	public:
		CCannonPath() { ttlTimer.reset(); }
		void setup(XMVECTOR xzvelocity, XMVECTOR yvelocity);
		void removeFromScene();
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed);
		inline void init() {}
	};

}

#endif