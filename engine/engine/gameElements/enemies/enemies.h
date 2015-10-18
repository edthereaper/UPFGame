#ifndef GAME_ELEMENTS_ENEMIES_H_
#define GAME_ELEMENTS_ENEMIES_H_

#include "../gameMsgs.h"

#include "handles/entity.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "fmod_User/fmodUser.h"
using namespace fmodUser;

#include "behavior/bt.h"
namespace gameElements{class EnemyBtExecutor;}
namespace behavior {
    typedef Bt<gameElements::EnemyBtExecutor> EnemyBt;
}
using namespace behavior;

namespace gameElements {
class CEnemy;

class EnemyBtExecutor {
    public:
        enum {
            COD_NONE=0,
            COD_DEAD = 1<<16,
            COD_DISABLE_DASH = 1<<17,
            COD_STUN = 1<<18,
        };
        enum nodeId_e {
            ENEMY				=	0xB001,
            EVENT				=	0xB002,
			FLYING              =   0xA000,
			FLYING_SETUP        =   0xA001,
			FLYING_ONAIR        =   0xA002,
			FLYING_BRAKE        =   0xA003,
            STUN				=	0xB003, 
            STUN_CAUSE			=	0xB004,
            TACKLE				=	0xB005,
            QUAKE				=	0xB006|COD_STUN,
            RANDOM_STUN			=	0xB007|COD_STUN,
            STUNNED				=	0xB008|COD_STUN,
            RANDOM_ANGRY		=	0xB009,
            ANGRY				=	0xB00A,
            SHOT				=	0xB00B,
            TRANSFORM			=	0xB00C|COD_DISABLE_DASH|COD_DEAD,
            INIT_TRANSFORMING	=	0xB00D|COD_DISABLE_DASH|COD_DEAD,
            TRANSFORMING		=	0xB00E|COD_DISABLE_DASH|COD_DEAD,
            TRANSFORMED			=	0xB00F|COD_DISABLE_DASH|COD_DEAD,
            CELEBRATE			=	0xB010,
            RANDOM_WAIT			=	0xB011, 
            WAIT				=	0xB012,
            ICELEBRATION		=	0xB013, 
            CELEBRATION			=	0xB014,
            OFFENSIVE			=	0xB015,
            IDLE				=	0xB016,  							
			WAS_SHOT			=	0xB017,
			LOOK_AROUND			=	0xB018,
			WAS_PROTECT		    =	0xB019,
			PROTECT			    =	0xB020,
			ALERT				=	0xB030,
			ALERT2				=	0xB031,
			WAS_SHOT2			=	0xB032,
			DEFEND				=	0xB033,
			

			NUM_ENUMS			=	0xB055,
			APPEAR_FROM_BOSS    =   0XB034
        };

        /* What happened to the enemy? */
        enum event_e {
            E_NOTHING,
            E_SHOT,
            E_TACKLED,
            E_TRANSFORMED,
            E_CELEBRATE,
            E_SENDFLYING,
        } lastEvent = E_NOTHING;

        friend behavior::EnemyBt;
        friend CEnemy;
    private:
        static const float ANGER_THRESHOLD;
		static const float GRAVITY;
    private:
        struct inbox_t {
            union {
                struct {
                    //bitfield
                    bool shot       :1;
                    bool megashot   :1;
                    bool tackled    :1;
                    bool quake      :1;
                    bool playerDead :1;
					bool alerted	:1;
					bool protect	:1;
					bool stopdef	:1;
					bool flyer	    :1;
					uint8_t _       :7;
                };
                uint16_t raw;
            };
            bool isEmpty() const {
                return raw == 0;
            }
            void clear() {
                raw = 0;
            }
            inline void receive(const MsgShot& msg) {
                shot = true;
                if (msg.mega) {megashot = true;}
            }
            inline void receive(const MsgDashTackled& msg) {
                tackled = true;
            }
            inline void receive(const MsgEarthquake& msg) {
                quake = true;
            }
            inline void receive(const MsgPlayerDead& msg) {
                playerDead = true;
            }
			inline void receive(const MsgAlert& msg) {
				alerted = true;
			}
			inline void receive(const MsgProtect& msg) {
				protect = true;
			}
			inline void receive(const MsgStopDefense& msg) {
				stopdef = true;
			} 
			inline void receive(const MsgTransform& msg) {
				megashot = true;
			}
        } inbox;

        Handle idleAi;
        Handle offensiveAi;
        Handle meEntity;
        Handle playerEntity;
		Handle bichitoEntity;

        XMVECTOR flyPosition;
        float flySpeed;

		int lives = 3;
													//tmp to remove CParticles
		utils::Counter<float> timer, timerTransform;

		FMOD::Channel *stunChannel = NULL;

        float angerTime = 0.f;
        float stunTime  = 0.f;
        float waitTime  = 0.f;
        bool stun = false;
		bool alert = false;
		bool didAlert = false;
		bool defensive = false;

