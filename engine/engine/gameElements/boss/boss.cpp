#include "mcv_platform.h"
#include "boss.h"

#include "app.h"
#include "smokePanel.h"
#include "../mobile.h"
#include "../flyingMobile.h"
#include "../props.h"
#include "../player/playerMov.h"
#include "../enemies/enemies.h"
#include "../enemies/flare.h"
#include "../enemies/melee.h"

using namespace DirectX;
using namespace utils;

#include "components/transform.h"
#include "components/color.h"
#include "components/detection.h"
#include "handles/prefab.h"
using namespace component;

#include "render/render_utils.h"
#include "render/renderManager.h"
#include "render/camera/component.h"
#include "render/illumination/ptLight.h"
using namespace render;

#include "weakSpot.h"

#include "animation/animation_max.h"
using namespace animation;

#include "Particles/ParticlesManager.h"
using namespace particles;

//DEBUG
#define ONLY_PUNCHES
//#define ONLY_MINIONS

namespace behavior {

BossBt::nodeId_t BossBt::rootNode = INVALID_NODE;
BossBt::container_t BossBt::nodes;
void BossBt::initType()
{
	/*                node-id              parent-id            node-type   condition        action            */
	BT_CREATEROOT    (BOSS,                                     SEQUENCE                                       );
	BT_CREATECHILD_C (EVENT,               BOSS,                PRIORITY,   hasEvents                          );
    BT_CREATECHILD_CA(FIRST_TIME,          EVENT,               SEQUENCE,   firstTime,       clearInbox        );
    BT_CREATECHILD_A (FIRST_TIME_SETUP,    FIRST_TIME,          LEAF,                        firstTimeSetup    );
    BT_CREATECHILD_A (FIRST_TIME_WAIT,     FIRST_TIME,          LEAF,                        wait              );
    BT_CREATECHILD_A (FIRST_TIME_PREPARE,  FIRST_TIME,          LEAF,                        firstTimePrepare  );
	BT_CREATECHILD_CA(MISSED_CRASH,        EVENT,               SEQUENCE,   missedCrash,     clearInbox        );
	BT_CREATECHILD_A (MISSED_CRASH_FX,     MISSED_CRASH,        LEAF,                        missedCrashFx     );
	BT_CREATECHILD_A (MISSED_CRASH_WAIT,   MISSED_CRASH,        LEAF,                        wait              );
	BT_CREATECHILD_CA(LAST_HIT_CRASH,      EVENT,               SEQUENCE,   crashedLastSpot, clearInbox        );
	BT_CREATECHILD_A (LAST_HIT_CRASH_FX,   LAST_HIT_CRASH,      LEAF,                        lastSpotCrashFx   );
	BT_CREATECHILD_A (GAME_OVER,           LAST_HIT_CRASH,      LEAF,                        gameOver          );
	BT_CREATECHILD_CA(WEAK_SPOT_CRASH,     EVENT,               SEQUENCE,   crashedWeakSpot, clearInbox        );
	BT_CREATECHILD_A (WEAK_SPOT_CRASH_FX,  WEAK_SPOT_CRASH,     LEAF,                        weakSpotCrashFx   );
	BT_CREATECHILD_A (WEAK_SPOT_CRASH_WAIT,WEAK_SPOT_CRASH,     LEAF,                        wait              );
	BT_CREATECHILD_A (RAISE_DIFFICULTY,    WEAK_SPOT_CRASH,     LEAF,                        raiseDifficulty   );
	BT_CREATECHILD_A (UPDATE_SKYBOX,       WEAK_SPOT_CRASH,     LEAF,                        updateSkybox      );
	BT_CREATECHILD_A (SETUP_IDLE,          BOSS,                CHAIN,                       setupIdle         );
	BT_CREATECHILD_A (IDLE,                SETUP_IDLE,          LEAF,                        wait              );
	BT_CREATECHILD   (ATTACK,              BOSS,                PRIORITY                                       );
#if !defined(ONLY_MINIONS)
	BT_CREATECHILD_C (PUNCH,               ATTACK,              SEQUENCE,   punchCond                          );
	BT_CREATECHILD_A (PREPARE_WARNING,     PUNCH,               CHAIN,                       setupPunchWarning );
	BT_CREATECHILD_A (WARNING,             PREPARE_WARNING,     LEAF,                        punchWarning      );
	BT_CREATECHILD_A (PREPARE_ARM_DROP,    PUNCH,               CHAIN,                       setupDropArm      );
    BT_CREATECHILD_A (ARM_DROP,            PREPARE_ARM_DROP,    LEAF,                        waitForHammer     );
    BT_CREATECHILD   (ARM_DOWN,            PUNCH,               SEQUENCE                                       );
    BT_CREATECHILD_A (HIT_GROUND,          ARM_DOWN,            LEAF,                        hitGround         );
#if !defined(ONLY_PUNCHES) || defined(ENABLE_SMOKE)
    BT_CREATECHILD_A (SMOKE_WARNING,       ARM_DOWN,            CHAIN,                       setupSmokeWarning );
    BT_CREATECHILD_A (EARTHQUAKE,          SMOKE_WARNING,       LEAF,                        earthQuake        );
    BT_CREATECHILD_A (SMOKE_START,         ARM_DOWN,            CHAIN,                       setupSmokeDamage  );
    BT_CREATECHILD_A (SMOKE_DURING,        SMOKE_START,         LEAF,                        duringSmoke       );
	BT_CREATECHILD_A (COOL_SMOKE,          ARM_DOWN,            LEAF,                        coolSmoke         );
#endif
    BT_CREATECHILD_A (SETUP_PUNCH_WAIT,    ARM_DOWN,            CHAIN,                       setupPunchWait    );
    BT_CREATECHILD_A (PUNCH_WAIT,          SETUP_PUNCH_WAIT,    LEAF,                        punchWait         );
	BT_CREATECHILD   (RAISE_ARM,           PUNCH,               PRIORITY                                       );
	BT_CREATECHILD_C (RAISE_TRANSFORMATED, RAISE_ARM,           SEQUENCE,   hammerTransform                    );
    BT_CREATECHILD_A (DAMAGED_DELAY_SETUP, RAISE_TRANSFORMATED, CHAIN,                       setupDamagedDelay );
	BT_CREATECHILD_A (DAMAGED_DELAY,       DAMAGED_DELAY_SETUP, LEAF,                        wait              );
    BT_CREATECHILD_A (ARM_RESISTING_SETUP, RAISE_TRANSFORMATED, CHAIN,                       setupArmResisting );
	BT_CREATECHILD_A (ARM_RESISTING,       ARM_RESISTING_SETUP, LEAF,                        armResisting      );
	BT_CREATECHILD_A (PREPARE_SUDDEN_RAISE,RAISE_TRANSFORMATED, CHAIN,                       setupRaiseArmSudden);
	BT_CREATECHILD_A (RAISE_ARM_SUDDENLY,  PREPARE_SUDDEN_RAISE,LEAF,                        waitForHammer     );
	BT_CREATECHILD_A (DROP_CANNON,         RAISE_TRANSFORMATED, LEAF,                        dropCannon        );
	BT_CREATECHILD_A (OPEN_WEAK_SPOT,      RAISE_TRANSFORMATED, LEAF,                        openWeakSpot      );
	BT_CREATECHILD_A (RAISE_DAMAGED_END_FX,RAISE_TRANSFORMATED, LEAF,                        raiseDamagedEndFx  );
	BT_CREATECHILD   (RAISE_UNTOUCHED,     RAISE_ARM,           SEQUENCE                                       );
	BT_CREATECHILD_A (PREPARE_SLOW_RAISE,  RAISE_UNTOUCHED,     CHAIN,                       setupRaiseArmSlow  );
	BT_CREATECHILD_A (RAISE_ARM_SLOWLY,    PREPARE_SLOW_RAISE,  LEAF,                        waitForHammer     );
	BT_CREATECHILD_A (RAISE_HEALTHY_END_FX,RAISE_UNTOUCHED,     LEAF,                        raiseHealthyEndFx  );
#endif
#ifndef ONLY_PUNCHES
	BT_CREATECHILD   (REGULAR_ATTACK,      ATTACK,              RANDOM                                         );
#if !defined(ONLY_MINIONS)
	BT_CREATECHILD   (FLAMES,              REGULAR_ATTACK,      SEQUENCE                                       );
	BT_CREATECHILD_A (SETUP_HEAT,          FLAMES,              CHAIN,                       setupHeat         );
	BT_CREATECHILD_A (HEAT,                SETUP_HEAT,          LEAF,                        heat              );
	BT_CREATECHILD_CA(SHOOT_N_TIMES,       FLAMES,              SEQUENCE_LOOP, shotNTimes,   resetNShots       );
	BT_CREATECHILD_A (SHOOT_SET,           SHOOT_N_TIMES,       CHAIN,                       shoot             );
	BT_CREATECHILD_A (SHOOT,               SHOOT_SET,           LEAF,                        wait              );
	BT_CREATECHILD_A (SHOOT_COOLDOWN_SET,  SHOOT_N_TIMES,       CHAIN,                       setupShootCooldown);
	BT_CREATECHILD_A (SHOOT_COOLDOWN,      SHOOT_COOLDOWN_SET,  LEAF,                        wait              );
	BT_CREATECHILD_A (RESET_N_SHOTS,       FLAMES,              LEAF,                        resetNShots       );
	BT_CREATECHILD_A (SETUP_SMOKING,       FLAMES,              CHAIN,                       setupSmoking      );
	BT_CREATECHILD_A (SMOKING,             SETUP_SMOKING,       LEAF,                        smoking           );
    BT_CREATECHILD_A (SMOKING_END,         FLAMES,              LEAF,                        endSmoking        );
#endif
	BT_CREATECHILD   (SPAWN_ENEMIES,       REGULAR_ATTACK,      SEQUENCE                                       );
	BT_CREATECHILD_A (WALL_OF_SMOKE,       SPAWN_ENEMIES,       LEAF,                        startWallOfSmoke  );
	BT_CREATECHILD_A (WAIT_FOR_MINIONS,    SPAWN_ENEMIES,       LEAF,                        wait              );
	BT_CREATECHILD_A (SEND_MINIONS,        SPAWN_ENEMIES,       LEAF,                        sendMinions       );
	BT_CREATECHILD_A (WAIT_FOR_SMOKE,      SPAWN_ENEMIES,       LEAF,                        waitForSmoke      );
    BT_CREATECHILD_A (END_WALL_OF_SMOKE,   SPAWN_ENEMIES,       LEAF,                        endWallOfSmoke    );

    
#if !defined(ONLY_MINIONS)
    BT_SET_WEIGHT(FLAMES, 7);
#endif
    BT_SET_WEIGHT(SPAWN_ENEMIES, 5);
#endif
}

}
namespace gameElements {

#define GRAVITY 3.5f*9.81f
#define TIME_MSG_TUTO 5.0f
const float BossBtExecutor::TIME_BEGIN = 3.f;
const float BossBtExecutor::TIME_CRASH_WAIT = TIME_BEGIN;
const float BossBtExecutor::TIME_SMOKING = 1.0f;
const float BossBtExecutor::TIME_ARM_RESIST_DELAY = 1.0f;
const float BossBtExecutor::TIME_ARM_RESISTING = 1.5f;
const float BossBtExecutor::TIME_SEND_MINIONS_BEFORE = 2.f;
const float BossBtExecutor::TIME_SEND_MINIONS_AFTER = 3.5f;
const float BossBtExecutor::HAMMER_DROP_ACCEL = GRAVITY+3.5f;
const float BossBtExecutor::HAMMER_DROP_SPEED = GRAVITY;
const float BossBtExecutor::HAMMER_SUDDEN_RAISE_TIME = 0.25f;
const float BossBtExecutor::HAMMER_SUDDEN_RAISE_ACCEL = 0.f;
const float BossBtExecutor::HAMMER_SLOW_RAISE_ACCEL = 0.f;
const float BossBtExecutor::HAMMER_SLOW_RAISE_TIME = 2.5f;
const float BossBtExecutor::HAMMER_WARNING_SHAKE_AMPLITUDE = 0.15f;
const float BossBtExecutor::HAMMER_WARNING_SHAKE_FREQUENCY = 10.f * M_2_PIf;
const float BossBtExecutor::HAMMER_EARTHQUAKE_SHAKE_AMPLITUDE = 0.075f;
const float BossBtExecutor::HAMMER_EARTHQUAKE_SHAKE_FREQUENCY = 9.f;
const float BossBtExecutor::CAM_SHAKE_AMOUNT = 0.04f;
const float BossBtExecutor::CAM_SHAKE_FREQ = 13.f;
const float BossBtExecutor::CAM_SHAKE_SMOKE_AMOUNT = 0.01f;
const float BossBtExecutor::CAM_SHAKE_SMOKE_FREQ = 16.f;
const float BossBtExecutor::CAM_SHAKE_SMOKE_AFTER_AMOUNT = 0.02f;
const float BossBtExecutor::CAM_SHAKE_SMOKE_AFTER_FREQ = 16.f;

const float BossBtExecutor::HAMMER_RESISTING_SHAKE_AMPLITUDE = 0.2f;
const float BossBtExecutor::HAMMER_RESISTING_SHAKE_FREQUENCY = 100.f;
const float BossBtExecutor::FLARE_SPEED = FlareBtExecutor::FLARE_SPEED;

/* DIFFICULTY SETTINGS */
const unsigned BossBtExecutor::N_SHOTS[N_STAGES] = {3,5,7};
const float BossBtExecutor::COOLDOWN_PUNCH_FIRST_TIME = 15.f;
const float BossBtExecutor::COOLDOWN_PUNCH[N_STAGES] = {10.f, 12.5f, 15.0f};
const unsigned BossBtExecutor::MIN_SPAWN[N_STAGES] = {2, 5, 8};
const float BossBtExecutor::TIME_HEATING[N_STAGES] = {4.0f, 4.0f, 4.0f};
const float BossBtExecutor::COOLDOWN_SHOOT[N_STAGES] = {1.5f, 1.2f, 0.9f};
const float BossBtExecutor::TIME_PUNCH_WARNING[N_STAGES] = {3.5f, 3.5f, 3.5f};
const float BossBtExecutor::TIME_DAMAGE_SMOKE[N_STAGES] = {2.5f, 2.5f, 2.5f};
const float BossBtExecutor::TIME_EARTHQUAKE[N_STAGES] = {2.5f, 2.0f, 1.5f};

const float BossBtExecutor::SKYBOX_BLEND[N_STAGES] = {0.75f, 0.85f, 1.0f};
const float BossBtExecutor::SKYBOX_BRIGHT[N_STAGES] = {0.30f, 0.37f, 0.45f};
const float BossBtExecutor::BOSS_LIGHT[N_STAGES] = {0.00f, 0.25f, 0.50f};


const std::function<float()> BossBtExecutor::TIME_PUNCH_WAIT[BossBtExecutor::N_STAGES] = {
    [=](){return inRange(3.00f,rand_normal(4.00f, 2.00f),5.50f);},
    [=](){return inRange(2.00f,rand_normal(3.25f, 1.50f),4.00f);},
    [=](){return inRange(1.00f,rand_normal(2.00f, 0.50f),2.50f);},
};
const std::function<float()> BossBtExecutor::TIME_IDLE[BossBtExecutor::N_STAGES] = {
    [=](){return inRange(2.00f,rand_normal(3.50f, 1.50f),5.00f);},
    [=](){return inRange(2.00f,rand_normal(2.75f, 1.50f),4.00f);},
    [=](){return inRange(0.75f,rand_normal(2.00f, 0.75f),2.00f);},
};

const std::function<bool()> BossBtExecutor::CHANCE_PUNCH[BossBtExecutor::N_STAGES] = {
    [=](){
        static bool firstPunch = true;
        bool ret = firstPunch;
        firstPunch = false;
        return firstPunch || chance(75,100);
    },
    [=](){return chance(50,100);},
    [=](){return chance(20,100);},
};

const std::function<bool()> BossBtExecutor::MINION_CHANCE[BossBtExecutor::N_STAGES] = {
    [=](){return chance(30,100);},
    [=](){return chance(50,100);},
    [=](){return chance(70,100);},
};

const std::function<bool()> BossBtExecutor::FLARE_CHANCE[BossBtExecutor::N_STAGES] = {
    [=](){return chance(75,1000);},
    [=](){return chance(150,1000);},
    [=](){return chance(200,1000);},
};

enum _smokePanels_e {
    _1 = 1<<0,  _3 = 1<<2,  _5 = 1<<4,  _7 = 1<<6,
    _2 = 1<<1,  _4 = 1<<3,  _6 = 1<<5,  _8 = 1<<7,
};

const std::vector<uint32_t> BossBtExecutor::smokePatterns[N_STAGES] = {
    //stage 0
    {_1|_2|_7|_8, _2|_4|_6|_8, _1|_3|_5|_7, _1|_2|_7|_8, _3|_4|_7|_8, _1|_2|_5|_6, },
    //Stage 1
    {_2|_4|_6|_8, _1|_3|_5|_7, _1|_4|_6|_7, _2|_3|_5|_8, _1|_4|_5|_8, _2|_3|_6|_7,
     _2|_4|_6|_8|_1|_7, _2|_4|_6|_8|_3|_5, _1|_3|_5|_7|_2|_8, _1|_3|_5|_7|_4|_6,
     _2|_4|_6|_8|_1|_7, _2|_4|_6|_8|_3|_5, _1|_3|_5|_7|_2|_8, _1|_3|_5|_7|_4|_6,
     _1|_3|_4|_6|_8, _1|_3|_5|_6|_8, _2|_4|_6|_5|_7, _2|_4|_3|_5|_7},
    //Stage 2
    {_2|_4|_6|_8|_3|_7, _2|_4|_6|_8|_1|_5, _1|_3|_5|_7|_2|_6, _1|_3|_5|_7|_4|_8,
     _2|_4|_6|_8|_3|_7, _2|_4|_6|_8|_1|_5, _1|_3|_5|_7|_2|_6, _1|_3|_5|_7|_4|_8,

     _2|_3|_5|_6|_7|_8, _1|_4|_5|_6|_7|_8, _1|_2|_4|_5|_6|_8, _1|_3|_5|_6|_7|_8,
     _2|_3|_4|_6|_7|_8, _1|_2|_3|_4|_5|_7, _1|_2|_4|_5|_6|_8, _1|_3|_4|_5|_6|_7,
     _2|_3|_4|_6|_7|_8, _1|_3|_4|_5|_7|_8, _1|_2|_4|_5|_6|_8, _1|_2|_3|_5|_6|_7,
     _1|_2|_3|_6|_7|_8, _2|_3|_4|_5|_7|_8, _1|_2|_3|_5|_6|_8, _1|_3|_4|_5|_6|_7,
     _2|_3|_4|_6|_7|_8, _1|_4|_5|_6|_7|_8, _1|_2|_4|_5|_7|_8, _1|_2|_3|_5|_7|_8,
     _1|_2|_3|_4|_6|_7, _1|_2|_3|_4|_5|_8, _1|_2|_3|_4|_6|_8, _2|_3|_4|_5|_6|_7,
     }
    };

void BossBtExecutor::spawnMinion(float elapsed)
{
    auto& m = minions[iSpawn];
        
    if ((!m.entity.isValid() || !m.entity.hasSon<CTransformable>() ||
    	((CTransformable*)m.entity.getSon<CTransformable>())->isTransformed())) {
          
    	if (MINION_CHANCE[stage]()) {
    		if(m.entity.isValid()) {
    			Entity* e = m.entity;
    			e->postMsg(MsgDeleteSelf());
    		}
    		m.entity = getManager<Entity>()->createObj();
    		Entity* e = m.entity;
    		if (FLARE_CHANCE[stage]()) {
    			PrefabManager::get().prefabricateComponents("components/flare", m.entity);
    		} else {
    			PrefabManager::get().prefabricateComponents("components/melee", m.entity);
    		}
    
    		//We up the enemy to avoid collisions with the ground
    		CTransform* transform = e->get<CTransform>();
    		transform->set(m.point);
    		transform->setPosition(transform->getPosition() + XMVectorSet(0, 0.45f, 0, 0));
    		
    		e->sendMsg(MsgSetPlayer(playerEntity));
    		e->sendMsg(MsgSetBichito(bichitoEntity));
    		
    		EntityListManager::get(CEnemy::TAG).add(e);
    		e->init();
    
    		nSpawn++;
    	}
    } else {
    	failed++;
	}
    iSpawn++;
    iSpawn %= ARRAYSIZE(minions);
}

bool BossBtExecutor::firstTime(float) const
{
    return inbox.firstTime;
}

bool BossBtExecutor::missedCrash(float) const
{
    return inbox.hitMissed;
}

bool BossBtExecutor::crashedLastSpot(float) const
{
    return inbox.hitWeakSpot && (stage == N_STAGES-1);
}

bool BossBtExecutor::crashedWeakSpot(float) const
{
    return inbox.hitWeakSpot;
}

bool BossBtExecutor::punchCond(float) const
{
    if (waitingForWeakSpot) {
        return false;
    } else {
#ifdef ONLY_PUNCHES
        return true;
#else
        bool allDestroyed = true;
        for(const auto& h : hammers) {allDestroyed &= (h.state == hammer_t::DESTROYED);}
        return (punchTimer.get() >= 0 && CHANCE_PUNCH[stage]())
            || punchTimer.get() >= COOLDOWN_PUNCH[stage];
#endif
    }
}

bool BossBtExecutor::hammerTransform(float) const
{
    CTransformable* t = hammers[currentHammer].hammer.getSon<CTransformable>();
	if (t->isTransformed()){
		CEmitter* emitter = hammers[currentHammer].hammer.getSon<CEmitter>();
		
		auto smoke = emitter->getKey("emitter_0");
		auto grava = emitter->getKey("emitter_1");
		auto smoke_green = emitter->getKey("emitter_2");
		auto leafs = emitter->getKey("emitter_3");

		ParticleUpdaterManager::get().setDeleteSelf(smoke);
		ParticleUpdaterManager::get().setDeleteSelf(grava);
		ParticleUpdaterManager::get().sendActive(leafs);
		ParticleUpdaterManager::get().sendActive(smoke_green);
		

	}
    return t != nullptr && t->isTransformed();
}

bool BossBtExecutor::shotNTimes(float) const
{
    return shotsFired < N_SHOTS[stage];
}

ret_e BossBtExecutor::wait(float elapsed)
{
	if (timer.count(elapsed) >= 0.0f) {
		timer.reset();
		return DONE;
	} else {
		return STAY;
	}
}

ret_e BossBtExecutor::firstTimeSetup(float elapsed)
{
    Entity* me(meEntity);
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* meTint = me->get<CTint>();
    meTint->set(Color::BROWN);
    CSelfIllumination* meSIll = me->get<CSelfIllumination>();
    meSIll->set(Color(0));
    RenderManager::updateKeys(me);

    for(auto& h : hammers) {
        Entity* hammer(h.hammer);
        CTint* tint = hammer->get<CTint>();
        tint->set(*meTint);
        CSelfIllumination* sIll = hammer->get<CSelfIllumination>();
        sIll->set(*meSIll);
        RenderManager::updateKeys(hammer);
    }
#endif
    timer.set(-TIME_BEGIN);
    return DONE;
}
ret_e BossBtExecutor::firstTimePrepare(float elapsed)
{
    punchTimer.set(-COOLDOWN_PUNCH_FIRST_TIME);
    return DONE;
}

ret_e BossBtExecutor::missedCrashFx(float elapsed)
{
    //TODO: particles etc
    timer.set(-TIME_CRASH_WAIT);
    return DONE;
}
  
ret_e BossBtExecutor::lastSpotCrashFx(float elapsed)
{
    //TODO: particles etc
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* meTint = me->get<CTint>();
    meTint->set(Color::GREEN);
    CSelfIllumination* meSIll = me->get<CSelfIllumination>();
    meSIll->set(Color(Color::LIME).setAf(0.33f));
    RenderManager::updateKeys(me);
    
    for(auto& h : hammers) {
        Entity* hammer(h.hammer);
        CTint* tint = hammer->get<CTint>();
        tint->set(*meTint);
        CSelfIllumination* sIll = hammer->get<CSelfIllumination>();
        sIll->set(*meSIll);
        RenderManager::updateKeys(hammer);
    }
#endif

    EntityListManager::get(CEnemy::TAG).broadcast(MsgTransform());
	EntityListManager::get(CSmokePanel::TAG).broadcast(MsgDeactivateSmoke());
	EntityListManager::get(CSmokePanel::TAG_ALWAYSHOT).broadcast(MsgDeactivateSmoke());
	EntityListManager::get(CWeakSpot::TAG).broadcast(MsgDeactivateSmoke());

    return DONE;
}

ret_e BossBtExecutor::gameOver(float elapsed)
{
    //TODO: YOU WIN!
	App &app = App::get();
	app.winGame = true;
    return STAY;
}

ret_e BossBtExecutor::weakSpotCrashFx(float elapsed)
{
	fmodUser::fmodUserClass::playSound("Glass_break", 1.0f, 0.0f);
    Entity* me(meEntity);
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* meTint = me->get<CTint>();
    meTint->set(Color::VIOLET);
    RenderManager::updateKeys(me);
#endif

    timer.set(-TIME_CRASH_WAIT);
	EntityListManager::get(CSmokePanel::TAG).broadcast(MsgDeactivateSmoke());

    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::raiseDifficulty(float elapsed)
{
    stage++;
    stage = inRange<uint8_t>(0,stage,N_STAGES-1); // for safety
    punchTimer.set(-COOLDOWN_PUNCH[stage]);
    waitingForWeakSpot = false;

    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    Entity* me(meEntity);
    CSelfIllumination* meSIll = me->get<CSelfIllumination>();
    meSIll->set(Color(Color::RED).setAf(0.2f*stage));
    RenderManager::updateKeys(me);

    for(auto& h : hammers) {
        Entity* hammer(h.hammer);
        CSelfIllumination* sIll = hammer->get<CSelfIllumination>();
        sIll->set(*meSIll);
        RenderManager::updateKeys(hammer);
    }
#endif
    timer.set(-TIME_CRASH_WAIT);
    return DONE_QUICKSTEP;
}

#define SKY_BLEND_DELTA 0.2f
#define SKY_BRIGHT_DELTA 0.2f
#define LIGHT_DELTA 0.1f

ret_e BossBtExecutor::updateSkybox(float elapsed)
{
    CPtLight* ptLight = meEntity.getSon<CPtLight>();

    float blend = SKYBOX_BLEND[stage];
    float bright = SKYBOX_BRIGHT[stage];
    float light = BOSS_LIGHT[stage];
    float& cblend = RenderConstsMirror::SkyBoxBlend;
    float& cbright = RenderConstsMirror::SkyBoxBright;
    float clight = ptLight->getIntensity();

    if (cblend < blend) {
        cblend += std::min(blend - cblend, elapsed*SKY_BLEND_DELTA);
    } else {
        cblend = blend;
    }
    
    if (cbright < bright) {
        cbright += std::min(bright - cbright, elapsed*SKY_BRIGHT_DELTA);
    } else {
        cbright = bright;
    }
    
    if (clight < light) {
        clight += std::min(light - clight, elapsed*LIGHT_DELTA);
    } else {
        clight = light;
    }
    ptLight->setIntensity(clight);

    RenderConstsMirror::update();
    return (cbright == bright && cblend == blend && light == clight) ? DONE : STAY;
}

ret_e BossBtExecutor::setupIdle(float elapsed)
{
    Entity* me(meEntity);
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* meTint = me->get<CTint>();
    meTint->set(Color::BROWN);
    RenderManager::updateKeys(me);
#endif

#ifdef ONLY_PUNCHES
    timer.reset();
#else
    timer.set(-TIME_IDLE[stage]());
#endif
    return DONE;
}

ret_e BossBtExecutor::setupPunchWarning(float elapsed)
{
    std::vector<int> v; 
    for (unsigned i=0;i<3;i++) {
        if (hammers[i].state != hammer_t::DESTROYED) {
            v.push_back(i);
        }
    }
    assert(!v.empty());
    currentHammer = v[die(int(v.size()))];
    
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::ORANGE);
    RenderManager::updateKeys(hammer);
#endif

    mobile->startShaking(
          HAMMER_WARNING_SHAKE_AMPLITUDE, HAMMER_WARNING_SHAKE_FREQUENCY,
          CMobile::SHAKE_SIN, xAxis_v, CMobile::REF_RELATIVE);
	fmodUser::fmodUserClass::playSound("boss_chargesmoke", 1.0f, 0.0f);
    //TODO: start fx
    timer.set(-TIME_PUNCH_WARNING[stage]);

    CTransformable* tr = hammer->get<CTransformable>();
    tr->setSelected(true);

    return DONE;
}

ret_e BossBtExecutor::punchWarning(float elapsed)
{
    //TODO: manage fx?
    return wait(elapsed);
}

ret_e BossBtExecutor::setupDropArm(float elapsed)
{
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::RED);
    RenderManager::updateKeys(hammer);
#endif

