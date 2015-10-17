#include "mcv_platform.h"
#include "melee.h"

using namespace DirectX;
using namespace utils;

#include "components/transform.h"
#include "components/color.h"
#include "components/slot.h"
#include "components/detection.h"
#include "handles/message.h"
using namespace component;

#include "animation/animationPlugger.h"
using namespace animation;

#include "../player/playerMov.h"
#include "cknife.h"
using namespace gameElements;

#define TIME_INTRO_ATTACK_WAIT			0.33f
#define CONTACT_DISTANCE				2.0f
#define INTRO_ATTACK_DISTANCE			6.0f
#define LOST_CONTACT_DISTANCE			CONTACT_DISTANCE + 1.0f	
#define TAUNT_END_DISTANCE				INTRO_ATTACK_DISTANCE + 1.0f
#define INTRO_ATTACK_SPEED				14.f
#define CHASE_SPEED						5.f
#define FORGET_DISTANCE					25.0f
#define CHASE_ROTATE_SPEED				deg2rad(360)
#define INTRO_ATTACK_ROTATE_SPEED		deg2rad(70)
#define ATTACK_ROTATE_SPEED				deg2rad(400)
#define ANGRY_ROTATE_SPEED				deg2rad(180)
#define TIME_DASH_RECOVERY				.4f
#define MINIMUM_IDLE_FIGHT				0.75f
#define ANGER_TIME						1.f
#define HEIGHT_DIST						0.8f

namespace behavior {
MeleeBt::nodeId_t MeleeBt::rootNode = INVALID_NODE;
MeleeBt::container_t MeleeBt::nodes;
void MeleeBt::initType()
{                                                                                 
    /*                node-id             parent-id        node-type        condition        action         */
    BT_CREATEROOT    (MELEE                              , SEQUENCE                                         );
    BT_CREATECHILD_C (AGGRESSIVE        , MELEE          , SEQUENCE_LOOP  , notForget                       );
	BT_CREATECHILD_CA(DEFENSIVE         , AGGRESSIVE	 , LEAF           , playerNAchiable, defend         );
	BT_CREATECHILD_A (UNSUBSCRIBE_SLOT  , AGGRESSIVE	 , LEAF							   , unsubscribeSlot);
    BT_CREATECHILD_CA(CHASE             , AGGRESSIVE     , LEAF           , playerAchiable , chase          );
    BT_CREATECHILD_C (TEST_SUBSCRIBE    , AGGRESSIVE     , PRIORITY       , onAttackRange                   );
    BT_CREATECHILD_C (CANSUBSCRIBE      , TEST_SUBSCRIBE , SEQUENCE       , canSubscribe                    );
    BT_CREATECHILD   (COMBAT            , CANSUBSCRIBE   , SEQUENCE                                         );
    BT_CREATECHILD   (INTRO_ATTACK      , COMBAT         , SEQUENCE                                         );
	BT_CREATECHILD_A (SUBSCRIBE_SLOT	, INTRO_ATTACK	 , LEAF							   , subscribeSlot	);
	BT_CREATECHILD_A (DASH_WAIT         , INTRO_ATTACK   , LEAF                            , waitintroAttack);
    BT_CREATECHILD_A (DASH              , INTRO_ATTACK   , LEAF                            , introAttack    );
    BT_CREATECHILD_CA(RANDOM_ANGRY1     , INTRO_ATTACK   , PRIORITY       , lostContact    , randomAngry    );
    BT_CREATECHILD_CA(ANGRY1            , RANDOM_ANGRY1  , LEAF           , isAngry        , angry          );
    BT_CREATECHILD_A (DASH_RECOVERY     , RANDOM_ANGRY1  , LEAF                            , dashRecovery   );
    BT_CREATECHILD_C (FIGHT             , COMBAT         , SEQUENCE_LOOPW , readyToFight                    );
    BT_CREATECHILD_A (RANDOM_IDLEF      , FIGHT          , CHAIN                           , randomIdleFight);
    BT_CREATECHILD_A (IDLE_FIGHT        , RANDOM_IDLEF   , LEAF                            , idleFight      );
    BT_CREATECHILD   (HIT               , FIGHT          , SEQUENCE                                         );
    BT_CREATECHILD_CA(COMBO1            , HIT            , CHAIN          , notLostContact , combo1         );
    BT_CREATECHILD_CA(COMBO2            , COMBO1         , CHAIN          , notLostContact , combo2         );
    BT_CREATECHILD_CA(COMBO3            , COMBO2         , LEAF           , notLostContact , combo3         );
	BT_CREATECHILD_CA(RANDOM_ANGRY2		, HIT			 , CHAIN		  , lostContact	   , randomAngry	);
	BT_CREATECHILD_CA(ANGRY2			, RANDOM_ANGRY2	 , CHAIN		  , isAngry		   , angry			);
	BT_CREATECHILD_A(ANGRY_FAIL_INTRO	, CANSUBSCRIBE	 , LEAF							   , angryFailIntro );
    BT_CREATECHILD_C (CANTSUBSCRIBE     , TEST_SUBSCRIBE , SEQUENCE_LOOP  , canTaunt                        );
    BT_CREATECHILD_A (TAUNT             , CANTSUBSCRIBE  , LEAF                            , taunt          );
    BT_CREATECHILD_CA(BACK              , MELEE          , LEAF           , forget         , back           );
	BT_CREATECHILD_CA(CLIF				, MELEE			 , LEAF			  , onClif		   , clifTaunt		);
}

}

