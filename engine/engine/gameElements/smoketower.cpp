#include "mcv_platform.h"
#include "smoketower.h"
#include "player/playerMov.h"
#include "player/playerStats.h"

#include "particles/ParticlesManager.h"
#include "paintManager.h"

#define DISTANCE_CHECKPOINT_SMOKE       7.5f
#define DISTANCE_DMG_SMOKE				0.5f
#define FX_THRESHOLD                    0.75f
#define TIME_BETWEEN_PHASE				5.0f
#define HEIGHT_INIT_PHASE0				66.0f
#define HEIGHT_INIT_PHASE1				16.0f
#define HEIGHT_INIT_PHASE2				31.8f
#define HEIGHT_INIT_PHASE3				49.3f
#define HEIGHT_INIT_PHASE4				53.6f
#define SPEED_UP_SMOKE_PHASE0			(26.f/66.f)
#define SPEED_UP_SMOKE_PHASE1			(16.f/50.f)
#define SPEED_UP_SMOKE_PHASE2			(18.f/98.f)
#define SPEED_UP_SMOKE_PHASE3			( 3.f/33.f)
#define SPEED_UP_SMOKE_PHASE4			(19.f/57.f)

using namespace behavior;
using namespace particles;

namespace behavior {
	SmokeTowerFSM::container_t SmokeTowerFSM::states;
	void SmokeTowerFSM::initType()
	{
		SET_FSM_STATE(waitingToInit);
		SET_FSM_STATE(smokeup);
		SET_FSM_STATE(changePhase);
	}
}

