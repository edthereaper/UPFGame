#include "mcv_platform.h"
#include "components.h"
#include "handles/entity.h"                                     //Entity, CName
#include "Transform.h"                                          //CTransform
#include "whitebox.h"                                           //CWhiteBox
#include "color.h"                                              //CTint, CSelfIllumination
#include "AABB.h"                                               //CAABB
#include "slot.h"                                               //CSlots
#include "animation/animationPlugger.h"                         //CAnimationPlugger
#include "detection.h"                                          //CDetection
#include "render/components.h"                                  //(many)
#include "gameElements/module.h"                                //(many)
#include "animation/components.h"                               //(many)
#include "render/texture/cskybox.h"                             //CSkyBox
#include "logic/trigger.h"                                      //CScriptTrigger
#include "logic/timer.h"                                        //CTimer
#include "PhysX_USER/pxcomponents.h"                            //(Many)
#include "gameElements/player/cannonPath.h"						//CCannonPath
#include "gameElements/liana.h"									//liana
#include "level/level.h"						                //CLevelData
#include "Particles/ParticleSystem.h"						    //ParticleSystem
#include "Particles/Emitter.h"									//ParticleSystem
#include "Lua_user/lua_component.h"							    //lua_component
#include "Cinematic/animation_camera.h"							//camera animation
#include "animation/animation_max.h"							//animation max
#include "gameElements/ambientSound.h"
#include "gameElements/PaintManager.h"                          //CPaintGroup

using namespace render;
using namespace animation;
using namespace gameElements;
using namespace logic;
using namespace physX_user;
using namespace level;
using namespace particles;
using namespace lua_user;
using namespace cinematic;

#ifdef _TEST
#include "handles/testHandles.h"
#endif

