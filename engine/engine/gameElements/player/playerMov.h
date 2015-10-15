#ifndef GAME_ELEMENTS_PLAYER_MOV_H_
#define GAME_ELEMENTS_PLAYER_MOV_H_

#include "../gameMsgs.h"

#include "controller.h"

#include "handles/entity.h"
using namespace component;

#include "render/mesh/component.h"
#include "gameElements/liana.h"


#include "behavior/bt.h"
namespace gameElements{ class PlayerMovBtExecutor; }
namespace behavior {
	typedef Bt<gameElements::PlayerMovBtExecutor> PlayerMovBt;
}
using namespace behavior;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "../props.h"


namespace gameElements {
	class CPlayerMov;

	class PlayerMovBtExecutor {
	public:
		static const float MASS_PLAYER;
		static const float MAX_SPEED;
		static const float ACCELERATION;
		static const float AIR_CONTROL;
		static const float AIR_MAX_SPEED;
		static const float AIR_DECEL;
		static const float ALIGN_ROTATE_SPEED;
		static const float IMPULSE_ALIGN_SPEED;
		static const float GROUND_DECEL;
		static const float WALL_DECEL;
		static const float LAND_FRICTION_FACTOR;
		static const float DEATH_FRICTION_FACTOR;
		static const float GRAVITY;
		static const float GRAVITYCANNON;
		static const float MAX_JUMP;
		static const float DASH_SPEED;
		static const float TRAMPOLINE_IMPULSE_V;
		static const float TRAMPOLINE_IMPULSE_H;
		static const float TRAMPOLINE_IMPULSE_CANNON_FACTOR;
		static const float QUAKE_RADIUS;
		static const float QUAKE_HEIGHT;
		static const float QUAKE_DROP_INITIAL_IMPULSE;
		static const float CREEP_SPEED;
		static const float CREEP_DROP_IMPULSE;
		static const float CREEP_JUMP_IMPULSE;
		static const float CREEP_JUMP_ANGLE;
		static const XMVECTOR LIANA_HOOK_OFFSET;
		static const float LIANA_GRAB_APPROACH_SPEED;
		static const float LIANA_ACCELERATION;
		static const float LIANA_DECELERATION;
		static const float LIANA_BRAKE;
		static const float LIANA_BRAKE_DELTA;
		static const float LIANA_MAX_IMPULSE;
		static const float LIANA_IMPULSE_HALF_FOV;
		static const float LIANA_ALIGN_ROTATE_SPEED;
		static const float LIANA_MINIMUM_ANGLE;
		static const float LIANA_MIN_JUMP;
		static const float LIANA_MAX_JUMP;
		static const float LIANA_MIN_JUMP_IMPULSE;
		static const float LIANA_MAX_JUMP_IMPULSE;
		static const float LIANA_NEGLIGIBLE_ANGLE;
        static const float LIANA_FORCE_RECOVER;
		static const float DASH_BOUNCE_IMPULSE;
		static const float TIME_LIANA_JUMP_REGRAB;
		static const float TIME_LIANA_DROP_REGRAB;
		static const float TIME_CANNON_IMPULSE;
		static const float TIME_CANNON_TIMEOUT;
		static const float TIME_CANNONPATH_SPAWN;

		/* extra info in the node id */
		enum nodeId_codification_e {
			COD_NOT_SPECIAL = 0,
			COD_ON_AIR = 1 << 16,
			COD_CANNON_AIR = 1 << 17,
			COD_TILT_GAUGE_Y = 1 << 18,
			COD_LIANA = 1 << 19,
			COD_DAMAGE_PROTECT = 1 << 20,
			COD_CAN_SHOOT = 1 << 21,
			COD_ON_DASH_HOLD = 1 << 22,
			COD_DASHING = 1 << 23,
			COD_CAN_QUAKE = 1 << 24,
			COD_DISABLE_LOOKAT = 1 << 25,
			COD_ON_QUAKE = 1 << 26,
			COD_TILT_ON_VELOCITY = 1 << 27,
			COD_MANAGES_TILT = 1 << 28,
			COD_DISABLE_ARMPOINT = 1 << 29,
			COD_DISABLE_LOOKAT_HEADONLY = 1 << 30,
			COD_HIGHLIGHT_TRANSFORMABLES = 1 << 31,
		};

