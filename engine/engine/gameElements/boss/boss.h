#ifndef GAME_ELEMENTS_BOSS_H_
#define GAME_ELEMENTS_BOSS_H_

#include "../gameMsgs.h"

#include "handles/entity.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "behavior/bt.h"
namespace gameElements{class BossBtExecutor;}
namespace behavior {
    typedef Bt<gameElements::BossBtExecutor> BossBt;
}
using namespace behavior;

namespace gameElements {
class CBoss;

class BossBtExecutor {
    public:
        static const unsigned N_STAGES = 3;
        static const unsigned N_SHOTS[N_STAGES];
        
        static const float TIME_BEGIN;
        static const float TIME_CRASH_WAIT;
        static const float COOLDOWN_PUNCH_FIRST_TIME;
        static const float COOLDOWN_PUNCH[N_STAGES];
        static const unsigned MIN_SPAWN[N_STAGES];
        static const float COOLDOWN_SHOOT[N_STAGES];
        static const float TIME_HEATING[N_STAGES];
        static const float TIME_SMOKING;
        static const float TIME_PUNCH_WARNING[N_STAGES];
        static const float TIME_ARM_RESIST_DELAY;
        static const float TIME_ARM_RESISTING;
        static const float TIME_EARTHQUAKE[N_STAGES];
        static const float TIME_DAMAGE_SMOKE[N_STAGES];
        static const float TIME_SEND_MINIONS_BEFORE;
        static const float TIME_SEND_MINIONS_AFTER;
        static const float SKYBOX_BLEND[N_STAGES];
        static const float SKYBOX_BRIGHT[N_STAGES];
        static const float BOSS_LIGHT[N_STAGES];
        static const float HAMMER_DROP_ACCEL;
        static const float HAMMER_DROP_SPEED;
        static const float HAMMER_SUDDEN_RAISE_ACCEL;
        static const float HAMMER_SUDDEN_RAISE_TIME;
        static const float HAMMER_SLOW_RAISE_ACCEL;
        static const float HAMMER_SLOW_RAISE_TIME;
        static const float HAMMER_WARNING_SHAKE_AMPLITUDE;
        static const float HAMMER_WARNING_SHAKE_FREQUENCY;
        static const float HAMMER_EARTHQUAKE_SHAKE_AMPLITUDE;
        static const float HAMMER_EARTHQUAKE_SHAKE_FREQUENCY;
        static const float CAM_SHAKE_AMOUNT;
        static const float CAM_SHAKE_FREQ;
        static const float HAMMER_RESISTING_SHAKE_AMPLITUDE;
        static const float HAMMER_RESISTING_SHAKE_FREQUENCY;
        static const float FLARE_SPEED;
        static const std::function<float()> TIME_PUNCH_WAIT[BossBtExecutor::N_STAGES];
        static const std::function<float()> TIME_IDLE[BossBtExecutor::N_STAGES];
        static const std::function<bool()> CHANCE_PUNCH[BossBtExecutor::N_STAGES];
        static const std::function<bool()> MINION_CHANCE[BossBtExecutor::N_STAGES];
        static const std::function<bool()> FLARE_CHANCE[BossBtExecutor::N_STAGES];


        static const std::vector<uint32_t> smokePatterns[N_STAGES];

    public:
        enum {
            COD_NONE            =0,
            COD_DONT_ALIGN      =1<<16,
            COD_ALIGN_FASTER    =1<<17,
            COD_HIGHLIGHT       =1<<18,
        };

