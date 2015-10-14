#ifndef GAME_ELEMENTS_BICHITO_H_
#define GAME_ELEMENTS_BICHITO_H_

#include "../gameMsgs.h"

#include "../enemies/enemies.h"

#include "handles/entity.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "behavior/bt.h"
#include "lua_user/lua_component.h"

namespace gameElements{ class BichitoBtExecutor; }
namespace behavior {
	typedef Bt<gameElements::BichitoBtExecutor> BichitoBt;
}
using namespace behavior;

namespace gameElements {
	class CBichito;

	class BichitoBtExecutor {
	public:
		enum nodeId_e {
			BICHITO = 0xC001,
			EVENT = 0xC002,
			FIGHT = 0xC003,
			ALERTENEMY = 0xC004,
			BATTLE = 0xC005,
			VICTORY = 0xC006,
			LOWHP = 0xC007,
			ALERTHP = 0xC008,
			QUAKE = 0xC009,
			ALERTQUAKE = 0xC010,
			DEAD = 0xC011,
			ALERTDEAD = 0xC012,
			ALERTTOOL = 0xC013,
			TOOLDETECTED = 0xC014,
			LOSTPLAYER = 0xC015,
			PROPCLOSE = 0xC016,
			TRANSFORMPROP = 0xC017,
			GOTOPROP = 0xC018,
			RANDOMWAITPROP = 0xC019,
			WAITPROP = 0xC020,
			GOTOTOOL = 0xC021,
			FOLLOWPLAYER = 0xC022,
			REFRESHPROP = 0xC023,
			TELEPORTTOPLAYER = 0xC024,
			TPTOPLAYER = 0xC025,
			TUTORIAL = 0xC026,
			ALERTTUTORIAL = 0xC027,
			DEADRAY = 0xC028,
			IDLE = 0xC040,
		};

		enum event_e {
			E_NOTHING,
			E_FIGHT,
			E_LOW_HP,
			E_EARTHQUAKE,
			E_DEAD,
			E_TELEPORT,
			E_TUTO
		} lastEvent = E_NOTHING;

		friend behavior::BichitoBt;
		friend CBichito;

	private:
		struct inbox_t {
			union {
				struct {
					//bitfield
					bool teleport : 1;
					bool enemy : 1;
					bool lowHP : 1;
					bool quake : 1;
					bool dead : 1;
					bool tutorial : 1;
				};
				uint8_t raw;
			};
			bool isEmpty() const {
				return raw == 0;
			}
			void clear() {
				raw = 0;
			}
			inline void receive(const MsgTeleportToPlayer& msg) {
				teleport = true;
			}
			inline void receive(const MsgAlert& msg) {
				enemy = true;
			}
			inline void receive(const MsgPlayerLowHP& msg) {
				lowHP = true;
			}
			inline void receive(const MsgEarthquake& msg) {
				quake = true;
			}
			inline void receive(const MsgPlayerDead& msg) {
				dead = true;
			}
			inline void receive(const MsgPlayerInTuto& msg) {
				tutorial = true;
			}
		} inbox;


		Handle meEntity;
		Handle cameraEntity;
		Handle playerEntity;

		Handle propEntity;

		XMVECTOR movObjective = utils::zero_v;
		XMVECTOR posObjective = utils::zero_v;
		XMVECTOR bichitoPos = utils::zero_v;
		XMVECTOR playerPos = utils::zero_v;
		XMVECTOR levitatePos = utils::zero_v;
		XMVECTOR prevPos = utils::zero_v;
		utils::Counter<float> timer, timerGhost, timerFollow;

		bool isDead = false;
		bool canAlertEnemy = true;
		bool becomeGhost = false;
		bool bichitoRdyToMove = false;
		bool rdyToFollow = true;
		bool doingTP = false;
		bool playSound = true;

		float speedFallDead = 0.0f;
		float totaltime = 0.0f;
		float timeTransform = 10.0f;
		float max_dist = 1000000.0f;
		float timeWaitProp;

		int tutoZone = -1;
		int helperType = -1;

	private:
		//Calculate the bullet speed vector from a given shot speed
		static XMVECTOR calculateAimAngle(XMVECTOR target, XMVECTOR origin, float offsetY, float shotSpeed);
		//actions
		void updateSpeed(float elapsed);
		void drawImage();

		inline ret_e deadCalculus(float);
		inline ret_e teleportToPlayer(float);
		inline ret_e refreshProp(float);
		inline ret_e transformProp(float);
		inline ret_e goToProp(float);
		inline ret_e goToPlayer(float);
		inline ret_e pointTool(float);
		inline ret_e definePos(float);
		inline ret_e goToPos(float);
		inline ret_e randomWait(float);
		inline ret_e waitProp(float);
		inline ret_e waitTool(float);
		inline ret_e treatEvents(float);
		inline ret_e doAlertEnemy(float);
		inline ret_e doAlertHP(float);
		inline ret_e doAlertQuake(float);
		inline ret_e doAlertDead(float);
		inline ret_e doAlertTuto(float);
		inline ret_e doFight(float);
		inline ret_e doVictory(float);
		inline ret_e doIdle(float);
		//conditions
		bool notCancelled(float) const;
		bool hasEvents(float) const;
		bool toolClose(float) const;
		bool propClose(float) const;
		bool lostPlayer(float) const;
		bool playerGone(float) const;
		bool toolNtransf(float) const;
		inline bool canAlert(float) const { return canAlertEnemy; }
		inline bool isInFight(float) const { return lastEvent == E_FIGHT; }
		inline bool isInQuake(float) const { return lastEvent == E_EARTHQUAKE; }
		inline bool isLowHP(float) const { return lastEvent == E_LOW_HP; }
		inline bool isPlayerDead(float) const { return lastEvent == E_DEAD; }
		inline bool isInTuto(float) const { return lastEvent == E_TUTO; }
		inline bool isTeleport(float) const { return lastEvent == E_TELEPORT || doingTP; }
		inline bool isnotCancel(float) const { return !toolClose(0.0f) && !lostPlayer(0.0f); }
	public:
		inline void init() { reset(); inbox.clear(); }
		inline void reset() { timer.reset(); timerGhost.reset(); timerFollow.reset(); }
		inline void update(float elapsed) {}
	};

	class CBichito {
	private:
		BichitoBt bt;
	public:
		static const EntityListManager::key_t TAG = 0xBADD1E73;
		static void initType();
	public:
		void update(float elapsed);
		inline void init() {
			bt.init();
			bt.getExecutor().meEntity = Handle(this).getOwner();
			bt.getExecutor().inbox.clear();

			Entity* bichito = bt.getExecutor().meEntity;
			bichito->sendMsg(MsgTeleportToPlayer());
		}
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}

		void renderHelpers();

		inline void receive(const MsgSetPlayer& msg) {
			bt.getExecutor().playerEntity = msg.playerEntity;
		}
		inline void receive(const MsgSetCam& msg) {
			bt.getExecutor().cameraEntity = msg.camEntity;
		}
		inline void receive(const MsgAlert& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}
		inline void receive(const MsgPlayerLowHP& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}
		inline void receive(const MsgEarthquake& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}
		inline void receive(const MsgPlayerDead& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}
		inline void receive(const MsgTeleportToPlayer& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}
		inline void receive(const MsgPlayerInTuto& msg) {
			bt.getExecutor().tutoZone = msg.tutoZone;
			bt.getExecutor().inbox.receive(msg.tutoZone);
			bt.reset();
		}
	};

}
#endif