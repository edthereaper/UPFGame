#include "mcv_platform.h"
#include "pickup.h"
#include "app.h"

#include "handles/prefab.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "enemies/enemies.h"
#include "props.h"
#include "liana.h"

using namespace DirectX;
using namespace utils;

#include "Particles/ParticlesManager.h"

using namespace particles;

#define HEALTH_AMOUNT_PICKUP		 2
#define ENERGY_AMOUNT_PICKUP		 2
#define TIME_INMORTAL_PICKUP		 7.5f
#define COIN_POINTS_PICKUP			 1
#define COLLECTIBLE_POINTS_PICKUP	 1

namespace gameElements {

void CPickup::loadFromProperties(std::string elem, utils::MKeyValue atts)
{
    std::string typeStr = atts.getString("type", "none");
    if (typeStr == "none") {
        type = NONE;
    } else if (typeStr == "health") {
        type = HEALTH;
    } else if (typeStr == "energy") {
        type = ENERGY;
    } else if (typeStr == "invincible") {
        type = INVINCIBLE;
	} else if (typeStr == "coin") {
		type = COIN;
	} else if (typeStr == "collectible") {
		type = COLLECTABLE;
	}
    
    float s = atts.getFloat("strength", 0);
    strength = s > 0 ? s : 0;
	
	if (atts.has("ttl")) {
        dies = true;
        float ttl = atts.getFloat("ttl", 5.0f);
        ttlTimer.reset();
        ttlTimer.count(-ttl);
    } else {
        dies = false;
    }
}

void CPickup::setupStationary()
{
	stationary = true;
	setupDone = true;
}

void CPickup::setup(XMVECTOR position, XMVECTOR velocity, XMVECTOR rotQ)
{
    notRemoved = true;
	Entity* me = Handle(this).getOwner();
	CTransform* meT = me->get<CTransform>();

	//issue lag generated when the pickups are created at same point and tim
	meT->setPosition(position);
	meT->setRotation(rotQ);

	CRigidBody* rigid = me->get<CRigidBody>();
	rigid->createRigidBody();

	if (XMVector3Equal(velocity, zero_v)) {
		setupStationary();
	} else {
		stationary = false;
		rigid->addVelocityObject(velocity, rotQ, position);
		setupDone = true;
	}
}

void CPickup::update(float elapsed)
{	
    static const XMVECTOR farAway = one_v*100000;

	lifeTime += elapsed;

	if (!stationaryUsed)
	{
		CTransform* meT = Handle(this).getBrother<CTransform>();
		Handle playerHandle = App::get().getPlayer();
        XMVECTOR playerCenter;
        if (playerHandle.isValid()) {
		    CTransform* playerTransform = ((Entity*)playerHandle)->get<CTransform>();
		    playerCenter = playerTransform == nullptr? farAway :
                playerTransform->getPosition() +
                XMVectorSet(0, XMVectorGetY(playerTransform->getScale()) / 2, 0, 0);
        } else {
            playerCenter = farAway;
        }
		Entity* me = Handle(this).getOwner();
		CRigidBody* rigid = me->get<CRigidBody>();
		float dist = sqEuclideanDistance(playerCenter, meT->getPosition());
		if (dist < 50.0f) {
			XMVECTOR ds = XMVectorSet(XMVectorGetX(playerCenter - meT->getPosition()), XMVectorGetY(playerCenter - meT->getPosition()), XMVectorGetZ(playerCenter - meT->getPosition()), 0);
			ds = XMVector3Normalize(ds);
			if (dist <= 2.0f){
				activate();
			}
			else{
				if (dist <= 10.0f){
					ds = 25 * ds;
				}
				else{
					ds = (50 / dist) * ds * 5;
				}
			}
			rigid->addVelocityObject(ds, meT->getRotation(), meT->getPosition());
		} else{
			if (stationary){
				rigid->addVelocityObject(XMVectorSet(0, 0, 0, 0), XMQuaternionIdentity(), meT->getPosition());
			}
		}

		if (dies && ttlTimer.count(elapsed) >= 0) {
			Entity* e = Handle(this).getOwner();
			
			CEmitter *emitter = e->get<CEmitter>();
			auto k = emitter->getKey("emitter_0");
			ParticleUpdaterManager::get().setDeleteSelf(k);
			
			e->postMsg(MsgDeleteSelf());
		}
	}
}

void CPickup::removeFromScene()
{
    notRemoved = false;
	Entity* e = nullptr;
	if (stationary){
	    //No lo eliminamos, en caso de gameover necesitamos saber que ahí havía un pickup
		stationaryUsed = true;
		e = Handle(this).getOwner();
		e->get<CMesh>().destroy();
		CRigidBody* cR = e->get<CRigidBody>();
		cR->setFilters(filter_t::NONE, filter_t::ALL_IDS, filter_t::NONE);
	} else {
		e = Handle(this).getOwner();
		e->postMsg(MsgDeleteSelf());
	}
    
#if !defined(_PARTICLES)
    auto& partMan = ParticleUpdaterManager::get();
	CEmitter *emitter = e->get<CEmitter>();
    for (auto& k : emitter->iterateKeys()) {
	    partMan.setDeleteSelf(k);
    }
#endif
}

void CPickup::activate()
{
	Entity* player = App::get().getPlayer();
	switch (type)
	{
	case gameElements::CPickup::HEALTH:
		fmodUser::fmodUserClass::playSound("Pickup_life", 1.0f, 0.0f);
		player->sendMsg(MsgPickupHeal(HEALTH_AMOUNT_PICKUP));
		break;

	case gameElements::CPickup::ENERGY:
		fmodUser::fmodUserClass::playSound("Pickup_energy", 1.0f, 0.0f);
		player->sendMsg(MsgPickupEnergy(ENERGY_AMOUNT_PICKUP));
		break;

	case gameElements::CPickup::INVINCIBLE:
		player->sendMsg(MsgPickupInvincible(TIME_INMORTAL_PICKUP));
		break;
	case gameElements::CPickup::COIN:
		fmodUser::fmodUserClass::playSound("Pickup_coin", 1.0f, 0.0f);
		player->sendMsg(MsgPickupCoin(COIN_POINTS_PICKUP));
		break;
	case gameElements::CPickup::COLLECTABLE:
		player->sendMsg(MsgPickupCollectible(COLLECTIBLE_POINTS_PICKUP));
		break;
	default:
		break;
	}
	removeFromScene();
}

void CThrowsPickups::activate()
{
    if (done) {return;}

	Handle h(this);
	CTransform* ctransf(h.getBrother<CTransform>());

	XMVECTOR pos = zero_v;
	if (h.hasBrother<CEnemy>()){
		pos = ctransf->getPosition() + yAxis_v;
	} else {
		if (h.hasBrother<CTransformable>()){
			if (h.hasBrother<CCreep>() || h.hasBrother<CLiana>()){
				pos = ctransf->getPosition();
			}
			else{
				CTransformable* prop = h.getBrother<CTransformable>();
				pos = prop->getCenterAim();
			}
		}
	}
	unsigned nPickups = utils::rand_uniform(4, 1);
	float velX, velY, velZ;
	for (unsigned i = 0; i != nPickups; i++){
		Entity* entity;
		unsigned pickupType = utils::die(3);
        switch (pickupType) {
            default:
            case 0:
                entity = PrefabManager::get().prefabricate("pickup/health-dynamic");
                break;
            case 1:
                entity = PrefabManager::get().prefabricate("pickup/energy-dynamic");
                break;
			case 2:
				entity = PrefabManager::get().prefabricate("pickup/coin-dynamic");
				break;
        }
		EntityListManager::get(CPickup::TAG).add(entity);
        // Set random speed
		velX = utils::rand_uniform(3.0f, -3.0f);
		velY = utils::rand_uniform(5.0f, 3.0f);	
		velZ = utils::rand_uniform(3.0f, -3.0f);
		pos += XMVectorSet(0, 0.2f, 0, 0);
		((CPickup*)entity->get<CPickup>())->setup(
            pos, XMVectorSet(velX, velY, velZ, 0), XMQuaternionIdentity());
		entity->init();
	}
    if (!repeat) {
        done = true;
    } else if (repeatCooldownTimer.get() >= 0) {
        repeatCooldownTimer.reset();
        repeatCooldownTimer.count(-repeatCooldown);
    }
}

}
