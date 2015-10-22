#include "mcv_platform.h"
#include "bichito.h"

#include "app.h"

using namespace DirectX;

using namespace utils;

#include "components/transform.h"
#include "components/color.h"
#include "components/detection.h"
using namespace component;

#include "animation/animationPlugger.h"
#include "animation/cskeleton.h"
using namespace animation;

#include "gameElements/player/playerStats.h"
#include "gameElements/player/bullet.h"
#include "gameElements/player/playermov.h"
using namespace gameElements;

#include "render/camera/component.h"

#define TIME_POINT_TOOL			8.0f
#define TIME_ALERT_ANIM			3.0f
#define TIME_ALERT_TRANSFORM	1.0f
#define TIME_MIN_TRANSFORM		5.0f
#define TIME_MAX_TRANSFORM		10.0f
#define TIME_WAIT_FOLLOW		1.5f
#define TIME_FALLING_DEAD		1.0f
#define DISTANCE_TOOL_PLAYER	10.0f
#define DISTANCE_MAX_FAR		15.0f
#define DISTANCE_TP_FAR			25.0f
#define DISTANCE_CLOSE_PLAYER	1.0f
#define DISTANCE_IDLE_PLAYER	4.5f
#define DISTANCE_CLOSE_PROP		3.0f
#define DISTANCE_PLAYER_PROP	8.0f
#define SPEED_CHASE				15.0f
#define SPEED_TRANSFORM			10.0f
#define SPEED_FOLLOW			7.5f
#define CHASE_ROTATE_SPEED		deg2rad(840)
#define BICHITO_HEIGHT			1.0f
#define BICHITO_LATERAL			0.5f
#define SPEED_SIN_HEIGHT		2
#define FORCE_SIN_HEIGHT		0.1f

namespace behavior {
	BichitoBt::nodeId_t BichitoBt::rootNode = INVALID_NODE;
	BichitoBt::container_t BichitoBt::nodes;
	void BichitoBt::initType()
	{
		/*                node-id             parent-id           node-type   condition        action           */
		BT_CREATEROOT	 (BICHITO								, PRIORITY										);
		BT_CREATECHILD_CA(EVENT				, BICHITO			, PRIORITY	, hasEvents		, treatEvents		);
		BT_CREATECHILD_CA(TELEPORTTOPLAYER	, EVENT				, LEAF		, isTeleport	, teleportToPlayer	);
		BT_CREATECHILD_CA(FIGHT				, EVENT				, SEQUENCE	, isInFight		, goToPlayer		);
		BT_CREATECHILD_CA(ALERTENEMY		, FIGHT				, LEAF		, canAlert		, doAlertEnemy		);
		BT_CREATECHILD_A (BATTLE			, FIGHT				, LEAF						, doFight			);
		BT_CREATECHILD_A (VICTORY			, FIGHT				, LEAF						, doVictory			);
		BT_CREATECHILD_CA(LOWHP				, EVENT				, CHAIN		, isLowHP		, goToPlayer		);
		BT_CREATECHILD_A (ALERTHP			, LOWHP				, LEAF						, doAlertHP			);
		BT_CREATECHILD_CA(QUAKE				, EVENT				, CHAIN		, isInQuake		, goToPlayer		);
		BT_CREATECHILD_A (ALERTQUAKE		, QUAKE				, LEAF						, doAlertQuake		);
		BT_CREATECHILD_CA(DEAD				, EVENT				, SEQUENCE	, isPlayerDead	, goToPlayer		);
		BT_CREATECHILD_A (DEADRAY			, DEAD				, LEAF						, deadCalculus		);
		BT_CREATECHILD_A (ALERTDEAD			, DEAD				, LEAF						, doAlertDead		);
		BT_CREATECHILD_CA(TUTORIAL			, EVENT				, CHAIN		, isInTuto		, goToPlayer		);
		BT_CREATECHILD_A (ALERTTUTORIAL		, TUTORIAL			, LEAF						, doAlertTuto		);
		BT_CREATECHILD_C (TOOLDETECTED		, BICHITO			, SEQUENCE	, toolClose		 					);
		BT_CREATECHILD_A (GOTOTOOL			, TOOLDETECTED		, LEAF						, goToProp			);
		BT_CREATECHILD_CA(ALERTTOOL			, TOOLDETECTED		, LEAF		, toolNtransf	, pointTool			);
		BT_CREATECHILD_C (PROPCLOSE			, BICHITO			, SEQUENCE	, propClose							);
		BT_CREATECHILD_A (RANDOMWAITPROP	, PROPCLOSE			, CHAIN						, randomWait		);
		BT_CREATECHILD_A (WAITPROP			, RANDOMWAITPROP	, LEAF						, waitProp			);
		BT_CREATECHILD_CA(REFRESHPROP		, PROPCLOSE			, LEAF		, isnotCancel	, refreshProp		);
		BT_CREATECHILD_CA(GOTOPROP			, PROPCLOSE			, LEAF		, isnotCancel	, goToProp			);
		BT_CREATECHILD_CA(TRANSFORMPROP		, PROPCLOSE			, LEAF		, isnotCancel	, transformProp		);
		BT_CREATECHILD_CA(LOSTPLAYER		, BICHITO			, LEAF		, lostPlayer	, goToPlayer		);
		BT_CREATECHILD_CA(TPTOPLAYER		, BICHITO			, LEAF		, playerGone	, teleportToPlayer	);
		BT_CREATECHILD_A (IDLE				, BICHITO			, LEAF						, doIdle			);
	}
}

