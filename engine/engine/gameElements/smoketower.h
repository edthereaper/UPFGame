#ifndef GAME_ELEMENTS_SMOKETOWER_H_
#define GAME_ELEMENTS_SMOKETOWER_H_

#include "mcv_platform.h"
#include "gameMsgs.h"
#include "handles/entity.h"
#include "render/mesh/mesh.h"
#include "components/Transform.h"

#include "behavior/fsm.h"

namespace gameElements {
	void  initSmokeTowerTypes();
	class SmokeTowerFSMExecutor;
}
namespace behavior { typedef Fsm<gameElements::SmokeTowerFSMExecutor> SmokeTowerFSM; }
namespace gameElements{

	class CSmokeTower;

	class SmokeTowerFSMExecutor {
	public:
		friend CSmokeTower;
		friend behavior::SmokeTowerFSM;

		enum states {
			STATE_waitingToInit,
			STATE_smokeup,
			STATE_changePhase,
		};

	private:
		//states
		behavior::fsmState_t waitingToInit(float elapsed);
		behavior::fsmState_t smokeup(float elapsed);
		behavior::fsmState_t changePhase(float elapsed);

	public:
		inline behavior::fsmState_t init() { return STATE_waitingToInit; }
		void update(float elapsed);
		inline void changeState(behavior::fsmState_t newState) {}

        utils::Counter<float> bottomTimer = 0;
		bool active = false;
		int phase = -1;
		float Y_tmp = 0;
		utils::Counter<float> timer;
		Handle playerEntity;
		Handle meEntity;
		bool beginDmgSmoke = false;
		bool calcSpeed = false;
		float heightTrigger = 0.0f;
        float initH = 0.0f;
		float speedUpChangePhase = 0.0f;
		float speedUpSmokePhase = 0.0f;
	};

class CSmokeTower {
	public:
		friend SmokeTowerFSMExecutor;
        static inline bool isFXActive() {return fxActive;}
		static const EntityListManager::key_t TAG = 0x18273645;
	private:
        static bool fxActive;

	private:
        float initH = 0;
		behavior::SmokeTowerFSM fsm;
		DirectX::XMVECTOR rotQ = DirectX::XMQuaternionIdentity();
	public:
		CSmokeTower() = default;
		CSmokeTower(DirectX::XMVECTOR rotQ) : rotQ(rotQ){}
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
		void update(float elapsed);
        void updatePaused(float elapsed);
		void init();
		void reset();
		static void initType();
		void setPlayer(Handle pE) { fsm.getExecutor().playerEntity = pE; }
		void receive(const MsgSmokeTower& msg);
		void receive(const MsgSmokeTowerResetPhase& msg);
		void changeState(){ fsm.changeState(SmokeTowerFSMExecutor::states::STATE_changePhase); }
        
		void receive(const MsgRevive& msg);
	};
}

#endif
