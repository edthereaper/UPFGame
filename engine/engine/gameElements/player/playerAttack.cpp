#include "mcv_platform.h"
#include "PlayerAttack.h"

using namespace DirectX;
using namespace utils;

#include "../enemies/enemies.h"
#include "../props.h"
#include "../input.h"
#include "playerMov.h"
#include "bullet.h"

#include "render/camera/component.h"
#include "render/mesh/component.h"
using namespace render;

#include "components/transform.h"
#include "components/detection.h"
#include "handles/prefab.h"
using namespace component;

#include "animation/animationPlugger.h"
#include "animation/cArmPoint.h"
#include "animation/cbonelookat.h"
using namespace animation;

#include "gameElements/liana.h"
#include "gameElements/boss/boss.h"

#include "gameElements/props.h"
#include "gameElements/pickup.h"
#include "gameElements/enemies/flare.h"
#include "playerStats.h"
using namespace gameElements;

namespace behavior {
	PlayerAttackBt::nodeId_t PlayerAttackBt::rootNode = INVALID_NODE;
	PlayerAttackBt::container_t PlayerAttackBt::nodes;
	void PlayerAttackBt::initType()
	{
	/*                node-id         parent-id       node-type       condition        action          */
    BT_CREATEROOT    (PLAYER_ATTACK                 , PRIORITY                                         );
    BT_CREATECHILD_CA(AIM_STAY      , PLAYER_ATTACK , SEQUENCE_LOOP , aimCondition   , startAim        );
    BT_CREATECHILD_A (AIM           , AIM_STAY      , LEAF                           , aim             );
    BT_CREATECHILD   (AIM_INPUT     , AIM_STAY      , PRIORITY                                         );
    BT_CREATECHILD_C (SHOOT         , AIM_INPUT     , SEQUENCE      , shootCondition                   );
	BT_CREATECHILD_C (MEGASHOT		, SHOOT			, SEQUENCE		, isMega						   );
    BT_CREATECHILD_A (MSHOT_CHARGE  , MEGASHOT      , LEAF                           , charge_mega     );
    BT_CREATECHILD_A (MSHOT_FIRE    , MEGASHOT      , LEAF                           , fire_mega       );
    BT_CREATECHILD_A (MSHOT_RECOIL  , MEGASHOT      , LEAF                           , recoil_mega     );
	BT_CREATECHILD_C (NORMAL_SHOT	, SHOOT			, SEQUENCE		, isNotMega						   );
    BT_CREATECHILD_A (NSHOT_CHARGE  , NORMAL_SHOT   , LEAF                           , charge          );
    BT_CREATECHILD_A (NSHOT_FIRE    , NORMAL_SHOT   , LEAF                           , fire            );
    BT_CREATECHILD_A (NSHOT_RECOIL  , NORMAL_SHOT   , LEAF                           , recoil          );
	BT_CREATECHILD_C (IDLE_STAY		, PLAYER_ATTACK , SEQUENCE_LOOP , NaimCondition					   );
	BT_CREATECHILD_A (IDLE			, IDLE_STAY		, LEAF							 , idle			   );
    BT_CREATECHILD   (AIM_INPUT2    , IDLE_STAY     , PRIORITY                                         );
    BT_CREATECHILD_C (SHOOT2        , AIM_INPUT2    , SEQUENCE      , shootCondition                   );
	BT_CREATECHILD_C (MEGASHOT2		, SHOOT2		, SEQUENCE		, isMega						   );
    BT_CREATECHILD_A (MSHOT_CHARGE2 , MEGASHOT2     , LEAF                           , charge_mega     );
    BT_CREATECHILD_A (MSHOT_FIRE2   , MEGASHOT2     , LEAF                           , fire_mega       );
    BT_CREATECHILD_A (MSHOT_RECOIL2 , MEGASHOT2     , LEAF                           , recoil_mega     );
	BT_CREATECHILD_C (NORMAL_SHOT2	, SHOOT2		, SEQUENCE		, isNotMega						   );
    BT_CREATECHILD_A (NSHOT_CHARGE2 , NORMAL_SHOT2  , LEAF                           , charge          );
    BT_CREATECHILD_A (NSHOT_FIRE2   , NORMAL_SHOT2  , LEAF                           , fire            );
    BT_CREATECHILD_A (NSHOT_RECOIL2 , NORMAL_SHOT2  , LEAF                           , recoil          );
	}

}

