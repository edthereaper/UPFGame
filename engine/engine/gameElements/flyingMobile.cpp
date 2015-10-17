#include "mcv_platform.h"
#include "flyingMobile.h"

#include "handles/handle.h"
#include "handles/entity.h"
#include "components/transform.h"
using namespace component;

#include "app.h"
#include "props.h"
#include "gameMsgs.h"
#include "fmod_User/fmodUser.h"
using namespace fmodUser;
using namespace DirectX;
using namespace utils;

#include "animation/animation_max.h"
using namespace animation;


namespace behavior {
FlyingMobileFSM::container_t FlyingMobileFSM::states;
void FlyingMobileFSM::initType()
{
    SET_FSM_STATE(waiting);
    SET_FSM_STATE(running);
    SET_FSM_STATE(done);
}
}

namespace gameElements {

bool FlyingMobileFSMExecutor::isComplete()
{
    CTransform* t = meEntity.getSon<CTransform>();
    return 
        (XMVectorSetY(t->getPosition(),0) == XMVectorSetY(target.getPosition(),0) &&
        angleBetweenVectors(t->getFront(), target.getFront()) < 5.f);
}

behavior::fsmState_t FlyingMobileFSMExecutor::waiting(float elapsed)
{
    if(signaled) {

        CTransform* t = meEntity.getSon<CTransform>();
        origin.set(*t);
        return STATE_running;
    } else {
        return STATE_waiting;
    }
}

behavior::fsmState_t FlyingMobileFSMExecutor::running(float elapsed)
{
    Entity* me = meEntity;
    CTransform* t = me->get<CTransform>();
    moveToTarget(t, target.getPosition(), elapsed, speed);
    rotateToTarget(t, target, elapsed, rotSpeed);

	App &app = App::get();

    if (isComplete()) {


		if (me->has<CCannon>() && app.getLvl() == 4){

			fmodUser::fmodUserClass::playSound("boss_canonland", 1.0f, 0.0f);
			CMaxAnim* maxAnim = me->get<CMaxAnim>();
			maxAnim->setPostFinish(true);
		}

        me->sendMsg(MsgFlyingMobileEnded());
		
		
        return STATE_done;
    } else {

		if (me->has<CCannon>() && app.getLvl() == 4){
			CMaxAnim* maxAnim = me->get<CMaxAnim>();
			maxAnim->setPlay(true);
		}

        return STATE_running;
    }
}

behavior::fsmState_t FlyingMobileFSMExecutor::done(float elapsed)
{
    return STATE_done;
}



}