    CTransform* hammerTransf = hammer->get<CTransform>();
    currentHammerY = XMVectorGetY(hammerTransf->getPosition());
    mobile->startMoving(
        (hammerY-currentHammerY)*yAxis_v, CMobile::REF_RELATIVE,
        CMobile::MOVE_ACCELERATE, HAMMER_DROP_ACCEL, HAMMER_DROP_SPEED);

    return DONE;
}

ret_e BossBtExecutor::hitGround(float elapsed)
{
    //TODO: start FX
	//Info for the first stage
	if (stage == 0){
		((Entity*)playerEntity)->sendMsg(MsgPlayerInTuto(11));
		((Entity*)bichitoEntity)->sendMsg(MsgPlayerInTuto(11));
	}

	Entity* hammer = hammers[currentHammer].hammer;
	CEmitter *emitter = hammer->get<CEmitter>();

	auto smoke = emitter->getKey("emitter_0");
	auto grava = emitter->getKey("emitter_1");
	
	ParticleUpdaterManager::get().sendActive(smoke);
	ParticleUpdaterManager::get().sendActive(grava);

    return DONE;
}

ret_e BossBtExecutor::setupSmokeWarning(float elapsed)
{
    //TODO: start FX 
    const auto& patterns = smokePatterns[stage];
    const auto pattern = patterns[rand_uniform(int(patterns.size())-1)];
    EntityListManager::get(CSmokePanel::TAG).broadcast(MsgActivateSmokeWarning(pattern));
    timer.set(-TIME_EARTHQUAKE[stage]);
	fmodUser::fmodUserClass::playSound("boss_smokeout", 1.0f, 0.0f);
    
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();
    mobile->startShaking(
          HAMMER_EARTHQUAKE_SHAKE_AMPLITUDE, 
          HAMMER_EARTHQUAKE_SHAKE_FREQUENCY*M_2_PIf,
          CMobile::SHAKE_SIN, yAxis_v, CMobile::REF_RELATIVE);

    return DONE;
}