		/* lower 16 bits are unique for each node, higher 16 bits are tagging with nodeId_codification_e */
		enum nodeId_e {
			PLAYER_MOVEMENT = 0x0000 | COD_CAN_QUAKE,			//New
			INBOX = 0x1000,
			DEAD = 0x1AAA | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			CANNON = 0x1100,
			CANNON_SETUP = 0x1110 | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			CANNON_ENTER = 0x1111 | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			CANNON_AIM = 0x1120 | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			CANNON_SHOOT = 0x1130 | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			CANNON_AIR = 0x1140,
			CANNON_AIR_IMPULSE = 0x1141 | COD_TILT_ON_VELOCITY | COD_DISABLE_LOOKAT | COD_ON_AIR | COD_CANNON_AIR | COD_CAN_SHOOT,
			CANNON_AIRUP = 0x1142 | COD_TILT_ON_VELOCITY | COD_ON_AIR | COD_CANNON_AIR | COD_CAN_SHOOT,
			CANNON_AIRDOWN = 0x1143 | COD_TILT_ON_VELOCITY | COD_ON_AIR | COD_CANNON_AIR | COD_CAN_SHOOT,
			CANNON_LAND = 0x1144 | COD_CAN_SHOOT | COD_ON_DASH_HOLD,
			TRAMPOLINE = 0x1200,
			TRAMPOLINE_CLEANIB = 0x1210 | COD_CAN_SHOOT,
			TRAMPOLINE_SETUP = 0x1220 | COD_CAN_SHOOT,
			QUAKE = 0x1300,
			QUAKE_CHECKAIR = 0x1310,
			QUAKE_DROP = 0x1311 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_DROP_SETUP = 0x1312 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_DROP_START = 0x1313 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_FALLING = 0x1314 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT | COD_ON_AIR,
			QUAKE_HITGROUND = 0x1315 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_PUNCHGROUND = 0x1318 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_STUNENEMIES = 0x1320 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_DURING = 0x1321 | COD_ON_QUAKE | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			QUAKE_GETUP = 0x1330 | COD_CAN_QUAKE,
			LIANA = 0x1400 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_GRAB = 0x1410 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_STAY = 0x14A0 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_INPUT = 0x14B0 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_DROP = 0x14B2 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_JUMP = 0x14B3 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_GRASP = 0x14B4 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_MOVE = 0x14B5 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_IDLE = 0x14B6 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			LIANA_EXIT = 0x1420 | COD_LIANA | COD_MANAGES_TILT | COD_DISABLE_LOOKAT,
			CREEP = 0x1500,
			CREEP_STAY = 0x1580,
			CREEP_INPUT = 0x1590 | COD_DISABLE_LOOKAT,
			CREEP_CLIMB = 0x1591 | COD_DISABLE_LOOKAT,
			CREEP_DROP = 0x1592 | COD_DISABLE_LOOKAT,
			CREEP_FALL = 0x1593 | COD_DISABLE_LOOKAT,
			CREEP_JUMP = 0x1594 | COD_DISABLE_LOOKAT,
			CREEP_MOVE = 0x1595 | COD_DISABLE_LOOKAT,
			CREEP_IDLE = 0x1596 | COD_DISABLE_LOOKAT,
			IMPULSE = 0x1600 |  COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE,
			IMPULSE_UP_ANIM = 0x1609 |  COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE,
			IMPULSE_UP = 0x1610 |  COD_TILT_ON_VELOCITY | COD_TILT_GAUGE_Y | COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE ,
			IMPULSE_DOWN = 0x1620 | COD_TILT_ON_VELOCITY | COD_TILT_GAUGE_Y | COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE ,
			IMPULSE_LAND = 0x1630 | COD_CAN_SHOOT | COD_ON_DASH_HOLD | COD_CAN_QUAKE,
			AIR = 0x1700 | COD_TILT_ON_VELOCITY | COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE,
			AIR_UP = 0x1710 | COD_TILT_ON_VELOCITY | COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE | COD_TILT_GAUGE_Y,
			AIR_DOWN = 0x1720 | COD_TILT_ON_VELOCITY | COD_ON_AIR | COD_CAN_SHOOT | COD_CAN_QUAKE | COD_TILT_GAUGE_Y,
			AIR_LAND = 0x1730 | COD_CAN_SHOOT | COD_ON_DASH_HOLD | COD_CAN_QUAKE,
			GROUND = 0x2000 | COD_CAN_SHOOT | COD_CAN_QUAKE,
			RUN = 0x2100 | COD_CAN_SHOOT | COD_ON_DASH_HOLD | COD_CAN_QUAKE,
			DASH = 0x2200,
			DASH_HOLD = 0x2210,
			DASH_SETUP = 0x2220 | COD_DISABLE_LOOKAT | COD_DASHING,
			DASH_DURING = 0x2221 | COD_DISABLE_LOOKAT | COD_DASHING,
			DASH_TACKLED = 0x2230 | COD_DISABLE_LOOKAT | COD_DASHING,
			DASH_BOUNCE = 0x2231 | COD_CAN_QUAKE,
			DASH_STOP = 0x2232 | COD_CAN_SHOOT | COD_CAN_QUAKE,
			JUMP = 0x2300 | COD_CAN_SHOOT | COD_CAN_QUAKE,
			JUMP_IMPULSE = 0x2310 | COD_TILT_ON_VELOCITY | COD_CAN_SHOOT | COD_ON_DASH_HOLD,		//can quake deleted
			JUMP_SET_JUMPED = 0x2320 | COD_TILT_ON_VELOCITY | COD_CAN_SHOOT | COD_CAN_QUAKE,
			IDLE = 0x2400 | COD_CAN_SHOOT | COD_ON_DASH_HOLD | COD_CAN_QUAKE | COD_HIGHLIGHT_TRANSFORMABLES,
			SPAWN = 0x2500,
			SPAWN_GROUND = 0x2501,
			SPAWN_FALL = 0x2502 | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			SPAWN_ANIM = 0x2504 | COD_DISABLE_LOOKAT | COD_DAMAGE_PROTECT,
			CREEP_MOVE_UP = 0xC001 | COD_DISABLE_LOOKAT,
			CREEP_MOVE_RIGHT = 0xC004 | COD_DISABLE_LOOKAT,
			IDLE_LONG = 0x24c1 | COD_DISABLE_LOOKAT_HEADONLY | COD_CAN_SHOOT |
            COD_ON_DASH_HOLD | COD_CAN_QUAKE | COD_HIGHLIGHT_TRANSFORMABLES,
		};

