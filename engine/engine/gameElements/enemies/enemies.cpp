#include "mcv_platform.h"
#include "enemies.h"

using namespace DirectX;

using namespace utils;

#include "components/transform.h"
#include "components/color.h"
#include "components/detection.h"
using namespace component;

#include "animation/animationPlugger.h"
#include "animation/cskeleton.h"
using namespace animation;

#include "gameElements/enemies/flare.h"
#include "gameElements/enemies/melee.h"
using namespace gameElements;

#include "Particles/ParticleSystem.h"
using namespace particles;

#define TIME_TRANSFORM		2.f
#define TIME_ALERT			0.5f
#define TIME_PROTECT		0.3f
#define TIME_SHOT			0.3f
#define TIME_TACKLED		0.25f
#define TIME_QUAKED			TIME_TACKLED

#define TIME_LOOK_AROUND    0.5f

#define TIME_SHOW_ONSHOT	 1.5f

#define ALERT_RADIUS     20.0f
#define ALERT_HEIGHT     7.0f
#define ALERT_ROTATE_SPEED deg2rad(360)

#define FORGET_DISTANCE		30.0f		//Same as melee
#define HEIGHT_DIST			1.0f		//Same as melee

namespace behavior {
	EnemyBt::nodeId_t EnemyBt::rootNode = INVALID_NODE;
	EnemyBt::container_t EnemyBt::nodes;
	void EnemyBt::initType()
	{

        //NOTE!!! Turn autoformatting off because it renders these completelly unreadable

		/*                node-id             parent-id           node-type   condition        action           */
		BT_CREATEROOT	 (ENEMY									, PRIORITY);
		BT_CREATECHILD_CA(EVENT				, ENEMY				, PRIORITY	, hasEvents		, treatEvents		);
		BT_CREATECHILD_C (TRANSFORM			, EVENT				, SEQUENCE	, wasTransformed					);
		BT_CREATECHILD_A (INIT_TRANSFORMING	, TRANSFORM			, CHAIN						, initTransforming	);
		BT_CREATECHILD_A (TRANSFORMING		, INIT_TRANSFORMING	, LEAF						, transform			);
		BT_CREATECHILD_A (TRANSFORMED		, TRANSFORM			, LEAF						, stay				);
		BT_CREATECHILD_C (STUN				, EVENT				, SEQUENCE	, isStunned							);
		BT_CREATECHILD	 (STUN_CAUSE		, STUN				, PRIORITY										);
		BT_CREATECHILD_CA(TACKLE			, STUN_CAUSE		, LEAF		, wasTackled	, tackledAnim		);
		BT_CREATECHILD_A (RANDOM_STUN		, STUN				, CHAIN						, randomStunned		);
		BT_CREATECHILD_A (STUNNED			, RANDOM_STUN		, LEAF						, stunned			);
		BT_CREATECHILD_A (RANDOM_ANGRY		, STUN				, CHAIN						, randomAngry		);
		BT_CREATECHILD_CA(ANGRY				, RANDOM_ANGRY		, LEAF		, isAngry		, angry				);
		BT_CREATECHILD_C (CELEBRATE			, EVENT				, SEQUENCE	, wasCelebrate						);
		BT_CREATECHILD_A (RANDOM_WAIT		, CELEBRATE			, CHAIN						, randomWait		);
		BT_CREATECHILD_A (WAIT				, RANDOM_WAIT		, LEAF						, wait				);
		BT_CREATECHILD_A (ICELEBRATION		, CELEBRATE			, CHAIN						, initCelebration	);
		BT_CREATECHILD_A (CELEBRATION		, ICELEBRATION		, LEAF						, stay				);
		BT_CREATECHILD_CA(ALERT				, ENEMY				, LEAF		, isDetected	, doAlert			);
		BT_CREATECHILD_C (WAS_SHOT			, EVENT				, SEQUENCE	, wasShotnotAlert					);
		BT_CREATECHILD_A (ALERT2			, WAS_SHOT			, CHAIN						, doAlert			);
		BT_CREATECHILD_C (WAS_SHOT2			, EVENT				, SEQUENCE	, wasShotinDef						);
		BT_CREATECHILD_A (PROTECT			, WAS_SHOT2			, LEAF						, protect			);
		BT_CREATECHILD_CA(OFFENSIVE			, ENEMY				, LEAF		, isAlerted		, doOffensive		);
		BT_CREATECHILD_A (IDLE				, ENEMY				, LEAF						, doIdle			);
	}

}


namespace gameElements {

