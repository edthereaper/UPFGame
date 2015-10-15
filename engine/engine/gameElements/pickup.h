#ifndef GAME_ELEMENTS_PICKUP_H_
#define GAME_ELEMENTS_PICKUP_H_

#include "gameElements/gameMsgs.h"

#include "mcv_platform.h"
#include "handles/entity.h"

using namespace component;

namespace gameElements {

class CPickup {
    public:
        enum type_e {
	        NONE=0,
	        HEALTH=1,
	        ENERGY=2,
	        INVINCIBLE=3,
			COIN = 4,
			COLLECTABLE=5,
        };
        
        static const component::EntityListManager::key_t TAG = 0xF000000D;

    private:
        type_e type;
        float strength = 1.0f;
        bool stationary = false;
        bool setupDone = false;
        bool dies = false;
		bool stationaryUsed = false;
        utils::Counter<float> ttlTimer;
		Handle playerEntity;
		float lifeTime = 0.0f;
        bool notRemoved = false;


		XMVECTOR currentPosition;
		XMVECTOR currentRotation;
		XMVECTOR currentVelocity;

		bool created = false;
		bool signalCreate = false;
		float wait;
		utils::Counter<float> counterShowit;

    public:
        ~CPickup() {
            if (notRemoved && !component::Handle::onCleanup()) {
                removeFromScene();
            }
        }

        inline void init() {notRemoved=true;}
        void update(float elapsed);
        void loadFromProperties(std::string elem, utils::MKeyValue atts);
		void setup();
		void setup(XMVECTOR origin, XMVECTOR velocity = utils::zero_v,
				   XMVECTOR rotQ = DirectX::XMQuaternionIdentity());
		void waitAndSetup(float timetoWait ,XMVECTOR origin, XMVECTOR velocity = utils::zero_v,
				   XMVECTOR rotQ = DirectX::XMQuaternionIdentity());
        void setupStationary();
		void removeFromScene();
		void activate();


		inline bool isStationaryandUsed() { return stationaryUsed; }
		inline int getType(){ return type; }
        inline float getStrength(void) const {return strength;}
        inline void setStrength(float nStrength) {strength = nStrength;}

		void receive(const MsgSetPlayer& msg) { 
			playerEntity = msg.playerEntity; 
		}
};

/** COMPONENT CThrowsPickups
 *  When activated, throws pickups and deletes itself
 *
 *  XML properties
 *      TBA
 */
class CThrowsPickups {
    private:
        bool repeat = false;
        bool done = false;

        utils::Counter<float> repeatCooldownTimer;
        float repeatCooldown = 1.0f;

        /* cosas relacionadas con cómo se lanzan los pickups */
    public:
        void activate();
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
        inline void update(float elapsed) {
            repeatCooldownTimer.count(elapsed);
        }

        inline void init() {
            repeatCooldownTimer.reset();
        }

        inline void receive(const MsgTransform&) {activate();}
        inline void receive(const MsgRevive&) {done = false;}
};

}

#endif