ret_e BossBtExecutor::earthQuake(float elapsed)
{
    CCamera* cam = App::get().getCamera().getSon<CCamera>();
    float f = M_PI_2f * -timer/TIME_EARTHQUAKE[stage];
    float shakeAmount = CAM_SHAKE_AMOUNT*std::cos(f);
    float shakeFreq = CAM_SHAKE_FREQ;
    cam->setShake(shakeAmount, shakeFreq);

    return wait(elapsed);
}

ret_e BossBtExecutor::setupSmokeDamage(float elapsed)
{
    //TODO: start boss FX
    CCamera* cam = App::get().getCamera().getSon<CCamera>();
    cam->setShake(0, 0);
    EntityListManager::get(CSmokePanel::TAG).broadcast(MsgActivateSmokeIfHot());
    timer.set(-TIME_DAMAGE_SMOKE[stage]);
    
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();
    mobile->startShaking(
          HAMMER_EARTHQUAKE_SHAKE_AMPLITUDE*.5f, 
          HAMMER_EARTHQUAKE_SHAKE_FREQUENCY*M_2_PIf*.75f,
          CMobile::SHAKE_SIN, yAxis_v, CMobile::REF_RELATIVE);

    return DONE;
}

ret_e BossBtExecutor::duringSmoke(float elapsed)
{
    //TODO: manage boss fx?
    return wait(elapsed);
}

