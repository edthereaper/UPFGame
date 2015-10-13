#include "mcv_platform.h"
#include "weakSpot.h"

using namespace utils;
using namespace DirectX;

#include "handles/handle.h"
#include "handles/entity.h"
#include "components/transform.h"
#include "components/color.h"
using namespace component;

#include "render/render_utils.h"
#include "render/renderManager.h"
using namespace render;

#include "../player/playerMov.h"

#include "Particles/ParticlesManager.h"
using namespace particles;

namespace behavior {
WeakSpotFSM::container_t WeakSpotFSM::states;
void WeakSpotFSM::initType()
{
    SET_FSM_STATE(waiting);
    SET_FSM_STATE(active);
    SET_FSM_STATE(broken);
    SET_FSM_STATE(fightOver);
}
}

namespace gameElements {

behavior::fsmState_t WeakSpotFSMExecutor::waiting(float elapsed)
{
    if(signaled) {
        Entity* me = meEntity;
        CMesh* mesh = me->get<CMesh>();
        mesh->switchMaterials();
        
		CEmitter* emitter = me->get<CEmitter>();
        //Deactivate fire particles
		auto key0 = emitter->getKey("emitter_0");
		auto key1 = emitter->getKey("emitter_1");
		ParticleUpdaterManager::get().setDeleteSelf(key0);
		ParticleUpdaterManager::get().setDeleteSelf(key1);

        //Activate soft, "cooling down" smoke particless
		//auto key2 = emitter->getKey("emitter_2");
		//ParticleUpdaterManager::get().sendActive(key2);

        signaled = false;
        return STATE_active;
    } else {
        return STATE_waiting;
    }
}

behavior::fsmState_t WeakSpotFSMExecutor::active(float elapsed)
{
    if(signaled) {
        Entity* me = meEntity;
        Entity* cbE(callback);
        cbE->sendMsg(MsgWeakSpotBreak(me));
        signaled = false;


		CEmitter* emitter = me->get<CEmitter>();
        //Deactivate soft, "cooling down" smoke particles
		//auto key2 = emitter->getKey("emitter_2");
		//ParticleUpdaterManager::get().setDeleteSelf(key2);

        //Activate heavy black smoke particless
		//auto key3 = emitter->getKey("emitter_3");
		//ParticleUpdaterManager::get().sendActive(key3);

		return STATE_broken;
    } else {
        return STATE_active;
    }
}

behavior::fsmState_t WeakSpotFSMExecutor::broken(float elapsed)
{
    if (signaled) {
        //Deactivate heavy black smoke particless
		//auto key3 = emitter->getKey("emitter_3");
		//ParticleUpdaterManager::get().setDeleteSelf(key3);
        return STATE_fightOver;
    } else {
        return STATE_broken;
    }
}

behavior::fsmState_t WeakSpotFSMExecutor::fightOver(float elapsed)
{
    return STATE_fightOver;
}



void CWeakSpot::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CWeakSpot, MsgHitEvent, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CWeakSpot, MsgRevive, receive);
}

void CWeakSpot::receive(const MsgDeactivateSmoke&)
{
    if (fsm.getState() == WeakSpotFSMExecutor::STATE_fightOver) {
        fsm.getExecutor().signaled = true;
    }
}

void CWeakSpot::receive(const physX_user::MsgHitEvent& msg)
{
    if (fsm.getState() == WeakSpotFSMExecutor::STATE_active) {
        Entity* e = msg.entity;
        if (e->has<CPlayerMov>()) {
            CPlayerMov* playerMov = e->get<CPlayerMov>();
            if ((playerMov->getPreviousAction() & PlayerMovBtExecutor::COD_CANNON_AIR) != 0) {
                fsm.getExecutor().signaled = true;
            }
        }
    }
}

void CWeakSpot::reset()
{
    Entity* me = Handle(this).getOwner();
    CTint* tint = me->get<CTint>();
    tint->set(Color::BROWN);
    RenderManager::updateKeys(me);
    fsm.init();
}

}