namespace gameElements {

fsmState_t SmokeTowerFSMExecutor::waitingToInit(float elapsed)
{
	if (active){
		timer.reset(); 
		return STATE_smokeup;
	}
	return STATE_waitingToInit;
}

fsmState_t SmokeTowerFSMExecutor::smokeup(float elapsed)
{	
	CTransform* meT = meEntity.getSon<CTransform>();
    meT->setPosition(meT->getPosition() +
        XMVectorSet(0, speedUpSmokePhase * elapsed, 0, 0));
	return STATE_smokeup;
}

fsmState_t SmokeTowerFSMExecutor::changePhase(float elapsed)
{
	CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
	if (calcSpeed) {
		speedUpChangePhase = (heightTrigger - XMVectorGetY(meT->getPosition())) / TIME_BETWEEN_PHASE;
		calcSpeed = false;
	}
	if ((XMVectorGetY(meT->getPosition()) - heightTrigger) >= 0){
		meT->setPosition(XMVectorSetY(meT->getPosition(), heightTrigger));
		return STATE_smokeup;
	}
	meT->setPosition(meT->getPosition() + XMVectorSet(0, speedUpChangePhase * elapsed, 0, 0));
	return STATE_changePhase;
}

#define TIME_BOTTOM 3.f

void SmokeTowerFSMExecutor::update(float elapsed)
{
	if (!active) return;
	Entity* player = playerEntity;
	CTransform* meT = meEntity.getSon<CTransform>();
	CTransform* playerT = player->get<CTransform>();
	CPlayerStats* playerS = player->get<CPlayerStats>();
    float playerY = XMVectorGetY(playerT->getPosition());
	CTransform* camT = App::get().getCamera().getSon<CTransform>();
	float diff = XMVectorGetY(meT->getPosition()) - playerY;

    bool bottomReset = false;
    if (playerY <= initH + 1.5f) {
        bottomReset = bottomTimer.count(elapsed) >= TIME_BOTTOM;    
    } else {
        bottomTimer.reset();
    }

	if (diff >= DISTANCE_CHECKPOINT_SMOKE || bottomReset){
		App &app = App::get();
		if (playerS->getHealth() > 30){
			app.spawn();
			PaintManager::reset(); //Delete all paint spheres
		}
		playerS->damage(30, true);
	} else if (diff > FX_THRESHOLD){
        ((CPlayerStats*)player->get<CPlayerStats>())->damage(10);
	}

	float camDiff = XMVectorGetY(meT->getPosition()) - XMVectorGetY(camT->getPosition());
    CSmokeTower::fxActive = camDiff >= FX_THRESHOLD;
}

void CSmokeTower::updatePaused(float elapsed)
{
	CTransform* meT = fsm.getExecutor().meEntity.getSon<CTransform>();
	CTransform* camT = App::get().getCamera().getSon<CTransform>();
	float camDiff = XMVectorGetY(meT->getPosition()) - XMVectorGetY(camT->getPosition());
    CSmokeTower::fxActive = camDiff >= FX_THRESHOLD;
}

void CSmokeTower::update(float elapsed)
{
	fsm.update(elapsed);
    Entity* e(Handle(this).getOwner());
    CTransform* t(e->get<CTransform>());
	PaintManager::setFireLevel(XMVectorGetY(t->getPosition()));
}

void CSmokeTower::init()
{
	Handle h(this);
	fsm.getExecutor().meEntity = h.getOwner();
	SmokeTowerFSM::initType();
	fsm.init();
    auto& fsme = fsm.getExecutor();
	CTransform* meT = fsme.meEntity.getSon<CTransform>();
	fsme.heightTrigger = XMVectorGetY(meT->getPosition());
    initH = fsme.heightTrigger;
	fsme.initH = initH;
}

void CSmokeTower::reset()
{
	fsm.getExecutor().active = false;
	fsm.getExecutor().phase = 0;
	fsm.changeState(SmokeTowerFSMExecutor::states::STATE_waitingToInit);
	CTransform* meT = ((Entity*)fsm.getExecutor().meEntity)->get<CTransform>();
    meT->setPosition(XMVectorSetY(meT->getPosition(), initH));
	fsm.getExecutor().heightTrigger = initH;
}

void CSmokeTower::initType()
{
	SUBSCRIBE_MSG_TO_MEMBER(CSmokeTower, MsgSmokeTower, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CSmokeTower, MsgSmokeTowerResetPhase, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CSmokeTower, MsgRevive, receive);
}

void CSmokeTower::receive(const MsgSmokeTower& msg)
{
	if (fsm.getExecutor().phase >= (int)msg.phase) return;
	switch (msg.phase) {
	case 0:
		fsm.getExecutor().active = true;
		fsm.getExecutor().phase = 0;
		fsm.getExecutor().heightTrigger = initH;
		fsm.getExecutor().speedUpSmokePhase = SPEED_UP_SMOKE_PHASE0;
		break;
	case 1:
		fsm.getExecutor().phase = 1;
		fsm.getExecutor().calcSpeed = true;
		fsm.getExecutor().heightTrigger = HEIGHT_INIT_PHASE1;
		fsm.getExecutor().speedUpSmokePhase = SPEED_UP_SMOKE_PHASE1;
		fsm.changeState(SmokeTowerFSMExecutor::states::STATE_changePhase);
		break;
	case 2:
		fsm.getExecutor().phase = 2;
		fsm.getExecutor().calcSpeed = true;
		fsm.getExecutor().heightTrigger = HEIGHT_INIT_PHASE2;
		fsm.getExecutor().speedUpSmokePhase = SPEED_UP_SMOKE_PHASE2;
		fsm.changeState(SmokeTowerFSMExecutor::states::STATE_changePhase);
		break;
	case 3:
		fsm.getExecutor().phase = 3;
		fsm.getExecutor().calcSpeed = true;
		fsm.getExecutor().heightTrigger = HEIGHT_INIT_PHASE3;
		fsm.getExecutor().speedUpSmokePhase = SPEED_UP_SMOKE_PHASE3;
		fsm.changeState(SmokeTowerFSMExecutor::states::STATE_changePhase);
		break;
	case 4:
		fsm.getExecutor().phase = 4;
		fsm.getExecutor().calcSpeed = true;
		fsm.getExecutor().heightTrigger = HEIGHT_INIT_PHASE4;
		fsm.getExecutor().speedUpSmokePhase = SPEED_UP_SMOKE_PHASE4;
		fsm.changeState(SmokeTowerFSMExecutor::states::STATE_changePhase);
		break;
	}

	Entity *e(Handle(this).getOwner());
	CEmitter *emitter = e->get<CEmitter>();

	auto key = emitter->getKey("emitter_0");
	
	ParticleUpdaterManager::get().sendReset(key);

}

void CSmokeTower::receive(const MsgSmokeTowerResetPhase& msg)
{
	CTransform* meT = ((Entity*)fsm.getExecutor().meEntity)->get<CTransform>();
	fsm.getExecutor().beginDmgSmoke = false;
	if (fsm.getExecutor().phase == 0){
		meT->setPosition(XMVectorSetY(meT->getPosition(), initH));
	} else {
		meT->setPosition(
            XMVectorSetY(meT->getPosition(), fsm.getExecutor().heightTrigger - 7.5f));
        fsm.changeState(SmokeTowerFSMExecutor::STATE_changePhase);
        fsm.getExecutor().calcSpeed = true;
	}
	CSmokeTower::fxActive = false;
}

   
bool CSmokeTower::fxActive = false;

void CSmokeTower::receive(const MsgRevive& msg)
{
    reset();
}

}