namespace gameElements {

const float MeleeBtExecutor::ANGER_THRESHOLD = 0.25;
const float MeleeBtExecutor::GRAVITY = 3.5f * 9.81f;

bool MeleeBtExecutor::playerAchiable(float) const
{
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	XMVECTOR origin = playerTransform->getPosition() + playerTransform->getUp();
	XMVECTOR dir = -playerTransform->getUp();
	PxReal distance = 50.0f;
	PxRaycastBuffer hit;
	XMVECTOR playerRayPos = playerTransform->getPosition();

	if (PhysicsManager::get().raycast(origin, dir, distance, hit,
		filter_t(
		filter_t::NONE,
		filter_t::id_t(filter_t::PLAYER | filter_t::PAINT_SPHERE | filter_t::ENEMY | filter_t::BULLET | filter_t::CANNONPATH | filter_t::KNIFE | filter_t::FLARESHOT),
		filter_t::ALL_IDS))){	
		playerRayPos = XMVectorSet(XMVectorGetX(playerRayPos), XMVectorGetY(playerRayPos) - hit.block.distance + 1.0f, XMVectorGetZ(playerRayPos), XMVectorGetW(playerRayPos));
	}

	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
	float dif = abs(XMVectorGetY(playerRayPos) - XMVectorGetY(meTransform->getPosition()));
	if (dif < HEIGHT_DIST){
		return true;
	}
	return false;
}

ret_e MeleeBtExecutor::defend(float elapsed) 
{
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	float dif = abs(XMVectorGetY(playerTransform->getPosition()) - XMVectorGetY(meTransform->getPosition()));
	alignXZ(meTransform, playerTransform->getPosition(), CHASE_ROTATE_SPEED*elapsed);
	
	if (!msgsendProtect){
		((Entity*)meEntity)->sendMsg(MsgProtect());
		msgsendProtect = true;
	}

	if (!playerAchiable(0.0f) && !forget(0.0f)) return STAY;

	((Entity*)meEntity)->sendMsg(MsgStopDefense());
	return DONE;
}

bool MeleeBtExecutor::contact(float) const
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();

	XMVECTOR XZpos = XMVectorSet(XMVectorGetX(playerTransform->getPosition()), XMVectorGetY(meTransform->getPosition()),
		XMVectorGetZ(playerTransform->getPosition()), XMVectorGetW(playerTransform->getPosition()));

    return testDistanceSqEu(
		meTransform->getPosition(), XZpos,
        CONTACT_DISTANCE);
}

