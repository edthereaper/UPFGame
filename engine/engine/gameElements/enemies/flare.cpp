#include "mcv_platform.h"
#include "flare.h"

using namespace DirectX;
using namespace utils;

#include "components/transform.h"
#include "components/color.h"
#include "components/detection.h"
#include "handles/prefab.h"
using namespace component;

#include "animation/animationPlugger.h"
using namespace animation;

#include "../Ambientsound.h"
#include "../module.h"

#include "Particles/ParticlesManager.h"
using namespace particles;

#define SYNC_MARGIN     0.5f
#define SYNC_DISTANCE   5.0f

#define LOOKAROUND_ROTATE_SPEED deg2rad(120)
#define AIM_ROTATE_SPEED deg2rad(720)

#define TIME_LOOKAROUND 2.5f
#define TIME_SHOOT_INIT 0.2f
#define MIN_SHOOT_COOLDOWN 0.75f
#define MAX_SHOOT_COOLDOWN 2.0f


namespace behavior {
FlareBt::nodeId_t FlareBt::rootNode = INVALID_NODE;
FlareBt::container_t FlareBt::nodes;
void FlareBt::initType()
{                                                                                 
    /*                node-id         parent-id       node-type         condition    action          */
    BT_CREATEROOT    (FLARE                         , SEQUENCE                                       );
    BT_CREATECHILD_C (AIM           , FLARE         , SEQUENCE_LOOPW  , notForget                    );
    BT_CREATECHILD_A (RAND_COOLDOWN , AIM           , LEAF                         , randCooldown    );
    BT_CREATECHILD_A (ALIGN         , AIM           , LEAF                         , align           );
    BT_CREATECHILD_A (SHOOT_INIT    , AIM           , LEAF                         , shootInit       );
    BT_CREATECHILD_A (SHOOT         , AIM           , LEAF                         , shoot           );
    BT_CREATECHILD_A (UNLOAD        , AIM           , LEAF                         , unload          );
    BT_CREATECHILD_CA(BACK          , FLARE         , LEAF            , forget     , back            );
}

}

namespace gameElements {

const float FlareBtExecutor::FLARE_SPEED = 19.f;

bool FlareBtExecutor::forget(float) const
{
    Entity* me((Entity*)meEntity);
    CDetection* meDetect = me->get<CDetection>();
    CTransform* meTransform = me->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	return !meDetect->testWithSight(meTransform, playerTransform->getPosition() + playerHeight,
		meDetect->sight + 5.f);
}

ret_e FlareBtExecutor::randCooldown(float elapsed)
{
    randCooldownTime = utils::inRange(MIN_SHOOT_COOLDOWN, rand_normal(1.0f, 1.0f), MAX_SHOOT_COOLDOWN);
    return DONE;
}

ret_e FlareBtExecutor::align(float elapsed)
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition() + playerHeight, elapsed*AIM_ROTATE_SPEED);
    if (synced) {
        synced = false;
        timer.reset();
        return DONE;
    } else if (timer.count(elapsed) >= randCooldownTime) {
        timer.reset();
        return DONE;
    } else {
        if (timer.get() >= MIN_SHOOT_COOLDOWN &&
            syncTarget(elapsed)) {
            timer.reset();
            return DONE;
        }
        return STAY;
    }
}

ret_e FlareBtExecutor::shootInit(float elapsed)
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition() + playerHeight, elapsed*AIM_ROTATE_SPEED);
    if (timer.count(elapsed) >= TIME_SHOOT_INIT) {
        timer.reset();
        shootFlare(elapsed);
        return DONE;
    } else {
        return STAY;
    }
}