		friend behavior::PlayerMovBt;
		friend CPlayerMov;

		struct tiltData_t {
			bool straighten = true;
			bool tilt = false, testRay = true;
			bool invertZ, invertX;
			float maxF = 0, maxL = 0;
			float speedF = 1, speedL = 1;
            float rayCast = 0.25f;

			tiltData_t() = default;
			tiltData_t(bool straighten, bool testRay,
				float maxF, float maxL,
				float speedF, float speedL,
				bool invertZ, bool invertX,
                float rayCast
				) : tilt(true), straighten(straighten), testRay(testRay), maxF(maxF),
				maxL(maxL), speedF(speedF), speedL(speedL),
				invertZ(invertZ), invertX(invertX), rayCast(rayCast) {}
		};
	private:
		XMVECTOR xzVelocity = utils::zero_v;
		XMVECTOR prevXZVelocity = utils::zero_v;
		XMVECTOR yVelocity = utils::zero_v;
		bool isOnCreepTop() const;
		bool onCeiling() const;

		Handle bichitoEntity;
		Handle meEntity;
		Handle sidekickEntity;
		Handle cameraEntity;
		Handle currentElement;
		Handle previousElement;
		btState_t previousAction = PLAYER_MOVEMENT;
		bool cannonCam = false;
		bool lockedRotation = false;
		utils::Counter<float> timer, dashCooldown, cannonPathTimer, idleTimer;
		utils::Counter<float> lianaTimer, lianaSoundTimer;
		XMVECTOR lianaOffset = one_v * 1000;
		XMVECTOR lianaImpulse = utils::zero_v;
		XMVECTOR lianaFaceDirCurrent = zAxis_v; //Guaranteed to be aligned with XZ plane
		XMVECTOR lianaFaceDirTarget = zAxis_v;  //Guaranteed to be aligned with XZ plane
		int lianaLink = -1;
		float lianaBrake = 0;
		int cannonTimeState = 0;