#define TIME_SHOOT_COOLDOWN 0.4f
#define TIME_CHARGE 0.1f
#define TIME_RECOIL 0.1f
#define MARKER_HEIGHT 1.00f

namespace gameElements {

const float PlayerAttackBtExecutor::AIM_ROTATE_SPEED = deg2rad(360 * 10);

bool PlayerAttackBtExecutor::aimCondition(float) const
{
	Entity* me = meEntity;
	CPlayerMov* playerMov = me->get<CPlayerMov>();
	if (((playerMov->getPreviousAction() & PlayerMovBtExecutor::COD_CAN_SHOOT) != 0) && isAimingSomething){
		return true;
	}
	else{
		return false;
	}
}

bool PlayerAttackBtExecutor::shootCondition(float) const
{
	return shootCooldown.get() >= TIME_SHOOT_COOLDOWN
		&& (((CPlayerMov*)((Entity*)meEntity)->get<CPlayerMov>())
		->getPreviousAction() & PlayerMovBtExecutor::COD_CAN_SHOOT) != 0
		&& App::get().getPad().getState(CONTROLS_SHOOT).isHit();
}

XMVECTOR PlayerAttackBtExecutor::calculateAimAngle(
	XMVECTOR target, XMVECTOR origin, float offsetY, float shotSpeed)
{
    return utils::calculateAimAngle(target, origin, offsetY, shotSpeed,
        XMVectorGetX(XMVector3Length(toXMVECTOR(Physics.getGravity()))));
}

void PlayerAttackBtExecutor::shoot(bool isMega, bool aiming)
{
	CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
	CArmPoint* armPoint(((Entity*)meEntity)->get<CArmPoint>());
	XMVECTOR mePos = meT->getPosition();

	CTransform* camT = ((Entity*)cameraEntity)->get<CTransform>();
	XMVECTOR target = armPoint->getPosHand() + camT->getFront()*15.f;

	float offset = 0.0f;
	if (aiming) {
		if (aimed.type == target_t::NONE || aimed.isTransformed()) {
			lookAtXZ(meT, camT->getPosition() + camT->getFront() * 15);
		}
		else {
			offset = 0;
			lookAtXZ(meT, aimed.getPosition());
			target = aimed.getPosition();
		}
	}
	shootCooldown.reset();
	XMVECTOR vel = calculateAimAngle(target, armPoint->getPosHand(), offset,
        isMega ? CBullet::BULLET_SPEED_MEGA : CBullet::BULLET_SPEED);
	Entity* bulletEntity = PrefabManager::get().prefabricate(
		isMega ? "bullet-mega" : "bullet-regular");
	bulletEntity->init();
	CBullet* bullet = bulletEntity->get<CBullet>();
	//The front value varies from the begining of the func shot looking at the back
	bullet->setup(armPoint->getPosHand(), vel,
        XMQuaternionRotationAxis(yAxis_v, getYawFromVector(target - armPoint->getPosHand())));
	((Entity*)meEntity)->sendMsg(MsgPlayerHasShot());
}

void PlayerAttackBtExecutor::testAimAndAdd(
	target_t t, std::vector<target_t>& list, CDetection* d, const CTransform& transform, XMVECTOR mePos)
{
	assert(d != nullptr);
    bool hammerNotTargetable = t.type == target_t::HAMMER && 
        !((CTransformable*)(t.h.getSon<CTransformable>()))->getSelected();
	if (!t.isTransformed() && !hammerNotTargetable) {
		t.score = d->score(&transform, t.getPosition());
		if (t.score != CDetection::NO_SCORE) {
			if (t.type == t.ENEMY) t.score = t.score * 0.00001f;
			list.push_back(t);
		}
	}
}

void PlayerAttackBtExecutor::selectTargets()
{
	Entity* me = meEntity;
	CDetection* meD = me->get<CDetection>();
	CTransform* meT = (CTransform*)me->get<CTransform>();
	XMVECTOR mePos = meT->getPosition();
	Entity* cam = cameraEntity;
	CTransform* camT = cam->get<CTransform>();
	CTransform detectT = *camT;

	auto begin = std::begin(aimList);
	auto end = std::end(aimList);
	for (auto i = begin; i != end; i++) { *i = target_t(); }

	std::vector<target_t> v;
	v.reserve(TARGET_LIST_MAX);

	detectT.setPosition(camT->getPosition());

	for (auto& h : EntityListManager::get(CEnemy::TAG)) {
        if (!h.isValid()) {continue;}
		testAimAndAdd(target_t(target_t::ENEMY, h), v, meD, detectT, mePos);
	}
	
	for (auto& h : EntityListManager::get(CTrampoline::TAG)) {
        if (!h.isValid()) {continue;}
		testAimAndAdd(target_t(target_t::TRAMPOLINE, h), v, meD, detectT, mePos);
	}
	for (auto& h : EntityListManager::get(CCannon::TAG)) {
        if (!h.isValid()) {continue;}
		testAimAndAdd(target_t(target_t::CANNON, h), v, meD, detectT, mePos);
	}
	for (auto& h : EntityListManager::get(CLiana::TAG)) {
        if (!h.isValid()) {continue;}
	    testAimAndAdd(target_t(target_t::LIANA, h), v, meD, detectT, mePos);
	}
	for (auto& h : EntityListManager::get(CCreep::TAG)) {
        if (!h.isValid()) {continue;}
		testAimAndAdd(target_t(target_t::CREEP, h), v, meD, detectT, mePos);
	}
	for (auto& h : EntityListManager::get(CProp::TAG)) {
		testAimAndAdd(target_t(target_t::PROP, h), v, meD, detectT, mePos);
	}
	for (auto& h : EntityListManager::get(CBoss::HAMMER_TAG)) {
		testAimAndAdd(target_t(target_t::HAMMER, h), v, meD, detectT, mePos);
	}

	std::sort(v.begin(), v.end());
	auto it = v.begin();
	for (unsigned i = 0; i<TARGET_LIST_MAX && it != v.end(); i++) {
		isAimingSomething = true;
		aimList[i] = *it;
		it++;
	}
}

void PlayerAttackBtExecutor::chooseBestTarget()
{
	isAimingSomething = false;
	selectTargets();
	prevAim = aimed;
    auto nextAim = aimList[0]; 
    if(prevAim != nextAim && prevAim.h.isValid()) {
        Entity* e = prevAim.h;
        CTransformable* transformable = e->get<CTransformable>();
        if (transformable != nullptr) {transformable->setMarked(false);}
    }
	aimed = nextAim;
}

void PlayerAttackBtExecutor::moveMarker()
{
	if (aimed.type != target_t::NONE) {
		Entity* me = meEntity;

#ifndef LOOKAT_TOOL
		CArmPoint* armPoint(me->get<CArmPoint>());
		armPoint->setTarget(aimPoint);
#endif
	}
}

void PlayerAttackBtExecutor::setAiming(bool b)
{	
	if (aimed.type == aimed.ENEMY){
		isAutoAimingEnemy = b;
	}
	else{
		isAutoAimingEnemy = false;
	}
    
    if(aimed.h.isValid()) {
        Entity* e = aimed.h;
        CTransformable* transformable = e->get<CTransformable>();
        if (transformable != nullptr) {transformable->setMarked(b);}
    }
#ifndef LOOKAT_TOOL
    activateArmpoint = b;
#endif
}

#define DELAY_LOOKING 1.5f
void PlayerAttackBtExecutor::setupLooking(float elapsed)
{
    static auto const headOnly = PlayerMovBtExecutor::COD_DISABLE_LOOKAT_HEADONLY;
	Entity* me = meEntity;
	CPlayerMov* mov = me->get<CPlayerMov>();
    auto action = mov->getPreviousAction();
    CBoneLookAt* boneLookat = me->get<CBoneLookAt>();
    bool headonlyNow = (action & headOnly) != 0;
#ifndef LOOKAT_TOOL
	if (aimed.isValid() && ((action & PlayerMovBtExecutor::COD_DISABLE_LOOKAT) == 0)) {

        if (lookingCounter.count(elapsed) < DELAY_LOOKING) {
            return;
        } else {
            if (headonlyNow) {
                boneLookat->getEntry(0).setActive(false);
                boneLookat->getEntry(1).setActive(false);
                boneLookat->getEntry(2).setActive(true);
                boneLookat->getEntry(3).setActive(true);
            } else {
                boneLookat->setAllActive(true);
            }
        }
		CTransform* meT = me->get<CTransform>();
		XMVECTOR target = aimed.getPosition();

		looking = true;
		
		if (aimed != prevAim) {
            lookingCounter.reset();
			boneLookat->setTarget(target, prevAim.isValid());
		} else {
			boneLookat->changeTarget(target);
		}
	} else {
        lookingCounter.reset();
			boneLookat->setAllActive(false);
		if (looking) {
			looking = false;
		}
	}
#endif
}

ret_e PlayerAttackBtExecutor::idle(float elapsed)
{
    freeAimingMode = false;
	setAiming(false);
	return DONE_QUICKSTEP;
}

ret_e PlayerAttackBtExecutor::startAim(float elapsed)
{
	return DONE_QUICKSTEP;
}

ret_e PlayerAttackBtExecutor::aim(float elapsed)
{
	Entity* me = meEntity;
	if (!aimed.isValid()) {
		freeAimingMode = true;
		setAiming(false);
	} else {
		if (aimed.isTransformed()){
			prevAim = target_t();
			aimed = target_t();
			setAiming(false);
		} else {
			setAiming(true);
			moveMarker();
			aimPoint = aimed.getPosition();
		}
	}
	return DONE_QUICKSTEP;
}