		bool transformed = false, changemesh = false, onGroundandKilled = false;
		XMVECTOR originScaleV = utils::one_v;

		XMVECTOR yVelocity = utils::zero_v;	
        mutable float prevY = -1000000;
		XMVECTOR xzVelocity = utils::zero_v;

    private:
        //conditions
        inline bool isFlying(float) const {return lastEvent == E_SENDFLYING;}
        bool isDetected(float) const;
		bool onGround(float) const;
		bool onWall(float) const;

		bool playerAchiable(float) const;

        bool hasEvents(float) const;
        inline bool isStunned(float) const {return stun;}
        inline bool wasTransformed(float) const  {return lastEvent == E_TRANSFORMED;}
        inline bool wasTackled(float) const {return lastEvent == E_TACKLED;}

		inline bool isAlerted(float) const { return alert; }

        inline bool wasShotnotAlert(float) const { 
			if (alert || defensive) return false;
			return lastEvent == E_SHOT; 
		}

		inline bool wasShotinDef(float) const {
			if (!defensive) return false;
			return lastEvent == E_SHOT;
		}

        inline bool wasCelebrate(float) const {return lastEvent == E_CELEBRATE;}
        inline bool isAngry(float) const {	return angerTime >= ANGER_THRESHOLD;}

        //actions
        inline ret_e stay(float) { return STAY; }
        ret_e treatEvents(float);
        ret_e initTransforming(float);
        ret_e startFlying(float);
        ret_e doFlying(float);
        ret_e brakeFlying(float);
        ret_e transform(float);
        ret_e angry(float);
        ret_e randomAngry(float);
        ret_e stunned(float);
        ret_e randomStunned(float);
        ret_e wait(float);
        ret_e protect(float);
		ret_e shot(float);
        ret_e randomWait(float);
        ret_e initCelebration(float);
        ret_e doOffensive(float);
		ret_e doAlert(float);
        ret_e doIdle(float);
        ret_e tackledAnim(float);
        ret_e quakedAnim(float);

		ret_e lookAround(float);

		void updatePosition(float elapsed);

		void endOffensive(){
			alert = false;
			defensive = false;
		}

    public:
        inline void init(){ reset(); inbox.clear();}
        inline void reset() { timer.reset();}
        inline void update(float elapsed) {}

};

class CEnemy {
    public:
        static const EntityListManager::key_t TAG = 0xBADD1E00;
        static void initType();
        friend EnemyBtExecutor;
    private:
        EnemyBt bt;
        bool dead=false;
		utils::Counter<float> timerOffParticles;
    public:
		bool isStunned(float elapsed) const { return bt.getExecutor().isStunned(elapsed); }

        void update(float elapsed);
        inline void init() {
            bt.init();
            bt.getExecutor().meEntity = Handle(this).getOwner();
        }

        void endOffensive(){
			bt.getExecutor().endOffensive();
			bt.reset();
		}

        inline void receive(const MsgShot& msg) {
            bt.getExecutor().inbox.receive(msg);
            bt.reset();
        }
        inline void receive(const MsgTransform& msg) {
            bt.getExecutor().inbox.receive(msg);
            bt.reset();
        }
        inline void receive(const MsgDashTackled& msg) {
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

		inline void receive(const MsgAlert& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}

		inline void receive(const MsgProtect& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}

        inline void receive(const MsgSetPlayer& msg) {
            bt.getExecutor().playerEntity = msg.playerEntity;
        }

		inline void receive(const MsgSetBichito& msg) {
			bt.getExecutor().bichitoEntity = msg.bichitoEntity;
		}

		inline void receive(const MsgStopDefense& msg) {
			bt.getExecutor().inbox.receive(msg);
			bt.reset();
		}

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}

        void setOffensive(Handle newOffensiveAi) {bt.getExecutor().offensiveAi = newOffensiveAi;}
        void setIdle(Handle newIdleAi) {bt.getExecutor().idleAi = newIdleAi;}

		inline int getAction() const {return bt.getCurrentAction();}

        inline bool isDead() const { return (getAction() & EnemyBtExecutor::COD_DEAD) != 0;}
		inline bool isPassive() const { 
			return (getAction() == EnemyBtExecutor::TRANSFORMED || getAction() == EnemyBtExecutor::IDLE ||
					getAction() == EnemyBtExecutor::TRANSFORMING || getAction() == EnemyBtExecutor::INIT_TRANSFORMING ||
					getAction() == EnemyBtExecutor::TRANSFORM);
		}

		bool transform(float elapsed);

        void setFlying(XMVECTOR pos, float s) {
            auto& bte = bt.getExecutor();
            bte.flyPosition = pos;
            bte.flySpeed = s;
            bte.inbox.flyer = true;
			bt.reset();
        }
};

}
#endif