		struct inbox_t {
			union {
				struct {
					bool cannon : 1;
					bool trampoline : 1;
					bool quake : 1;
					bool liana : 1;
					bool creep : 1;
					bool air : 1;
					bool impulse : 1;
					bool dead : 1;
					bool spawn : 1;
				};
				uint16_t raw = 0;
			};
			inbox_t() : raw(0) {}
			void clear() { raw = 0; }
			bool isEmpty() const { return raw == 0; }
		} inbox;

		bool prevFell = false;
		bool fell = false;
		bool onAir = false;
		bool onCannonAir = false;
		bool fromCannon = false;
		bool tackled = false;
		bool stayOnCurrentElement = false;
		bool collisionLostCreep = false;
		bool deathByHole = false;
		bool resetIdleTimer = false;
		bool doingQuake = false;
		bool playLongIdle = false;
		bool unplugJump = false;
		bool playLongIdleCycle = false;
		bool startCreepRight = false;
		bool startCreepLeft = false;
		bool startCreepUp = false;
		bool startCreepDown = false;
		int priorityUp = 0;
		int priorityDown = 0;
		int priorityRight = 0;
		int priorityLeft = 0;
		bool playedSoundDeadFall = false;

		struct cannonCalc_t {
			XMVECTOR yVel, xzVel, camPos, camFront;
		};
		cannonCalc_t PlayerMovBtExecutor::calculateCannonVelocities();
		void spawnPathMarker(XMVECTOR rot);

	private:
		void updateLianaMovement(float elapsed, bool grasp, bool input = true);
		XMVECTOR calculateLianaImpulseDelta(float elapsed);
		void updateSpeed(float elapsed, bool input, bool gravity, bool decel, float decelFactor = 1.f);

		void setup3PCam();
		void setupCannonCam();
		bool canFall();
		bool creepClimbTest() const;
		CamCannonController camCannon;
		Cam3PController cam3P;

		static inline float calculateImpulse(float amount) { return sqrt(amount * 2 * GRAVITY); }

		//conditions
		inline bool inboxIsNotEmpty(float) const { return !inbox.isEmpty(); }
		inline bool inboxCannon(float) const { return inbox.cannon; }
		inline bool inboxTrampoline(float) const { return inbox.trampoline; }
		inline bool inboxQuake(float) const { return inbox.quake; }
		inline bool inboxLiana(float) const { return inbox.liana; }
		inline bool inboxCreep(float) const { return inbox.creep; }
		inline bool inboxAir(float) const { return inbox.air; }
		inline bool inboxImpulse(float) const { return inbox.impulse; }
		inline bool inboxDead(float) const { return inbox.dead; }
		inline bool inboxSpawn(float) const { return inbox.spawn; }

		bool canRun(float) const;
		bool canDash(float) const;
		bool keyJump(float) const;
		bool keyDrop(float) const;
		bool keyGraspLiana(float) const;
		bool velocityUp(float) const;
		inline bool isOnAir(float) const { return onAir; }
		inline bool hadTackled(float) const { return tackled; }
		bool willHoldDash(float) const;
		bool stayOnElement(float) const;
		bool creepCanDrop(float) const;
		bool creepCanClimb(float) const;
		bool creepCanMove(float) const;

