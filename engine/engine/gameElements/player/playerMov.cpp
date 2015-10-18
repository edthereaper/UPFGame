#include "mcv_platform.h"
#include "playerMov.h"

using namespace DirectX;
using namespace utils;

#include "../enemies/enemies.h"
#include "../props.h"
#include "../destructible.h"
#include "../input.h"

#include "playerAttack.h"

#include "render/render_utils.h"
#include "render/camera/component.h"
#include "render/mesh/component.h"
using namespace render;

#include "components/transform.h"
#include "components/detection.h"

#include "handles/prefab.h"
#include "bullet.h"
#include "cannonPath.h"
#include "../PaintManager.h"
using namespace component;

#include "animation/animationPlugger.h"
#include "animation/CArmPoint.h"
using namespace animation;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "playerStats.h"
using namespace gameElements;

#include "Cinematic/camera_manager.h"
using namespace cinematic;

namespace behavior {
PlayerMovBt::nodeId_t PlayerMovBt::rootNode = INVALID_NODE;
PlayerMovBt::container_t PlayerMovBt::nodes;
void PlayerMovBt::initType()
{                                                                                 
    /*                node-id             parent-id          node-type      condition        action          */
    BT_CREATEROOT    (PLAYER_MOVEMENT                      , PRIORITY                                        );
    BT_CREATECHILD_C (INBOX             , PLAYER_MOVEMENT  , PRIORITY     , inboxIsNotEmpty                  );
    BT_CREATECHILD_CA(DEAD              , INBOX            , SEQUENCE     , inboxDead      , death           );
	BT_CREATECHILD_CA(SPAWN             , INBOX            , SEQUENCE	  , inboxSpawn     , clearInbox      );
	BT_CREATECHILD_A (SPAWN_FALL        , SPAWN            , LEAF                          , airDown         );
	BT_CREATECHILD_A (SPAWN_ANIM        , SPAWN            , LEAF						   , spawnAnim       );
    BT_CREATECHILD_CA(CANNON            , INBOX            , SEQUENCE     , inboxCannon    , clearInbox      );
    BT_CREATECHILD_A (CANNON_SETUP      , CANNON           , CHAIN                         , cannonSetup     );
    BT_CREATECHILD_A (CANNON_ENTER      , CANNON_SETUP     , LEAF                          , cannonEnter     );
    BT_CREATECHILD_A (CANNON_AIM        , CANNON           , LEAF                          , cannonAim       );
    BT_CREATECHILD_A (CANNON_SHOOT      , CANNON           , LEAF                          , cannonShoot     );
    BT_CREATECHILD   (CANNON_AIR        , CANNON           , SEQUENCE                                        );
    BT_CREATECHILD_A (CANNON_AIR_IMPULSE, CANNON_AIR       , LEAF                          , cannonImpulse   );
    BT_CREATECHILD_A (CANNON_AIRUP      , CANNON_AIR       , LEAF                          , noControlAirUp  );
    BT_CREATECHILD_A (CANNON_AIRDOWN    , CANNON_AIR       , LEAF                          , noControlAirDown);
    BT_CREATECHILD_A (CANNON_LAND       , CANNON_AIR       , LEAF                          , land            );
    BT_CREATECHILD_C (TRAMPOLINE        , INBOX            , SEQUENCE     , inboxTrampoline                  );
    BT_CREATECHILD_A (TRAMPOLINE_CLEANIB, TRAMPOLINE       , LEAF                          , clearInbox      );
	BT_CREATECHILD_A (IMPULSE_UP_ANIM   , TRAMPOLINE       , LEAF						   , impulsedAirAnim );
    BT_CREATECHILD_A (TRAMPOLINE_SETUP  , TRAMPOLINE       , LEAF                          , setupTrampoline );
    BT_CREATECHILD_CA(QUAKE             , INBOX            , SEQUENCE     , inboxQuake     , clearInbox      );
    BT_CREATECHILD   (QUAKE_CHECKAIR    , QUAKE            , PRIORITY                                        );
    BT_CREATECHILD_C (QUAKE_DROP        , QUAKE_CHECKAIR   , SEQUENCE     , isOnAir                          );
    BT_CREATECHILD_A (QUAKE_DROP_SETUP  , QUAKE_DROP       , CHAIN                         , quakeDropSetup  );
    BT_CREATECHILD_A (QUAKE_DROP_START  , QUAKE_DROP_SETUP , LEAF                          , quakeDropStart  );
    BT_CREATECHILD_A (QUAKE_FALLING     , QUAKE_DROP       , LEAF                          , quakeDrop       );
    BT_CREATECHILD_A (QUAKE_PUNCHGROUND , QUAKE_CHECKAIR   , LEAF                          , punchGround     );
    BT_CREATECHILD_A (QUAKE_STUNENEMIES , QUAKE            , CHAIN                         , quakeEnemies    );
    BT_CREATECHILD_A (QUAKE_DURING      , QUAKE_STUNENEMIES, LEAF                          , duringQuake     );
    BT_CREATECHILD_A (QUAKE_GETUP       , QUAKE            , LEAF                          , quakeGetUp      );
    BT_CREATECHILD_CA(LIANA             , INBOX            , SEQUENCE     , inboxLiana     , setupLiana      );
    BT_CREATECHILD_A (LIANA_GRAB        , LIANA            , LEAF                          , lianaGrab       );
    BT_CREATECHILD_C (LIANA_STAY        , LIANA            , SEQUENCE_LOOP, stayOnElement                    );
    BT_CREATECHILD   (LIANA_INPUT       , LIANA_STAY       , PRIORITY                                        );
    BT_CREATECHILD_CA(LIANA_JUMP        , LIANA_INPUT      , LEAF         , lianaJumpCond  , lianaJump       );
    BT_CREATECHILD_CA(LIANA_DROP        , LIANA_INPUT      , LEAF         , lianaDropCond  , lianaDrop       );
    BT_CREATECHILD_CA(LIANA_GRASP       , LIANA_INPUT      , LEAF         , keyGraspLiana  , lianaGrasp      );
	BT_CREATECHILD_CA(LIANA_MOVE		, LIANA_INPUT	   , LEAF		  , lianaMoving	   , lianaMove       );		
    BT_CREATECHILD_A (LIANA_IDLE        , LIANA_INPUT      , LEAF                          , lianaIdle       );
    BT_CREATECHILD_A (LIANA_EXIT        , LIANA            , LEAF                          , lianaExit       );
    BT_CREATECHILD_CA(CREEP             , INBOX            , CHAIN        , inboxCreep     , setupCreep      );
    BT_CREATECHILD_C (CREEP_STAY        , CREEP            , SEQUENCE_LOOP, stayOnElement                    );
    BT_CREATECHILD   (CREEP_INPUT       , CREEP_STAY       , PRIORITY                                        );
    BT_CREATECHILD_CA(CREEP_CLIMB       , CREEP_INPUT      , LEAF         , creepCanClimb  , creepClimb      );
    BT_CREATECHILD_CA(CREEP_DROP        , CREEP_INPUT      , LEAF         , creepCanDrop   , creepDrop       );
    BT_CREATECHILD_CA(CREEP_FALL        , CREEP_INPUT      , LEAF         , fellFromCreep  , fallFromCreep   );
    BT_CREATECHILD_CA(CREEP_JUMP        , CREEP_INPUT      , LEAF         , keyJump        , creepJump       );
    BT_CREATECHILD_CA(CREEP_MOVE        , CREEP_INPUT      , LEAF         , creepCanMove   , creepMove       );
    BT_CREATECHILD_A (CREEP_IDLE        , CREEP_INPUT      , LEAF                          , creepIdle       );
    BT_CREATECHILD_CA(IMPULSE           , INBOX            , SEQUENCE     , inboxImpulse   , clearInbox      );
    BT_CREATECHILD_CA(IMPULSE_UP        , IMPULSE          , LEAF         , velocityUp     , impulsedAirUp   );
    BT_CREATECHILD_A (IMPULSE_DOWN      , IMPULSE          , LEAF						   , impulsedAirDown );
    BT_CREATECHILD_A (IMPULSE_LAND      , IMPULSE          , LEAF						   , land            );
    BT_CREATECHILD_CA(AIR               , INBOX            , SEQUENCE     , inboxAir       , clearInbox      );
    BT_CREATECHILD_CA(AIR_UP            , AIR              , LEAF         , velocityUp     , airUp           );
    BT_CREATECHILD_A (AIR_DOWN          , AIR              , LEAF                          , airDown         );
    BT_CREATECHILD_A (AIR_LAND          , AIR              , LEAF                          , land            );
    BT_CREATECHILD   (GROUND            , PLAYER_MOVEMENT  , PRIORITY                                        );
    BT_CREATECHILD_C (JUMP              , GROUND           , SEQUENCE     , keyJump                          );
    BT_CREATECHILD_A (JUMP_IMPULSE      , JUMP             , LEAF                          , jumpImpulse     );
    BT_CREATECHILD_A (JUMP_SET_JUMPED   , JUMP             , LEAF                          , jumpInitJump    );
    BT_CREATECHILD_CA(DASH              , GROUND           , SEQUENCE     , canDash        , dashInit        );
    BT_CREATECHILD_CA(DASH_HOLD         , DASH             , LEAF         , willHoldDash   , dashHold        );
    BT_CREATECHILD_A (DASH_SETUP        , DASH             , CHAIN                         , dashSetup       );
    BT_CREATECHILD_A (DASH_DURING       , DASH_SETUP       , LEAF                          , dashDuring      );
    BT_CREATECHILD   (DASH_TACKLED      , DASH             , PRIORITY                                        );
    BT_CREATECHILD_CA(DASH_BOUNCE       , DASH_TACKLED     , LEAF         , hadTackled     , dashBounce      );
    BT_CREATECHILD_A (DASH_STOP         , DASH_TACKLED     , LEAF                          , dashStop        );
    BT_CREATECHILD_CA(RUN				, GROUND		   , LEAF		  , canRun		   , groundRun		 );
	BT_CREATECHILD_CA(IDLE_LONG			, GROUND           , LEAF         , hasStayonIdle  , groundIdleLong  );
    BT_CREATECHILD_A (IDLE              , GROUND           , LEAF                          , groundIdle      );	
}

}

#define TIME_DASH_COOLDOWN			2.5f
#define TIME_QUAKE_PUNCH			0.57f
#define TIME_QUAKE_START			0.7f
#define TIME_QUAKE_DURING			2.67f
#define TIME_LAND					0.1f
#define TIME_LIANA_GRAB				0.0f
#define TIME_LIANA_BACK_TO_IDLE		1.0f
#define TIME_DASH_BOUNCE_ANIM		1.33f
#define TIME_DASH_ANIM				0.82f	
#define TIME_ANIM_MUSHROOM_GROUND	0.035f
#define TIME_CANNON_ENTER			0.2f
#define TIME_ANIM_JUMP_IMPULSE		0.1f
#define TIME_CLIMB_ANIM				1.15f
#define TIME_SPAWN_ANIM				5.5f
#define TIME_IDLE_ANIM				10.f		//Double than the time we want (5s)
#define TIME_IDLE_TRANS_ANIM		TIME_IDLE_ANIM + 4.666f - 0.4f		//Previous Time + Transition duration (x2) - delay cycle (x2)