ret_e BossBtExecutor::setupPunchWait(float elapsed)
{
	Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();
    mobile->stop();
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::YELLOW);
    RenderManager::updateKeys(hammer);
#endif

    CTransformable* hammerTransf = hammer->get<CTransformable>();
    hammerTransf->setInert(false);
    timer.set(-TIME_PUNCH_WAIT[stage]());
    return DONE;
}

ret_e BossBtExecutor::coolSmoke(float elapsed)
{
	EntityListManager::get(CSmokePanel::TAG).broadcast(MsgDeactivateSmoke());
	//Info for the first stage
	if (stage == 0){
		((Entity*)playerEntity)->sendMsg(MsgPlayerOutTuto());
		((Entity*)bichitoEntity)->sendMsg(MsgPlayerOutTuto());
	}
	
	Entity* hammer = hammers[currentHammer].hammer;

	CEmitter *emitter = hammer->get<CEmitter>();
	auto smoke = emitter->getKey("emitter_0");
	auto grava = emitter->getKey("emitter_1");

	ParticleUpdaterManager::get().sendInactive(smoke);
	ParticleUpdaterManager::get().sendInactive(grava);
	
    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::punchWait(float elapsed)
{
    return wait(elapsed);
}

ret_e BossBtExecutor::setupDamagedDelay(float elapsed)
{
    //TODO: start fx
    hammers[currentHammer].state = hammer_t::DESTROYED;
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::LIME);
    RenderManager::updateKeys(hammer);
#endif

    timer.set(-TIME_ARM_RESIST_DELAY);
    return DONE;
}

