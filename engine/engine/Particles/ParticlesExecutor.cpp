#include "mcv_platform.h"
#include "ParticleSystem.h"
#include "handles/message.h"
#include "handles/handle.h"
#include "handles/importXML.h"
#include "handles/entity.h"
#include "handles/objectManager.h"

#include "components/transform.h"
#include "handles/entity.h"

#include "Particles/ParticlesManager.h"

using namespace DirectX;
using namespace component;

#include "behavior/fsm.h"

using namespace behavior;

namespace behavior {
	ParticlesFSM::container_t ParticlesFSM::states;
	void ParticlesFSM::initType()
	{
		SET_FSM_STATE(create);
		SET_FSM_STATE(active);
		SET_FSM_STATE(wait_active);
		SET_FSM_STATE(inactive);
		SET_FSM_STATE(wait_inactive);
		SET_FSM_STATE(kill);
		SET_FSM_STATE(wait_kill);
		SET_FSM_STATE(done);
		SET_FSM_STATE(reset);
		SET_FSM_STATE(wait_reset);
	}
}

namespace particles{


	behavior::fsmState_t ParticlesFSMExecutor::create(float elapsed){

		return STATE_create;
	}
	behavior::fsmState_t ParticlesFSMExecutor::active(float elapsed){

		Entity *me = meEntity;
		Entity *attatch = attatchEntity;
		CParticleSystem *ps = me->get<CParticleSystem>();

		ps->setEmitting(true);

		return STATE_active;
	}


	behavior::fsmState_t ParticlesFSMExecutor::reset(float elapsed){

		Entity *me = meEntity;
		Entity *attatch = attatchEntity;
		CParticleSystem *ps = me->get<CParticleSystem>();
		ps->reload();

		return STATE_active;
	}

	behavior::fsmState_t ParticlesFSMExecutor::inactive(float elapsed){

		Entity *me = meEntity;
		Entity *attatch = attatchEntity;
		CParticleSystem *ps = me->get<CParticleSystem>();
		ps->setEmitting(false);

		return STATE_inactive;
	}
	behavior::fsmState_t ParticlesFSMExecutor::wait_active(float elapsed){
		if (timerActive.count(elapsed) >= time_to_active){
			timerActive.reset();
			return STATE_active;
		}
		return STATE_wait_active;
	}
	behavior::fsmState_t ParticlesFSMExecutor::wait_inactive(float elapsed){
		if (timerInactive.count(elapsed) >= time_to_inactive){
			timerInactive.reset();
			return STATE_inactive;
		}
		return STATE_wait_inactive;
	}

	behavior::fsmState_t ParticlesFSMExecutor::wait_reset(float elapsed){
	
		if (timerReset.count(elapsed) >= time_to_reset){
			timerReset.reset();
			return STATE_reset;
		}
		return STATE_wait_reset;
	}

	behavior::fsmState_t ParticlesFSMExecutor::wait_kill(float elapsed){

		if (timerKiller.count(elapsed) >= time_to_kill){
			timerKiller.reset();
			return STATE_kill;
		}
		return STATE_wait_kill;
	}

	behavior::fsmState_t ParticlesFSMExecutor::kill(float elapsed){

		if (!attatchEntity.isValid())
		{
			Entity *me = meEntity;
			me->postMsg(MsgDeleteSelf());
			me = nullptr;
		}
		return STATE_kill;
	}



}