namespace component {

#define N_ENTITIES 2048

DECLARE_OBJECT_MANAGER(Entity, Entity,                              N_ENTITIES);
DECLARE_OBJECT_MANAGER(CName, Name,                                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CTransform, Transform,                       N_ENTITIES);
DECLARE_OBJECT_MANAGER(CRestore, Restore,                           N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCamera, Camera,                             N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCulling, Culling,                           CCulling::MAX_CULLERS);
DECLARE_OBJECT_MANAGER(CCullingCube, CullingCube,                   CCulling::MAX_CULLERS);
DECLARE_OBJECT_MANAGER(CTint, Tint,                                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CSelfIllumination, SelfIllumination,         N_ENTITIES);
DECLARE_OBJECT_MANAGER(CSlots, Slots,                               1);
DECLARE_OBJECT_MANAGER(CDetection, Detection,                       N_ENTITIES);
DECLARE_OBJECT_MANAGER(CAABB, AABB,                                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCullingAABB, CullingAABB,                   N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCullingAABBSpecial, CullingAABBSpecial,     N_ENTITIES);
DECLARE_OBJECT_MANAGER(CInstancedMesh, InstancedMesh,               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CMesh, Mesh,                                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CTransformable, Transformable,               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CThrowsPickups, ThrowsPickups,               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CProp, Prop,                                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CTrampoline, Trampoline,                     N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCannon, Cannon,                             N_ENTITIES);
DECLARE_OBJECT_MANAGER(CDestructible, Destructible,                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CDestructibleRestorer, DestructibleRestorer, N_ENTITIES);
DECLARE_OBJECT_MANAGER(CLiana, Liana,                               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCreep, Creep,                               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CEnemy, Enemy,                               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CFlare, Flare,                               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CFlareShot, FlareShot,                       N_ENTITIES);
DECLARE_OBJECT_MANAGER(CMelee, Melee,                               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CBoss, Boss,                                 1);
DECLARE_OBJECT_MANAGER(CSmokePanel, SmokePanel,                     N_ENTITIES);
DECLARE_OBJECT_MANAGER(CWeakSpot, WeakSpot,                         N_ENTITIES);
DECLARE_OBJECT_MANAGER(CMobile, Mobile,                             N_ENTITIES);
DECLARE_OBJECT_MANAGER(CFlyingMobile, FlyingMobile,                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CPlayerMov, PlayerMov,                       1);
DECLARE_OBJECT_MANAGER(CPlayerAttack, PlayerAttack,                 1);
DECLARE_OBJECT_MANAGER(CPlayerStats, PlayerStats,                   1);
DECLARE_OBJECT_MANAGER(CPickup, Pickup,                             N_ENTITIES);
DECLARE_OBJECT_MANAGER(CBullet, Bullet,                             N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCannonPath, CannonPath,						N_ENTITIES);
DECLARE_OBJECT_MANAGER(CAnimationPlugger, AnimationPlugger,         N_ENTITIES);
DECLARE_OBJECT_MANAGER(CAnimationSounds, AnimationSounds,			N_ENTITIES);
DECLARE_OBJECT_MANAGER(CScriptTrigger, ScriptTrigger,               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CSpatialIndex, SpatialIndex,                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CTimer, Timer,                               N_ENTITIES);
DECLARE_OBJECT_MANAGER(CRigidBody, RigidBody,                       N_ENTITIES);
DECLARE_OBJECT_MANAGER(CStaticBody, StaticBody,                     N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCharacterController, CharacterController,   N_ENTITIES);
DECLARE_OBJECT_MANAGER(CTrigger, Trigger,							N_ENTITIES);
DECLARE_OBJECT_MANAGER(CExtraShapes, ExtraShapes, 					N_ENTITIES);
DECLARE_OBJECT_MANAGER(CSkeleton, Skeleton,							N_ENTITIES);
DECLARE_OBJECT_MANAGER(CBoneLookAt, BoneLookAt,						N_ENTITIES);
DECLARE_OBJECT_MANAGER(CArmPoint, ArmPoint,						    N_ENTITIES);
DECLARE_OBJECT_MANAGER(CPtLight, PtLight,							N_ENTITIES);
DECLARE_OBJECT_MANAGER(CDirLight, DirLight,						    N_ENTITIES);
DECLARE_OBJECT_MANAGER(CShadow, Shadow,							    N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCubeShadow, CubeShadow,						N_ENTITIES);
DECLARE_OBJECT_MANAGER(CSkyBox, SkyBox,							    1);
DECLARE_OBJECT_MANAGER(CCheckPoint, CheckPoint,                     N_ENTITIES);
DECLARE_OBJECT_MANAGER(CLevelData, LevelData,                       N_ENTITIES);
DECLARE_OBJECT_MANAGER(CWhiteBox, WhiteBox,                         N_ENTITIES);
DECLARE_OBJECT_MANAGER(CKnife, Knife,							    N_ENTITIES);
DECLARE_OBJECT_MANAGER(CParticleSystem, particle_system,			N_ENTITIES);
DECLARE_OBJECT_MANAGER(CSmokeTower, SmokeTower,                     1);
DECLARE_OBJECT_MANAGER(CLua, Lua,									N_ENTITIES);
DECLARE_OBJECT_MANAGER(CBichito, Bichito,							1);
DECLARE_OBJECT_MANAGER(CVolPtLight, VolPtLight,						N_ENTITIES);
DECLARE_OBJECT_MANAGER(CCameraAnim, CameraAnim,						N_ENTITIES);
DECLARE_OBJECT_MANAGER(CMaxAnim, MaxAnim,						    N_ENTITIES);
DECLARE_OBJECT_MANAGER(CEmitter, Emitter,							N_ENTITIES);
DECLARE_OBJECT_MANAGER(CMist, Mist,                                 N_ENTITIES);
DECLARE_OBJECT_MANAGER(CAmbientSound, AmbientSound,					N_ENTITIES);
DECLARE_OBJECT_MANAGER(CPaintGroup, PaintGroup, 					N_ENTITIES);
DECLARE_OBJECT_MANAGER(CTextHelper, TextHelper,						1);

/* Used only for testing purposes */
#ifdef _TEST
using namespace test;
DECLARE_OBJECT_MANAGER(_TCA, _TCA, 50);
DECLARE_OBJECT_MANAGER(_TCB, _TCB, 50);
#endif

void init()
{
    getManager<Entity>()->init();
    getManager<CName>()->init();
    getManager<CTransform>()->init();
    getManager<CRestore>()->init();
    getManager<CCamera>()->init();
    getManager<CCulling>()->init();
    getManager<CCullingCube>()->init();
    getManager<CDirLight>()->init();
    getManager<CPtLight>()->init();
    getManager<CShadow>()->init();
    getManager<CCubeShadow>()->init();
    getManager<CTint>()->init();
    getManager<CSelfIllumination>()->init();
    getManager<CSlots>()->init();
    getManager<CDetection>()->init();
    getManager<CInstancedMesh>()->init();
    getManager<CMesh>()->init();
    getManager<CAABB>()->init();
    getManager<CCullingAABB>()->init();
    getManager<CCullingAABBSpecial>()->init();
    getManager<CSkeleton>()->init();
    getManager<CBoneLookAt>()->init();
    getManager<CArmPoint>()->init();
    getManager<CTransformable>()->init();
    getManager<CThrowsPickups>()->init();
    getManager<CProp>()->init();
    getManager<CCannon>()->init();
    getManager<CDestructible>()->init();
    getManager<CDestructibleRestorer>()->init();
    getManager<CTrampoline>()->init();
    getManager<CLiana>()->init();
    getManager<CCreep>()->init();
    getManager<CEnemy>()->init();
    getManager<CMelee>()->init();
	getManager<CKnife>()->init();
    getManager<CFlare>()->init();
    getManager<CFlareShot>()->init();
    getManager<CBoss>()->init();
    getManager<CSmokePanel>()->init();
    getManager<CWeakSpot>()->init();
    getManager<CMobile>()->init();
    getManager<CFlyingMobile>()->init();
    getManager<CAnimationPlugger>()->init();
	getManager<CAnimationSounds>()->init();
    getManager<CScriptTrigger>()->init();
    getManager<CSpatialIndex>()->init();
    getManager<CTimer>()->init();
	getManager<CRigidBody>()->init();
	getManager<CStaticBody>()->init();
    getManager<CCharacterController>()->init();
	getManager<CExtraShapes>()->init();
    getManager<CPlayerMov>()->init();
    getManager<CPlayerAttack>()->init();
    getManager<CPlayerStats>()->init();
	getManager<CTrigger>()->init();
    getManager<CPickup>()->init();
    getManager<CBullet>()->init();
	getManager<CCannonPath>()->init();
	getManager<CSkyBox>()->init();
	getManager<CCheckPoint>()->init();
	getManager<CLevelData>()->init();
    getManager<CWhiteBox>()->init();
	getManager<CParticleSystem>()->init();
	getManager<CSmokeTower>()->init();
	getManager<CBichito>()->init();
	getManager<CLua>()->init();
	getManager<CVolPtLight>()->init();
	getManager<CCameraAnim>()->init();
	getManager<CEmitter>()->init();
    getManager<CMist>()->init();
	getManager<CAmbientSound>()->init();
	getManager<CPaintGroup>()->init();
	getManager<CTextHelper>()->init();
	getManager<CMaxAnim>()->init();
    
    Entity::initType();

    gameElements::initGameElements();

    CScriptTrigger::initType();
    CCheckPoint::initType();
    
/* Used only for testing purposes */
#ifdef _TEST
    getManager<_TCA>()->init();
    getManager<_TCB>()->init();
#endif
}

void cleanup()
{
    Handle::setCleanup();

    /* When an entity gets destroyed, it attempts to destroy all its components, *
     * So it should be deleted first                                             */
    component::getManager<component::Entity>()->~ObjectManager();

    /* Other object managers will be deleted on program teardown in irrelevant order
     * An alternative would be to declare all ObjectManagers in
     * reverse dependant order (i.e. EntityManager first and so on),
     * because according to §3.6.3 of the C++ standard (C++03)
     * destructors are called in reverse order of completion of constructors.
     *
     * But this is Visual C++, which is far from being a C++11 compliant
     * compiler no matter what it claims to be, so I choose not to trust it.
     */

    // Other things
    EntityListManager::cleanup();

    //PhysX
    component::getManager<CCharacterController>()->~ObjectManager();
    component::getManager<CRigidBody>()->~ObjectManager();
	component::getManager<CStaticBody>()->~ObjectManager();
	component::getManager<CParticleSystem>()->~ObjectManager();
    component::getManager<CLiana>()->~ObjectManager();
    
    //Objects that call to ItemsByName<T>::destroy() upon deletion
    component::getManager<CShadow>()->~ObjectManager();
    component::getManager<CCubeShadow>()->~ObjectManager();
    component::getManager<CMesh>()->~ObjectManager();
	

}


}