#define CREEP_CLIMB_MARGIN		CHARACTER_HEIGHTS(1.2f)
#define CREEP_FALL_MARGIN		CHARACTER_HEIGHTS(0.5f)

namespace gameElements {

const float PlayerMovBtExecutor::MASS_PLAYER = 55.0;  //kg
const float PlayerMovBtExecutor::MAX_SPEED = 10.f;
const float PlayerMovBtExecutor::ACCELERATION = MAX_SPEED/0.1f;
const float PlayerMovBtExecutor::GROUND_DECEL = MAX_SPEED*3.5f;
const float PlayerMovBtExecutor::WALL_DECEL = GROUND_DECEL+ACCELERATION*0.9f;
const float PlayerMovBtExecutor::LAND_FRICTION_FACTOR = 3.5f;
const float PlayerMovBtExecutor::DEATH_FRICTION_FACTOR = 1000000;

const float PlayerMovBtExecutor::AIR_MAX_SPEED = MAX_SPEED/2.0f;
const float PlayerMovBtExecutor::AIR_CONTROL = AIR_MAX_SPEED / 0.425f;
const float PlayerMovBtExecutor::AIR_DECEL = MAX_SPEED*0.8f;
const float PlayerMovBtExecutor::GRAVITY = 3.5f * 9.81f;
const float PlayerMovBtExecutor::GRAVITYCANNON = 3.5f;

const float PlayerMovBtExecutor::ALIGN_ROTATE_SPEED = deg2rad(360 * 1.5f);
const float PlayerMovBtExecutor::IMPULSE_ALIGN_SPEED = deg2rad(360 * 100);
const float PlayerMovBtExecutor::MAX_JUMP = 4.25f;
const float PlayerMovBtExecutor::TRAMPOLINE_IMPULSE_V = 10.f;
const float PlayerMovBtExecutor::TRAMPOLINE_IMPULSE_H = 22.5f;
const float PlayerMovBtExecutor::TRAMPOLINE_IMPULSE_CANNON_FACTOR = 2.f;

const float PlayerMovBtExecutor::QUAKE_RADIUS = 8.0f;
const float PlayerMovBtExecutor::QUAKE_HEIGHT = 5.0f;
const float PlayerMovBtExecutor::QUAKE_DROP_INITIAL_IMPULSE = MAX_JUMP * 20;
const float PlayerMovBtExecutor::CREEP_SPEED = 3.f;
const float PlayerMovBtExecutor::CREEP_DROP_IMPULSE = 3.f;
const float PlayerMovBtExecutor::CREEP_JUMP_IMPULSE = MAX_JUMP;
const float PlayerMovBtExecutor::CREEP_JUMP_ANGLE = deg2rad(60);

const XMVECTOR PlayerMovBtExecutor::LIANA_HOOK_OFFSET = XMVectorSet(0,-0.35f,0,0);
const float PlayerMovBtExecutor::LIANA_GRAB_APPROACH_SPEED = 5.f;
const float PlayerMovBtExecutor::LIANA_MAX_IMPULSE = 250.f;
const float PlayerMovBtExecutor::LIANA_ACCELERATION = LIANA_MAX_IMPULSE*(1.f/0.7f);  //Inverse of time to max
const float PlayerMovBtExecutor::LIANA_DECELERATION = LIANA_MAX_IMPULSE*(1.f/0.3f); //Inverse of time to stop
const float PlayerMovBtExecutor::LIANA_BRAKE = LIANA_MAX_IMPULSE*0.5f;
const float PlayerMovBtExecutor::LIANA_BRAKE_DELTA = LIANA_BRAKE*(1.f/0.1f);
const float PlayerMovBtExecutor::LIANA_MINIMUM_ANGLE = deg2rad(10);
const float PlayerMovBtExecutor::LIANA_ALIGN_ROTATE_SPEED = ALIGN_ROTATE_SPEED;
const float PlayerMovBtExecutor::LIANA_IMPULSE_HALF_FOV = deg2rad(65);
const float PlayerMovBtExecutor::LIANA_MIN_JUMP = MAX_JUMP*0.75f;
const float PlayerMovBtExecutor::LIANA_MAX_JUMP = MAX_JUMP*0.95f;
const float PlayerMovBtExecutor::LIANA_MIN_JUMP_IMPULSE = MAX_SPEED*0.5f;
const float PlayerMovBtExecutor::LIANA_MAX_JUMP_IMPULSE = MAX_SPEED*1.25f;
const float PlayerMovBtExecutor::LIANA_NEGLIGIBLE_ANGLE = deg2rad(3);
const float PlayerMovBtExecutor::LIANA_FORCE_RECOVER = 0.5f;


const float PlayerMovBtExecutor::DASH_BOUNCE_IMPULSE = 1.25f;
const float PlayerMovBtExecutor::TIME_LIANA_JUMP_REGRAB=0.35f;
const float PlayerMovBtExecutor::TIME_LIANA_DROP_REGRAB=0.75f;
const float PlayerMovBtExecutor::TIME_CANNON_IMPULSE=0.5f;
const float PlayerMovBtExecutor::TIME_CANNON_TIMEOUT=5.0f;
const float PlayerMovBtExecutor::TIME_CANNONPATH_SPAWN=0.25f;

void PlayerMovBtExecutor::setupSpawn()
{
	yVelocity = -yAxis_v * 20;
    xzVelocity = zero_v;
	onAir = true;
}

PlayerMovBtExecutor::tiltData_t PlayerMovBtExecutor::getTiltData(behavior::btState_t action)
{
    static const tiltData_t onCannonTilt(
        true, false,
        deg2rad(60), deg2rad(2),
        deg2rad(360*5), deg2rad(10),
        true, false, 2.f
        );
    static const tiltData_t onCannonDownTilt(
        true, false,
        deg2rad(12), deg2rad(3),
        deg2rad(60), deg2rad(30),
        true, false, 3.f
        );
    static const tiltData_t onAirTilt(
        true, true,
        deg2rad(7), deg2rad(3),
        deg2rad(35), deg2rad(30),
        true, false, 0.25f
        );
    static const tiltData_t noTilt(false,false,0,0,0,0,false,false, 0.25);

    tiltData_t ans;
    if (action & COD_TILT_ON_VELOCITY) {
        if ((action & COD_CANNON_AIR) != 0) {
            ans = (action == CANNON_AIRDOWN) ? onCannonDownTilt : onCannonTilt;
        } else {
            ans = onAirTilt;
        }
    } else if ((action & COD_MANAGES_TILT) != 0) {
        ans = noTilt;
    } else {
        ans = tiltData_t();
    }

    if ((action & COD_TILT_GAUGE_Y)!=0) {
        float y = XMVectorGetY(yVelocity);
        if (y < 0) { ans.invertZ = !ans.invertZ; }
        else {
        auto f = std::min(y/(GRAVITY*0.5f), 1.f);
        ans.maxF *= f;
        }
    }
    return ans;
}

bool PlayerMovBtExecutor::creepCanMove(float) const
{
    const auto& pad(App::get().getPad());
	
    bool up = pad.getState(CONTROLS_UP).isPressed();
    bool down = pad.getState(CONTROLS_DOWN).isPressed();
    bool left = pad.getState(CONTROLS_LEFT).isPressed();
    bool right = pad.getState(CONTROLS_RIGHT).isPressed();

    if ((up && !down) && !(left ^ right)) {
		//If I'm only moving up, check if I'm on top (can't climb further)
		//return !(onCeiling() || isOnCreepTop());
		return !isOnCreepTop();
    } else {
        return (up ^ down)||(left ^ right);
    }
}

bool PlayerMovBtExecutor::canRun(float) const
{
    const auto& pad(App::get().getPad());
    bool up = pad.getState(CONTROLS_UP).isPressed();
    bool down = pad.getState(CONTROLS_DOWN).isPressed();
    bool left = pad.getState(CONTROLS_LEFT).isPressed();
    bool right = pad.getState(CONTROLS_RIGHT).isPressed();
    if((up ^ down)||(left ^ right)) {
        // TODO: OVERLAP TEST?
        return true;
    } else {
        return false;
    }
}

bool PlayerMovBtExecutor::velocityUp(float) const
{
    return XMVectorGetX(XMVector3Dot(yVelocity, yAxis_v)) > 0;
}

bool PlayerMovBtExecutor::canDash(float) const
{
	Entity* me = meEntity;
	CPlayerAttack* mePA(me->get<CPlayerAttack>());
	if (!mePA->canDash()) return false;
	return dashCooldown.get() >= TIME_DASH_COOLDOWN
        && App::get().getPad().getState(CONTROLS_DASH).isHit();
}

bool PlayerMovBtExecutor::keyJump(float) const
{
	return App::get().getPad().getState(CONTROLS_JUMP).isHit();
}

bool PlayerMovBtExecutor::creepCanDrop(float elapsed) const
{
    const auto& pad(App::get().getPad());
    if (pad.getState(CONTROLS_DOWN).isPressed()) {
        if(pad.getState(CONTROLS_JUMP).isHit()) {
            return true;
        } else {
            CCharacterController* cc(meEntity.getSon<CCharacterController>());
            return cc->onGround();
        }
    }
    return pad.getState(CONTROLS_JUMP).isHit() &&
           pad.getState(CONTROLS_DOWN).isPressed();
}

bool PlayerMovBtExecutor::keyDrop(float) const
{
    const auto& pad(App::get().getPad());
    return pad.getState(CONTROLS_JUMP).isHit() &&
           pad.getState(CONTROLS_DOWN).isPressed();
}

bool PlayerMovBtExecutor::keyGraspLiana(float) const
{
    return App::get().getPad().getState(CONTROLS_GRASP).isPressed();
}

bool PlayerMovBtExecutor::stayOnElement(float) const
{
    return stayOnCurrentElement;
}

bool PlayerMovBtExecutor::lianaDropCond(float elapsed) const
{
    const auto& pad(App::get().getPad());
    bool ret = pad.getState(CONTROLS_JUMP).isHit() && (
        !lianaMoving(elapsed) || pad.getState(CONTROLS_GRASP).isPressed()
        );
    return ret;
}

bool PlayerMovBtExecutor::lianaJumpCond(float elapsed) const
{
    const auto& pad(App::get().getPad());
    return pad.getState(CONTROLS_JUMP).isHit() && (
            pad.getState(CONTROLS_UP).isPressed() ||
            pad.getState(CONTROLS_DOWN).isPressed() ||
            pad.getState(CONTROLS_LEFT).isPressed() ||
            pad.getState(CONTROLS_RIGHT).isPressed() ||
            lianaMoving(elapsed));
}

bool PlayerMovBtExecutor::lianaMoving(float) const
{
    if (lianaImpulse != zero_v) {
        return false;
    } else {
        CLiana* liana = currentElement.getSon<CLiana>();
        CTransform* lianaT = currentElement.getSon<CTransform>();
        auto distanceToCenter = lianaT->getPosition() - liana->getLinkPosition(lianaLink);
        auto velocity = XMVectorGetX(XMVector3Length(liana->getLinkVelocity(lianaLink)));
        return angleBetweenVectors(distanceToCenter, yAxis_v) >= LIANA_NEGLIGIBLE_ANGLE && velocity > 0.5f;
    }
}

ret_e PlayerMovBtExecutor::setupLiana(float elapsed)
{
	inbox.clear();
	fromCannon = false;
    Entity* liana_e = currentElement;
    CLiana* liana = liana_e->get<CLiana>();
    CTransform* lianaT = liana_e->get<CTransform>();
    liana->setControlLink(lianaLink);
	stayOnCurrentElement = true;
    CTransform * meT = meEntity.getSon<CTransform>();
    lianaFaceDirCurrent = projectPlane(meT->getFront(), yAxis_v);
    lianaFaceDirTarget = lianaFaceDirCurrent;
    
    lianaImpulse = xzVelocity/MAX_SPEED * LIANA_MAX_IMPULSE;

    return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::lianaGrab(float elapsed)
{
    Entity* me = meEntity;

    lianaOffset += deltaMovement(lianaOffset, LIANA_HOOK_OFFSET, LIANA_GRAB_APPROACH_SPEED * elapsed);
    updateLianaMovement(elapsed, false, false);
    CLiana* liana = currentElement.getSon<CLiana>();
    CTransform* meT = me->get<CTransform>();
    Transform desired(liana->getTransform(lianaLink));

    //XMVECTOR p_0_desired = XMVector3TransformCoord(lianaOffset, desired.getWorld());
    //XMVECTOR p_0 = XMVector3TransformCoord(meT->getPivot(), meT->getWorld());
    //meT->refPosition() += p_0_desired-p_0;

    if (lianaOffset==LIANA_HOOK_OFFSET) {
        CCharacterController* charCo = meEntity.getSon<CCharacterController>();
        charCo->enslave(liana->getLinkBody(lianaLink));
        return DONE;
    } else {
        return STAY;
    }
}

ret_e PlayerMovBtExecutor::lianaExit(float elapsed)
{
    CCharacterController* charCo(meEntity.getSon<CCharacterController>());
    charCo->liberate();

    CLiana* liana = currentElement.getSon<CLiana>();
    liana->resetControlLink();
    lianaOffset = one_v*1000;
    lianaLink = -1;
    lianaImpulse = zero_v;
    lianaFaceDirCurrent = zAxis_v;
    lianaFaceDirTarget = zAxis_v;
    currentElement = Handle();
    return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::lianaDrop(float)
{
	stayOnCurrentElement = false;
    CLiana* liana = currentElement.getSon<CLiana>();
    auto velocity = liana->getLinkVelocity(lianaLink);
    xzVelocity = projectPlane(velocity, yAxis_v)*0.75f;
    yVelocity = zero_v;
	inbox.impulse = true;
	lianaTimer.set(-TIME_LIANA_DROP_REGRAB);
    return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::lianaJump(float elapsed)
{
    static const float minImpulse(LIANA_MIN_JUMP_IMPULSE);
    static const float maxImpulse(LIANA_MAX_JUMP_IMPULSE);
    static const float minJump(calculateImpulse(LIANA_MIN_JUMP));
    static const float maxJump(calculateImpulse(LIANA_MAX_JUMP));

	stayOnCurrentElement = false;
    CLiana* liana = currentElement.getSon<CLiana>();
    auto velocity = liana->getLinkVelocity(lianaLink);
    xzVelocity = projectPlane(velocity, yAxis_v);
    xzVelocity = XMVector3ClampLength(xzVelocity, minImpulse, maxImpulse);
	
    const auto& pad = App::get().getPad();
    CTransform* camT = cameraEntity.getSon<CTransform>();
    XMVECTOR delta = zero_v;
    if (pad.getState(CONTROLS_UP).isPressed()) { delta += XMVectorSetY(camT->getFront(), 0);}
	if (pad.getState(CONTROLS_DOWN).isPressed()) { delta -= XMVectorSetY(camT->getFront(), 0);}
	if (pad.getState(CONTROLS_LEFT).isPressed()) { delta += XMVectorSetY(camT->getLeft(), 0);}
	if (pad.getState(CONTROLS_RIGHT).isPressed()) { delta -= XMVectorSetY(camT->getLeft(), 0);}
    if (delta != zero_v) {
        xzVelocity = XMVector3Normalize(delta) * XMVector3Length(xzVelocity);
    }

    yVelocity = project(velocity, yAxis_v);
    if (testIsBehind(yVelocity, yAxis_v)) {
        yVelocity = yAxis_v*minJump;
    } else {
        yVelocity = XMVector3ClampLength(yVelocity, minJump, maxJump);
    }
	inbox.impulse = true;
	lianaTimer.set(-TIME_LIANA_JUMP_REGRAB);
    return DONE_QUICKSTEP;
}

XMVECTOR PlayerMovBtExecutor::calculateLianaImpulseDelta(float elapsed)
{
    Entity* cam_e = Handle(cameraEntity);
    CTransform* camT = cam_e->get<CTransform>();

    XMVECTOR delta = zero_v;
	const auto& pad(App::get().getPad());
	if (pad.getState(CONTROLS_UP).isPressed())    { delta += XMVectorSetY(camT->getFront(), 0);}
	if (pad.getState(CONTROLS_DOWN).isPressed())  { delta -= XMVectorSetY(camT->getFront(), 0);}
	if (pad.getState(CONTROLS_LEFT).isPressed())  { delta += XMVectorSetY(camT->getLeft(),  0);}
	if (pad.getState(CONTROLS_RIGHT).isPressed()) { delta -= XMVectorSetY(camT->getLeft(),  0);}
	return XMVector3Normalize(delta);
}

void PlayerMovBtExecutor::updateLianaMovement(float elapsed, bool grasp, bool input)
{
    Entity* me_e(meEntity);
    CTransform* meT(me_e->get<CTransform>());
	CCharacterController* charControl(me_e->get<CCharacterController>());

	Entity* liana_e(currentElement);
    CTransform* lianaT(liana_e->get<CTransform>());
	CLiana* liana(liana_e->get<CLiana>());

    float nLink_f(static_cast<float>(liana->getNLinks()));
	auto delta = input ? calculateLianaImpulseDelta(elapsed) : zero_v;
    auto distanceToCenter = lianaT->getPosition()-liana->getLinkPosition(lianaLink);
    auto normal = XMVector3Normalize(distanceToCenter);
    auto angleY = angleBetweenVectors(normal, yAxis_v);
    const auto& limit = liana->getLimit(normal);
    auto cosAngle = std::cos(inRange(0.f,((angleY*0.7f)*M_PI_2f)/limit,M_PI_2f));
   // cosAngle = sign(cosAngle)*std::sqrt(std::abs(cosAngle));
    if (delta == zero_v) {
        auto len = XMVectorGetX(XMVector3Length(lianaImpulse));
        if (len >= LIANA_DECELERATION*elapsed) {
            lianaImpulse -= XMVector3Normalize(lianaImpulse)*LIANA_DECELERATION*elapsed;
        } else {
            lianaImpulse = zero_v;
        }
    }else if (testIsBehind(delta, lianaImpulse)) {
        lianaImpulse += delta*LIANA_DECELERATION*elapsed;
    } else {
        lianaImpulse += delta*LIANA_ACCELERATION*elapsed*cosAngle;
    }
    lianaImpulse = XMVector3ClampLength(lianaImpulse, 0, LIANA_MAX_IMPULSE);
    
    if (delta != zero_v) {lianaFaceDirTarget = projectPlane(delta, yAxis_v);}

    if (lianaImpulse != zero_v) {
        auto distanceToAxis = projectPlane(distanceToCenter, yAxis_v);
        auto angleXZ = angleBetweenVectors(distanceToAxis, delta);
        bool fwd = angleY < LIANA_MINIMUM_ANGLE || angleXZ <= LIANA_IMPULSE_HALF_FOV;
        if (!grasp && fwd) {
            const auto& factor = elapsed * nLink_f * CLiana::LINK_MASS;
            auto force = rotateToPlane(lianaImpulse, normal)  * factor;
            liana->propagateForceUp(lianaLink, force, 0.5f, 0.8f);
            if(unsigned(lianaLink) > liana->getNLinks()) {
                liana->propagateForceDown(lianaLink+1, force*1e-3f, -0.5f, 0.8f);
            }
        } else {
            delta = zero_v;
        }
    } else if (angleY > LIANA_NEGLIGIBLE_ANGLE) {
        liana->propagateForceUp(lianaLink,
            -XMVector3Normalize(distanceToCenter)*elapsed*LIANA_FORCE_RECOVER,
            0.5f, 0.5);
    }
    
    Transform desired(liana->getTransform(lianaLink));
    meT->setRotation(desired.getRotation());

    alignFixedUp(meT, lianaFaceDirCurrent);
    if (!coplanar(yAxis_v, lianaFaceDirCurrent, lianaFaceDirTarget)) {
        alignFixedUp(meT, lianaFaceDirTarget, elapsed * LIANA_ALIGN_ROTATE_SPEED);
    }
    lianaFaceDirCurrent = projectPlane(meT->getFront(), yAxis_v);
    
    XMVECTOR p_0_desired = XMVector3TransformCoord(lianaOffset, desired.getWorld());
    XMVECTOR p_0 = XMVector3TransformCoord(meT->getPivot(), 
        XMMatrixRotationQuaternion(meT->getRotation()));
    meT->setPosition(p_0_desired-p_0);

    auto up = meT->getUp();
    if (testIsBehind(up, yAxis_v)) {
        meT->applyRotation(
            XMQuaternionRotationAxis(up, deg2rad(180)));
    } 

    if (delta == zero_v && input) {
        XMVECTOR unitImpulse(XMVector3Normalize(lianaImpulse));
	    lianaImpulse -= unitImpulse * LIANA_DECELERATION * elapsed;
	    if (testIsBehind(unitImpulse, lianaImpulse)) {
		    //changed direction => overshoot
		    lianaImpulse = zero_v;
	    }
    }

    if (grasp) {
        lianaBrake += LIANA_DECELERATION*elapsed;
        lianaBrake = inRange(0.f, lianaBrake, LIANA_BRAKE);
        auto factor = elapsed*CLiana::LINK_MASS*lianaBrake*nLink_f*sign(cosAngle)*cosAngle*cosAngle;
        liana->propagateBrakeUp(  lianaLink,   factor,      -0.5f, 0.8f, 0.001f, 0.5f, 1.5f, 0.5f);
        liana->propagateBrakeDown(lianaLink-1, factor*0.1f, -0.5f, 0.6f, 0.001f, 0.2f, 1.1f, 0.5f);
    } else {
        lianaBrake = 0;
    }

	if (lianaSoundTimer.count(elapsed) > 4.0f){
		char cstr[32] = "vine_rope";
		int randomV = rand_uniform(4, 1);
		char randomC[32] = "";
		sprintf(randomC, "%d", randomV);
		strcat(cstr, randomC);
		fmodUser::fmodUserClass::playSound(cstr, 1.5f, 0.0f);
		lianaSoundTimer.reset();
	}
}

ret_e PlayerMovBtExecutor::lianaGrasp(float elapsed)
{
    updateLianaMovement(elapsed, true);
    return DONE;
}

ret_e PlayerMovBtExecutor::lianaMove(float elapsed)
{
    updateLianaMovement(elapsed, false);
    return DONE;
}

ret_e PlayerMovBtExecutor::lianaIdle(float elapsed)
{
    updateLianaMovement(elapsed, false);
    return DONE;
}

ret_e PlayerMovBtExecutor::creepIdle(float elapsed)
{
	//dbg("creepIdle\n");
	priorityUp = 0;
	priorityRight = 0;
	priorityLeft = 0;
	priorityDown = 0;
	startCreepRight = false;
	startCreepLeft = false;
	startCreepDown = false;
	startCreepUp = false;
	return DONE;
}

bool PlayerMovBtExecutor::onCeiling() const
{
    auto shape = Physics.createBoxShape(PxVec3(0.20f,0.20f,0.20f));
    Entity* me = meEntity;
    CTransform* meT = me->get<CTransform>();
    CCharacterController* charCo = me->get<CCharacterController>();
    
    PxTransform transform1 = PxTransform::createIdentity();
    XMVECTOR above(yAxis_v*(CHARACTER_HEIGHTS(1.0f)));
    transform1.p = toPxVec3(meT->getPosition()+above);
    bool success = Physics.overlap(shape, transform1, 
        physx::PxQueryFilterData(charCo->getFilter(),
        PxSceneQueryFilterFlag::ePREFILTER|PxSceneQueryFilterFlag::eSTATIC));
    shape->release();
    return success;
}

float creepHeightY;
float maxValueH;

bool PlayerMovBtExecutor::isOnCreepTop() const
{
    Entity* me = meEntity;
    CTransform* meT = me->get<CTransform>();
    
    assert(currentElement.isValid());
    assert(currentElement.hasSon<CCreep>());
    Entity* creepEntity = currentElement;
    CCreep* creepData = creepEntity->get<CCreep>();
	CTransform* creepT = creepEntity->get<CTransform>();

	CAABB* creepAABB = creepEntity->get<CAABB>();
	XMMATRIX mat = XMMatrixTransformation(XMVectorSet(0, 0, 0, 0), XMVectorSet(0, 0, 0, 0), XMVectorSet(1, 1, 1, 1), XMVectorSet(0, 0, 0, 0), creepT->getRotation(), creepT->getPosition());
	XMVECTOR newmin = XMVector3Transform(creepAABB->getMin(), mat);
	XMVECTOR newmax = XMVector3Transform(creepAABB->getMax(), mat);

	if (XMVectorGetY(newmin) > XMVectorGetY(newmax)){
		maxValueH = XMVectorGetY(newmin) - CREEP_CLIMB_MARGIN;	
	}
	else{
		maxValueH = XMVectorGetY(newmax) - CREEP_CLIMB_MARGIN;
	}

	if (XMVectorGetY(meT->getPosition()) >= maxValueH)		return true;
	return false;
}

bool PlayerMovBtExecutor::creepClimbTest() const
{
	Entity* me = meEntity;
    CTransform* meT(me->get<CTransform>());
    CCharacterController* meChCo(me->get<CCharacterController>());
	XMVECTOR origin = meT->getPosition() + meT->getUp() * 2;
	XMVECTOR dir = meT->getFront();
	PxReal distance = 2.0f;
	PxRaycastBuffer hit;
	if (!PhysicsManager::get().raycast(origin, dir, distance, hit, meChCo->getFilter()))			return true;
	return false;
}

bool PlayerMovBtExecutor::creepCanClimb(float) const
{
	if (App::get().getPad().getState(CONTROLS_UP).isPressed()) {
		CCharacterController* charControl = meEntity.getSon<CCharacterController>();
		if (isOnCreepTop()) {
            return creepClimbTest();
        } else {
            return false;
        }
	}
	return false;
}

bool PlayerMovBtExecutor::fellFromCreep(float) const
{
    if (collisionLostCreep) return true;
	if (App::get().getPad().getState(CONTROLS_DOWN).isPressed()) {
		Entity* me = meEntity;
		CTransform* meT = me->get<CTransform>();
		Entity* creepEntity = currentElement;
		CCreep* creepData = creepEntity->get<CCreep>();
		CTransform* creepT = creepEntity->get<CTransform>();
		CAABB* creepAABB = creepEntity->get<CAABB>();
		XMMATRIX mat = XMMatrixTransformation(XMVectorSet(0, 0, 0, 0), XMVectorSet(0, 0, 0, 0), XMVectorSet(1, 1, 1, 1), XMVectorSet(0, 0, 0, 0), creepT->getRotation(), creepT->getPosition());
		XMVECTOR newmin = XMVector3Transform(creepAABB->getMin(), mat);
		XMVECTOR newmax = XMVector3Transform(creepAABB->getMax(), mat);
		float minValueH;
		if (XMVectorGetY(newmin) < XMVectorGetY(newmax)){
			minValueH = XMVectorGetY(newmin) + CREEP_FALL_MARGIN;
		}
		else{
			minValueH = XMVectorGetY(newmax) + CREEP_FALL_MARGIN;
		}
		if (XMVectorGetY(meT->getPosition()) <= minValueH)		return true;
	}
	return false;
}

ret_e PlayerMovBtExecutor::setupCreep(float elapsed)
{	
	//dbg("setupCreep\n");
	unplugJump = true;

	clearInbox(elapsed);
	stayOnCurrentElement = true;
    collisionLostCreep = false;
	onCannonAir = false;
	fromCannon = false;

	priorityUp = 0;
	priorityDown = 0;
	priorityLeft = 0;
	priorityRight = 0;

	startCreepRight = false;
	startCreepLeft = false;
	startCreepDown = false;
	startCreepUp = false;

	xzVelocity = zero_v;
	yVelocity = zero_v;

	CTransform* meT = meEntity.getSon<CTransform>();
	assert(currentElement.hasSon<CCreep>());
	CCreep* creepData = currentElement.getSon<CCreep>();

	float yaw(getYawFromVector(-creepData->getNormal()));
	meT->setRotation(XMQuaternionInverse(XMQuaternionRotationAxis(yAxis_v, -yaw)));

	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::creepClimb(float elapsed)
{
	if (timer.count(elapsed) >= TIME_CLIMB_ANIM) {
		timer.reset();
		stayOnCurrentElement = false;
		currentElement = Handle();
		return DONE;
	}
	else {
		return STAY;
	}
}

ret_e PlayerMovBtExecutor::fallFromCreep(float)
{	
	collisionLostCreep = false;
    assert(currentElement.isValid());
    assert(currentElement.hasSon<CCreep>());
    CCreep* creepData = currentElement.getSon<CCreep>();
	xzVelocity = creepData->getNormal() * CREEP_DROP_IMPULSE;
	yVelocity = -yAxis_v;
	stayOnCurrentElement = false;
	inbox.impulse = true;
    currentElement = Handle();
	return DONE;
}

ret_e PlayerMovBtExecutor::creepDrop(float)
{
	assert(currentElement.isValid());
    assert(currentElement.hasSon<CCreep>());

    CCreep* creepData = currentElement.getSon<CCreep>();
	xzVelocity = creepData->getNormal() * CREEP_DROP_IMPULSE;
	stayOnCurrentElement = false;
	inbox.impulse = true;
    currentElement = Handle();
	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::creepJump(float)
{
	//dbg("creepJump\n");
	
	Entity *creepEntity = currentElement;
	CCreep* creep = creepEntity->get<CCreep>();
	CTransform* t = meEntity.getSon<CTransform>();

	static const float impulse = calculateImpulse(CREEP_JUMP_IMPULSE);
	xzVelocity = -t->getFront() * impulse * cos(CREEP_JUMP_ANGLE);
	yVelocity = yAxis_v * impulse * sin(CREEP_JUMP_ANGLE);

	stayOnCurrentElement = false;
    currentElement = Handle();
	inbox.impulse = true;
	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::creepMove(float elapsed)
{
	int highestP = 0;
	if (priorityUp > priorityRight && priorityUp > priorityLeft && priorityUp > priorityDown)			highestP = priorityUp;
	if (priorityRight > priorityUp && priorityRight > priorityLeft &&priorityRight > priorityDown)		highestP = priorityRight;
	if (priorityLeft > priorityRight && priorityLeft > priorityUp && priorityLeft > priorityDown)		highestP = priorityLeft;
	if (priorityDown > priorityUp && priorityDown > priorityLeft && priorityDown > priorityRight)		highestP = priorityDown;
	
	const auto& pad(App::get().getPad());
	if (pad.getState(CONTROLS_UP).isPressed() && !isOnCreepTop() && priorityUp == 0){
		highestP++;
		priorityUp = highestP + 1;
	}
	if (pad.getState(CONTROLS_RIGHT).isPressed() && priorityRight == 0){
		highestP++;
		priorityRight = highestP + 1;
	}
	if (pad.getState(CONTROLS_LEFT).isPressed() && priorityLeft == 0){
		highestP++;
		priorityLeft = highestP + 1;
	}
	if (pad.getState(CONTROLS_DOWN).isPressed() && priorityDown == 0){
		highestP++;
		priorityDown = highestP + 1;
	}
	if (!pad.getState(CONTROLS_UP).isPressed() && priorityUp > 0){
		priorityUp = 0;
		startCreepUp = false;
	}
	if (!pad.getState(CONTROLS_RIGHT).isPressed() && priorityRight > 0){
		priorityRight = 0;
		startCreepRight = false;
	}
	if (!pad.getState(CONTROLS_LEFT).isPressed() && priorityLeft > 0){
		priorityLeft = 0;
		startCreepLeft = false;
	}
	if (!pad.getState(CONTROLS_DOWN).isPressed() && priorityDown > 0){
		priorityRight = 0;
		startCreepDown = false;
	}

	//dbg("up priority: %i\n", priorityUp);
	//dbg("down priority: %i\n", priorityDown);
	//dbg("right priority: %i\n", priorityRight);
	//dbg("left priority: %i\n", priorityLeft);
	//dbg("startCreepUp: %i\n", startCreepUp);

	return DONE;
}

ret_e PlayerMovBtExecutor::quakeDropSetup(float)
{
	//dbg("quakeDropSetup\n");
	App &app = App::get();
	Entity* levi = app.getBichito();
	levi->sendMsg(MsgEarthquake());
	doingQuake = true;
    return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::quakeDropStart(float elapsed)
{
	//dbg("quakeDropStart\n");
	updateSpeed(elapsed, false, false, false);
    if (timer.count(elapsed) >= TIME_QUAKE_START) {
        timer.reset();
        yVelocity = -yAxis_v * QUAKE_DROP_INITIAL_IMPULSE; //Must be negative if we want to drop w
		return DONE;
    } else {
		xzVelocity = xzVelocity * (elapsed * 0.05f);
		yVelocity = -yAxis_v * 0.05f;
        return STAY;
    }
}



ret_e PlayerMovBtExecutor::quakeDrop(float elapsed)
{
	updateSpeed(elapsed, false, true, true);
    CCharacterController* charControl = meEntity.getSon<CCharacterController>();
    return charControl->onGround() ? DONE : STAY;
}
 
ret_e PlayerMovBtExecutor::punchGround(float elapsed)
{
	if (timer.count(elapsed) >= TIME_QUAKE_PUNCH) {
        timer.reset();
		App &app = App::get();
		Entity* levi = app.getBichito();
		levi->sendMsg(MsgEarthquake());
        return DONE;
    } else {
		doingQuake = true;
        return STAY;
    }
}

ret_e PlayerMovBtExecutor::quakeEnemies(float elapsed)
{   
	CTransform* tMe(meEntity.getSon<CTransform>());
	for (Entity* e : EntityListManager::get(CEnemy::TAG)) {
        if (e == nullptr) {continue;}
        CTransform* tE(e->get<CTransform>());
		CEnemy* eE(e->get<CEnemy>());
		if (testCyllinder(tE->getPosition(), tMe->getPosition(), QUAKE_RADIUS, QUAKE_HEIGHT) && !eE->isDead()) {
			e->sendMsg(MsgEarthquake());	
        }
	}
	for (Entity* e : EntityListManager::get(CProp::TAG)) {
        if (e == nullptr) {continue;}
        CTransform* tE(e->get<CTransform>());
		CTransformable* tT(e->get<CTransformable>());
		if (testCyllinder(tE->getPosition(), tMe->getPosition(),
            QUAKE_RADIUS, QUAKE_HEIGHT) && !tT->isTransformed()) {
			e->sendMsg(MsgShot(true));	
        }
	}
    return DONE_QUICKSTEP;
}

#define MAX_CAM_SHAKE_AMOUNT 0.035f
#define MAX_CAM_SHAKE_FREQ 17

ret_e PlayerMovBtExecutor::duringQuake(float elapsed)
{
	//dbg("duringQuake\n");
	if (timer.count(elapsed) >= TIME_QUAKE_DURING) {
        timer.reset();
		yVelocity = -yAxis_v * 8;
		xzVelocity = zero_v;
		doingQuake = false;
        CCamera* cam = cameraEntity.getSon<CCamera>();
        cam->setShake(0, 0);
        return DONE;
    } else {
        float f = M_PI_2f * timer/TIME_QUAKE_DURING;
        CCamera* cam = cameraEntity.getSon<CCamera>();
        float cosf = std::cos(f);
        float shakeAmount = MAX_CAM_SHAKE_AMOUNT*cosf*cosf*cosf;
        float shakeFreq = MAX_CAM_SHAKE_FREQ;
        cam->setShake(shakeAmount, shakeFreq);
        return STAY;
    }
}

ret_e PlayerMovBtExecutor::quakeGetUp(float elapsed)
{
	return DONE;
}

bool PlayerMovBtExecutor::willHoldDash(float) const
{
	return App::get().getPad().getState(CONTROLS_DASH).isPressed()
        && previousAction & COD_ON_DASH_HOLD;
}

bool onceDashHold = true;
bool isAimingEnemy = false;
XMVECTOR dashHoldDir = zero_v;

ret_e PlayerMovBtExecutor::dashHold(float elapsed)
{
    CTransform* t = meEntity.getSon<CTransform>();
    CTransform* camT = cameraEntity.getSon<CTransform>();
	Entity* me = meEntity;
	CPlayerAttack* mePA(me->get<CPlayerAttack>());
	if (onceDashHold){
		if (mePA->isAutoAimingEnemy()){
			dashHoldDir = mePA->autoAimingPosition();
			isAimingEnemy = true;
		}
		else{
			isAimingEnemy = false;
		}
		onceDashHold = false;
	}
	if (isAimingEnemy){
		alignXZ(t, dashHoldDir, IMPULSE_ALIGN_SPEED*elapsed);
	}
	else{
		alignXZ(t, t->getPosition() + camT->getFront(), IMPULSE_ALIGN_SPEED*elapsed);
	}
	return DONE;
}

ret_e PlayerMovBtExecutor::dashInit(float elapsed)
{
	onceDashHold = true;
	dashHoldDir = zero_v;
	
	dashCooldown.reset();
    fell = false;
    return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::dashSetup(float elapsed)
{
    return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::dashDuring(float elapsed)
{
	if (timer.count(elapsed) >= TIME_DASH_ANIM || hadTackled(elapsed)) {
        timer.reset();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e PlayerMovBtExecutor::dashBounce(float elapsed)
{
	//dbg("dash Bounce\n");
	static const auto bounceImpulse(calculateImpulse(DASH_BOUNCE_IMPULSE)*yAxis_v);
	yVelocity = bounceImpulse;
	Entity* me = meEntity;
	CTransform* meT = me->get<CTransform>();
#ifdef _DEBUG
	xzVelocity = -meT->getFront() * (MAX_SPEED / 3);
#else
	xzVelocity = -meT->getFront() * (MAX_SPEED / 1.42f);
#endif	
	inbox.impulse = true;
	tackled = false;
	dashBounceisOn = true;
	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::dashStop(float elapsed)
{
	return DONE;
}

ret_e PlayerMovBtExecutor::cannonSetup(float elapsed)
{
	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::cannonEnter(float elapsed)
{
	if (timer.count(elapsed) >= TIME_CANNON_ENTER) {
		timer.reset();
		cannonPathTimer.reset();
		setupCannonCam();
		onCannonAir = false;
		Entity* me = meEntity;
		((CMesh*)me->get<CMesh>())->setVisible(false);
		((CMesh*)currentElement.getSon<CMesh>())->setVisible(false);
		CTransform* cannonT = currentElement.getSon<CTransform>();
		((CCharacterController*)me->get<CCharacterController>())->teleport(cannonT->getPosition() + yAxis_v, false);
		return DONE;
	}
	else {
		return STAY;
	}
}

PlayerMovBtExecutor::cannonCalc_t PlayerMovBtExecutor::calculateCannonVelocities()
{
	CTransform* camT = cameraEntity.getSon<CTransform>();
	CCannon* cannon = currentElement.getSon<CCannon>();
	XMVECTOR camPos = camT->getPosition();
	float pitch = getPitchFromVector(camT->getFront());
	float module = calculateImpulse(cannon->getImpulse());
	cannonCalc_t ret;
	ret.yVel = ((module * std::sin(pitch)) * yAxis_v);
	ret.xzVel = (module * std::cos(pitch)) * XMVector3Normalize(XMVectorSetY(camT->getFront(), 0.f));
	ret.camPos = camT->getPosition();
	ret.camFront = XMVector3Normalize(XMVectorSetY(camT->getFront(), 0.f));
	return ret;
}

ret_e PlayerMovBtExecutor::cannonAim(float elapsed)
{
	CCannon* cannon = currentElement.getSon<CCannon>();

	float time = timer.count(elapsed);
	if (App::get().getPad().getState(CONTROLS_SHOOT_CANNON).isHit()) {
		cannonPathTimer.reset();
		timer.reset();
		cannonTimeState = 0;
		return DONE;	
	} else if (time >= TIME_CANNON_TIMEOUT) {
		cannonTimeState = 0;
		cannonPathTimer.reset();
		timer.reset();
		return DONE;
	} else if ((time >= (TIME_CANNON_TIMEOUT - 1.5f)) && cannonTimeState == 2) {
		fmodUser::fmodUserClass::playSound("canon_count", 1.0f, 0.0f);
		cannonTimeState = 3;
	} else if ((time >= (TIME_CANNON_TIMEOUT - 3.0f)) && cannonTimeState == 1) {
		fmodUser::fmodUserClass::playSound("canon_count", 1.0f, 0.0f);
		cannonTimeState = 2;
	} else if ((time >= (TIME_CANNON_TIMEOUT - 4.5f)) && cannonTimeState == 0) {
		fmodUser::fmodUserClass::playSound("canon_count", 1.0f, 0.0f);
		cannonTimeState = 1;
	}
	return STAY;
}
void PlayerMovBtExecutor::setup3PCam()
{
	Entity* me = meEntity;
	((CMesh*)me->get<CMesh>())->setVisible(true);
	CameraManager::get().setCannonCam(false);
	cannonCam = false;
	cinematic::CameraManager::get().setCannonCam(false);
	cam3P.setEye(XMVectorSet(0, 1.85f, -3.65f, 0));
	cam3P.setTarget(XMVectorSet(0, 1.5f, 0, 0));
}

void PlayerMovBtExecutor::setupCannonCam()
{
	Entity* me = meEntity;
	Entity* cannonEntity = currentElement;


	((CMesh*)me->get<CMesh>())->setVisible(false);

	CTransform* meT(me->get<CTransform>());
	assert(cannonEntity->has<CTransform>());
	assert(cannonEntity->has<CCannon>());
	CTransform* cannonTransform(cannonEntity->get<CTransform>());
	CCannon* cannonData = cannonEntity->get<CCannon>();
	CAABB* cannonAABB(cannonEntity->get<CAABB>());
	auto cannonPos(cannonTransform->getPosition());

	CameraManager::get().setCannonCam(true);
	cannonPos = meT->getPosition();

	if (XMVectorGetY(cannonTransform->getUp()) >= 0){
		cannonPos = XMVectorSetY(cannonPos, XMVectorGetY(cannonPos) + CHARACTER_HEIGHTS(1) / 2);
	}
	else {
		cannonPos = XMVectorSetY(cannonPos, XMVectorGetY(cannonPos) - CHARACTER_HEIGHTS(1) / 2);
	}

	cannonCam = true;
	cinematic::CameraManager::get().setCannonCam(true);
	camCannon.setup(cannonPos, cannonData->getNormal(), deg2rad(cannonData->getFovH()), deg2rad(cannonData->getFovV()));
}

ret_e PlayerMovBtExecutor::cannonShoot(float elapsed)
{
	auto calcs = calculateCannonVelocities();
	yVelocity = calcs.yVel;
	xzVelocity = calcs.xzVel;

    CTransform* meT = meEntity.getSon<CTransform>();
    meT->lookAt(meT->getPosition() + xzVelocity);

	component::getManager<CCannonPath>()->forall(&CCannonPath::removeFromScene);
	Entity* cannon = currentElement;
	((CMesh*)cannon->get<CMesh>())->setVisible(true);

	setup3PCam();
	//Setup the 3p cam params for the exit of the cannon
	cam3P.setEye(XMVectorSet(
        -3.65f * XMVectorGetX(calcs.camFront),
        1.85f,
        -3.65f * XMVectorGetZ(calcs.camFront),
        0));
	cam3P.setTarget(XMVectorSet(0, 1.5f, 0, 0));

	currentElement = Handle();
	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::cannonImpulse(float elapsed)
{
	onAir = true;
	onCannonAir = true;
	fromCannon = true;
	updateSpeed(elapsed, false, true, false);
    Entity* me = meEntity;
	CCharacterController* charControl = me->get<CCharacterController>();
	CTransform* t = me->get<CTransform>();
    if (!lockedRotation) {  //THIS BOOL MUST BE DELETED LATER ON
        alignXZ(t, t->getPosition() + yVelocity + xzVelocity, IMPULSE_ALIGN_SPEED*elapsed);
    }
    if (timer.count(elapsed) >= TIME_CANNON_IMPULSE) {
        lockedRotation = false;  //THIS BOOL MUST BE DELETED LATER ON
        timer.reset();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e PlayerMovBtExecutor::setupTrampoline(float elapsed)
{		
	CTransform* mePlayer = meEntity.getSon<CTransform>();
	Entity* eTrampoline = currentElement;
	CTransform* trampT = eTrampoline->get<CTransform>();

	XMVECTOR normal = trampT->getUp();
	float pitch = getPitchFromVector(normal);
    
    //Radius of an ellipse
    const auto& a = TRAMPOLINE_IMPULSE_V;
    const auto& b = TRAMPOLINE_IMPULSE_H;
    const auto cosine = std::sin(pitch);
    const auto sine = std::cos(pitch);
	float trampolineImpulse = (a*b)/sqrt(a*a*sine*sine+b*b*cosine*cosine);

	if (fromCannon)		trampolineImpulse *= TRAMPOLINE_IMPULSE_CANNON_FACTOR;

	if (yAxis_v != normal) {
		float yaw = getYawFromVector(normal);
		mePlayer->setRotation(XMQuaternionRotationAxis(mePlayer->getUp(), yaw));
	}

    const float module = calculateImpulse(trampolineImpulse);

	yVelocity = (module * std::sin(pitch)) * yAxis_v;
	xzVelocity = (module * std::cos(pitch)) * XMVectorSetY(normal, 0.f);

	onCannonAir = false;

	inbox.impulse = true;
	currentElement = Handle();

	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::impulsedAirAnim(float elapsed)
{
	Entity* me = meEntity;
	CTransform* t = me->get<CTransform>();
	if (timer.count(elapsed) >= TIME_ANIM_MUSHROOM_GROUND) {
		return DONE;
	} else {
		return STAY;
	}
}

ret_e PlayerMovBtExecutor::impulsedAirUp(float elapsed)
{
	//dbg("impulsedAirUp\n");
	Entity* me = meEntity;
	CCharacterController* charControl = me->get<CCharacterController>();
	CTransform* t = me->get<CTransform>();
	if (!dashBounceisOn)	alignXZ(t, t->getPosition() + yVelocity + xzVelocity, IMPULSE_ALIGN_SPEED*elapsed);
	onAir = true;
	bool typeMov = false;	
	if (fromCannon || dashBounceisOn){
		updateSpeed(elapsed, false, true, false);
	}
	else{
		updateSpeed(elapsed, true, true, false);
	}
	return velocityUp(elapsed) ? STAY : DONE;
}

bool PlayerMovBtExecutor::isTrampolineUnder(float) const
{
	Entity* me = meEntity;
	CTransform* t = me->get<CTransform>();
	XMVECTOR origin = t->getPosition();
	XMVECTOR dir = -yAxis_v;
	PxReal distance = 1.3f;
	PxRaycastBuffer hit;
	if (PhysicsManager::get().raycast(origin, dir, distance, hit,
		filter_t(filter_t::NONE, ~filter_t::id_t(filter_t::PROP), filter_t::PROP))){
		Handle hitHandle = Handle::fromRaw(hit.block.shape->userData);
		Entity *eOther = hitHandle;
		if(eOther->has<CTransformable>()){
			CTransformable* transf = eOther->get<CTransformable>();
			if (transf->isTransformed() && eOther->has<CTrampoline>()){
				me->sendMsg(MsgPushTrampoline(eOther));
				return true;
			}
		}
	}
	return false;
}

ret_e PlayerMovBtExecutor::impulsedAirDown(float elapsed)
{
	//dbg("impulsedAirDown\n");
	onAir = true;
	if (!dashBounceisOn){
		updateSpeed(elapsed, true, true, false);
	}
	else{
		updateSpeed(elapsed, false, true, false);
	}
	if (isTrampolineUnder(0.0f)) return DONE;
	CCharacterController* charControl = meEntity.getSon<CCharacterController>();
	return charControl->onGround() ? DONE : STAY;
}

ret_e PlayerMovBtExecutor::noControlAirUp(float elapsed)
{
	onAir = true;
	onCannonAir = false;
	
	updateSpeed(elapsed, false, true, false);
	return velocityUp(elapsed) ? STAY : DONE;
}

ret_e PlayerMovBtExecutor::noControlAirDown(float elapsed)
{
	onAir = true;
	fromCannon = false;
	updateSpeed(elapsed, false, true, false);
	CCharacterController* charControl = meEntity.getSon<CCharacterController>();
	if (isTrampolineUnder(0.0f)) return DONE;
	return charControl->onGround() ? DONE : STAY;
}

ret_e PlayerMovBtExecutor::airUp(float elapsed)
{
	updateSpeed(elapsed, true, true, true);
	onAir = true;
	return velocityUp(elapsed) ? STAY : DONE;
}

ret_e PlayerMovBtExecutor::airDown(float elapsed)
{
	updateSpeed(elapsed, true, true, true);
	onAir = true;
	CCharacterController* charControl = meEntity.getSon<CCharacterController>();
	if (isTrampolineUnder(0.0f)) return DONE;
	return charControl->onGround() ? DONE : STAY;
}

ret_e PlayerMovBtExecutor::land(float elapsed)
{
	updateSpeed(elapsed, false, true, true, LAND_FRICTION_FACTOR);
	if (canFall() || timer.count(elapsed) >= TIME_LAND) {
		timer.reset();
		fromCannon = false;
		onAir = false;
		onCannonAir = false;
		dashBounceisOn = false;
		return DONE;
	} else {
		return STAY;
	}
}

bool PlayerMovBtExecutor::canFall()
{
	CCharacterController* charControl = meEntity.getSon<CCharacterController>();
	//return !charControl->onGround() ? true : false;

	Entity* me = meEntity;
	CTransform* t = me->get<CTransform>();
	XMVECTOR origin = t->getPosition() + yAxis_v*0.1f;
	XMVECTOR dir = -yAxis_v;
	PxReal distance = 1.0f;
	PxRaycastBuffer hit;
	if (!PhysicsManager::get().raycast(origin, dir, distance, hit,
		filter_t(
		filter_t::NONE,
		~filter_t::id_t(filter_t::SCENE | filter_t::ENEMY | filter_t::PROP),
		filter_t::ALL_IDS))){
		if (!charControl->onGround()){
			return true;
		}
	}
	return false;	
}

ret_e PlayerMovBtExecutor::death(float elapsed)
{
	if (!deathByHole){
		updateSpeed(elapsed, false, true, true, DEATH_FRICTION_FACTOR);
	}else{
		if(!playedSoundDeadFall){
			fmodUser::fmodUserClass::playSound("vine_deadhole", 1.0f, 0.0f);
			playedSoundDeadFall = true;
		}
		updateSpeed(elapsed, true, true, true);
	}
    clearInbox(elapsed);
	return STAY;
}

void PlayerMovBtExecutor::ground(float elapsed){
	if (canFall())		inbox.air = true;
	fromCannon = false;
	onAir = false;
	onCannonAir = false;
	updateSpeed(elapsed, true, true, true);
}

bool PlayerMovBtExecutor::hasStayonIdle(float) const
{
	return playLongIdle;
}

ret_e PlayerMovBtExecutor::groundIdle(float elapsed)
{
	ground(elapsed);
	if (idleTimer.count(elapsed) >= TIME_IDLE_ANIM
#ifdef _DEBUG
        && App::get().allowLongIdle
#endif
        ) {
        playLongIdle = true;
    }
	return DONE;
}

ret_e PlayerMovBtExecutor::groundIdleLong(float elapsed)
{
	ground(elapsed);
	return DONE;
}

ret_e PlayerMovBtExecutor::groundRun(float elapsed)
{
	ground(elapsed);
	return DONE;
}

ret_e PlayerMovBtExecutor::jumpImpulse(float elapsed)
{
	updateSpeed(elapsed, true, false, true);
	if (timer.count(elapsed) >= TIME_ANIM_JUMP_IMPULSE) {
		timer.reset();	
		return DONE;
	}
	else {
		return STAY;
	}
}

ret_e PlayerMovBtExecutor::jumpInitJump(float elapsed)
{
    static const auto jumpImpulse = calculateImpulse(MAX_JUMP)*yAxis_v;
	updateSpeed(elapsed, true, false, true);
	inbox.air = true;
	onAir = true;
	yVelocity = jumpImpulse;
	return DONE_QUICKSTEP;
}

ret_e PlayerMovBtExecutor::spawnAnim(float elapsed)
{
	updateSpeed(elapsed, false, false, true);
	if (timer.count(elapsed) >= TIME_SPAWN_ANIM) {
		timer.reset();
		Entity* me = meEntity;
		CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
		animPlugger->plug(0x2400);
		return DONE;
	}
	else {
		return STAY;
	}
}

void PlayerMovBtExecutor::updateSpeed(float elapsed, bool input, bool gravity, bool decel, float decelFactor)
{
	Entity* me = meEntity;
	CCharacterController* charControl = me->get<CCharacterController>();
	CTransform* t = me->get<CTransform>();

	bool wall = charControl->onWall();

	bool moved = false;
	if (input) {
		float maxSpeed = MAX_SPEED;
		CTransform* camT = cameraEntity.getSon<CTransform>();
		XMVECTOR delta = zero_v;

		const auto& pad(App::get().getPad());
		if (pad.getState(CONTROLS_UP).isPressed()) { delta += XMVectorSetY(camT->getFront(), 0); moved = true; }
		if (pad.getState(CONTROLS_DOWN).isPressed()) { delta -= XMVectorSetY(camT->getFront(), 0); moved = true; }
		if (pad.getState(CONTROLS_LEFT).isPressed()) { delta += XMVectorSetY(camT->getLeft(), 0); moved = true; }
		if (pad.getState(CONTROLS_RIGHT).isPressed()) { delta -= XMVectorSetY(camT->getLeft(), 0); moved = true; }
		delta = XMVector3Normalize(delta);
		delta *= elapsed * (onAir ? AIR_CONTROL : ACCELERATION);
		xzVelocity += delta;
		xzVelocity = XMVector3ClampLength(xzVelocity, 0, maxSpeed);

		CPlayerAttack* playerAttack = me->get<CPlayerAttack>();
		if (moved && !onAir && !onAir && 
			((playerAttack->getCurrentAction() & PlayerAttackBtExecutor::COD_OWNS_ROTATION) == 0)) {
			alignXZ(t, t->getPosition() + delta, ALIGN_ROTATE_SPEED*elapsed);
		}
	}

	if (charControl->onCeiling())		//Pickups activate this
	{ 
		//yVelocity = zero_v; 
	}
	if (gravity) {
		if (onAir){
			yVelocity -= (onCannonAir?GRAVITYCANNON:GRAVITY)*elapsed*yAxis_v;
		} else {
			yVelocity = -yAxis_v * 8;
		}
	}
	if (decel || wall) {
		XMVECTOR unitXZVelocity(XMVector3Normalize(xzVelocity));
        float deceleration = wall? WALL_DECEL : onAir ? AIR_DECEL : GROUND_DECEL;
        deceleration *= decelFactor;
		xzVelocity -= unitXZVelocity * deceleration * elapsed;
		if (XMVectorGetX(XMVector3Dot(unitXZVelocity, xzVelocity)) < 0) {
			//changed direction => overshoot
			xzVelocity = zero_v;
		}
	}
	charControl->setDisplacement((xzVelocity + yVelocity)*elapsed);
	//dbg("vel: %f %f %f\n", XMVectorGetX(xzVelocity + yVelocity), XMVectorGetY(xzVelocity + yVelocity), XMVectorGetZ(xzVelocity + yVelocity));
	//dbg("pos: %f %f %f\n", XMVectorGetX(t->getPosition()), XMVectorGetY(t->getPosition()), XMVectorGetZ(t->getPosition()));
}

void CPlayerMov::updatePaused(float elapsed)
{
    auto& bte(bt.getExecutor());
	Entity* me = bte.meEntity;
	CTransform* meT = me->get<CTransform>();
	CTransform* camT(bte.cameraEntity.getSon<CTransform>());

	if (!bte.cannonCam) {
		bte.cam3P.update(*camT, *meT, elapsed);
	}
}

const float CPlayerMov::TILT_MIN = deg2rad(0.1f);
void CPlayerMov::tilt(CTransform* meT, behavior::btState_t currentAction, float elapsed)
{
    auto bte(bt.getExecutor());
    auto tiltData(bte.getTiltData(currentAction));
    if (bte.xzVelocity != zero_v && tiltData.tilt) {
        physx::PxRaycastBuffer hits;
        static const filter_t filter = filter_t(
            filter_t::NONE,
            ~(filter_t::SCENE|filter_t::PROP|filter_t::TOOL),
            filter_t::ALL_IDS);
        static const float rayLen = tiltData.rayCast;
        if (testIsBehind(bte.yVelocity, yAxis_v) &&
            Physics.raycast(meT->getPosition(), -yAxis_v, rayLen, hits, filter)) {
            float p = 1 - hits.block.distance/rayLen;
            straightenPartially(meT, elapsed*p*deg2rad(360));
        } else {
            const float& maxTiltYZ = tiltData.maxF;
            float rotYZ(elapsed*tiltData.speedF);

            //Project vectors into the world perpendiclar axes and planes
            auto frontXZ(projectPlane(meT->getFront(), yAxis_v));
            auto leftXZ (XMVector3Cross(frontXZ, yAxis_v));
            if ( testIsBehind(meT->getFront(), frontXZ)) {frontXZ=-frontXZ;}
            if (!testIsBehind(meT->getLeft(),  leftXZ )) {leftXZ=-leftXZ;}
            auto upYZ = projectPlane(meT->getUp(), leftXZ);

            //YZ (front plane)
            auto vZ(project(bte.xzVelocity, frontXZ));
            float tiltYZ = 0.f;
            float currTiltYZ(angleBetweenVectors(meT->getUp(), yAxis_v));
            if (currTiltYZ < TILT_MIN) {currTiltYZ = 0.f;}
            if (vZ == zero_v) {
                tiltYZ = 0;
            } else if (currTiltYZ > maxTiltYZ || (currTiltYZ + rotYZ) > maxTiltYZ) {
                tiltYZ = maxTiltYZ-currTiltYZ; //Overshoot or initial offset
                if ((currTiltYZ + rotYZ) > maxTiltYZ) {rotYZ *= 2;}
            } else {
                //Proportional to speed
                tiltYZ = (XMVectorGetX(XMVector3Length(vZ)) * maxTiltYZ)/ PlayerMovBtExecutor::MAX_SPEED;
                float s = sign(tiltYZ);
                if (tiltYZ*s < TILT_MIN*s) {
                    tiltYZ = 0.f; //correct overshoot
                } else {
                    tiltYZ += s*currTiltYZ;
                }
            }

            //Set in the right direction
            if (testIsBehind(vZ, frontXZ) ^ tiltData.invertZ) {tiltYZ=-tiltYZ;}

            //final rotation
            auto rotQ = rotatePartiallyQ(tiltYZ, leftXZ, rotYZ);
            meT->applyRotation(rotQ);
        }
    } else if (tiltData.straighten) {
        straightenPartially(meT, elapsed*deg2rad(360*1));
    }
}

void CPlayerMov::update(float elapsed)
{
	auto& bte(bt.getExecutor());
	Entity* me = bte.meEntity;
	CTransform* meT = me->get<CTransform>();
	CMesh* mesh = me->get<CMesh>();
	CTransform* camT(bte.cameraEntity.getSon<CTransform>());
	CCamera* cam(bte.cameraEntity.getSon<CCamera>());

	if (bte.cannonCam) {
		bte.camCannon.update(*camT, elapsed);			//Aqui cambia el modo de camera
	} else {
		bte.cam3P.update(*camT, *meT, elapsed);
	}


	CPlayerStats* mePS = me->get<CPlayerStats>();
	bt.update(elapsed);

    //update velocity every few frames to make sure physx has updated
    static float velocity = 0;
    static float accElapsed = 0;
    static unsigned frameCount = 0;
    static unsigned const VELOCITY_UPDATE = 4;
    static float const BLUR_FACTOR = 1/(PlayerMovBtExecutor::MAX_SPEED*0.9f);
    accElapsed += elapsed;

    if ((frameCount++ % VELOCITY_UPDATE) == 0) {
        static XMVECTOR prevPosition = getPosition();
        XMVECTOR currPosition = getPosition();
        velocity = XMVectorGetX(XMVector3Length(currPosition-prevPosition))/accElapsed;
        mesh->setMotionBlurModifier(1 + velocity*BLUR_FACTOR);
        prevPosition = currPosition;
        accElapsed = 0;
    }

	auto currentAction = bt.getCurrentAction();
	bte.lianaTimer.count(elapsed);

    
    TransformableFSMExecutor::updateHighlights(elapsed, 
        ((currentAction & PlayerMovBtExecutor::COD_HIGHLIGHT_TRANSFORMABLES) != 0)
#ifdef _DEBUG
        && App::get().highlightTransformables
#endif
        );
    
#ifndef LOOKAT_TOOL
	CPlayerAttack* attack = me->get<CPlayerAttack>();
    CArmPoint* armPoint = me->get<CArmPoint>();
    bool activateArmpoint = attack->wantsToUseArmpoint() &&
        ((currentAction & PlayerMovBtExecutor::COD_DISABLE_ARMPOINT)==0);
    armPoint->setAllActive(activateArmpoint);
#endif

	tilt(meT, currentAction, elapsed);


#define PAINT_DELAY 0.01f

    if (paintDelay.count(elapsed) >= PAINT_DELAY) {
        paintDelay.reset();
        bool mega = mePS->hasMegashot();
        CPaintGroup* paint = PaintManager::getBrush(mega?0:1);
        if (paint != nullptr) {
            float megaShotFactor = mega?mePS->getMegashotFactor():0;
            auto pos = meT->getPosition() + meT->getPivot()-meT->getFront()*0.35f;
            paint->addInstance(pos,
                megaShotFactor * megaPaintSize +
                (1-megaShotFactor)*regularPaintSize);
        }
    }

	bool nowOnCannonAir = (currentAction & PlayerMovBtExecutor::COD_CANNON_AIR) != 0;
	if (!onCannonAir && nowOnCannonAir) {
		CCharacterController* cct = me->get<CCharacterController>();
		cct->setFilters(filter_t::PLAYERCANNON);
	}
	else if (onCannonAir && !nowOnCannonAir) {
		CCharacterController* cct = me->get<CCharacterController>();
		cct->removeFilters(filter_t::PLAYERCANNON);
	}
	onCannonAir = nowOnCannonAir;

	CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
	uint32_t action = currentAction & 0xFFFF;

	if (action == 0x1595){
		if (bt.getExecutor().priorityUp > bt.getExecutor().priorityRight &&
			bt.getExecutor().priorityUp > bt.getExecutor().priorityDown &&
			bt.getExecutor().priorityUp > bt.getExecutor().priorityLeft && !bte.startCreepUp){
			animPlugger->plug(0xd0c3);
			animPlugger->plug(0xd0c4);
			animPlugger->plug(0xc001);
			bte.startCreepUp = true;
			bte.startCreepRight = false;
			bte.startCreepLeft = false;
			bte.startCreepDown = false;
		}
		if (bt.getExecutor().priorityDown > bt.getExecutor().priorityLeft &&
			bt.getExecutor().priorityDown > bt.getExecutor().priorityRight &&
			bt.getExecutor().priorityDown > bt.getExecutor().priorityUp && !bte.startCreepDown){
			animPlugger->plug(0xd0c3);
			animPlugger->plug(0xd0c4);
			animPlugger->plug(0xc002);
			animPlugger->plug(0xc0c2);
			bte.startCreepDown = true;
			bte.startCreepUp = false;
			bte.startCreepLeft = false;
			bte.startCreepRight = false;
		}
		if (bt.getExecutor().priorityLeft > bt.getExecutor().priorityRight &&
			bt.getExecutor().priorityLeft > bt.getExecutor().priorityDown &&
			bt.getExecutor().priorityLeft > bt.getExecutor().priorityUp && !bte.startCreepLeft){
			animPlugger->plug(0xd0c3);
			animPlugger->plug(0xd0c4);
			animPlugger->plug(0xc003);
			animPlugger->plug(0xc0c3);
			bte.startCreepLeft = true;
			bte.startCreepUp = false;
			bte.startCreepRight = false;
			bte.startCreepDown = false;
		}
		if (bt.getExecutor().priorityRight > bt.getExecutor().priorityLeft &&
			bt.getExecutor().priorityRight > bt.getExecutor().priorityDown &&
			bt.getExecutor().priorityRight > bt.getExecutor().priorityUp && !bte.startCreepRight){
			animPlugger->plug(0xd0c3);
			animPlugger->plug(0xd0c4);
			animPlugger->plug(0xc004);
			animPlugger->plug(0xc0c4);
			bte.startCreepRight = true;
			bte.startCreepUp = false;
			bte.startCreepLeft = false;
			bte.startCreepDown = false;
		}
	}
	if (action == 0x2400 || action == 0x24c2 || action == 0x24c1){
		bte.idleTimer.count(elapsed);
	}
	else{
		bte.resetIdleTimer = true;
	}
	if (bte.resetIdleTimer){
		bte.playLongIdle = false;
		bte.idleTimer.reset();
		uint32_t previousAction = bte.previousAction & 0xFFFF;
		if (previousAction == 0x24c1){
			animPlugger->plug(0x24c3);
		}
		bte.resetIdleTimer = false;
	}

	if (bt.hasActionChanged()) {
		switch (action) {
			case 0x1591:	//Creep climb
				animPlugger->plug(0x2400);
				break;
			case 0x1609:	//Jumping Mushroom
				animPlugger->plug(0x1720);
				break;
			case 0x1141:	//Out Cannon
				animPlugger->plug(0x1720);
				break;
			case 0x2310:	//jump
				animPlugger->plug(0x1720);
				break;
			case 0x2221:	//Dash
				animPlugger->plug(0x2400);
				animPlugger->plug(0xd0c1);
				animPlugger->plug(0xd0c2);
				break;
			case 0x1596:	//Unplug creep transition if button is not pressed
				animPlugger->plug(0xb0c4);
				animPlugger->plug(0xb0c3);
				animPlugger->plug(0xb0c2);
				break;
			case 0x24c1:	//LongIdle (1st here cycle, later the transition)
				animPlugger->plug(0x24c2);
				break;
			case 0x1321:	//Earthquake End Ground
				animPlugger->plug(0x131c);
				animPlugger->plug(0x2400);
				break;
			case 0x1314:	//Unplug jumping if earthquake pressed (in air)
				animPlugger->plug(0x231c);
				break;
			case 0x1313:	//Earthquake Start Falling
				animPlugger->plug(0x831c);
				animPlugger->plug(0x8314);
				break;
			case 0x1318:	//Earthquake Ground
				animPlugger->plug(0x8314);
				break;
			case 0x1594:	//Jumping in creep
				animPlugger->plug(0xc594);
				break;
			case 0x2504:	//Begin
				animPlugger->plug(0x2400);
				break;
			case 0x2231:	//Dash Bounce
				animPlugger->plug(0x835c);
				animPlugger->plug(0xd0c1);
				animPlugger->plug(0xd0c2);
				break;
		}
		if (bte.unplugJump){
			animPlugger->plug(0x231c);
			bte.unplugJump = false;
		}
		//dbg("action: %x\n", action);
		animPlugger->plug(action);
	}

	if (!bte.cannonCam){
		bte.cam3P.updateEntity(*camT, *meT);
	}
	bte.previousAction = bt.getCurrentAction();
}

void CPlayerMov::resetCamSpawn()
{
	PlayerMovBtExecutor& btEx = bt.getExecutor();
	Entity* me = btEx.meEntity;
	CTransform* meT = me->get<CTransform>();
	auto euler = quaternionToEuler(meT->getRotation());
	btEx.cam3P.setEye(XMVectorSet(-3.65f * sin(euler.yaw), 1.85f, -3.65f * cos(euler.yaw), 0));
	btEx.cam3P.setTarget(XMVectorSet(0, 1.5f, 0, 0));
}

void CPlayerMov::init()
{
	Handle h(this);
    Handle meEntity = h.getOwner();
    PlayerMovBtExecutor& btEx = bt.getExecutor();
    btEx.meEntity = meEntity;
    btEx.cam3P.setEye(XMVectorSet(0, 1.85f, -3.65f, 0));
    btEx.cam3P.setTarget(XMVectorSet(0, 1.5f, 0, 0));

	Entity* me = btEx.meEntity;
	CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
	animPlugger->plug(0x2400);

    paintDelay.reset();
    bt.init();
}

void CPlayerMov::initType()
{
    PlayerMovBt::initType();
    SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgCollisionEvent, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgHitEvent, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgSetCam, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPlayerDead, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPlayerTrampoline, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPlayerCannnon, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPlayerCreep, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgSetBichito, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPushTrampoline, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPlayerSpawn, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgMeleeHit, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgFlareHit, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CPlayerMov, MsgPlayerHasShot, receive);
}

void CPlayerMov::receive(const MsgPlayerDead &msg)
{
	bt.getExecutor().inbox.dead = true;
	bt.reset();
}

void CPlayerMov::receive(const MsgHitEvent &msg)
{
	Entity * e = msg.entity;
	if (e != nullptr && e->has<CEnemy>() &&
		((bt.getCurrentAction() & PlayerMovBtExecutor::COD_DASHING) != 0) &&
		((((CEnemy*)e->get<CEnemy>())->getAction() & EnemyBtExecutor::COD_DISABLE_DASH) == 0)) {
		setTackled(true);
		e->sendMsg(MsgDashTackled());
	}
	else if ((msg.filter.is & filter_t::BOSS) != 0
        && ((bt.getCurrentAction() & PlayerMovBtExecutor::COD_CANNON_AIR) != 0)) {
		auto& bte = bt.getExecutor();
        Entity* me = Handle(this).getOwner();
        Entity* cannon = bte.previousElement;

        dbg("Collided with Boss\n");

        //Fake a bounce that ends in the platforms

		CTransform* meT = me->get<CTransform>();
		CTransform* cannonT = cannon->get<CTransform>();

        XMVECTOR o = cannonT->getPosition();
        XMVECTOR p = meT->getPosition();
        XMVECTOR c = XMVectorSetY(p, XMVectorGetY(o));

        float r = XMVectorGetX(XMVector3Length(o));
        XMVECTOR reflected = reflect(c-o, msg.worldNormal); 
        XMVECTOR target = c + r*XMVector3Normalize(projectPlane(reflected, yAxis_v));

        auto bounce = calculateAimAngle(target, p, 0.5f,
            PlayerMovBtExecutor::calculateImpulse(
                PlayerMovBtExecutor::TRAMPOLINE_IMPULSE_V *
                PlayerMovBtExecutor::TRAMPOLINE_IMPULSE_CANNON_FACTOR),
            PlayerMovBtExecutor::GRAVITY
            );

		CTransform* bossT = msg.entity.getSon<CTransform>();
		lookAtXZ(meT, bossT->getPosition());
		bte.lockedRotation = true;

		auto v = meT->getFront();
		bte.yVelocity  = project(bounce, yAxis_v);
		bte.xzVelocity = projectPlane(bounce, yAxis_v);
        if (testIsBehind(bte.yVelocity, yAxis_v)) {
            bte.yVelocity = -bte.yVelocity;
        }
        if (testIsBehind(bte.xzVelocity, msg.worldNormal)) {
            bte.xzVelocity = -bte.xzVelocity;
        }
        dbgXMVECTOR3(bte.yVelocity, false);
        dbg("  +  ");
        dbgXMVECTOR3(bte.xzVelocity, true);
	}
}

void CPlayerMov::receive(const MsgCollisionEvent &msg)
{
	Entity * e = msg.entity;
	auto& bte(bt.getExecutor());
	if (e != nullptr && !bt.getExecutor().doingQuake) {
		if (msg.enter) {
			if (e->has<CTransformable>()){
				CTransformable * t = (CTransformable *)(e->get<CTransformable>());
				if (t->isTransformed()){
					bool launchEvent = false;
					if (e->has<CTrampoline>()) { 
                        bte.inbox.trampoline = true;
                        launchEvent = true;
                        t->spring();
                    } else if (e->has<CCannon>())	    { bte.inbox.cannon = true; launchEvent = true; }
					else if (e->has<CCreep>())	    { bte.inbox.creep = true; launchEvent = true; }
					else {
						bool hasCurrentElement = bte.currentElement.isValid();
						if (bte.lianaTimer >= 0 && e->has<CLiana>()
							&& (!hasCurrentElement || msg.entity == bte.currentElement)) {
							CLiana* liana(e->get<CLiana>());
							bte.inbox.liana = true;
							unsigned link = liana->getNodeIndex(msg.shape);
							assert(link >= 0);

							Transform linkT(liana->getTransform(link));
							CTransform* meT = Handle(this).getBrother<CTransform>();
							XMVECTOR p_0 = XMVector3TransformCoord(meT->getPivot(), meT->getWorld());
							XMVECTOR offset = XMVector3TransformCoord(p_0,
								XMMatrixInverse(nullptr, linkT.getWorld()));
							if (moduleSq(offset) < moduleSq(bte.lianaOffset)) {
								bte.lianaOffset = offset;
								bte.lianaLink = link;
							}
							launchEvent = true;
						}
					}
					if (launchEvent) {
						bt.reset();
						bte.currentElement = msg.entity;
						bte.previousElement = msg.entity;
					}
				}
			}
			else if (e->has<CDestructible>() &&
				(bt.getCurrentAction() & PlayerMovBtExecutor::COD_CANNON_AIR) != 0) {
				CDestructible* destructible(e->get<CDestructible>());
				destructible->breakGlass();
			}
		}
		else if (e->has<CTransformable>()) {
			if (e->has<CCreep>()){ bt.getExecutor().collisionLostCreep = true; }
		}
	}
}

void CPlayerMov::receive(const MsgPlayerTrampoline &msg)
{
	auto& bte(bt.getExecutor());
	bte.inbox.trampoline = true; 
	bt.reset();
	bte.currentElement = msg.trampoline;
}

void CPlayerMov::receive(const MsgPushTrampoline& msg) {
	auto& bte(bt.getExecutor());
	bte.inbox.trampoline = true;
	bt.reset();
	bte.currentElement = msg.TrampolineEntity;
}

void CPlayerMov::receive(const MsgPlayerCannnon &msg)
{
	auto& bte(bt.getExecutor());
	bte.inbox.cannon = true;
	bt.reset();
	bte.currentElement = msg.cannon;
}

void CPlayerMov::receive(const MsgPlayerCreep &msg)
{
	auto& bte(bt.getExecutor());
	bte.inbox.creep = true;
	bt.reset();
	bte.currentElement = msg.creep;
}

void CPlayerMov::receive(const MsgPlayerSpawn &msg)
{
	bt.getExecutor().inbox.spawn = true;
	bt.getExecutor().reset();
}

void CPlayerMov::receive(const MsgMeleeHit &msg)
{
	bt.getExecutor().resetIdleTimer = true;
}

void CPlayerMov::receive(const MsgFlareHit &msg)
{
	bt.getExecutor().resetIdleTimer = true;
}

void CPlayerMov::receive(const MsgPlayerHasShot &msg)
{
	bt.getExecutor().resetIdleTimer = true;
}

}
