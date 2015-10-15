#ifndef GAME_ELEMENTS_PLAYER_STATS_H_
#define GAME_ELEMENTS_PLAYER_STATS_H_

#include "mcv_platform.h"

#include "../gameMsgs.h"
#include "../input.h"
#include "behavior/fsm.h"
#include "handles/entity.h"
#include "render/render_utils.h"
#include "../enemies/enemies.h"

using namespace utils;

namespace gameElements{
	class MegashotFSMExecutor;
	class InvencibleFSMExecutor;
}

namespace behavior {
	typedef Fsm<gameElements::MegashotFSMExecutor> MegashotFSM;
	typedef Fsm<gameElements::InvencibleFSMExecutor> InvencibleFSM;
}
using namespace behavior;

namespace gameElements {
	class CPlayerStats;

	class MegashotFSMExecutor {
	public:
		enum states {
			STATE_check,
			STATE_activate,
			STATE_during,
			STATE_ending,
			STATE_ended,
		};
		friend MegashotFSM;
		friend CPlayerStats;

	private:
		utils::Counter<float> timer;
		Handle meEntity;
		bool active = false;
		float powerUpTime = 0.f;
        
        float regularSaturation;
        float regularBrightness;
        float regularSelfIll;
        float regularContrast;

		//states
		fsmState_t check(float elapsed);
		fsmState_t activate(float elapsed);
		fsmState_t during(float elapsed);
		fsmState_t ending(float elapsed);
		fsmState_t ended(float elapsed);

	public:
		inline fsmState_t init() { return reset(); }
		inline fsmState_t reset() {
			((component::Entity*)meEntity)->sendMsg(MsgPlayerHasMegashot(false));
			powerUpTime = 0.f;
			active = false;
			return STATE_check;
		}
		inline void update(float elapsed) {	}

		inline void start(float time) {
			powerUpTime = time;
		}
		inline bool isActive() const { return active; }

        
        float getMegashotFactor() const;
	};

	class InvencibleFSMExecutor {
	public:
		enum states {
			STATE_check,
			STATE_activate,
			STATE_during,
			STATE_ending,
			STATE_ended,
		};
		friend InvencibleFSM;
		friend CPlayerStats;

	private:
		utils::Counter<float> timer;
		Handle meEntity;
		bool active = false;
		float powerUpTime;

		//states
		fsmState_t check(float elapsed);
		fsmState_t activate(float elapsed);
		fsmState_t during(float elapsed);
		fsmState_t ending(float elapsed);
		fsmState_t ended(float elapsed);

	public:
		inline fsmState_t init() { return reset(); }
		inline fsmState_t reset() {
			((component::Entity*)meEntity)->sendMsg(MsgPlayerHasInvencible(false));
			powerUpTime = 0.f;
			active = false;
			return STATE_check;
		}
		inline void update(float elapsed) {}

		inline void start(float time) {
			powerUpTime = time;
		}

		inline bool isActive() const { return active; }
	};

#define MAX_LIFE			   150
#define MAX_ENERGY			   100	
#define TIME_INVENCIBLE        7.5f	
#define TIME_MEGASHOT		   15.0f	
#define TIME_GET_DMG		   0.5f	
#define HP_ALERT			   30	
#define TIME_ANIM_DEAD		   2.0f

	class CPlayerStats {
	public:
		static void initType();

		//static const float TIME_MEGASHOT;
		static const unsigned ENERGY_MEGAPOWER = 100;

	private:
		Handle bichitoEntity;

		int health;
		int energy;
		int points;
		int pointsCollectible;
		int pointsCheckpoint;
		int pointsCollectibleCheckpoint;
		bool playerDead;
		bool alertLowHP;
		float timeDeathAnim;
		utils::Counter<float> timerPlayerDead;
		utils::Counter<float> timerDmg;
		float elapsedDmg;
		float fadeoutTim;

		MegashotFSM megashot;
		InvencibleFSM invencible;