		bool creepCanMoveUp(float) const;
		bool creepCanMoveRight(float) const;

		bool fellFromCreep(float) const;
		bool lianaDropCond(float) const;
		bool lianaJumpCond(float) const;
		bool lianaMoving(float) const;

		bool isTrampolineUnder(float) const;
		bool hasStayonIdle(float) const;

		//actions  
		inline ret_e clearInbox(float) { inbox.clear(); return DONE; }
		ret_e death(float);
		ret_e groundIdle(float);
		ret_e groundRun(float);
		ret_e groundIdleLong(float);
		ret_e land(float);
		ret_e jumpImpulse(float);
		ret_e jumpInitJump(float);
		ret_e airUp(float);
		ret_e airDown(float);
		ret_e cannonImpulse(float);
		ret_e noControlAirUp(float);
		ret_e noControlAirDown(float);
		ret_e setupTrampoline(float);
		ret_e impulsedAirAnim(float);
		ret_e toGround(float);
		ret_e spawnAnim(float);
		ret_e impulsedAirUp(float);
		ret_e impulsedAirDown(float);
		ret_e cannonSetup(float);
		ret_e cannonEnter(float);
		ret_e cannonAim(float);
		ret_e cannonShoot(float);
		ret_e dashHold(float);
		ret_e dashInit(float);
		ret_e dashSetup(float);
		ret_e dashDuring(float);
		ret_e dashBounce(float);
		ret_e dashStop(float);
		ret_e quakeDropSetup(float);
		ret_e quakeDropStart(float);
		ret_e quakeDrop(float);
		ret_e punchGround(float);
		ret_e quakeEnemies(float);
		ret_e duringQuake(float);
		ret_e quakeGetUp(float);
		ret_e setupCreep(float);
		ret_e creepClimb(float);
		ret_e creepDrop(float);
		ret_e fallFromCreep(float);
		ret_e creepJump(float);
		ret_e creepMove(float);
		ret_e creepMoveUp(float);
		ret_e creepMoveRight(float);
		ret_e creepIdle(float);
		ret_e dontStayOnCurrent(float);
		ret_e setupLiana(float);
		ret_e lianaDrop(float);
		ret_e lianaJump(float);
		ret_e lianaGrasp(float);
		ret_e lianaMove(float);
		ret_e lianaIdle(float);
		ret_e lianaGrab(float);
		ret_e lianaExit(float);

	public:
		inline void init(){ reset(); }
		inline void reset() {
			timer.reset();
		}
		inline void update(float elapsed) {
			dashCooldown.count(elapsed);
		}

		XMVECTOR getPosition(){
			Entity* me(meEntity);
			CTransform* meTransform(me->get<CTransform>());
			return meTransform->getPosition();
		}

		inline void setTackled(bool tackled_){ tackled = tackled_; }

		tiltData_t getTiltData(behavior::btState_t action);

		void setupSpawn();
		void ground(float);
	};

	class CPlayerMov {
	public:
		static void initType();

		static const float TILT_MIN;
	private:
		PlayerMovBt bt;
		bool onCannonAir = false;
        utils::Counter<float> paintDelay;
        float regularPaintSize = 2.5f;
        float megaPaintSize = 3.5f;

	public:
		void update(float elapsed);
		void updatePaused(float elapsed);

		void init();

		inline btState_t getState() const { return bt.getState(); }
		inline void setTackled(bool tackled_){ bt.getExecutor().tackled = true; }

		inline void setDeathByHole(){ bt.getExecutor().deathByHole = true; bt.getExecutor().playedSoundDeadFall = false; }
		inline bool getDeathByHole(){ return bt.getExecutor().deathByHole; }

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            regularPaintSize = atts.getFloat("regularPaintSize", regularPaintSize);
            megaPaintSize = atts.getFloat("megaPaintSize", megaPaintSize);
        }