	const float EnemyBtExecutor::ANGER_THRESHOLD = 0.25;
	const float EnemyBtExecutor::GRAVITY = 3.5f * 9.81f;

	bool EnemyBtExecutor::playerAchiable(float) const
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
			filter_t::id_t(filter_t::PLAYER | filter_t::ENEMY | filter_t::BULLET |
                filter_t::CANNONPATH | filter_t::KNIFE | filter_t::FLARESHOT | filter_t::PAINT_SPHERE),
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

	bool EnemyBtExecutor::isDetected(float) const
	{
		if (lastEvent == E_TRANSFORMED || alert) return false;
		Entity* me(meEntity);
		Entity* player(playerEntity);
		CDetection* meDetection(me->get<CDetection>());
		CTransform* meTransform(me->get<CTransform>());
		CTransform* playerTransform(player->get<CTransform>());
		return meDetection->test(meTransform, playerTransform->getPosition());
	}

	bool EnemyBtExecutor::hasEvents(float) const
	{
		if (lastEvent == E_TRANSFORMED) return false;
		return !inbox.isEmpty();
	}

	ret_e EnemyBtExecutor::initCelebration(float)
	{
		return DONE;
	}

	ret_e EnemyBtExecutor::initTransforming(float)
	{
		((Entity*)meEntity)->sendMsg(MsgTransform());
		if (stunChannel != NULL)		fmodUser::fmodUserClass::stopAmbient3DSound(stunChannel);
		return DONE;
	}

	ret_e EnemyBtExecutor::transform(float elapsed)
	{
		Entity* me = meEntity;
		CTransform* ctransf = me->get<CTransform>();
		if (!changemesh){
			//To change mesh of an entity
			me->get<CSkeleton>().destroy();
			me->get<CAnimationPlugger>().destroy();
			me->get<CDetection>().destroy();
			me->get<CMesh>().destroy();
			CTransform* meTransform = me->get<CTransform>();
			XMVECTOR origin = meTransform->getPosition() + meTransform->getUp();
			XMVECTOR dir = -meTransform->getUp();
			PxReal distance = 5.0f;
			PxRaycastBuffer hit;
			XMVECTOR playerRayPos = meTransform->getPosition();
			//To avoid plants flying we send a raycast to the ground and move there
			if (PhysicsManager::get().raycast(origin, dir, distance, hit,
				filter_t(filter_t::NONE, ~(filter_t::STATIC), filter_t::STATIC))){
				meTransform->setPosition(XMVectorSet(XMVectorGetX(meTransform->getPosition()), XMVectorGetY(meTransform->getPosition()) - hit.block.distance + 1.0f, XMVectorGetZ(meTransform->getPosition()), XMVectorGetW(meTransform->getPosition())));
			}
			me->get<CCharacterController>().destroy();
			CMesh::load("planta_enemigo", Handle(me));
			CMesh* cmesh = me->get<CMesh>();
			CTint* self = me->get<CTint>();
			*self = 0xFFFFFFFF;
			cmesh->init();
			changemesh = true;
			char cstr[32] = "Prop_transform";
			int randomV = rand_uniform(9, 1);
			char randomC[32] = "";
			sprintf(randomC, "%d", randomV);
			strcat(cstr, randomC);
			fmodUser::fmodUserClass::play3DSingleSound(cstr, meTransform->getPosition());
		}

		if (timerTransform.count(elapsed) >= TIME_SHOW_ONSHOT && timerTransform.count(elapsed) < TIME_SHOW_ONSHOT) {
			float s = timerTransform.count(elapsed) / TIME_SHOW_ONSHOT;
			ctransf->setScale(originScaleV * s);
		}
		if (timerTransform.count(elapsed) >= TIME_SHOW_ONSHOT){
			ctransf->setScale(originScaleV);
			return DONE;
		}
			
        CEnemy* meE  = me->get<CEnemy>();
        meE->dead = true;
		return STAY;
	}

	ret_e EnemyBtExecutor::angry(float elapsed)
	{
		if (timer.count(elapsed) >= angerTime) {
			angerTime = 0;
			return DONE;
		}
		else {
			return STAY;
		}
	}

	ret_e EnemyBtExecutor::stunned(float elapsed)
	{
		Entity* me(meEntity);
		if (timer.count(elapsed) >= stunTime) {
			stunTime = 0;
			stun = false;
			if (stunChannel != NULL)		fmodUser::fmodUserClass::stopAmbient3DSound(stunChannel);
			return DONE;
		}
		else {
			CTransform* meTransform = me->get<CTransform>();
			if (stunChannel != NULL)		fmodUser::fmodUserClass::updateAmbient3DSound(stunChannel, meTransform->getPosition());
			return STAY;
		}
	}