        enum nodeId_e {
            BOSS                    = 0x0000,
            EVENT                   = 0x1000,
            FIRST_TIME              = 0x1400,
            FIRST_TIME_SETUP        = 0x1410,
            FIRST_TIME_WAIT         = 0x1420,
            FIRST_TIME_PREPARE      = 0x1430,
            MISSED_CRASH            = 0x1100|COD_DONT_ALIGN,   
            MISSED_CRASH_FX         = 0x1110|COD_DONT_ALIGN,
            MISSED_CRASH_WAIT       = 0x1120|COD_DONT_ALIGN,
            LAST_HIT_CRASH          = 0x1200|COD_DONT_ALIGN, 
            LAST_HIT_CRASH_FX       = 0x1210|COD_DONT_ALIGN,
            GAME_OVER               = 0x1220|COD_DONT_ALIGN,      
            WEAK_SPOT_CRASH         = 0x1300|COD_DONT_ALIGN,
            WEAK_SPOT_CRASH_FX      = 0x1310|COD_DONT_ALIGN,
            WEAK_SPOT_CRASH_WAIT    = 0x1320|COD_DONT_ALIGN,
            RAISE_DIFFICULTY        = 0x1330|COD_DONT_ALIGN,
            UPDATE_SKYBOX           = 0x1340,
            SETUP_IDLE              = 0x2000,     
            IDLE                    = 0x2100,           
            ATTACK                  = 0x3000,         
            PUNCH                   = 0xA000,          
            PREPARE_WARNING         = 0xA100,
            WARNING                 = 0xA110,        
            PREPARE_ARM_DROP        = 0xA200,
            ARM_DROP                = 0xA210,       
            ARM_DOWN                = 0xA300,       
            HIT_GROUND              = 0xA310,     
            SMOKE_WARNING           = 0xA320,  
            EARTHQUAKE              = 0xA321,     
            SMOKE_START             = 0xA330,    
            SMOKE_DURING            = 0xA331,   
            SETUP_PUNCH_WAIT        = 0xA340|COD_HIGHLIGHT,
            PUNCH_WAIT              = 0xA341|COD_HIGHLIGHT,
            COOL_SMOKE              = 0xA500,
            RAISE_ARM               = 0xA400,       
            RAISE_TRANSFORMATED     = 0xA410,
            DAMAGED_DELAY_SETUP     = 0xA430, 
            DAMAGED_DELAY           = 0xA431,       
            ARM_RESISTING_SETUP     = 0xA432, 
            ARM_RESISTING           = 0xA411,  
            DROP_CANNON             = 0xA412,    
            PREPARE_SUDDEN_RAISE    = 0xA413,
            RAISE_ARM_SUDDENLY      = 0xA41A,
            OPEN_WEAK_SPOT          = 0xA414, 
            RAISE_DAMAGED_END_FX    = 0xA415,
            RAISE_UNTOUCHED         = 0xA420, 
            PREPARE_SLOW_RAISE      = 0xA421,
            RAISE_ARM_SLOWLY        = 0xA42A,
            RAISE_HEALTHY_END_FX    = 0xA422,
            REGULAR_ATTACK          = 0xB000, 
            FLAMES                  = 0xB100,         
            SETUP_HEAT              = 0xB110,     
            HEAT                    = 0xB111|COD_ALIGN_FASTER,           
            SHOOT_N_TIMES           = 0xB120|COD_ALIGN_FASTER,  
            SHOOT_SET               = 0xB121|COD_ALIGN_FASTER,
            SHOOT                   = 0xB12A|COD_ALIGN_FASTER,             
            SHOOT_COOLDOWN_SET      = 0xB122|COD_ALIGN_FASTER,
            SHOOT_COOLDOWN          = 0xB12B|COD_ALIGN_FASTER, 
            RESET_N_SHOTS           = 0xB130,  
            SETUP_SMOKING           = 0xB140,  
            SMOKING                 = 0xB141,   
            SMOKING_END             = 0xB150,        
            SPAWN_ENEMIES           = 0xB200,  
            WALL_OF_SMOKE           = 0xB210,  
            WAIT_FOR_MINIONS        = 0xB220,
            SEND_MINIONS            = 0xB230,   
            WAIT_FOR_SMOKE          = 0xB240,
            END_WALL_OF_SMOKE       = 0xB250,
        };
        
#ifdef _DEBUG
        static std::string DEBUG_NAME(nodeId_e e);
#endif

        friend behavior::BossBt;
        friend CBoss;

    private:
        struct inbox_t {
            union {
                struct {
                    //bitfield
                    bool hitMissed      :1;
                    bool hitWeakSpot    :1;
                    bool earthquake     :1;
                    bool playerDead     :1;
                    bool firstTime      :1;
                    uint8_t _           :3;
                };
                uint8_t raw;
            };
            inline bool isEmpty() const {return raw == 0;}
            inline void clear() {raw = 0;}  

            void receive();
        } inbox;
        Handle meEntity;
        Handle playerEntity;
		Handle bichitoEntity;
		utils::Counter<float> timer, punchTimer, tutoAlertTimer;

		bool tutoCannon = false;
        uint8_t stage = 0;
        uint8_t shotsFired = 0;
        float hammerY = 0;
        float currentHammerY = 0;
		
		int currentEnemyCreated = 0;
		unsigned nSpawn = 0;
		unsigned failed = 0;

        bool waitingForWeakSpot = false;

        struct hammer_t {
            component::Handle hammer;
            component::Handle weakSpot;
            component::Handle cannon;
            component::Transform cannonMark;
            enum {SAFE, DESTROYED} state = SAFE;
        } hammers[3];
        uint8_t currentHammer = 0;