namespace gameElements {

	bool BichitoBtExecutor::hasEvents(float) const
	{
		return !inbox.isEmpty();
	}

	bool canBeTransformedandClose(Handle h, XMVECTOR pos, float distance){
		Entity* prop = h;
		CTransformable* tt = prop->get<CTransformable>();
		if (!tt->isTransformed()){
			//CTransform* t = prop->get<CTransform>();
			if (testDistanceSqEu(pos, tt->getCenterAim(), distance)){
				//dbg("Elemet found\n");
				return true; 
			}
		}	
		return false;
	}

	bool BichitoBtExecutor::toolClose(float elapsed) const
	{
		if (lostPlayer(elapsed))	return false;
		
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		for (auto& h : EntityListManager::get(CTrampoline::TAG)) {
			if (!h.isValid()) { continue; }
			if (canBeTransformedandClose(h, playerT->getPosition(), DISTANCE_TOOL_PLAYER)){
				const_cast <Handle &> (propEntity) = h;
				return true;
			}
		}
		for (auto& h : EntityListManager::get(CCannon::TAG)) {
			if (!h.isValid()) { continue; }
			if (canBeTransformedandClose(h, playerT->getPosition(), DISTANCE_TOOL_PLAYER)){
				const_cast <Handle &> (propEntity) = h;
				return true;
			}
		}
		for (auto& h : EntityListManager::get(CLiana::TAG)) {
			if (!h.isValid()) { continue; }
			if (canBeTransformedandClose(h, playerT->getPosition(), DISTANCE_TOOL_PLAYER)){
				const_cast <Handle &> (propEntity) = h;
				return true;
			}
		}
		for (auto& h : EntityListManager::get(CCreep::TAG)) {
			if (!h.isValid()) { continue; }
			if (canBeTransformedandClose(h, playerT->getPosition(), DISTANCE_TOOL_PLAYER)){
				const_cast <Handle &> (propEntity) = h;
				return true;
			}
		}		
		return false;
	}

	bool BichitoBtExecutor::toolNtransf(float elapsed) const
	{
		CTransformable* propT = ((Entity*)propEntity)->get<CTransformable>();
		if(!propT->isTransformed()) return true;
		return false;
	}

	bool BichitoBtExecutor::propClose(float elapsed) const
	{
		if (lostPlayer(elapsed))	return false;
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		for (auto& h : EntityListManager::get(CProp::TAG)) {
			if (!h.isValid() || h == propEntity) { continue; }
			if (canBeTransformedandClose(h, playerT->getPosition(), DISTANCE_PLAYER_PROP)){				
				const_cast <Handle &> (propEntity) = h;
				return true;
			}
		}
		return false;
	}