	ret_e EnemyBtExecutor::tackledAnim(float elapsed)
	{
		if (timer.count(elapsed) >= TIME_SHOT) {
			waitTime = 0;
			return DONE;
		}
		else {
			return STAY;
		}
	}

	ret_e EnemyBtExecutor::protect(float elapsed)
	{
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		fmodUser::fmodUserClass::play3DSingleSound("melee_defend", meT->getPosition());
		return DONE;
	}

	ret_e EnemyBtExecutor::wait(float elapsed)
	{
		if (timer.count(elapsed) >= waitTime) {
			waitTime = 0;
			return DONE;
		}
		else {
			return STAY;
		}
	}

	ret_e EnemyBtExecutor::treatEvents(float elapsed)
	{
		//We will not treat event if we are "dead"
		if (lastEvent == E_TRANSFORMED){
			return DONE;
		}
		lastEvent = E_NOTHING;
		if (inbox.megashot || (stun && inbox.shot)) {
			lastEvent = E_TRANSFORMED;
			Entity* me = meEntity;
			CTransform* ctransf = me->get<CTransform>();
			originScaleV = ctransf->getScale();
			me->sendMsg(MsgTransform());
		}
		else if (inbox.tackled) {
			lastEvent = E_TACKLED;
			Entity* me = meEntity;
			me->sendMsg(MsgStun());
			stun = true;
		}
		else if (inbox.alerted) {
			alert = true;
		}
		else if (inbox.quake) {
			lastEvent = E_TRANSFORMED;
			Entity* me = meEntity;
			CTransform* ctransf = me->get<CTransform>();
			originScaleV = ctransf->getScale();
			me->sendMsg(MsgTransform());
		}
		else if (inbox.shot) {
			if (!defensive){
				lives--;
				if (lives <= 0){
					lastEvent = E_TRANSFORMED;
					Entity* me = meEntity;
					CTransform* ctransf = me->get<CTransform>();
					originScaleV = ctransf->getScale();
					me->sendMsg(MsgTransform());
				}
				else{
					CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();
					aniP->plug(0xB036);
					char cstr[32] = "enemy_damage";
					int randomV = rand_uniform(3, 1);
					char randomC[32] = "";
					sprintf(randomC, "%d", randomV);
					strcat(cstr, randomC);
					Entity* me = meEntity;
					CTransform* ctransf = me->get<CTransform>();
					fmodUser::fmodUserClass::play3DSingleSound(cstr, ctransf->getPosition(), 0.8f);
					lastEvent = E_SHOT;
				}
			}
			else{
				lastEvent = E_SHOT;
			}
		}
		else if (inbox.protect) {
			defensive = true;
		}
		else if (inbox.stopdef) {
			defensive = false;
		}
		if (inbox.playerDead &&	!stun && !wasTransformed(elapsed)) {
			lastEvent = E_CELEBRATE;
		}
		inbox.clear();
		return DONE;
	}

	ret_e EnemyBtExecutor::doAlert(float elapsed)
	{
		CAnimationPlugger* aniP = ((Entity*)meEntity)->get<CAnimationPlugger>();
		if (timer.count(elapsed) >= aniP->getActualPlugDuration()) {
			//dbg("end Alert\n");
			didAlert = false;
			Entity* me = meEntity;
			me->sendMsg(MsgAlert());
			return DONE;
		}
		else {
			if (!didAlert && timer.count(elapsed) >= TIME_ALERT){
				CTransform* tMe(((Entity*)meEntity)->get<CTransform>());
				for (Entity* e : EntityListManager::get(CEnemy::TAG)) {
                    if (e==nullptr) {continue;}
					CTransform* tE(e->get<CTransform>());
					CEnemy* eE(e->get<CEnemy>());
					if (testCyllinder(tE->getPosition(), tMe->getPosition(), ALERT_RADIUS, ALERT_HEIGHT) && !eE->isDead() && e != meEntity) {
						CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
						if (testDistanceSqEu(tE->getPosition(), playerTransform->getPosition(), FORGET_DISTANCE)){
							e->sendMsg(MsgAlert());
							//dbg("Msg alert send\n");
						}
					}
				}
				Entity* levi = bichitoEntity;
				levi->sendMsg(MsgAlert());
				didAlert = true;
			}
			CTransform* meTransform = ((Entity*)meEntity)->get<CTransform>();
			CTransform* playerTransform = ((Entity*)playerEntity)->get<CTransform>();
			alignXZ(meTransform, playerTransform->getPosition(), ALERT_ROTATE_SPEED*elapsed);
			return STAY;
		}
	}