	ret_e PlayerAttackBtExecutor::fire(float)
	{
		Entity* me = meEntity;
		CPlayerMov* mePlayerMov = me->get<CPlayerMov>();
		shoot(false, true);
		return DONE;
	}

	ret_e PlayerAttackBtExecutor::fire_mega(float)
	{
		Entity* me = meEntity;
		CPlayerMov* mePlayerMov = me->get<CPlayerMov>();
		shoot(true, true);
		return DONE;
	}

	ret_e PlayerAttackBtExecutor::charge(float elapsed)
	{
		if (timer.count(elapsed) >= TIME_CHARGE) {
			timer.reset();
			return DONE;
		}
		else {
			return STAY;
		}
	}

	ret_e PlayerAttackBtExecutor::recoil(float elapsed)
	{
		if (timer.count(elapsed) >= TIME_RECOIL) {
			timer.reset();
			return DONE;
		}
		else {
			return STAY;
		}
	}

	void CPlayerAttack::update(float elapsed)
	{
		auto& bte(bt.getExecutor());
		Entity* me = bte.meEntity;
		CPlayerStats* mePS = me->get<CPlayerStats>();
		if (!mePS->isPlayerDead()){
			bte.previousAction = bt.getCurrentAction();
			bte.chooseBestTarget();
			bt.update(elapsed);
			bte.setupLooking(elapsed);
			if (bt.hasActionChanged()) {
				Entity* me = bte.meEntity;
				CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
				uint32_t action = bt.getCurrentAction() & 0xFFFF;
				animPlugger->plug(action);
			}
		}
	}

	XMVECTOR PlayerAttackBtExecutor::target_t::getPosition() const
	{
		Entity* e = h;
		CTransform* t = e->get<CTransform>();
		if (e->has<CEnemy>()){
			return t->getPosition() + yAxis_v;
		} else if (e->has<CTransformable>()){
			CTransformable* prop = e->get<CTransformable>();
			return prop->getCenterAim();
		} else {
		    return t->getPosition();
        }
	}

	bool PlayerAttackBtExecutor::target_t::isTransformed()
	{
		Entity* e = h;
        if (e == nullptr) {return true;}
		assert(e->has<CTransformable>());
		CTransformable* t = e->get<CTransformable>();
		return t->isTransformed();
	}

	void CPlayerAttack::init()
	{
		Handle h(this);
		Handle meEntity = h.getOwner();
		PlayerAttackBtExecutor& btEx = bt.getExecutor();
		btEx.meEntity = meEntity;
		bt.init();
	}

	void CPlayerAttack::initType()
	{
		PlayerAttackBt::initType();
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerAttack, MsgPlayerHasMegashot, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerAttack, MsgSetCam, receive);
	}
}