		inline Handle getCam() const { return bt.getExecutor().cameraEntity; }
		inline void setCamEntity(Handle h) { bt.getExecutor().cameraEntity = h; }
		inline void setCam1P(Handle cameraEntity) { bt.getExecutor().cannonCam = true; }
		inline void setCam3P(Handle cameraEntity) { bt.getExecutor().cannonCam = false; }
		void resetCamSpawn();

		inline void setSidekick(Handle sidekickEntity) { bt.getExecutor().sidekickEntity = sidekickEntity; }

		inline PlayerMovBtExecutor::nodeId_e getPreviousAction() const {
			return (PlayerMovBtExecutor::nodeId_e)bt.getExecutor().previousAction;
		}

		inline void pushEarthQuake() {
			bt.getExecutor().inbox.quake = true;
			bt.reset();
		}

		inline XMVECTOR getPosition() {
			return bt.getExecutor().getPosition();
		}

		inline void receive(const MsgSetCam& msg) { setCamEntity(msg.camEntity); }
		void receive(const MsgCollisionEvent &msg);
		void receive(const MsgHitEvent &msg);
		void receive(const MsgPlayerDead &msg);
		void receive(const MsgPlayerTrampoline &msg);
		void receive(const MsgPlayerCannnon &msg);
		void receive(const MsgPlayerCreep &msg);
		void receive(const MsgPushTrampoline& msg);
		void receive(const MsgPlayerSpawn& msg);
		void receive(const MsgMeleeHit& msg);
		void receive(const MsgFlareHit& msg);
		void receive(const MsgPlayerHasShot& msg);

		inline Handle getCurrentElement(){
			return bt.getExecutor().currentElement;
		}

		void tilt(CTransform* meT, behavior::btState_t currentAction, float elapsed);

		inline bool isOnCannon(){
			if (bt.getState() == PlayerMovBtExecutor::CANNON_AIM || bt.getState() == PlayerMovBtExecutor::CANNON_SHOOT) return true;
			return false;
		}
		
		inline bool isOnCreep(){
			if (bt.getState() == PlayerMovBtExecutor::CREEP_CLIMB ||
				bt.getState() == PlayerMovBtExecutor::CREEP_INPUT||
				bt.getState() == PlayerMovBtExecutor::CANNON_SHOOT ||
				bt.getState() == PlayerMovBtExecutor::CREEP_DROP ||
				bt.getState() == PlayerMovBtExecutor::CREEP_FALL ||
				bt.getState() == PlayerMovBtExecutor::CREEP_JUMP ||
				bt.getState() == PlayerMovBtExecutor::CREEP_MOVE ||
				bt.getState() == PlayerMovBtExecutor::CREEP_IDLE) return true;
			return false;
		}

		inline bool isOnLiana(){
			if (bt.getState() == PlayerMovBtExecutor::LIANA_GRAB ||
				bt.getState() == PlayerMovBtExecutor::LIANA_INPUT ||
				bt.getState() == PlayerMovBtExecutor::LIANA_JUMP ||
				bt.getState() == PlayerMovBtExecutor::LIANA_DROP ||
				bt.getState() == PlayerMovBtExecutor::LIANA_GRASP ||
				bt.getState() == PlayerMovBtExecutor::LIANA_MOVE ||
				bt.getState() == PlayerMovBtExecutor::LIANA_IDLE ||
				bt.getState() == PlayerMovBtExecutor::LIANA_EXIT) return true;
			return false;
		}

		inline int getCannonState() { return bt.getExecutor().cannonTimeState; }

		inline void setupSpawn() {
			bt.reset();
			bt.getExecutor().setupSpawn();
			bt.getExecutor().deathByHole = false;
		}

		inline void receive(const MsgSetBichito& msg)  { bt.getExecutor().bichitoEntity = msg.bichitoEntity; }
	};

}
#endif