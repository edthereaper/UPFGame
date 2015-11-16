#ifndef GAME_ELEMENTS_PLAYER_ATTACK_H_
#define GAME_ELEMENTS_PLAYER_ATTACK_H_

#include "../gameMsgs.h"

#include "handles/entity.h"
#include "components/detection.h"
#include "components/transform.h"
using namespace component;

#include "behavior/bt.h"
namespace gameElements{ class PlayerAttackBtExecutor; }
namespace behavior {
	typedef Bt<gameElements::PlayerAttackBtExecutor> PlayerAttackBt;
}



using namespace behavior;

namespace gameElements {
	class CPlayerAttack;

	class PlayerAttackBtExecutor {
	public:
		static const unsigned TARGET_LIST_MAX = 16;
		static const float AIM_ROTATE_SPEED;

		enum nodeId_cod_e {
			COD_NOTHING = 0,
			COD_OWNS_ROTATION = (1 << 16),
			COD_AIM = (1 << 17),
		};

		enum nodeId_e {
			PLAYER_ATTACK	= 0x4000,
			AIM_STAY		= 0x5001,
			AIM				= 0x5100 | COD_AIM,
			AIM_INPUT		= 0x5200 | COD_AIM,
			CYCLE_TARGET	= 0x5A00 | COD_AIM,
			SHOOT			= 0x5B00,
			SHOT_TYPE		= 0x5B01,
			MEGASHOT		= 0x5B10,
			MSHOT_CHARGE	= 0x5B11,
			MSHOT_FIRE		= 0x5B12,
			MSHOT_RECOIL	= 0x5B13,
			NORMAL_SHOT		= 0x5B20,
			NSHOT_CHARGE	= 0x5B21,
			NSHOT_FIRE		= 0x5B22,
			NSHOT_RECOIL	= 0x5B23,
			IDLE			= 0x7000,
			AIM_INPUT2		= 0x9000,
			SHOOT2			= 0x9001,
			NORMAL_SHOT2	= 0x9002,
			NSHOT_CHARGE2	= 0x9003,
			NSHOT_FIRE2		= 0x9004,
			NSHOT_RECOIL2	= 0x9005,
			IDLE_STAY		= 0x9006,
			MEGASHOT2		= 0x9007,
			MSHOT_CHARGE2	= 0x9008,
			MSHOT_FIRE2		= 0x9009,
			MSHOT_RECOIL2	= 0x9010,

		};
		friend behavior::PlayerAttackBt;
		friend CPlayerAttack;

	private:
		struct target_t {
			enum target_e {
				NONE,
				ENEMY,
				PROP,
				TRAMPOLINE,
				CANNON,
				LIANA,
				CREEP,
				HAMMER,
			} type = NONE;
			Handle h;

			XMVECTOR getPosition() const;
			bool isTransformed();
			inline bool isValid() { return h.isValid(); }
			float score = CDetection::NO_SCORE;

			bool isAimed = false;

			target_t() = default;
			target_t(target_e type, Handle h) : type(type), h(h) {}
			inline bool operator==(const target_t& b) const { return h == b.h; }
			inline bool operator!=(const target_t& b) const { return h != b.h; }
			inline bool operator<(const target_t& b) const { return score < b.score; }
		};
		typedef target_t targetList_t[TARGET_LIST_MAX];

		static void testAimAndAdd(
			target_t, std::vector<target_t>& list,
			CDetection*, const CTransform&, XMVECTOR mePos);

		//Calculate the bullet speed vector from a given shot speed
		static XMVECTOR calculateAimAngle(XMVECTOR target, XMVECTOR origin, float offsetY, float shotSpeed);
		void setAiming(bool b);

	private:

		Handle meEntity;
		Handle cameraEntity;
		Handle markerEntity;

		XMVECTOR aimPoint;
		target_t aimed, prevAim;
		targetList_t aimList;

		btState_t previousAction;
		bool cameFromAimState = false;
		bool freeAimingMode = false;

