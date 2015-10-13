#ifndef GAME_ELEMENTS_FLARE_H_
#define GAME_ELEMENTS_FLARE_H_

#include "../gameMsgs.h"
#include "enemies.h"

#include "physX_USER/PhysicsManager.h"

#include "handles/entity.h"
using namespace component;

#include "behavior/bt.h"
namespace gameElements{ class FlareBtExecutor; }
namespace behavior {
	typedef Bt<gameElements::FlareBtExecutor> FlareBt;
}
using namespace behavior;

namespace gameElements {
	class CFlare;

	class FlareBtExecutor {
	public:
		enum nodeId_e {
			FLARE = EnemyBtExecutor::NUM_ENUMS,
			AIM = 0xC018,
			RAND_COOLDOWN = 0xC019,
			ALIGN = 0xC020,
			STEP_ROTATE = 0xC021,
			SYNC_SHOOT = 0xC022,
			SHOOT_INIT = 0xC023,
			SHOOT = 0xC024,
			UNLOAD = 0xC025,
			LOOK_AROUND = 0xC026,
			BACK = 0xC027,
		};

		friend behavior::FlareBt;
		friend CFlare;

        static const float FLARE_SPEED;

	private:

		Handle enemyAi_h;
		Handle playerEntity;
		Handle meEntity;
		
		utils::Counter<float> timer;
		float randCooldownTime = 0.f;
		bool synced = false;
		XMVECTOR playerHeight = XMVectorSet(0, CHARACTER_HEIGHTS(0.5f), 0, 0);

	private:
		bool syncTarget(float elapsed);
		void shootFlare(float elapsed);


		//conditions
		bool forget(float) const;
		inline bool notForget(float elapsed) const { return !forget(elapsed); }
		bool chanceLA(float) const;

		//actions
		ret_e lookAround(float);
		ret_e align(float);
		ret_e randCooldown(float);
		ret_e shootInit(float);
		ret_e shoot(float);
		ret_e unload(float);
		ret_e back(float);

		XMVECTOR getPosition(){
			Entity* me(meEntity);
			CTransform* meTransform(me->get<CTransform>());
			return meTransform->getPosition();
		}

	public:
		inline void init(){ reset(); }
		inline void reset() { timer.reset(); synced = false; }
		inline void update(float elapsed) {}

		inline void setSynced() { synced = true; }

	};

	class CFlare {
	private:
		FlareBt bt;
		
	public:
		static void initType();
	public:
		void update(float elapsed);
		void init();

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}

		void receive(const MsgFlareSync&) { bt.getExecutor().setSynced(); }
		void receive(const MsgSetPlayer& msg) { bt.getExecutor().playerEntity = msg.playerEntity; }

		void setPlayer(Handle playerEntity) {
			bt.getExecutor().playerEntity = playerEntity;
		}

		//Test (check app.cpp line 426)
		XMVECTOR getPosition(){
			return bt.getExecutor().getPosition();
		}

		int getState(){
			return bt.getState();
		}
	};

	class CFlareShot {
	public:
		static const EntityListManager::key_t TAG = 0xF12EBA11;
		static void initType();
	private:
		utils::Counter<float> ttltimer;
		XMVECTOR flareVelocity;
		float lifeTime = 0.0f;
        bool notRemoved = false;
	public:
		CFlareShot() = default;
        ~CFlareShot() {
            if (notRemoved && !component::Handle::onCleanup()) {
                removeFromScene();
            }
        }

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed);
		inline void init() {}
		void removeFromScene();
		void setup(XMVECTOR origin, XMVECTOR velocity, XMVECTOR rotQ);
		XMVECTOR getVelocity() { return flareVelocity; }
		void resetVelocity(XMVECTOR velocity);
		void receive(const MsgCollisionEvent& msg);
	};

}
#endif