        struct spawn_t {
            component::Transform point;
            component::Handle entity;
        };
        spawn_t minions[12];

    private:
        void spawnMinions(float elapsed);
        void shootFlare();

        //conditions
        inline bool hasEvents(float) const{return !inbox.isEmpty();}
        bool firstTime(float) const;
        bool missedCrash(float) const;
        bool crashedLastSpot(float) const;
        bool crashedWeakSpot(float) const;
        bool punchCond(float) const;
        bool hammerTransform(float) const;
        bool shotNTimes(float) const;


        //actions
        inline ret_e clearInbox(float) {inbox.clear(); return DONE_QUICKSTEP;}
        ret_e wait(float); 
        ret_e waitForHammer(float);
        ret_e firstTimeSetup(float);
        ret_e firstTimePrepare(float);
        ret_e missedCrashFx(float);
        ret_e lastSpotCrashFx(float);     
        ret_e gameOver(float);  
        ret_e weakSpotCrashFx(float);
        ret_e raiseDifficulty(float);
        ret_e updateSkybox(float);
        ret_e setupIdle(float);
        ret_e setupPunchWarning(float);
        ret_e punchWarning(float);
        ret_e setupDropArm(float);
        ret_e hitGround(float);
        ret_e setupSmokeWarning(float);
        ret_e earthQuake(float);
        ret_e setupSmokeDamage(float);
        ret_e duringSmoke(float);
        ret_e setupPunchWait(float);
        ret_e punchWait(float);
        ret_e coolSmoke(float);
        ret_e setupDamagedDelay(float);
        ret_e setupArmResisting(float);
        ret_e armResisting(float);
        ret_e dropCannon(float);
        ret_e setupRaiseArmSudden(float);
        ret_e openWeakSpot(float);
        ret_e raiseDamagedEndFx(float);
        ret_e setupRaiseArmSlow(float);
        ret_e raiseHealthyEndFx(float);
        ret_e setupHeat(float);
        ret_e heat(float);
        ret_e shoot(float);
        ret_e setupShootCooldown(float);
        ret_e resetNShots(float);
        ret_e setupSmoking(float);
        ret_e smoking(float);
        ret_e endSmoking(float);
        ret_e startWallOfSmoke(float);
        ret_e sendMinions(float);
        ret_e endWallOfSmoke(float);
        
    public:
        inline void init(){
            reset();
            inbox.clear();
            inbox.firstTime = true;
            punchTimer.reset();
            waitingForWeakSpot = false;
            stage=0;
            shotsFired=0;
        }
        inline void reset() { timer.reset();}
        inline void update(float elapsed) {
            punchTimer.count(elapsed);
        }
};

class CBoss {
    private:
        BossBt bt;

        struct marks_t {
            component::Transform cannonTop[3];
            component::Transform cannonBottom[3];
        } marks;

        static const unsigned MAX_SPINNERS = 64;
        static const unsigned MAX_SPAWNERS = 12;

        struct spinner_t {
            component::Handle e;
            float spin;
        } spinners[MAX_SPINNERS];
        unsigned nSpinners = 0;


    public:
        static const EntityListManager::key_t TAG = 0xBADDB055;
        static const EntityListManager::key_t HAMMER_TAG = 0xB055A288;
        static void initType();
    public:
        void update(float elapsed);
        void init();
        void reset();

        void receive(const MsgEarthquake& msg);
        void receive(const MsgPlayerDead& msg);
        void receive(const MsgCollisionEvent& msg);
        void receive(const MsgSetPlayer& msg);
		void receive(const MsgSetBichito& msg);
        void receive(const MsgWeakSpotBreak& msg);
        inline void receive(const MsgRevive& msg) {
            reset();
        }

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}

		inline int getAction() const {return bt.getCurrentAction();}

        void setMarks(
            component::Transform cannonsTop[3],
            component::Transform cannonsBottom[3]);

        void setHammer(component::Handle h, unsigned index);
        void setWeakSpot(component::Handle h, unsigned index);

        void addSpawn(component::Transform t, unsigned n) {
            assert (nSpinners < MAX_SPAWNERS);
            bt.getExecutor().minions[n].point = t;
        }

        inline void addSpinner(component::Handle h, float spin) {
            if (nSpinners >= MAX_SPINNERS) {utils::fatal("Too many boss spinners!");}
            spinners[nSpinners].e = h;
            spinners[nSpinners].spin = spin;
            nSpinners++;
        }

        inline void setHammerY(float y) {
            bt.getExecutor().hammerY = y;
        }
};

}
#endif