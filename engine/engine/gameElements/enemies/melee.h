#ifndef GAME_ELEMENTS_MELEE_H_
#define GAME_ELEMENTS_MELEE_H_

#include "../gameMsgs.h"
#include "enemies.h"

#include "handles/entity.h"
#include "components/slot.h"
using namespace component;

#include "behavior/bt.h"

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

namespace gameElements{class MeleeBtExecutor;}
namespace behavior {
    typedef Bt<gameElements::MeleeBtExecutor> MeleeBt;
}
using namespace behavior;

namespace gameElements {
	class CMelee;

	class MeleeBtExecutor {
	public:
		enum nodeId_e {
			MELEE = EnemyBtExecutor::NUM_ENUMS,
			AGGRESSIVE			= 0xB018,
			CHASE				= 0xB019,
			TEST_SUBSCRIBE		= 0xB01A,
			CANTSUBSCRIBE		= 0xB01B,
			TAUNT				= 0xB01C,
			CANSUBSCRIBE		= 0xB01D,
			COMBAT				= 0xB01E,
			INTRO_ATTACK		= 0xB01F,
			RANDOM_ANGRY1		= 0xB020,
			DASH				= 0xB021,
			UNSUBSCRIBE_SLOT	= 0xB022,
			DASH_RECOVERY		= 0xB023,
			ANGRY1				= 0xB024,
			SUBSCRIBE_SLOT		= 0xB025,
			FIGHT				= 0xB026,
			RANDOM_IDLEF		= 0xB027,
			IDLE_FIGHT			= 0xB028,
			HIT					= 0xB029,
			COMBO1				= 0xB02A,
			COMBO2				= 0xB02B,
			COMBO3				= 0xB02C,
			RANDOM_ANGRY2		= 0xB02D,
			ANGRY2				= 0xB02E,
			BACK				= 0xB060,
			ANGRY_FAIL_INTRO	= 0xB061,
			CLIF				= 0xB062,
			DEFENSIVE			= 0xB063,
			DASH_WAIT			= 0xB064
		};

		friend behavior::MeleeBt;
		friend CMelee;
	private:
		static const float ANGER_THRESHOLD;
		static const float GRAVITY;
	private:

		Handle enemyAi_h;
		Handle playerEntity;
		Handle meEntity;

		utils::Counter<float> timer;

		float angerFactor;
		float idleFightTime;
		bool subscribedToSlot = false;

		XMVECTOR yVelocity = utils::zero_v;
		XMVECTOR xzVelocity = utils::zero_v;
		float movSpeed = 1;

		bool changeDir = false;
		int doubtDirection = 0;
		bool msgsendProtect = false;
		bool msgsendStopDefense = false;
		bool obstacleDetected = false;
		XMVECTOR dirEnemy = zero_v;
		XMVECTOR pos;
		bool rayFront = false;

	private:
		ret_e doCombo(float elapsed, float timeCombo);
		void onTransform(void);

		void updatePosition(float elapsed);

		//conditions
		inline bool isAngry(float) const { return angerFactor >= ANGER_THRESHOLD; }
		bool contact(float) const;
		bool lostContact(float) const;
		bool forget(float) const;
		bool onClif(float) const;

		inline bool notForget(float elapsed) const { return !forget(elapsed); }
		inline bool notLostContact(float elapsed) const { return !lostContact(elapsed); }

		bool playerAchiable(float) const;
		bool playerNAchiable(float elapsed) const {	return !playerAchiable(elapsed); }

		bool onAttackRange(float) const;
		bool outsideAttackRange(float) const;
		bool canTaunt(float) const;
		bool readyToFight(float e) const { return subscribedToSlot && contact(e); }
		bool canSubscribe(float) const;

		//actions
		ret_e back(float);
		ret_e clifTaunt(float);
		ret_e chase(float);
		ret_e randomIdleFight(float);
		ret_e waitintroAttack(float);
		ret_e introAttack(float);
		ret_e dashRecovery(float);
		ret_e idleFight(float);
		ret_e defend(float elapsed);
		ret_e combo1(float elapsed);
		ret_e combo2(float elapsed);
		ret_e combo3(float elapsed);
		ret_e angry(float);
		ret_e randomAngry(float);
		ret_e subscribeSlot(float elapsed);
		ret_e unsubscribeSlot(float elapsed=0);
		ret_e angryFailIntro(float elapsed);
		ret_e taunt(float elapsed);

		XMVECTOR getPosition(){
			Entity* me(meEntity);
			CTransform* meTransform(me->get<CTransform>());
			return meTransform->getPosition();
		}

	public:
    ~MeleeBtExecutor() {unsubscribeSlot();}
		inline void init(){ reset(); }
		inline void reset() {
			timer.reset();
			subscribedToSlot = false;
		}
		inline void update(float elapsed) {}

		inline int whichSlot(){
			Entity* player = playerEntity;
			CSlots* slots = player->get<CSlots>();
			return slots->whichSlot(meEntity);
		}
	};

	class CMelee {
	private:
		MeleeBt bt;

		int introDamage = 0;
		int combo1Damage = 0;
		int combo2Damage = 0;
		int combo3Damage = 0;
		int idleDamage = 0;
		int stunDamage = 0;
		int aggressiveDamage = 0;

	public:
		static void initType();

		void update(float elapsed);
		void init();

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

		inline void receive(const MsgTransform&) { bt.getExecutor().onTransform(); }
		inline void receive(const MsgSetPlayer& msg) { bt.getExecutor().playerEntity = msg.playerEntity; }
		inline void receive(const MsgStun& msg) { bt.changeState(MeleeBtExecutor::RANDOM_IDLEF); }

		inline void setPlayer(Handle playerEntity) { bt.getExecutor().playerEntity = playerEntity; }

		inline int getAction(){ return bt.getCurrentAction(); }

		int getDamage();

		inline XMVECTOR getPosition(){ return bt.getExecutor().getPosition(); }

		inline int getSlot(){ return bt.getExecutor().whichSlot(); }	

		inline btState_t getState(){ return bt.getState(); }

		inline bool getrayfront(){ return bt.getExecutor().rayFront; }
		inline bool getobstacleDetected(){ return bt.getExecutor().obstacleDetected; }
};

}
#endif