	bool BichitoBtExecutor::playerGone(float elapsed) const
	{
		CPlayerStats* playerS = ((Entity*)playerEntity)->get<CPlayerStats>();
		if(playerS->isPlayerDead())	return false;
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		if (playerT->getPosition() == zero_v) return false;
		if (!testDistanceSqEu(playerT->getPosition(), meT->getPosition() - levitatePos, DISTANCE_TP_FAR)){
			//dbg("playerGone: %f\n", sqEuclideanDistance(playerT->getPosition(), meT->getPosition() - levitatePos));
			return true;
		}
		return false;
	}

	bool BichitoBtExecutor::lostPlayer(float elapsed) const
	{
		if (playerGone(elapsed)) return false;
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		if (playerT->getPosition() == zero_v) return false;
		if (!testDistanceSqEu(playerT->getPosition(), meT->getPosition() - levitatePos, DISTANCE_TP_FAR)){
			//dbg("lostPlayer: %f\n", sqEuclideanDistance(playerT->getPosition(), meT->getPosition() - levitatePos));
			return true;
		}
		return false;
	}

	ret_e BichitoBtExecutor::refreshProp(float elapsed)
	{
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		for (auto& h : EntityListManager::get(CProp::TAG)) {
			if (!h.isValid() || h == propEntity) { continue; }
			if (canBeTransformedandClose(h, playerT->getPosition(), DISTANCE_PLAYER_PROP)){
				Entity* prop = h;
				CTransformable* tt = prop->get<CTransformable>();
				if (!tt->isTransformed()){
					if (testDistanceSqEu(playerT->getPosition(), tt->getCenterAim(), DISTANCE_PLAYER_PROP)){
						float distPropPlayer = sqEuclideanDistance(playerT->getPosition(), tt->getCenterAim());
						if (distPropPlayer < max_dist){
							max_dist = distPropPlayer;
							const_cast <Handle &> (propEntity) = h;
						}
					}
				}
			}
		}
		max_dist = 1000000.0f;
		return DONE;
	}

	ret_e BichitoBtExecutor::treatEvents(float elapsed)
	{		
		lastEvent = E_NOTHING;
		if (inbox.enemy && bichitoRdyToMove){
			//dbg("inbox Enemy\n");
			lastEvent = E_FIGHT;
		}
		if (inbox.lowHP){
			//dbg("inbox lowHP\n");
			lastEvent = E_LOW_HP;
		}
		if (inbox.quake){
			//dbg("inbox lowHP\n");
			lastEvent = E_EARTHQUAKE;
		}
		if (inbox.dead){
			//dbg("inbox lowHP\n");
			lastEvent = E_DEAD;
		}
		if (inbox.teleport){
			//dbg("inbox tp\n");
			lastEvent = E_TELEPORT;
			doingTP = true;
		}
		if (inbox.tutorial){
			//dbg("inbox tuto\n");
			lastEvent = E_TUTO;
		}
		inbox.clear();
		return DONE;
	}

	ret_e BichitoBtExecutor::doAlertEnemy(float elapsed)
	{
		//dbg("Bichito Alert Enemy!\n");
		if (timer.count(elapsed) >= TIME_ALERT_ANIM) {
			timer.reset();
			canAlertEnemy = false;
			helperType = -1;
			return DONE;
		}
		helperType = 2;
		return STAY;
	}

	ret_e BichitoBtExecutor::doFight(float elapsed)
	{
		//dbg("Bichito Fight!\n");
		for (auto& h : EntityListManager::get(CEnemy::TAG)) {
			if (h == meEntity) { continue; }
			Entity* him(h);
			if (him == nullptr) { continue; }
			CEnemy* eE(him->get<CEnemy>());
			if (!eE->isPassive()){
				helperType = 2;
				return STAY;
			}
		}
		return DONE;
	}

