#include "mcv_platform.h"
#include "bullet.h"

#include "handles/entity.h"
#include "components/transform.h"
using namespace component;

#include "Physx_USER/pxcomponents.h"
using namespace physX_user;

#include "../module.h"

#include "../PaintManager.h"

using namespace utils;
using namespace DirectX;

#include "Particles/ParticlesManager.h"
using namespace particles;

namespace gameElements {
    
const float CBullet::BULLET_SPEED      = 80.f;
const float CBullet::BULLET_SPEED_MEGA = 90.f;

void CBullet::update(float elapsed)
{
    if (collided) {
        collided = false;
        treatCollision();
    }

	Entity* me = Handle(this).getOwner();
	CTransform* meT = me->get<CTransform>();
	
	life += elapsed;

	//El disparo con los melee a veces no se detecta bien porque la bala se mueve y el melee tambien, asi que lo "traviessa"
	//(nunca entran en contacto segun physX), al detectarse por trigger no hay la opcion de activar CCD.
	//Si se hace por colision aparece el bug de que los enemigos se levantan del suelo por la colision de la bala.
	//Solución: raycast de la nueva posicion a la antigua, si colisiono con enemigo es que le he dado.
	XMVECTOR origin = meT->getPosition();
	if (prevPos != zero_v && origin != prevPos){
		XMVECTOR dir = XMVector3Normalize(prevPos - origin);
		PxReal distance = 2.55f;
		PxRaycastBuffer hit;
		if (PhysicsManager::get().raycast(origin, dir, distance, hit,
			filter_t(filter_t::NONE, ~filter_t::id_t(filter_t::ENEMY), filter_t::ENEMY))){
			Handle HitHandle = Handle::fromRaw(hit.block.shape->userData);
			Entity *eOther = HitHandle;
			if (eOther->has<CEnemy>()){
				eOther->sendMsg(MsgShot(isMega()));
				finishEffect();
				removeFromScene();
				
			}
		}
	}
	prevPos = meT->getPosition();

	if (ttlTimer.count(elapsed) >= -(ttl / 2) && once) {
		CRigidBody* rigid = me->get<CRigidBody>();
		XMVECTOR velocity = rigid->getVelocityObject(meT->getPosition());
		velocity = XMVectorSet(XMVectorGetX(velocity) / 2, XMVectorGetY(velocity) / 2, XMVectorGetZ(velocity) / 2, XMVectorGetW(velocity));
		rigid->addVelocityObject(velocity, meT->getRotation(), meT->getPosition());
		once = false;
	}

	if (ttlTimer.count(elapsed) >= 0) {
		finishEffect();
        removeFromScene();	
    }
	if(timer.count(elapsed) >= 0.1f){
		didSound = false;
	}
}

void CBullet::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CBullet, MsgCollisionEvent, receive);
}

void CBullet::treatCollision()
{
    Entity* e(collision.entity);
	Entity* owner = Handle(this).getOwner();
    
    CTransform* meT = owner->get<CTransform>();

    CPaintGroup* paint = PaintManager::getBrush(mega?0:1);
    if (paint != nullptr) {
        auto pos = meT->getPosition();
        paint->addInstance(pos, paintSize);
    }

    if (e->has<CTransformable>()) {
        CTransformable* transformable(e->get<CTransformable>());
        if ((!transformable->getInert() && !transformable->isTransformed())) {
		    e->sendMsg(MsgShot(isMega()));
		    removeFromScene();
		}
		else{
			if (!didSound){
				fmodUser::FmodStudio::play3DSingleEvent(
                    fmodUser::FmodStudio::getEventInstance("SFX/Bullet_bounce"),
                    meT->getPosition());
				didSound = true;
				timer.reset();
			}
		}
	}else{
		if(!didSound){
			CTransform* ownerT = owner->get<CTransform>();
			fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/Bullet_bounce"), ownerT->getPosition());
			didSound = true;
			timer.reset();
		}
	}
}

void CBullet::finishEffect(){

	Entity* owner = Handle(this).getOwner();
	
	if (!finishEffectDone && owner->has<CEmitter>()){
        finishEffectDone = true;

		CEmitter *emitter = owner->get<CEmitter>();
		auto k0 = emitter->getKey("emitter_0");
		auto k1 = emitter->getKey("emitter_1");

		// kill by time the speel emisor of bullet
		ParticleUpdaterManager::get().sendActive(k1);
		
		// active the bullet impact emisor
		ParticleUpdaterManager::get().setDeleteSelf(k0);
		// kill it before time
		ParticleUpdaterManager::get().setDeleteSelf(k1);

	}
}

void CBullet::removeFromScene()
{
	Entity* e = Handle(this).getOwner();
	finishEffect();
	e->postMsg(MsgDeleteSelf());
}

void CBullet::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    mega = atts.getBool("mega", false);
    ttl = atts.getFloat("ttl", 1.0f);
    ttlTimer.reset();
    ttlTimer.count(-ttl);
    paintSize = atts.getFloat("paintSize", paintSize);
}

void CBullet::setup(XMVECTOR origin, XMVECTOR velocity, XMVECTOR rotQ)
{
    finishEffectDone = false;
	Entity* me = Handle(this).getOwner();
    CTransform* meT = me->get<CTransform>();
    meT->setPosition(origin);
    meT->setRotation(rotQ);

    CRigidBody* rigid = me->get<CRigidBody>();
    rigid->addVelocityObject(velocity, XMQuaternionIdentity(), origin);

	CEmitter *emitter = me->get<CEmitter>();
	auto k0 = emitter->getKey("emitter_0");
	auto k1 = emitter->getKey("emitter_1");

	ParticleUpdaterManager::get().sendActive(k0);
	ParticleUpdaterManager::get().sendInactive(k1);

}

}