bool MeleeBtExecutor::lostContact(float) const
{
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();

	XMVECTOR XZpos = XMVectorSet(XMVectorGetX(playerTransform->getPosition()), XMVectorGetY(meTransform->getPosition()),
		XMVectorGetZ(playerTransform->getPosition()), XMVectorGetW(playerTransform->getPosition()));

	return !testDistanceSqEu(
		meTransform->getPosition(), XZpos,
		LOST_CONTACT_DISTANCE);
}

bool MeleeBtExecutor::forget(float) const
{
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();

	if (onClif(0.0f)){
		return true;
	}

	return !testDistanceSqEu(
		meTransform->getPosition(), playerTransform->getPosition(),
		FORGET_DISTANCE);
}

bool MeleeBtExecutor::onClif(float) const
{
	Entity* me = meEntity;
	CTransform* meT = me->get<CTransform>();
	XMVECTOR origin = meT->getPosition() + meT->getFront() + meT->getUp();
	XMVECTOR dir = XMVector3Normalize(-meT->getUp());
	PxReal distance = 8.0f;
	PxRaycastBuffer hit;
	if (!PhysicsManager::get().raycast(origin, dir, distance, hit,
		filter_t(
		filter_t::NONE,
		filter_t::id_t(filter_t::PLAYER | filter_t::PAINT_SPHERE | filter_t::ENEMY | filter_t::BULLET | filter_t::CANNONPATH | filter_t::KNIFE),
		filter_t::ALL_IDS))){
		return true;
	}
	return false;
}

bool MeleeBtExecutor::onAttackRange(float elapsed) const
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
   
	XMVECTOR XZpos = XMVectorSet(XMVectorGetX(playerTransform->getPosition()), XMVectorGetY(meTransform->getPosition()),
		XMVectorGetZ(playerTransform->getPosition()), XMVectorGetW(playerTransform->getPosition()));

	return testDistanceSqEu(
		meTransform->getPosition(), XZpos,
        INTRO_ATTACK_DISTANCE);
}

bool MeleeBtExecutor::outsideAttackRange(float elapsed) const
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();

	XMVECTOR XZpos = XMVectorSet(XMVectorGetX(playerTransform->getPosition()), XMVectorGetY(meTransform->getPosition()),
		XMVectorGetZ(playerTransform->getPosition()), XMVectorGetW(playerTransform->getPosition()));

    return !testDistanceSqEu(
        meTransform->getPosition(), XZpos, 
        TAUNT_END_DISTANCE);
}