	ret_e BichitoBtExecutor::doVictory(float elapsed)
	{
		//dbg("Bichito Victory!\n");
		if (timer.count(elapsed) >= TIME_ALERT_ANIM) {
			timer.reset();
			canAlertEnemy = true;
			playSound = true;
			helperType = -1;
			return DONE;
		}
		if(playSound){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();			
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/levi_alert_win"), meT->getPosition());
			playSound = false;
		}
		helperType = 8;
		return STAY;
	}

	ret_e BichitoBtExecutor::doAlertHP(float elapsed)
	{
		//dbg("Bichito Alert HP!\n");
		if (timer.count(elapsed) >= TIME_ALERT_ANIM) {
			timer.reset();
			for (auto& h : EntityListManager::get(CEnemy::TAG)) {
				if (h == meEntity) { continue; }
				Entity* him(h);
				if (him == nullptr) { continue; }
				CEnemy* eE(him->get<CEnemy>());
				if (!eE->isPassive()){
					Entity* me = meEntity;
					me->sendMsg(MsgAlert());
					playSound = true;
					helperType = -1;
					return DONE;
				}
			}
			playSound = true;
			helperType = -1;
			return DONE;
		}
		if(playSound){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/levi_alert_hp"), meT->getPosition());
			playSound = false;
		}
		helperType = 3;
		return STAY;
	}

	ret_e BichitoBtExecutor::doAlertTuto(float elapsed)
	{
		//dbg("Bichito Alert Tuto!\n");
		if (timer.count(elapsed) >= TIME_ALERT_ANIM) {
			timer.reset();
			canAlertEnemy = false;
			playSound = true;
			helperType = -1;
			return DONE;
		}
		if(playSound){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/levi_alert_tuto"), meT->getPosition());
			playSound = false;
		}
		helperType = 7;
		return STAY;
	}

	ret_e BichitoBtExecutor::doAlertQuake(float elapsed)
	{
		//dbg("Bichito Alert Quake!\n");
		if (timer.count(elapsed) >= TIME_ALERT_ANIM) {
			timer.reset();
			canAlertEnemy = false;
			playSound = true;
			helperType = -1;
			return DONE;
		}
		if(playSound){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/levi_alert_earthquake"), meT->getPosition());
			playSound = false;
		}
		helperType = 4;
		return STAY;
	}

	ret_e BichitoBtExecutor::deadCalculus(float elapsed)
	{
		//dbg("deadCalculus!\n");
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		XMVECTOR origin = meT->getPosition();
		XMVECTOR dir = -yAxis_v;
		PxReal distance = 5.0f;
		PxRaycastBuffer hit;
		if (PhysicsManager::get().raycast(origin, dir, distance, hit,
			filter_t(
			filter_t::NONE,
			~filter_t::id_t(filter_t::SCENE | filter_t::ENEMY | filter_t::PROP),
			filter_t::ALL_IDS))){
			speedFallDead = hit.block.distance / TIME_FALLING_DEAD;
		}
		else{
			speedFallDead = 0.0f;
		}
		return DONE;
	}

	ret_e BichitoBtExecutor::doAlertDead(float elapsed)
	{
		//dbg("Bichito Alert Dead!\n");
		float tim = timer.count(elapsed);
		if (tim >= TIME_ALERT_ANIM) {
			timer.reset();
			canAlertEnemy = false;
			playSound = true;
			return DONE;
		}
		if (tim < TIME_FALLING_DEAD){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
			align3D(meT, playerT->getPosition() + playerT->getFront() * 1000, 10);
			bichitoPos -= meT->getUp() * speedFallDead * elapsed;
		}
		isDead = true;
		return STAY;
	}

	ret_e BichitoBtExecutor::pointTool(float elapsed)
	{
		//dbg("pointTool\n");
		if (timer.count(elapsed) >= TIME_POINT_TOOL) {
			timer.reset();
			playSound = true;
			return DONE;
		}
		if(playSound){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/levi_alert_tool"), meT->getPosition());
			playSound = false;
		}
		helperType = 6;
		return STAY;
	}

	ret_e BichitoBtExecutor::randomWait(float elapsed)
	{	
		timeWaitProp = utils::rand_uniform(TIME_MAX_TRANSFORM, TIME_MIN_TRANSFORM);
		//dbg("new randomWait\n");
		return DONE;
	}