ret_e BossBtExecutor::setupArmResisting(float elapsed)
{
    //TODO: start fx
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::GREEN);
    RenderManager::updateKeys(hammer);
#endif

    mobile->startShaking(
          HAMMER_RESISTING_SHAKE_AMPLITUDE, HAMMER_RESISTING_SHAKE_FREQUENCY,
          CMobile::SHAKE_SIN, xAxis_v, CMobile::REF_RELATIVE);
    timer.set(-TIME_ARM_RESISTING);
	fmodUser::fmodUserClass::playSound("boss_damage", 1.0f, 0.0f);
    return DONE;
}

ret_e BossBtExecutor::armResisting(float elapsed)
{
    //TODO: manage fx?
    return wait(elapsed);
}

ret_e BossBtExecutor::dropCannon(float elapsed)
{
	//Info for the first stage
	if (stage == 0 && !tutoCannon){
		tutoCannon = true;
		((Entity*)playerEntity)->sendMsg(MsgPlayerInTuto(12));
		((Entity*)bichitoEntity)->sendMsg(MsgPlayerInTuto(12));
		tutoAlertTimer.reset();
	}
	
	//TODO: export a travelling animation in MAX
    Entity* cannon = hammers[currentHammer].cannon;
    /*CFlyingMobile* fm = cannon->get<CFlyingMobile>();
	
    assert(fm != nullptr);
    
	fm->setParameters(hammers[currentHammer].cannonMark, 15.f, deg2rad(60), true);*/
	CMaxAnim *anim = cannon->get<CMaxAnim>();
//	anim->setTiming(1.0);
	anim->setPlay(true);
	fmodUser::fmodUserClass::playSound("boss_cannonout", 1.0f, 0.0f);
    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::setupRaiseArmSudden(float elapsed)
{
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::MOUNTAIN_GREEN);
    RenderManager::updateKeys(hammer);
#endif

    mobile->startMoving(
        (currentHammerY-hammerY)*yAxis_v, CMobile::REF_RELATIVE,
        CMobile::MOVE_ACCELERATE,
        HAMMER_SUDDEN_RAISE_ACCEL,
        (currentHammerY-hammerY)/HAMMER_SUDDEN_RAISE_TIME );


	CEmitter* emitter = hammers[currentHammer].hammer.getSon<CEmitter>();

	auto smoke_green = emitter->getKey("emitter_2");
	auto leafs = emitter->getKey("emitter_3");

	ParticleUpdaterManager::get().setDeleteSelf(smoke_green);
	ParticleUpdaterManager::get().setDeleteSelf(leafs);

    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::waitForHammer(float elapsed)
{
    CMobile* mobile = hammers[currentHammer].hammer.getSon<CMobile>();
    if(mobile->isComplete()) {
        mobile->stop();
        return DONE;
    } else {
        return STAY;
    }
}

ret_e BossBtExecutor::openWeakSpot(float elapsed)
{
    CWeakSpot* weakSpot = hammers[currentHammer].weakSpot.getSon<CWeakSpot>();
    assert(weakSpot != nullptr);
    weakSpot->open(meEntity);
    waitingForWeakSpot = true;
    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::raiseDamagedEndFx(float elapsed)
{
    //TODO: start fx
    punchTimer.set(-COOLDOWN_PUNCH[stage]/2);
    return DONE;
}

ret_e BossBtExecutor::setupRaiseArmSlow(float elapsed)
{
    Entity* hammer = hammers[currentHammer].hammer;
    CMobile* mobile = hammer->get<CMobile>();
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* hammerTint = hammer->get<CTint>();
    hammerTint->set(Color::BROWN);
    RenderManager::updateKeys(hammer);
#endif

    CTransformable* hammerTransf = hammer->get<CTransformable>();
    hammerTransf->setInert(true);
    mobile->startMoving(
        (currentHammerY-hammerY)*yAxis_v, CMobile::REF_RELATIVE,
        CMobile::MOVE_ACCELERATE,
        HAMMER_SLOW_RAISE_ACCEL,
        (currentHammerY-hammerY)/HAMMER_SLOW_RAISE_TIME );
    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::raiseHealthyEndFx(float elapsed)
{
    //TODO: start FX
    punchTimer.set(-COOLDOWN_PUNCH[stage]/2);
    
	Entity* hammer = hammers[currentHammer].hammer;
    CTransformable* tr = hammer->get<CTransformable>();
    tr->setSelected(false);

    return DONE;
}

ret_e BossBtExecutor::setupHeat(float elapsed)
{
    Entity* me(meEntity);
    
#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* tint = me->get<CTint>();
    tint->set(Color::ORANGE);
    RenderManager::updateKeys(meEntity);
#endif

    //TODO: start fx
	fmodUser::fmodUserClass::playSound("boss_chargecanon", 2.0f, 0.0f);
    timer.set(-TIME_HEATING[stage]);
    return DONE;
}

ret_e BossBtExecutor::heat(float elapsed)
{
    //TODO: manage fx
	return wait(elapsed);
}

void BossBtExecutor::shootFlare()
{
	Entity* me = meEntity;
	CTransform* meT = me->get<CTransform>();
	Entity* shot = PrefabManager::get().prefabricate("boss/flare-shot");
	shot->init();
	CTransform* shotT = shot->get<CTransform>();
	CTransform* playerT = ((Entity*)playerEntity)->get<CTransform>();
	XMVECTOR origin = meT->getPosition() + meT->getFront()*10 - meT->getUp() - meT->getLeft() * 0.5f;
	float randomSpeed = rand_uniform(2.0f, 1.0f);
	XMVECTOR vel = XMVector3Normalize((playerT->getPosition() + playerT->getPivot()) - origin) * FLARE_SPEED * randomSpeed;
	CFlareShot* flareShot = shot->get<CFlareShot>();
	flareShot->setup(origin, vel, XMQuaternionIdentity());
	fmodUser::fmodUserClass::playSound("boss_shootcanon", 1.0f, 0.0f);
}

ret_e BossBtExecutor::shoot(float elapsed)
{
    shotsFired++;
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* tint = me->get<CTint>();
    tint->set(Color::RED);
    RenderManager::updateKeys(meEntity);
#endif

    timer.set(-COOLDOWN_SHOOT[stage]*1/4);
    shootFlare();
    return DONE;
}

ret_e BossBtExecutor::setupShootCooldown(float elapsed)
{
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* tint = me->get<CTint>();
    tint->set(Color::YELLOW);
    RenderManager::updateKeys(meEntity);
#endif

    timer.set(-COOLDOWN_SHOOT[stage]*3/4);
    return DONE;
}

ret_e BossBtExecutor::resetNShots(float elapsed)
{
    shotsFired = 0;
    return DONE_QUICKSTEP;
}

ret_e BossBtExecutor::setupSmoking(float elapsed)
{
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* tint = me->get<CTint>();
    tint->set(Color::DARK_GRAY);
    RenderManager::updateKeys(meEntity);
#endif

    //TODO: start fx
    timer.set(-TIME_SMOKING);
    return DONE;
}

ret_e BossBtExecutor::smoking(float elapsed)
{
    //TODO: manage fx?
    return wait(elapsed);
}

ret_e BossBtExecutor::endSmoking(float elapsed)
{
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* tint = me->get<CTint>();
    tint->set(Color::BROWN);
    RenderManager::updateKeys(meEntity);
#endif

	Entity* hammer = hammers[currentHammer].hammer;
	CEmitter *emitter = hammer->get<CEmitter>();

	auto smoke = emitter->getKey("emitter_0");
	auto grava = emitter->getKey("emitter_1");

	if (ParticleUpdaterManager::get().isExist(smoke))
			ParticleUpdaterManager::get().sendInactive(smoke);

	if (ParticleUpdaterManager::get().isExist(grava))
			ParticleUpdaterManager::get().sendInactive(grava);


    return DONE;
}

ret_e BossBtExecutor::startWallOfSmoke(float elapsed)
{
    //TODO: start fx
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* meTint = me->get<CTint>();
    meTint->set(Color::BURGUNDY);
    RenderManager::updateKeys(me);
#endif

    timer.set(-TIME_SEND_MINIONS_BEFORE);
	//fmodUser::fmodUserClass::playSound("boss_invokeenemies", 1.0f, 0.0f);

    nSpawn = 0;
    failed = 0;
    iSpawn = 0;
    
    CCamera* cam = App::get().getCamera().getSon<CCamera>();
    cam->setShake(CAM_SHAKE_SMOKE_AMOUNT, CAM_SHAKE_SMOKE_FREQ);

    return DONE;
}

ret_e BossBtExecutor::sendMinions(float elapsed)
{
	if (nSpawn < MIN_SPAWN[stage] && failed < ARRAYSIZE(minions)) {

        #if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
            CTint* meTint = me->get<CTint>();
            meTint->set(Color::CERULEAN);
            RenderManager::updateKeys(me);
        #endif
            
        CCamera* cam = App::get().getCamera().getSon<CCamera>();
        cam->setShake(CAM_SHAKE_SMOKE_AFTER_AMOUNT, CAM_SHAKE_SMOKE_AFTER_FREQ);
		spawnMinion(elapsed);
		return STAY;
	} else {
		timer.set(-TIME_SEND_MINIONS_AFTER);
        EntityListManager::get(CEnemy::TAG).broadcast(MsgSetPlayer(playerEntity));
	    EntityListManager::get(CEnemy::TAG).broadcast(MsgSetBichito(bichitoEntity));
        return DONE;
    }
}


ret_e BossBtExecutor::waitForSmoke(float elapsed)
{
    CCamera* cam = App::get().getCamera().getSon<CCamera>();
    float f = std::cos(1 - M_PI_2f * -timer/TIME_SEND_MINIONS_AFTER);
    float shakeAmount = CAM_SHAKE_SMOKE_AFTER_AMOUNT*f*f*f;
    float shakeFreq = CAM_SHAKE_SMOKE_AFTER_FREQ*f*f*f;
    cam->setShake(shakeAmount, shakeFreq);

    return wait(elapsed);
}

ret_e BossBtExecutor::endWallOfSmoke(float elapsed)
{
    //TODO: start fx
    Entity* me(meEntity);

#if defined(_DEBUG) && defined(DEBUG_BOSS_TINTING)
    CTint* meTint = me->get<CTint>();
    meTint->set(Color::BROWN);
    RenderManager::updateKeys(me);
#endif
    
    CCamera* cam = App::get().getCamera().getSon<CCamera>();
    cam->setShake(0, 0);

    return DONE;
}


void CBoss::setMarks(
    component::Transform cannonsTop[3],
    component::Transform cannonsBottom[3])
{
    for(unsigned i=0; i<3; i++){
        marks.cannonTop[i] = cannonsTop[i];
        marks.cannonBottom[i] = cannonsBottom[i];
    }
}

void CBoss::reset()
{
    auto& bte(bt.getExecutor());
    for(auto& system : bte.hammers) {
        {
            system.cannon.destroy();
            system.cannon = Handle();
        }
        {
            //Reset hammer

            Entity* e_prev = system.hammer;
            Entity* e = getManager<Entity>()->createObj();
            PrefabManager::get().prefabricateComponents("boss/hammer", e);
            
			CTransform* t_prev = e_prev->get<CTransform>();
            CTransform* t = e->get<CTransform>();
            t->set(*t_prev);
            
			CMesh* m_prev = e_prev->get<CMesh>();
            CMesh* m = e->get<CMesh>();
			
			*m = *m_prev;
            system.hammer = e;
            e->init();
            e_prev->destroy();
        }
        {
            //Reset Weak Spot
            Entity* e_prev(system.weakSpot);
            Entity* e = getManager<Entity>()->createObj();
            PrefabManager::get().prefabricateComponents("boss/weak-spot", e);
            CTransform* t_prev = e_prev->get<CTransform>();
            CTransform* t = e->get<CTransform>();
            t->set(*t_prev);
            CMesh* m_prev = e_prev->get<CMesh>();
            CMesh* m = e->get<CMesh>();
            *m = *m_prev;
            CStaticBody* b_prev = e_prev->get<CStaticBody>();
            CStaticBody* b = e->get<CStaticBody>();
            b->setTriangleMesh(b_prev->getShapeDesc().triangleMesh.mesh);
            CWeakSpot* w_prev = e_prev->get<CWeakSpot>();
            CWeakSpot* w = e->get<CWeakSpot>();
            if (w->isBroken()) {
                m->switchMaterials();
            }
            system.weakSpot = e;
            e->init();
            e_prev->destroy();

            EntityListManager::get(CWeakSpot::TAG).add(e);
        }
    }
    for(auto& m: bte.minions) {
        if(m.entity.isValid()) {
            m.entity.destroy();
        }
    }
    init();
}

void CBoss::init()
{
    auto& bte(bt.getExecutor());
    bte.meEntity = Handle(this).getOwner();
    for(unsigned i=0; i<3; i++) {
        auto& system = bte.hammers[i];

        system.cannon   = PrefabManager::get().prefabricate("boss/cannon");
        system.state = BossBtExecutor::hammer_t::SAFE;

        {
            Entity* cannon = system.cannon;
            CTransform* cannonT = cannon->get<CTransform>();
            /*cannonT->set(marks.cannonTop[i]);
            system.cannonMark = marks.cannonBottom[i];*/
			CMaxAnim *max = cannon->get<CMaxAnim>();
			max->loadCannon(i+1);

            cannon->init(); 
        }
    }
    bt.init();
}

void CBoss::setHammer(component::Handle h, unsigned index)
{
    assert(index < 3);
    auto& bte = bt.getExecutor();
    bte.hammers[index].hammer = h;

    Entity* hammer = h;
    CMobile* mobile = hammer->get<CMobile>();
    mobile->enslave(bte.hammers[index].cannon);
}

void CBoss::setWeakSpot(component::Handle h, unsigned index)
{
    assert(index < 3);
    bt.getExecutor().hammers[index].weakSpot = h;
}

void CBoss::update(float elapsed)
{
    Entity* me = Handle(this).getOwner();
	bt.update(elapsed);
#ifdef _DEBUG
    if (bt.hasActionChanged()) {
        //dbg("%s changed to action %s\n", me->getName().c_str(),
        //    BossBtExecutor::DEBUG_NAME(BossBtExecutor::nodeId_e(bt.getCurrentAction())).c_str());
    }
#endif
    CTransform* playerT = bt.getExecutor().playerEntity.getSon<CTransform>();
    CTransform* meT = me->get<CTransform>();
    auto action = bt.getCurrentAction();
    if ((action & BossBtExecutor::COD_DONT_ALIGN)==0) {
        float rotSpeed =
            (action & BossBtExecutor::COD_ALIGN_FASTER)!=0 ?
            deg2rad(35) : deg2rad(15);
        XMVECTOR q = alignXZ(meT, playerT->getPosition(), rotSpeed*elapsed);
        for (unsigned i=0; i<nSpinners; ++i) { 
            XMVECTOR axis;
            float angle;
            XMQuaternionToAxisAngle(&axis, &angle, q);
            angle *= testIsBehind(axis, meT->getUp()) ? -1.f : 1.f;
            Entity* e = spinners[i].e;
            CTransform* t = e->get<CTransform>();
            t->applyRotation(XMQuaternionRotationAxis(t->getUp(), angle*spinners[i].spin));
        }
    }
	//Info for the first stage
	if (bt.getExecutor().tutoCannon){
		if (bt.getExecutor().tutoAlertTimer.count(elapsed) >= TIME_MSG_TUTO){
			if (bt.getExecutor().stage == 0){
				((Entity*)bt.getExecutor().playerEntity)->sendMsg(MsgPlayerOutTuto());
				((Entity*)bt.getExecutor().bichitoEntity)->sendMsg(MsgPlayerOutTuto());
				bt.getExecutor().tutoCannon = false;
			}
		}
	}
    TransformableFSMExecutor::updateSpecialHighlights(elapsed,
        (action & BossBtExecutor::COD_HIGHLIGHT) != 0);
}

inline void CBoss::receive(const MsgEarthquake& msg)
{
    bt.getExecutor().inbox.earthquake = true;
    bt.reset();
}

inline void CBoss::receive(const MsgPlayerDead& msg)
{
    bt.getExecutor().inbox.playerDead = true;
    bt.reset();
}

inline void CBoss::receive(const MsgCollisionEvent& msg)
{
    if(msg.entity.hasSon<CPlayerMov>()) {
        bt.getExecutor().inbox.hitMissed = true;
        bt.reset();
    }
}

inline void CBoss::receive(const MsgSetPlayer& msg)
{
    bt.getExecutor().playerEntity = msg.playerEntity;
    bt.reset();
}

inline void CBoss::receive(const MsgSetBichito& msg)
{
    bt.getExecutor().bichitoEntity = msg.bichitoEntity;
    bt.reset();
}

inline void CBoss::receive(const MsgWeakSpotBreak& msg)
{
    bt.getExecutor().inbox.hitWeakSpot = true;
    bt.reset();
}

void CBoss::initType()
{
	BossBt::initType();
 	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgSetPlayer,        receive);
	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgSetBichito,       receive);
	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgEarthquake,       receive);
	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgPlayerDead,       receive);
	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgCollisionEvent,   receive);
	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgWeakSpotBreak,    receive);
	SUBSCRIBE_MSG_TO_MEMBER(CBoss, MsgRevive,           receive);
}