ret_e FlareBtExecutor::shoot(float elapsed)
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition() + playerHeight, elapsed*AIM_ROTATE_SPEED);
	CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();
	if (timer.count(elapsed) >= aniP->getActualPlugDuration()) {
        timer.reset();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e FlareBtExecutor::unload(float elapsed)
{
    CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();
	if (timer.count(elapsed) >= aniP->getActualPlugDuration()) {
        timer.reset();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e FlareBtExecutor::back(float elapsed)
{
    CEnemy* enemyAi = enemyAi_h;
    enemyAi->endOffensive();
    return DONE;
}

void FlareBtExecutor::shootFlare(float elapsed)
{
	Entity* me = meEntity;
	CTransform* meTransform = me->get<CTransform>();
	Entity* shot = PrefabManager::get().prefabricate("flare-shot");
	shot->init();
	CTransform* shotTransform = shot->get<CTransform>();
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	XMVECTOR origin = meTransform->getPosition() + shotTransform->refPosition() + meTransform->getFront() * 2;
	XMVECTOR vel = XMVector3Normalize((playerTransform->getPosition() + playerHeight) - origin) * FLARE_SPEED;
	CFlareShot* flareShot = shot->get<CFlareShot>();
	flareShot->setup(origin, vel, XMQuaternionIdentity());

}

bool FlareBtExecutor::syncTarget(float elapsed)
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    for (auto& h : EntityListManager::get(CEnemy::TAG)) {
        if (h == meEntity) {continue;}
        Entity* him(h);
        if (him==nullptr) {continue;}
        CTransform* himTransform = him->get<CTransform>();
        XMVECTOR himPos(himTransform->getPosition());
        if (testDistanceSqEu(meTransform->getPosition(), himPos, SYNC_DISTANCE),
            lineDetection(meTransform,himPos, SYNC_MARGIN)) {
            him->sendMsg(MsgFlareSync());
        }
    }
    return false;
}

void CFlare::initType()
{
    FlareBt::initType();
    SUBSCRIBE_MSG_TO_MEMBER(CFlare, MsgFlareSync, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CFlare, MsgSetPlayer, receive);
}

void CFlare::update(float elapsed)
{
    bt.update(elapsed);
    if (bt.hasActionChanged()) {
        CAnimationPlugger* animPlugger(Handle(this).getBrother<CAnimationPlugger>());
        animPlugger->plug(bt.getCurrentAction()&0xFFFF);
    }
}

void CFlare::init()
{
    Handle h(this);
    Handle meEntity = h.getOwner();
    FlareBtExecutor& btEx = bt.getExecutor();
    btEx.meEntity = meEntity;
    Handle enemyAi_h = ((Entity*) meEntity)->get<CEnemy>();
    btEx.enemyAi_h = enemyAi_h;
    ((CEnemy*) enemyAi_h)->setOffensive(h);
    bt.init();
}

void CFlareShot::setup(XMVECTOR origin, XMVECTOR velocity, XMVECTOR rotQ)
{
	Entity* me = Handle(this).getOwner();
	CTransform* meT = me->get<CTransform>();
    notRemoved = true;
	meT->setPosition(origin);
	meT->setRotation(rotQ);
	CRigidBody* rigid = me->get<CRigidBody>();
	rigid->addVelocityObject(velocity, XMQuaternionIdentity(), origin);

	CAmbientSound* meAS = me->get<CAmbientSound>();
	meAS->playSound();

	CEmitter *emitter = me->get<CEmitter>();

	auto& particleManager(ParticleUpdaterManager::get());
#if !defined(_PARTICLES)
	for (auto& i : emitter->iterateKeys()) {
		particleManager.sendActive(i);
	}
#endif
	
}

void CFlareShot::resetVelocity(XMVECTOR velocity)
{
	Entity* me = Handle(this).getOwner();
	CTransform* meT = me->get<CTransform>();
	CRigidBody* rigid = me->get<CRigidBody>();
	flareVelocity = velocity;
	rigid->addVelocityObject(velocity, meT->getRotation(), meT->getPosition());
}

void CFlareShot::update(float elapsed)
{
	if (ttltimer.count(elapsed) >= 0) {		
		Entity* e = Handle(this).getOwner();
		CAmbientSound* meAS = e->get<CAmbientSound>();
		meAS->stopSound();
        removeFromScene();
	}
	lifeTime += elapsed;
}

void CFlareShot::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
	float ttl = atts.getFloat("ttl", 1.0f);
	ttltimer.reset();
	ttltimer.count(-ttl);
}

void CFlareShot::removeFromScene()
{
	Entity* e = Handle(this).getOwner();
    notRemoved = false;
	e->postMsg(MsgDeleteSelf());
	CAmbientSound* meAS = e->get<CAmbientSound>();
	meAS->stopSound();

	CTransform* meT = e->get<CTransform>();
	fmodUser::fmodUserClass::play3DSingleSound("Flare_end", meT->getPosition(), 0.5f);

	CEmitter *emitter = e->get<CEmitter>();
    auto& particleManager(ParticleUpdaterManager::get());

#if !defined(_PARTICLES)
    for (auto& i : emitter->iterateKeys()) {
        particleManager.setDeleteSelf(i);
    }
#endif
}

void CFlareShot::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CFlareShot, MsgCollisionEvent, receive);
}

void CFlareShot::receive(const MsgCollisionEvent& msg)
{
    Entity* eOther = msg.entity;
    if (eOther->has<CPlayerMov>()) {
        eOther->sendMsg(MsgFlareHit(10));
    }
	removeFromScene();
}

}