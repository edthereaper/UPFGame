#include "mcv_platform.h"
#include "smokePanel.h"

#include "handles/handle.h"
#include "handles/entity.h"
#include "components/transform.h"
#include "components/color.h"
using namespace component;

#include "render/render_utils.h"
#include "render/renderManager.h"
using namespace render;

using namespace utils;
using namespace physX_user;

#include "gameElements/player/playerStats.h"
using namespace gameElements;

#include "fmod_User/fmodUser.h"
using namespace fmodUser;

#include "Particles/ParticlesManager.h"
using namespace particles;

#define DMG_SMOKE						10

namespace behavior {
	SmokePanelFSM::container_t SmokePanelFSM::states;
	void SmokePanelFSM::initType()
	{
		SET_FSM_STATE(cold);
		SET_FSM_STATE(starting);
		SET_FSM_STATE(heating);
		SET_FSM_STATE(firing);
		SET_FSM_STATE(cooling);
	}
}

namespace gameElements {

	behavior::fsmState_t SmokePanelFSMExecutor::cold(float elapsed)
	{
		if (signal) {
			signal = false;
			Entity* me = meEntity;
			return STATE_starting;
		}
		else {
			return STATE_cold;
		}
	}

	behavior::fsmState_t SmokePanelFSMExecutor::starting(float elapsed)
	{
		//launch Fx, wait...
		Entity* me = meEntity;
		CTransform* meT = ((Entity*)me)->get<CTransform>();
		fmodUser::fmodUserClass::play3DSingleSound("smokepanel_tremor", meT->getPosition());

		if (me->has<CEmitter>()){
			CEmitter *emitter = me->get<CEmitter>();
			auto key1 = emitter->getKey("emitter_1");
			ParticleUpdaterManager::get().sendActive(key1);
		}

		return STATE_heating;
	}

	behavior::fsmState_t SmokePanelFSMExecutor::heating(float elapsed)
	{
		if (signal) {
			signal = false;
			Entity* me = meEntity;
			CTransform* meT = me->get<CTransform>();
			char cstr[32] = "smokepanel_firing";
			int randomV = rand_uniform(3, 1);
			char randomC[32] = "";
			sprintf(randomC, "%d", randomV);
			strcat(cstr, randomC);

				if (me->has<CEmitter>()){

					CEmitter *emitter = me->get<CEmitter>();

					auto key0 = emitter->getKey("emitter_1");
					ParticleUpdaterManager::get().sendInactive(key0);

					auto key1 = emitter->getKey("emitter_0");
					auto key2 = emitter->getKey("emitter_3");
					ParticleUpdaterManager::get().sendActive(key1);
					ParticleUpdaterManager::get().sendActive(key2);

					fmodUser::fmodUserClass::play3DSingleSound(cstr, meT->getPosition());
				}
			return STATE_firing;
		}
		else {
			return STATE_heating;
		}
	}

	behavior::fsmState_t SmokePanelFSMExecutor::firing(float elapsed)
	{
		if (signal && !alwaysHot) {
			signal = false;
			Entity* me = meEntity;
			CTransform* meT = ((Entity*)me)->get<CTransform>();
			char cstr[32] = "smokepanel_cooling";
			int randomV = rand_uniform(3, 1);
			char randomC[32] = "";
			sprintf(randomC, "%d", randomV);
			strcat(cstr, randomC);
			fmodUser::fmodUserClass::play3DSingleSound(cstr, meT->getPosition());

			if (me->has<CEmitter>()){

				CEmitter *emitter = me->get<CEmitter>();

				auto key1 = emitter->getKey("emitter_0");
				auto key2 = emitter->getKey("emitter_3");
				ParticleUpdaterManager::get().sendInactive(key1);
				ParticleUpdaterManager::get().sendInactive(key2);

				auto key3 = emitter->getKey("emitter_2");
				ParticleUpdaterManager::get().sendActive(key3);

			}

			return STATE_cooling;
		}
		else {
			return STATE_firing;
		}
	}

	behavior::fsmState_t SmokePanelFSMExecutor::cooling(float elapsed)
	{
		if (timer.count(elapsed) >= 4.f) {

			Entity* me = meEntity;

			timer.reset();
			signal = false;

			if (me->has<CEmitter>()){

				CEmitter *emitter = me->get<CEmitter>();
				auto key3 = emitter->getKey("emitter_2");
				ParticleUpdaterManager::get().sendInactive(key3);

			}
			return STATE_cold;
		}
		else {
			return STATE_cooling;
		}
	}

	void CSmokePanel::init()
	{
		fsm.init();
		fsm.getExecutor().meEntity = Handle(this).getOwner();
	}

	void CSmokePanel::reset()
	{
		Entity* me = Handle(this).getOwner();
		fsm.init();
	}

	void CSmokePanel::update(float e)
	{
		fsm.update(e);
		if (isHot() && playerInside) {
			Entity* player = fsm.getExecutor().playerEntity;
			assert(player != nullptr);
			((CPlayerStats*)player->get<CPlayerStats>())->damage(DMG_SMOKE);
		}
	}

	void CSmokePanel::loadFromProperties(std::string elem, utils::MKeyValue atts)
	{
		index = atts.getInt("index", index);
		subindex = atts.getInt("subindex", subindex);
		fsm.getExecutor().alwaysHot = atts.getBool("alwaysHot", fsm.getExecutor().alwaysHot);
	}

	void CSmokePanel::initType()
	{
		SUBSCRIBE_MSG_TO_MEMBER(CSmokePanel, MsgActivateSmokeWarning, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CSmokePanel, MsgActivateSmokeIfHot, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CSmokePanel, MsgDeactivateSmoke, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CSmokePanel, MsgSetPlayer, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CSmokePanel, MsgCollisionEvent, receive);
	}

}