	ret_e BichitoBtExecutor::waitProp(float elapsed)
	{
		//dbg("waitprop %f of %f!\n", timer.count(elapsed), timeWaitProp);
		if (timer.count(elapsed) >= timeWaitProp) {
			timer.reset();
			return DONE;
		}
		if (lostPlayer(elapsed)){
			//dbg("AAA\n");
			timer.reset();
			return DONE;
		}
		if (toolClose(elapsed)){
			//dbg("BBB\n");
			timer.reset();
			return DONE;
		}
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		if (rdyToFollow){
			if (!testDistanceSqEu(meT->getPosition() - levitatePos, playerT->getPosition(), DISTANCE_CLOSE_PLAYER)){
				align3D(meT, playerT->getPosition() + levitatePos, CHASE_ROTATE_SPEED * elapsed);
				bichitoPos += meT->getFront() * elapsed * SPEED_FOLLOW;
			}
			else{
				rdyToFollow = false;
			}
		}
		else{
			if (timerFollow.count(elapsed) >= TIME_WAIT_FOLLOW) {
				timerFollow.reset();
				rdyToFollow = true;
			}
		}
		return STAY;
	}

	ret_e BichitoBtExecutor::goToPlayer(float elapsed)
	{
		helperType = -1;
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();

		align3D(meT, playerT->getPosition() + levitatePos, CHASE_ROTATE_SPEED * elapsed);
		bichitoPos += meT->getFront() * elapsed * SPEED_CHASE;

		if (testDistanceSqEu(meT->getPosition() - levitatePos, playerT->getPosition(), DISTANCE_CLOSE_PLAYER)){
			bichitoPos = zero_v;
			playerPos = meT->getPosition() - levitatePos;
			return DONE;
		}
		else{
			//dbg("goToPlayer\n");
			return STAY;
		}
	}

	XMVECTOR BichitoBtExecutor::calculateAimAngle(XMVECTOR target, XMVECTOR origin, float offsetY, float shotSpeed)
	{
		XMVECTOR d(target - origin);
		float sqv = sq(shotSpeed);
		float z = XMVectorGetZ(d);
		float x = XMVectorGetX(d);

		float dy = XMVectorGetY(d) + offsetY;
		float dz(sqrt(sq(z) + sq(x)));
		const float& g(XMVectorGetX(XMVector3Length(physX_user::toXMVECTOR(Physics.getGravity()))));
		const float r((std::sqrt(sq(sqv) - g*(g*sq(dz) + 2 * dy*sqv))));
		const float gdz = g*dz;
		float angle = std::min(std::atan2f(sqv + r, gdz), std::atan2f(sqv - r, gdz));

		XMFLOAT3 ret;
		float yaw = getYawFromVector(d);
		ret.y = shotSpeed * std::sinf(angle);
		ret.x = shotSpeed * std::cosf(angle) * std::sinf(yaw);
		ret.z = shotSpeed * std::cosf(angle) * std::cosf(yaw);
		return XMVectorSet(ret.x, ret.y, ret.z, 0);
	}

	ret_e BichitoBtExecutor::transformProp(float elapsed)
	{
		//dbg("transformProp\n");
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		CTransformable* propT = ((Entity*)propEntity)->get<CTransformable>();
		align3D(meT, propT->getCenterAim(), CHASE_ROTATE_SPEED * elapsed);
		if (((Entity*)propEntity)->has<CTrampoline>() || ((Entity*)propEntity)->has<CCannon>() ||
			((Entity*)propEntity)->has<CLiana>() || ((Entity*)propEntity)->has<CCreep>()){
			//dbg("this should not happen\n");
			return DONE;
		}
		if (timer.count(elapsed) >= TIME_ALERT_TRANSFORM) {
			timer.reset();

			XMVECTOR vel = calculateAimAngle(propT->getCenterAim(), meT->getPosition(), 0.0f, CBullet::BULLET_SPEED);
			Entity* bulletEntity = PrefabManager::get().prefabricate("bullet-regular");
			bulletEntity->init();
			CBullet* bullet = bulletEntity->get<CBullet>();
			//The front value varies from the begining of the func shot looking at the back
			bullet->setup(meT->getPosition(), vel, XMQuaternionRotationAxis(yAxis_v, getYawFromVector(propT->getCenterAim() - meT->getPosition())));
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/levi_alert_earthquake"), meT->getPosition());
			playSound = false;
			return DONE;
		}
		return STAY;
	}

