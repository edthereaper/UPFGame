#ifndef GAME_ELEMENTS_BULLET_H_
#define GAME_ELEMENTS_BULLET_H_

#include "handles/message.h"
#include "handles/entity.h"

#include "physX_USER/PhysicsManager.h"

namespace gameElements {

class CBullet {
    public:
        static const float BULLET_SPEED;
        static const float BULLET_SPEED_MEGA;

        static void initType();

    private:
        bool mega = false;
        bool finishEffectDone = true;
        utils::Counter<float> ttlTimer, timer;
		XMVECTOR prevPos = utils::zero_v;
		bool didSound = false;

		float gravity = 0.0f;
		bool once = true;
		float ttl;
		float life = 0.0f;
    public:
		void removeFromScene(); //TODO: private

        CBullet() {ttlTimer.reset();}
        ~CBullet() {
            if (!finishEffectDone && !component::Handle::onCleanup()) {
                finishEffect();
            }
        }
        void setup(XMVECTOR origin, XMVECTOR velocity, XMVECTOR rotQ);
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        void update(float elapsed);
		inline bool isMega() { return mega; }
        inline void init() {}
		void finishEffect();

        void receive(const physX_user::MsgCollisionEvent&);
};

}

#endif