		utils::Counter<float> timer, shootCooldown, lookingCounter;
		bool mega = false;
		bool looking = false;

		bool isAutoAimingEnemy = false;
		bool isAimingSomething = false;
        bool activateArmpoint = false;
        bool previousMovHeadonlyCheck = false;

	private:
		void shoot(bool isMega, bool aiming);
		void cycle(bool next);
		void chooseBestTarget();
		void selectTargets();
		void moveMarker();
		void setupLooking(float elapsed);

		//conditions
		bool aimCondition(float) const;
		inline bool NaimCondition(float) const { return !aimCondition(0.0f); }

		bool cycleCondition(float) const;
		bool shootCondition(float) const;
		inline bool isMega(float) const { return mega; }
		inline bool isNotMega(float) const { return !isMega(0.0f); }

		//actions
		ret_e idle(float elapsed);
		ret_e startAim(float elapsed);
		ret_e aim(float);
		ret_e cycleTarget(float);
		ret_e checkLastActionAim(float);
		ret_e fire(float);
		ret_e fire_mega(float);
		ret_e charge(float);
		ret_e recoil(float);
		inline ret_e charge_mega(float elapsed) { return charge(elapsed); }
		inline ret_e recoil_mega(float elapsed) { return recoil(elapsed); }

	public:
		inline void init(){ reset(); }
		inline void reset() {
			timer.reset();
		}
		inline void update(float elapsed) {
			shootCooldown.count(elapsed);
			lookingCounter.count(elapsed);
		}
	};

	class CPlayerAttack {
	public:
		static void initType();
	private:
		PlayerAttackBt bt;
	public:

		void update(float elapsed);
		void init();
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
		inline void setCamEntity(Handle h) { bt.getExecutor().cameraEntity = h; }
		inline void setMarkerEntity(Handle h) { bt.getExecutor().markerEntity = h; }

		inline btState_t getCurrentAction() { return bt.getCurrentAction(); }

		inline void receive(const MsgPlayerHasMegashot& msg) {
			bt.getExecutor().mega = msg.state;
		}
		inline void receive(const MsgSetCam& msg) { setCamEntity(msg.camEntity); }

		inline bool isAutoAimingEnemy() const { return bt.getExecutor().isAutoAimingEnemy; }
		inline XMVECTOR autoAimingPosition() const { return bt.getExecutor().aimed.getPosition(); }
		inline bool wantsToUseArmpoint() const { return bt.getExecutor().activateArmpoint; }
		
		inline bool canDash() const {
			uint32_t action = bt.getState() & 0xFFFF;
			if (bt.getState() == PlayerAttackBtExecutor::MSHOT_FIRE ||
				bt.getState() == PlayerAttackBtExecutor::NSHOT_FIRE ||
				bt.getState() == PlayerAttackBtExecutor::NSHOT_FIRE2 ||
				bt.getState() == PlayerAttackBtExecutor::MSHOT_FIRE2 ||
				bt.getState() == PlayerAttackBtExecutor::NORMAL_SHOT ||
				bt.getState() == PlayerAttackBtExecutor::NORMAL_SHOT2 ||
				bt.getState() == PlayerAttackBtExecutor::MSHOT_RECOIL ||
				bt.getState() == PlayerAttackBtExecutor::NSHOT_RECOIL ||
				bt.getState() == PlayerAttackBtExecutor::NSHOT_RECOIL2 ||
				bt.getState() == PlayerAttackBtExecutor::MSHOT_RECOIL2 ||
				bt.getState() == PlayerAttackBtExecutor::MSHOT_CHARGE ||
				bt.getState() == PlayerAttackBtExecutor::NSHOT_CHARGE ||
				bt.getState() == PlayerAttackBtExecutor::NSHOT_CHARGE2 ||
				bt.getState() == PlayerAttackBtExecutor::MSHOT_CHARGE2) return false;
			return true;
		}
	};

}

#endif