	ret_e BichitoBtExecutor::goToProp(float elapsed)
	{
		if (lostPlayer(elapsed))	return DONE;

		helperType = -1;

		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		CTransformable* propT = ((Entity*)propEntity)->get<CTransformable>();

		align3D(meT, propT->getCenterAim() /*- levitatePos*/, CHASE_ROTATE_SPEED * elapsed);
		bichitoPos += meT->getFront() * elapsed * SPEED_TRANSFORM;

		if (testDistanceSqEu(meT->getPosition() - levitatePos, propT->getCenterAim(), DISTANCE_CLOSE_PROP)){
			bichitoPos = zero_v;
			playerPos = meT->getPosition() - levitatePos;
			return DONE;
		}
		else{
			return STAY;
		}
	}

	ret_e BichitoBtExecutor::doIdle(float elapsed)
	{
		helperType = -1;
		isDead = true;
		//dbg("idle\n");
		if (bichitoRdyToMove){
			CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
			CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
			if (rdyToFollow){
				if (!testDistanceSqEu(meT->getPosition() - levitatePos, playerT->getPosition(), DISTANCE_CLOSE_PLAYER)){
					align3D(meT, playerT->getPosition() + levitatePos, CHASE_ROTATE_SPEED * elapsed);
					bichitoPos += meT->getFront() * elapsed * SPEED_FOLLOW;
					//dbg("AAA\n");
				}
				else{
					//dbg("BBB\n");
					rdyToFollow = false;
					timerFollow.reset();
				}
			}
			else{
				//dbg("CCC %f\n", timerFollow.count(elapsed));
				align3D(meT, playerT->getPosition() + levitatePos + playerT->getFront() * 10, (CHASE_ROTATE_SPEED / 5) * elapsed);
				if (timerFollow.count(elapsed) >= TIME_WAIT_FOLLOW) {
					if (!testDistanceSqEu(meT->getPosition() - levitatePos, playerT->getPosition(), DISTANCE_IDLE_PLAYER)){
						timerFollow.reset();
						rdyToFollow = true;
					}	
				}
			}
		}	
		return DONE;
	}

	ret_e BichitoBtExecutor::teleportToPlayer(float elapsed)
	{
		CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
		if (playerT->getPosition() == zero_v)	return STAY;
		helperType = -1;
		bichitoPos = zero_v;
		playerPos = playerT->getPosition();
		bichitoRdyToMove = true;
		doingTP = false;
		CTransform* meT = ((Entity*)meEntity)->get<CTransform>();
		meT->setRotation(playerT->getRotation());
		rdyToFollow = false;
		return DONE;
	}

	bool hasToBecomeGhost(XMVECTOR pos, XMVECTOR dir){
		PxReal distance = 0.5f;
		PxRaycastBuffer hit;
		if (PhysicsManager::get().raycast(pos, dir, distance, hit,
			filter_t(filter_t::NONE, filter_t::PLAYER | filter_t::PICKUP | filter_t::PAINT_SPHERE, filter_t::ALL_IDS))){
			return true;
		}
		return false;
	}

	void BichitoBtExecutor::updateSpeed(float elapsed)
	{
		Entity* me = meEntity;
		CTransform* t = me->get<CTransform>();
		totaltime += elapsed;
		if(!isDead){
			levitatePos = BICHITO_LATERAL * zAxis_v + BICHITO_HEIGHT * yAxis_v + (FORCE_SIN_HEIGHT * yAxis_v * sin(totaltime * SPEED_SIN_HEIGHT));
		}	
		if (bichitoRdyToMove && !becomeGhost){
			if (hasToBecomeGhost(t->getPosition(), t->getFront())){
				becomeGhost = true;
				timerGhost.reset();
			}
		}
		if (becomeGhost){
			if (timerGhost.count(elapsed) >= 2.0f) {
				timerGhost.reset();
				if (!hasToBecomeGhost(t->getPosition(), t->getFront())){
					becomeGhost = false;
				}	
			}
		}
		t->setPosition(playerPos + bichitoPos + levitatePos);		
		//dbg("pos: %f %f %f\n", XMVectorGetX(t->getPosition()), XMVectorGetY(t->getPosition()), XMVectorGetZ(t->getPosition()));
	}