ret_e MeleeBtExecutor::dashRecovery(float elapsed)
{
	if (timer.count(elapsed) >= TIME_DASH_RECOVERY) {
        timer.reset();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e MeleeBtExecutor::idleFight(float elapsed)
{
	if (timer.count(elapsed) >= idleFightTime) {
        timer.reset();
        idleFightTime = 0;
        return DONE;
    } else {
        if (lostContact(elapsed)) {
            return DONE;
        } else {
			CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
			CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
			alignXZ(meTransform, playerTransform->getPosition(), CHASE_ROTATE_SPEED*elapsed);
            return STAY;
        }
    }
}

ret_e MeleeBtExecutor::combo1(float elapsed) {
	App &app = App::get();
	if (app.getPlayerDead()) return DONE;
	CAnimationPlugger* aniP = meEntity.getSon<CAnimationPlugger>();
	return doCombo(elapsed, aniP->getActualPlugDurationWithDelay());
}
ret_e MeleeBtExecutor::combo2(float elapsed) {
	App &app = App::get();
	if (app.getPlayerDead()) return DONE;
	CAnimationPlugger* aniP = meEntity.getSon<CAnimationPlugger>();
	return doCombo(elapsed, aniP->getActualPlugDurationWithDelay());
}
ret_e MeleeBtExecutor::combo3(float elapsed) {
	App &app = App::get();
	if (app.getPlayerDead()) return DONE;
	CAnimationPlugger* aniP = meEntity.getSon<CAnimationPlugger>();
	return doCombo(elapsed, aniP->getActualPlugDurationWithDelay());
}

ret_e MeleeBtExecutor::doCombo(float elapsed, float timeCombo)
{
    CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition(), ATTACK_ROTATE_SPEED*elapsed);
    if (timer.count(elapsed) >= timeCombo) {
        timer.reset();
        return DONE;
    } else {
        if (lostContact(elapsed)) {
            return DONE;
        } else {
            return STAY;
        }
    }
}

ret_e MeleeBtExecutor::chase(float elapsed)
{
	if (!playerAchiable(elapsed)) return DONE;
	msgsendProtect = false;

	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();

	CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
	CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
	CDetection* meDetection = ((Entity*)meEntity)->get<CDetection>();
	rayFront = false;
	float distFront = 4.0f;
	XMVECTOR origin = meT->getPosition() + meT->getUp();
	XMVECTOR dirFront = XMVector3Normalize(playerT->getPosition() - meT->getPosition());
	PxReal distance = 4.0f;
	PxRaycastBuffer hit;
	//Front ray
	if (PhysicsManager::get().raycast(origin, dirFront, distance, hit,
		filter_t(
		filter_t::NONE,
		filter_t::id_t(filter_t::PLAYER | filter_t::PAINT_SPHERE | filter_t::ENEMY | filter_t::BULLET | filter_t::CANNONPATH | filter_t::KNIFE),
		filter_t::ALL_IDS)))
	{
		rayFront = true;
	}
	if (!obstacleDetected){
		if (rayFront && doubtDirection == 0){
			float yaw = rad2deg(getYawFromVector(meT->getPosition() - playerT->getPosition()));
			float yawFront = rad2deg(getYawFromVector(-meT->getFront()));
			float dif = yawFront - yaw;
			//Vemos si tenemos a Vinedetta a nuestra izquierda o derecha
			if (dif >= 0 || dif <= -180){
				//dbg("Enemy tiene a Player a su RIGHT\n");
				doubtDirection = 1;
				obstacleDetected = true;
				timer.reset();
				dirEnemy = meT->getFront() * 0.5f + meT->getLeft() * 0.5f;
			}
			else{
				//dbg("Enemy tiene a Player a su LEFT\n");
				doubtDirection = 2;
				obstacleDetected = true;
				timer.reset();
				dirEnemy = meT->getFront() * 0.5f - meT->getLeft() * 0.5f;
			}
			changeDir = true;
		}
		else{
			changeDir = false;
		}
	}
	else{
		if (timer.count(elapsed) > 0.5f){
			timer.reset();
			obstacleDetected = false;
			changeDir = false;
			doubtDirection = 0;
		}
	}
	
	if (!changeDir){
		alignXZ(meTransform, playerTransform->getPosition(), CHASE_ROTATE_SPEED*elapsed);
	}
	else{
		alignXZ(meT, meT->getPosition() + dirEnemy, CHASE_ROTATE_SPEED*elapsed);
	}

	//We have to avoid melee to move forward if we jump in front of him (we go away in Y but not in XZ).
	float x = XMVectorGetX(meTransform->getPosition()) - XMVectorGetX(playerTransform->getPosition());
	float z = XMVectorGetZ(meTransform->getPosition()) - XMVectorGetZ(playerTransform->getPosition());
	float distEnemyPlayerXZ = sqrt(x*x + z*z);
	if (forget(elapsed)) {
		changeDir = false;
		obstacleDetected = false;
		doubtDirection = 0;
		return DONE;
	}
	else{
		if (distEnemyPlayerXZ < INTRO_ATTACK_DISTANCE){
			//didTouchPlayer = false;
			changeDir = false;
			obstacleDetected = false;
			doubtDirection = 0;
			return DONE;
		}
		xzVelocity = meTransform->getFront();
		movSpeed = CHASE_SPEED;
		return STAY;
	}
}

ret_e MeleeBtExecutor::waitintroAttack(float elapsed)
{
	if (timer.count(elapsed) >= TIME_INTRO_ATTACK_WAIT) {
		timer.reset();
		return DONE;
	}
	return STAY;
}

ret_e MeleeBtExecutor::introAttack(float elapsed)
{
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();

	switch (whichSlot()) {
	case 0:
		pos = playerTransform->getPosition();
		break;
	case 1:
		pos = playerTransform->getPosition() - meTransform->getLeft();
		break;
	case 2:
		pos = playerTransform->getPosition() + meTransform->getLeft();
		break;
	default: break;
	}

	if (!contact(elapsed)){
		alignXZ(meTransform, pos, INTRO_ATTACK_ROTATE_SPEED*elapsed);
		xzVelocity = meTransform->getFront();
		movSpeed = INTRO_ATTACK_SPEED;
	} else {
		return DONE;
	}
	if (timer.count(elapsed) > aniP->getActualPlugDuration() - 0.3f){	//Time of animation he ends
		timer.reset();
		return DONE;
	}
	return STAY;
}

void MeleeBtExecutor::updatePosition(float elapsed)
{	
	if (!onClif(elapsed)){
		Entity* me = meEntity;
		CCharacterController* charControl = me->get<CCharacterController>();
        assert(charControl != nullptr);
		charControl->setDisplacement(xzVelocity * elapsed * movSpeed);
		xzVelocity = utils::zero_v;
		movSpeed = 1;
	}
}

ret_e MeleeBtExecutor::angry(float elapsed)
{	
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition(), ANGRY_ROTATE_SPEED*elapsed);

	CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();
    if (timer.count(elapsed) >= aniP->getActualPlugDuration()) {
        timer.reset();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e MeleeBtExecutor::randomIdleFight(float) {
    //angerTime = std::max(rand_normal(1.f, 1.f), MINIMUM_IDLE_FIGHT);
	idleFightTime = std::max(rand_normal(1.f, 1.f), MINIMUM_IDLE_FIGHT);
    return DONE;
}

ret_e MeleeBtExecutor::randomAngry(float) {	
	angerFactor = rand_uniform(1.f);
    return DONE;
}

ret_e MeleeBtExecutor::clifTaunt(float elapsed)
{
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition(), ANGRY_ROTATE_SPEED*elapsed);
	return DONE;
}

ret_e MeleeBtExecutor::back(float elapsed)
{	
	//Reset variables
	changeDir = false;
	doubtDirection = 0;
	msgsendProtect = false;
	obstacleDetected = false;
	rayFront = false;

	//Unsubscribe slot if necessary
	if (subscribedToSlot){
		Entity* player = playerEntity;
		CSlots* slots = player->get<CSlots>();
		assert(slots != nullptr);
		slots->unsubscribe(meEntity);
		subscribedToSlot = false;
	}

	CEnemy* enemyAi = enemyAi_h;
    enemyAi->endOffensive();
    return DONE;
}

bool MeleeBtExecutor::canSubscribe(float elapsed) const
{
	Entity* player = playerEntity;
    CSlots* slots = player->get<CSlots>();
    assert (slots != nullptr);
    return slots->canSubscribe(meEntity);
}

ret_e MeleeBtExecutor::subscribeSlot(float elapsed)
{
	//dbg("subscribeSlot\n");
	Entity* player = playerEntity;
    CSlots* slots = player->get<CSlots>();
    assert (slots != nullptr);
    if (slots->subscribe(meEntity)) {
		//dbg("DONE\n");
        subscribedToSlot = true;
    }
    return DONE;
}

ret_e MeleeBtExecutor::unsubscribeSlot(float elapsed)
{
	if (subscribedToSlot){
		Entity* player = playerEntity;
        if (player != nullptr) {
		    CSlots* slots = player->get<CSlots>();
		    if(slots != nullptr) {
		        slots->unsubscribe(meEntity);
            }
        }
		subscribedToSlot = false;
	}
	return DONE;
}

ret_e MeleeBtExecutor::angryFailIntro(float elapsed)
{
	if (!playerAchiable(elapsed)) return DONE;
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
	CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition(), ANGRY_ROTATE_SPEED*elapsed);
	CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();
	if (timer.count(elapsed) >= aniP->getActualPlugDuration()) {
		timer.reset();
		return DONE;
	}
	return STAY;
}