		//Actions use 2 bytes. Anything else should use all 4 to rule out conflicts
		static const uint32_t PLUG_DAMAGE		= 0xAAAAAAAA;
		static const uint32_t PLUG_DEATH		= 0xAAAADEAD;
		static const uint32_t PLUG_DEATH_STOP	= 0xAADDDEAD;

		//Related to DrawHUD
		bool playerHealed = false;
		bool playerDamaged = false;
		bool animPlaying = false;
		bool playAnimEnergy = false;
		float angleLeaf = 0.0f;
		float totalTimeMove = 0.0f;
		float totalTimeFall = 0.0f;
		float totalTimeHeal = 0.0f;
		int lastFrameLife = MAX_LIFE;
		bool playAnimation5Heal = false;
		bool playAnimation4Heal = false;
		bool playAnimation3Heal = false;
		bool playAnimation2Heal = false;
		bool playAnimation1Heal = false;
		bool playAnimation5Fall = false;
		bool playAnimation4Fall = false;
		bool playAnimation3Fall = false;
		bool playAnimation2Fall = false;
		bool playAnimation1Fall = false;
		bool playAnimation5Move = false;
		bool playAnimation4Move = false;
		bool playAnimation3Move = false;
		bool playAnimation2Move = false;
		bool playAnimation1Move = false;
		int fallLeafHeight = 0;

	public:
		inline void init() {
			Handle meEntity(Handle(this).getOwner());
			megashot.getExecutor().meEntity = meEntity;
			invencible.getExecutor().meEntity = meEntity;
			megashot.init();
			invencible.init();
			pointsCollectible = 0;
			playerDead = false;
			alertLowHP = false;
			elapsedDmg = 0;
			pointsCollectibleCheckpoint = 0;
			App &app = App::get();
			health = app.getGlobalHealth();
			energy = app.getGlobalEnergy();
			points = app.getGlobalPoints();
			pointsCheckpoint = points;
		}

		void update(float elapsed);

		inline void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}

		inline bool hasMegashot() const { return megashot.getExecutor().isActive(); }
		inline bool hasInvencibility() const { return invencible.getExecutor().isActive(); }

		inline void	becomeInvincible() {
			if (health > 0){
				invencible.getExecutor().start(TIME_INVENCIBLE);
			}
		}

        inline float getMegashotFactor() const {
            return megashot.getExecutor().getMegashotFactor();
        }

		void drawHUD(float elapsed);

		void damage(unsigned amount, bool dmgFromFalling = false);
		void heal(unsigned amount);
		void sumPoints(unsigned amount);
		void sumCollectible(unsigned amount);
		void consumeEnergy(unsigned amount);
		void absorbEnergy(unsigned amount);
		void checkPoint();
		void revive(const MsgRevive& = MsgRevive());
		inline int getHealth() const { return health; }
		inline int getEnergy() const { return energy; }
		inline int getPoints() const { return points; }
		inline int getPointsCollectible() const { return pointsCollectible; }
		inline bool isPlayerDead() const { return playerDead; }

		inline void receive(const MsgMeleeHit& msg)  { damage(msg.damage); }
		void receive(const MsgFlareHit& msg);

		inline void receive(const MsgPickupHeal& msg) { heal(msg.heal); }
		inline void receive(const MsgPickupEnergy& msg) { absorbEnergy(msg.absorbEnergy); }
		inline void receive(const MsgPickupInvincible& msg) { becomeInvincible(); }
		inline void receive(const MsgPickupCoin& msg) { sumPoints(msg.absorbCoin); }
		inline void receive(const MsgPickupCollectible& msg) { sumCollectible(msg.absorbColectible); }
		inline void receive(const MsgPlayerAchievedCheckpoint& msg) { checkPoint(); }
		inline void receive(const MsgSetBichito& msg)  { bichitoEntity = msg.bichitoEntity; }

	};

}

#endif