	void CBichito::renderHelpers()
	{
		CPlayerStats* playerS = ((Entity*)bt.getExecutor().playerEntity)->get<CPlayerStats>();
		CPlayerMov* playerM = ((Entity*)bt.getExecutor().playerEntity)->get<CPlayerMov>();
		if (!playerS->isPlayerDead() || playerM->isOnCannon()){
			Texture* texture = nullptr;
			switch (bt.getExecutor().helperType){
				case -1:	return;															break;
				case 2:		texture = Texture::getManager().getByName("alertenemy");		break;
				case 3:		texture = Texture::getManager().getByName("alerthp");			break;
				case 4:		texture = Texture::getManager().getByName("alertquake");		break;
				case 5:		texture = Texture::getManager().getByName("alertsmoketower");	break;
				case 6:		texture = Texture::getManager().getByName("alerttool");			break;
				case 7:		texture = Texture::getManager().getByName("alerttutorial");		break;
				case 8:		texture = Texture::getManager().getByName("alertwin");			break;
			}		

			//Method: 3d image (Billboard XZ)
			Entity* me = bt.getExecutor().meEntity;
			CTransform* t = me->get<CTransform>();
			const Technique* technique = Technique::getManager().getByName("tutoBichito");
			technique->activate();
			texture->activate(0);
			activateBlendConfig(BLEND_CFG_COMBINATIVE);
			activateZConfig(ZCFG_TEST_LT);
			XMVECTOR pos = t->getPosition() + XMVectorSet(0, 0.85f, 0, 0);
			CCamera* cam = App::get().getCamera().getSon<CCamera>();
			XMVECTOR vCam = pos - cam->getPosition();
			//Calculate the rotation that needs to be applied to the 
			//billboard model to face the current camera position using 
			//the arc tangent function.
			float angleY = deg2rad(atan2(XMVectorGetX(vCam), XMVectorGetZ(vCam)) * (180.0f / M_PIf));
			XMVECTOR vEnd = DirectX::XMQuaternionMultiply(
				DirectX::XMQuaternionRotationAxis(zAxis_v, deg2rad(180)),
				DirectX::XMQuaternionRotationAxis(yAxis_v, angleY));
			int tint = 4294967295;
			float distCamLevi = sqEuclideanDistance(pos, cam->getPosition());
			if (distCamLevi < 20.0f)		tint -= int(255 * ((20 - distCamLevi) / 20));
			setObjectConstants(XMMatrixAffineTransformation(XMVectorSet(1.5f, 1, 1, 1)/*one_v*/, zero_v, vEnd, pos));
			mesh_textured_quad_xy_centered.activateAndRender();
			activateZConfig(ZCFG_DEFAULT);
			activateBlendConfig(BLEND_CFG_DEFAULT);
		}
	}

	void CBichito::update(float elapsed)
	{		
		bt.update(elapsed);
		bt.getExecutor().updateSpeed(elapsed);
		if (bt.getExecutor().becomeGhost){
			//Shader for ghost or FX?
		}
		//Animations
		if (bt.hasActionChanged()) {
			Entity* me = bt.getExecutor().meEntity;
			CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
			animPlugger->plug(bt.getCurrentAction() & 0xFFFF);
		}
	}
	
	void CBichito::initType()
	{
		BichitoBt::initType();
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgSetPlayer, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgSetCam, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgAlert, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgPlayerLowHP, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgEarthquake, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgPlayerDead, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgTeleportToPlayer, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CBichito, MsgPlayerInTuto, receive);
	}
}