	ret_e EnemyBtExecutor::doOffensive(float elapsed)
	{
		if (offensiveAi.isValid()) {
			offensiveAi.update(elapsed);
			return STAY;
		}
		else {
			return DONE;
		}

	}

	ret_e EnemyBtExecutor::doIdle(float elapsed)
	{
		if (isDetected(elapsed)) {
			return DONE;
		}
		else if (idleAi.isValid()) {
			idleAi.update(elapsed);
			return STAY;
		}
		else {
			return DONE;
		}
	}

	ret_e EnemyBtExecutor::randomAngry(float)
	{
		stunTime = utils::rand_uniform(1.f);
		return DONE;
	}

	ret_e EnemyBtExecutor::randomStunned(float)
	{
		stunTime = utils::rand_uniform(3.5f, 2.5f);
		if (stunChannel == NULL){
			fmodUser::fmodUserClass::playAmbient3DSound(stunChannel, "enemy_stun");
		}
		else{
			fmodUser::fmodUserClass::stopAmbient3DSound(stunChannel);
			fmodUser::fmodUserClass::playAmbient3DSound(stunChannel, "enemy_stun");
		}
		return DONE;
	}

	ret_e EnemyBtExecutor::randomWait(float)
	{
		waitTime = utils::rand_normal(1.f, 1.f);
		return DONE;
	}

	//Hecho anteriormente, comentado porque CREO que no es necesario, los enemigos no pueden caer 
	//(si se diera el caso caerian con la velocidad marcada en updatePositition)
	/*bool EnemyBtExecutor::onGround(float) const{
		Entity* me = meEntity;
		CTransform* meT = me->get<CTransform>();
		XMVECTOR origin = meT->getPosition();
		XMVECTOR dir = -meT->getUp();
		PxReal distance = 0.1f;
		PxRaycastBuffer hit;
		if (PhysicsManager::get().raycast(origin, dir, distance, hit,
			filter_t(
			filter_t::NONE,
			filter_t::id_t(filter_t::KNIFE | filter_t::PAINT_SPHERE | filter_t::ENEMY | filter_t::BULLET | filter_t::CANNONPATH | filter_t::FLARESHOT),
			filter_t::id_t(filter_t::TOOL | filter_t::SCENE | filter_t::SMOKE_PANEL | filter_t::TOOLS_SELECTABLE)))){
			return true;
		}
		return false;
	}*/

	void EnemyBtExecutor::updatePosition(float elapsed){
		if (!changemesh){
			/*if (!onGround(elapsed)){
				yVelocity = -yAxis_v * 8;
			}
			else{
				yVelocity = zero_v;
			}*/
			yVelocity = -yAxis_v * 8;
			Entity* me = meEntity;
			CCharacterController* charControl = me->get<CCharacterController>();
			charControl->setDisplacement(yVelocity * elapsed * movSpeed);
		}
	}

	void CEnemy::update(float elapsed)
	{
        if (dead) {return;}
		bt.update(elapsed);
		bt.getExecutor().updatePosition(elapsed);

		if (bt.hasActionChanged() && !bt.getExecutor().wasTransformed(elapsed)) {
			CAnimationPlugger* animPlugger(Handle(this).getBrother<CAnimationPlugger>());
			uint32_t action = bt.getCurrentAction() & 0xFFFF;
			animPlugger->plug(action);
			if (action == 0xB014){
				animPlugger->plug(0xB01A);
				animPlugger->plug(0xB01B);
				animPlugger->plug(0xB01C);
			}	
		}

		//-------------------------------------------delete after test-----------------------------------------//

		if (Handle(this).hasBrother<CParticleSystem>() && (getAction() == EnemyBtExecutor::TRANSFORMED)){
			if (timerOffParticles.count(elapsed) >= 7){
				Handle(this).getBrother<CParticleSystem>().destroy();	
			}
		}

		//-------------------------------------------delete after test-----------------------------------------//
	}

	void CEnemy::initType()
	{
		EnemyBt::initType();
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgSetPlayer, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgSetBichito, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgDashTackled, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgEarthquake, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgProtect, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgShot, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgPlayerDead, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgAlert, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgStopDefense, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CEnemy, MsgTransform, receive);
	}

}