#ifdef _DEBUG
std::string BossBtExecutor::DEBUG_NAME(nodeId_e e) {
    static std::map<nodeId_e, std::string> map = {
        {BOSS                , TXT(BOSS                )},
        {EVENT               , TXT(EVENT               )},
        {MISSED_CRASH        , TXT(MISSED_CRASH        )},
        {MISSED_CRASH_FX     , TXT(MISSED_CRASH_FX     )},
        {LAST_HIT_CRASH      , TXT(LAST_HIT_CRASH      )},
        {LAST_HIT_CRASH_FX   , TXT(LAST_HIT_CRASH_FX   )},
        {GAME_OVER           , TXT(GAME_OVER           )},
        {WEAK_SPOT_CRASH     , TXT(WEAK_SPOT_CRASH     )},
        {WEAK_SPOT_CRASH_FX  , TXT(WEAK_SPOT_CRASH_FX  )},
        {RAISE_DIFFICULTY    , TXT(RAISE_DIFFICULTY    )},
        {SETUP_IDLE          , TXT(SETUP_IDLE          )},
        {IDLE                , TXT(IDLE                )},
        {ATTACK              , TXT(ATTACK              )},
        {PUNCH               , TXT(PUNCH               )},
        {PREPARE_WARNING     , TXT(PREPARE_WARNING     )},
        {WARNING             , TXT(WARNING             )},
        {PREPARE_ARM_DROP    , TXT(PREPARE_ARM_DROP    )},
        {ARM_DROP            , TXT(ARM_DROP            )},
        {ARM_DOWN            , TXT(ARM_DOWN            )},
        {HIT_GROUND          , TXT(HIT_GROUND          )},
        {SMOKE_WARNING       , TXT(SMOKE_WARNING       )},
        {EARTHQUAKE          , TXT(EARTHQUAKE          )},
        {SMOKE_START         , TXT(SMOKE_START         )},
        {SMOKE_DURING        , TXT(SMOKE_DURING        )},
        {SETUP_PUNCH_WAIT    , TXT(SETUP_PUNCH_WAIT    )},
        {PUNCH_WAIT          , TXT(PUNCH_WAIT          )},
        {RAISE_ARM           , TXT(RAISE_ARM           )},
        {RAISE_TRANSFORMATED , TXT(RAISE_TRANSFORMATED )},
        {ARM_RESISTING       , TXT(ARM_RESISTING       )},
        {DROP_CANNON         , TXT(DROP_CANNON         )},
        {PREPARE_SUDDEN_RAISE, TXT(PREPARE_SUDDEN_RAISE)},
        {RAISE_ARM_SUDDENLY  , TXT(RAISE_ARM_SUDDENLY  )},
        {OPEN_WEAK_SPOT      , TXT(OPEN_WEAK_SPOT      )},
        {RAISE_DAMAGED_END_FX, TXT(RAISE_DAMAGED_END_FX)},
        {RAISE_UNTOUCHED     , TXT(RAISE_UNTOUCHED     )},
        {PREPARE_SLOW_RAISE  , TXT(PREPARE_SLOW_RAISE  )},
        {RAISE_ARM_SLOWLY    , TXT(RAISE_ARM_SLOWLY    )},
        {RAISE_HEALTHY_END_FX, TXT(RAISE_HEALTHY_END_FX)},
        {REGULAR_ATTACK      , TXT(REGULAR_ATTACK      )},
        {FLAMES              , TXT(FLAMES              )},
        {SETUP_HEAT          , TXT(SETUP_HEAT          )},
        {HEAT                , TXT(HEAT                )},
        {SHOOT_N_TIMES       , TXT(SHOOT_N_TIMES       )},
        {SHOOT_SET           , TXT(SHOOT_SET           )},
        {SHOOT               , TXT(SHOOT               )},
        {SHOOT_COOLDOWN_SET  , TXT(SHOOT_COOLDOWN_SET  )},
        {SHOOT_COOLDOWN      , TXT(SHOOT_COOLDOWN      )},
        {RESET_N_SHOTS       , TXT(RESET_N_SHOTS       )},
        {SETUP_SMOKING       , TXT(SETUP_SMOKING       )},
        {SMOKING             , TXT(SMOKING             )},
        {SMOKING_END         , TXT(SMOKING_END         )},
        {SPAWN_ENEMIES       , TXT(SPAWN_ENEMIES       )},
        {WALL_OF_SMOKE       , TXT(WALL_OF_SMOKE       )},
        {WAIT_FOR_MINIONS    , TXT(WAIT_FOR_MINIONS    )},
        {SEND_MINIONS        , TXT(SEND_MINIONS        )},
        {WAIT_FOR_SMOKE      , TXT(WAIT_FOR_SMOKE      )},
        {END_WALL_OF_SMOKE   , TXT(END_WALL_OF_SMOKE   )},
        {FIRST_TIME          , TXT(FIRST_TIME          )},
        {FIRST_TIME_SETUP    , TXT(FIRST_TIME_SETUP    )},
        {FIRST_TIME_WAIT     , TXT(FIRST_TIME_WAIT     )},
        {FIRST_TIME_PREPARE  , TXT(FIRST_TIME_PREPARE  )},
        {DAMAGED_DELAY_SETUP , TXT(DAMAGED_DELAY_SETUP )}, 
        {DAMAGED_DELAY       , TXT(DAMAGED_DELAY       )},            
        {ARM_RESISTING_SETUP , TXT(ARM_RESISTING_SETUP )}, 
        {COOL_SMOKE          , TXT(COOL_SMOKE          )},
        {WEAK_SPOT_CRASH_WAIT, TXT(WEAK_SPOT_CRASH_WAIT)},
        {MISSED_CRASH_WAIT   , TXT(MISSED_CRASH_WAIT   )},
    };
    return atOrDefault(map, e, "<undefined>");
}
#endif

}