bool MeleeBtExecutor::canTaunt(float elapsed) const
{
	if (subscribedToSlot || outsideAttackRange(elapsed) || !playerAchiable(elapsed)){
		return false;
	}
	Entity* player = playerEntity;
	CSlots* slots = player->get<CSlots>();
	assert(slots != nullptr);
	return !slots->canSubscribe(meEntity);
}

ret_e MeleeBtExecutor::taunt(float elapsed)
{
	CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
    CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
	alignXZ(meTransform, playerTransform->getPosition(), ANGRY_ROTATE_SPEED*elapsed);
	if (!playerAchiable(elapsed)) return DONE;
    if (outsideAttackRange(elapsed)) {
        return DONE;
    } else {
        Entity* player = playerEntity;
        CSlots* slots = player->get<CSlots>();
        assert (slots != nullptr);
        if (slots->canSubscribe(meEntity)) {
            return DONE;
        } else {
            return STAY;
        }
    }
}

void MeleeBtExecutor::onTransform()
{
    Entity* me = meEntity;
    if (me->has<CKnife>()) {me->get<CKnife>().destroy();}
	if (subscribedToSlot){
		Entity* player = playerEntity;
		CSlots* slots = player->get<CSlots>();
		assert(slots != nullptr);
		slots->unsubscribe(meEntity);
		subscribedToSlot = false;
	}
}

void CMelee::initType()
{
    MeleeBt::initType();
    SUBSCRIBE_MSG_TO_MEMBER(CMelee, MsgTransform, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CMelee, MsgSetPlayer, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CMelee, MsgStun, receive);
}


void CMelee::update(float elapsed)
{
	bt.update(elapsed);
	bt.getExecutor().updatePosition(elapsed);

	if (bt.hasActionChanged()){
		Entity* me = bt.getExecutor().meEntity;
		CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
		auto currentAction = bt.getCurrentAction();
		uint32_t action = currentAction & 0xFFFF;
		if (action == 0xB064)	animPlugger->plug(0xB028);		//Intro attack ends to idle_fight, not run.
		if (action == 0xB005 || action == 0xB006 || action == 0xB008)	animPlugger->plug(0xB01D);		//When stunned we must unplug lost anim.
		animPlugger->plug(action);
	}
}

void CMelee::init()
{
    Handle h(this);
    Handle meEntity = h.getOwner();
    MeleeBtExecutor& btEx = bt.getExecutor();
    btEx.meEntity = meEntity;
    Handle enemyAi_h = ((Entity*) meEntity)->get<CEnemy>();
    btEx.enemyAi_h = enemyAi_h;
    ((CEnemy*) enemyAi_h)->setOffensive(h);
    bt.init();
}


void CMelee::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    introDamage         = atts.getInt("damage_intro", introDamage);
    combo1Damage        = atts.getInt("damage_combo1", combo1Damage);
    combo2Damage        = atts.getInt("damage_combo2", combo2Damage);
    combo3Damage        = atts.getInt("damage_combo3", combo3Damage);
    idleDamage          = atts.getInt("damage_idle", idleDamage);
    stunDamage          = atts.getInt("damage_stun", stunDamage);
    aggressiveDamage    = atts.getInt("damage_aggressive", aggressiveDamage);
}

int CMelee::getDamage()
{
    CEnemy* enemy = Handle(this).getBrother<CEnemy>();
    auto action(enemy->getAction());
    if ((action & EnemyBtExecutor::COD_DEAD) != 0) {
        return 0;
    } else if ((action & EnemyBtExecutor::COD_STUN) != 0) {
        return stunDamage;
    } else if (action == EnemyBtExecutor::OFFENSIVE) {
        switch(getAction()) {
            case MeleeBtExecutor::DASH: return introDamage; break;
            case MeleeBtExecutor::COMBO1: return combo1Damage; break;
            case MeleeBtExecutor::COMBO2: return combo2Damage; break;
            case MeleeBtExecutor::COMBO3: return combo3Damage; break;
            default: {
                return aggressiveDamage;
            }
        }
    } else {
        return idleDamage;
    } 
}
}
