#include "mcv_platform.h"
#include "whiteboxes.h"

#include "logic/trigger.h"

using namespace DirectX;
using namespace utils;
using namespace logic;

#include "render/components.h"
using namespace render;

#include "handles/entity.h"
#include "handles/handle.h"
#include "handles/prefab.h"
#include "components/transform.h"
#include "components/AABB.h"
#include "components/tint.h"
using namespace component;

#include "PhysX_USER/Physic_Component.h"
#include "PhysX_USER/CollisionMesh.h"
using namespace physX_user;

#include "gameElements/props.h"
#include "gameElements/pickup.h"
#include "gameElements/enemies/enemies.h"
#include "gameElements/enemies/melee.h"
#include "gameElements/enemies/flare.h"
using namespace gameElements;

#include "PhysX_USER\TriggerFilterSimulater.h"

#include "animation\components.h"
using namespace animation;

const uint32_t EOF_TAG = 0x123456FF;

struct boxInfo_t {
    static const uint32_t TAG = 0x12345601;
    XMFLOAT3 bbmin;
    XMFLOAT3 bbmax;
    uint8_t r,g,b,a;
    uint32_t tagTail;
};

struct boxPhysXInfo_t {
	static const uint32_t TAG = 0x12345AFF;
	XMFLOAT3 pos;
	XMFLOAT4 rotQ;
	float height;
	float length;
	float width;
	uint32_t tagTail;
};

//struct ropeNodeBase {
//
//	static const uint32_t TAG = 0x12934A0C;
//	XMFLOAT3 pos;
//	XMFLOAT4 rotQ;
//	uint32_t tagTail;
//};


struct trampolineInfo_t {
	
public:
    static const uint32_t TAG = 0x12345602;
    XMFLOAT3 pos;
    XMFLOAT4 rotQ;
    uint32_t tagTail;
};

struct cannonInfo_t {
    static const uint32_t TAG = 0x12345603;
    XMFLOAT3 pos;
    XMFLOAT4 rotQ;
    float fovH;
    float fovV;
    uint32_t tagTail;
};

struct lianaInfo_t {

	private:
		static const XMVECTOR SIZE;
	public:
		static const uint32_t TAG = 0x12934A0C;
		XMFLOAT3 pos;
		XMFLOAT4 rotQ;
		uint32_t tagTail;
};

const XMVECTOR lianaInfo_t::SIZE = XMVectorSet(0.3f, 0.f, 0.3f, 1.f);

struct creepInfo_t {
    static const uint32_t TAG = 0x12345605;
    XMFLOAT3 pos;
    float height;
    float length;
    float width;
    uint32_t tagTail;
};

struct meshInfo_t {
    static const uint32_t TAG = 0x12345606;
    XMFLOAT3 pos;
    XMFLOAT4 rotQ;
    XMFLOAT3 bbmin;
    XMFLOAT3 bbmax;
    uint8_t r,g,b,a;
    char name[32];
    uint32_t tagTail;
};

struct sceneInfo_t {
	static const uint32_t TAG = 0x23154FF2;
	XMFLOAT3 pos;
	XMFLOAT4 rotQ;
	XMFLOAT3 bbmin;
	XMFLOAT3 bbmax;
	uint8_t r, g, b, a;
	char name[32];
	uint32_t tagTail;
};

struct destructibleBoxInfo_t {
    static const uint32_t TAG = 0x12345607;
    static const uint32_t COLOR = 0x703010B7;
    XMFLOAT3 bbmin;
    XMFLOAT3 bbmax;
    uint32_t tagTail;
};

struct checkpointInfo_t {
    static const uint32_t TAG = 0x12345608;
    XMFLOAT3 bbmin;
    XMFLOAT3 bbmax;
    XMFLOAT3 pos;
    uint32_t tagTail;
};


struct pickupInfo_t {
    static const uint32_t TAG = 0x12345609;
    XMFLOAT3 pos;
    CPickup::type_e type;
    uint32_t tagTail;
};

struct levelInfo_t {
    static const uint32_t TAG = 0x12345610;
    XMFLOAT3 playerSpawnPos;
    XMFLOAT4 playerSpawnRot;
    XMFLOAT3 finishBbmin;
    XMFLOAT3 finishBbmax;
    uint32_t tagTail;
};

struct enemyInfo_t {
    static const uint32_t TAG = 0x1234560A;
    XMFLOAT3 pos;
    XMFLOAT4 rotQ;
    uint32_t wp;
    enum {NONE=0, MELEE=1, FLARE=2, SNIPER=3} type;
    uint32_t tagTail;
};

struct wpHeader_t {
    static const uint32_t TAG = 0x1234560B;
    uint32_t nknots;
};

Handle createTransformableEntity(
    const XMVECTOR& pos,
    const XMVECTOR& rot,
    const std::string meshName
    )
{
    Handle entity_h, h;
    Entity* entity;
    entity_h = getManager<Entity>()->createObj();
    entity = entity_h;

    h = getManager<CTransform>()->createObj();
    CTransform* transform = h;
    transform->setPosition(pos);
    transform->setRotation(rot);
    entity->add(h);
    
    entity->add(getManager<CTint>()->createObj());

    h = getManager<CTransformable>()->createObj();
    CTransformable* transformable = h;
    entity->add(h);
    CMesh::load(meshName, Handle(entity));

    CMesh* mesh = entity->get<CMesh>();
    mesh->init();
    transformable->init();
    
    h = getManager<CThrowsPickups>()->createObj();
    CThrowsPickups* throwsPickups = h;
    entity->add(h);
    
    h = getManager<CAABB>()->createObj();
    CAABB* aabb = h;
    *aabb = mesh->getMesh()->getAABB();
    entity->add(aabb);
    
    return entity_h;
}

Handle createRigidbodyEntity(
	const XMVECTOR& pos,
	const XMVECTOR& rot,
	const Mesh*		meshData
	)
{

	Handle entity_h, h;
	Entity* entity;
	entity_h = getManager<Entity>()->createObj();
	entity = entity_h;

	h = getManager<CMesh>()->createObj();
	CMesh* mesh = h;
	entity->add(h);

	h = getManager<CTransform>()->createObj();
	CTransform* transform = h;
	transform->setPosition(pos);
	transform->setRotation(rot);
	entity->add(h);

	entity->add(getManager<CTint>()->createObj());

	h = getManager<CTransformable>()->createObj();
	CTransformable* transformable = h;
	entity->add(h);

	h = getManager<CAABB>()->createObj();
	CAABB* aabb = h;
	*aabb = mesh->getMesh()->getAABB();
	entity->add(aabb);


	return entity_h;

}

Handle createTransformableBoxEntity(
    const XMVECTOR& pos,
    const XMVECTOR& rot,
    const char* meshName,
    XMVECTOR bbmin, XMVECTOR bbmax
    )
{
    Mesh* mesh = Mesh::getManager().getByName(meshName);
    mesh->setAABB(CAABB(bbmin, bbmax));
    auto h = createTransformableEntity(pos,rot,meshName);
    Entity* e(h);
    ((CTransform*)e->get<CTransform>())->setScale(bbmax-bbmin);
    return h;
}


void createRope(XMVECTOR position, XMVECTOR rotation, int numNodes){


	int i = 0;

	Entity* entity = nullptr;
	Handle  h;

	CStaticBody				*s = nullptr;
	CCollider				*c = nullptr;
	CRigidBody				*r = nullptr;
	CJoint					*j = nullptr;
	CTrigger				*t = nullptr;
	CLianaNode				*l = nullptr;
	CJointDynamic			*lDynamic = nullptr;

	XMVECTOR pos = position;

	PxRigidActor * baseActorTmp = nullptr;
	PxRigidActor * actorTmp = nullptr;
	PxRevoluteJoint * base = nullptr;


	while (i < numNodes){

		entity = createTransformableEntity(
			pos, rotation,
			"liana");

		h = getManager<CCollider>()->createObj();
		c = h;
		entity->add(h);


		h = getManager<CLianaNode>()->createObj();
		l = h;
		entity->add(h);

		h = getManager<CJointDynamic>()->createObj();
		lDynamic = h;
		entity->add(h);

		float angle = 60;
		
		if (i == 0){

					h = getManager<CStaticBody>()->createObj();
					s = h;
					entity->add(h);

					EntityListManager::get(CLianaNode::TAG).add(entity);

					c->setBox(XMVectorSet(0.5, 2, 0.5, 0));
					c->createShape();

					s->createStaticBody();

					actorTmp = s->getStaticbody();
					baseActorTmp = s->getStaticbody();
			}
			else{

					h = getManager<CRigidBody>()->createObj();
					r = h;
					entity->add(h);

					h = getManager<CJoint>()->createObj();
					j = h;
					entity->add(h);

					h = getManager<CTrigger>()->createObj();
					t = h;
					entity->add(h);


					EntityListManager::get(CLianaNode::TAG).add(entity);

					c->setCapsule(1, 0.5);
					c->createShape();
	
					r->createRigidBody();
					t->createTrigger(Filter::FilterIdentifier::FILTER_LIANA);
					t->setupTrigger();
					
					assert(actorTmp || "conector isnt work");
					if (i == 1){

						lDynamic->setBaseDynamic(baseActorTmp);
						base = j->connectToRigidDynamic(actorTmp, yAxis_v * 1.5f, 7 * PxPi / 8);
					}
					else{
						lDynamic->setJointDynamic(base);
						lDynamic->setBaseDynamic(baseActorTmp);
						j->connectToRigidDynamic(actorTmp, yAxis_v * 1.5f, 7 * PxPi / 8);
					}
					
					
					actorTmp = r->getRigidbody();

			}

			if (i == numNodes - 1){
				CTransform *transform = entity->get<CTransform>();
				r->addVelocityObject(
									 transform->getFront() * 2,
									 transform->getRotation(),
									 transform->getPosition()
									 );
			}
			pos -= (XMVectorSet(0, 3.0f, 0, 0)); // avoid collision when created any node-- delte after create a liana with bones
			i++;
	}

	

	actorTmp = nullptr;
	baseActorTmp = nullptr;
}


WhiteboxesLevel loadWhiteboxes(DataProvider& dp)
{
    assert(dp.isValid());

    WhiteboxesLevel ret;

    uint32_t tag = 0;

    boxInfo_t box;
	boxPhysXInfo_t boxPhysxData;

    trampolineInfo_t trampolineData;
    cannonInfo_t cannonData;
    lianaInfo_t lianaData;
    creepInfo_t creepData;
    meshInfo_t meshData;
	sceneInfo_t sceneData;
    destructibleBoxInfo_t destructibleData;
    levelInfo_t levelData;
    checkpointInfo_t checkpointData;
    pickupInfo_t pickupData;
    enemyInfo_t enemyData;
    wpHeader_t wpData;
	lianaInfo_t ropeBase;
	

    XMVECTOR bbmin, bbmax, pos, rotQ;
    uint32_t tint = 0xBADC0102;
    XMFLOAT3 point;
    typedef std::vector<XMVECTOR> wps_t;
    wps_t wps;
    std::vector<wps_t> wpsList;

    CAABB bbMelee;

    Mesh* Cube = Mesh::getManager().getByName("Pyramid001");
    Mesh* tree2 = Mesh::getManager().getByName("arbol2");
    Mesh* tree3 = Mesh::getManager().getByName("arbol3");
	Mesh* boxMeshData = Mesh::getManager().getByName("Box563");



    Entity* entity = nullptr;
    Handle  h;
    CTransform*  transform;
    CAABB* aabb;
    CTrampoline*		trampoline;
    CCannon*			cannon;
    CCreep*				creep;

	CStaticBody			*s;
	CTrigger			*t;
	CCollider			*c;

    CTint* ctint;
    CMesh* cmesh;
	PxTriangleMesh * triangleMesh;

	int index = 0;

    bool notDone = true;

	CCharacterController *charContrl;
	XMVECTOR size;
	CAnimationPlugger* animPlugger = nullptr;
	
    while (notDone) {

		dp.read(tag);

        switch(tag) {

            case boxInfo_t::TAG :
                dp.read(box);
                assert(tag == box.tagTail);
                bbmin = XMVectorSet(box.bbmin.x, box.bbmin.y,box.bbmin.z,1);
                bbmax = XMVectorSet(box.bbmax.x, box.bbmax.y,box.bbmax.z,1);
                tint = (box.r<<24)|(box.g<<16)|(box.b<<8)|box.a;
                entity = getManager<Entity>()->createObj();
                h = getManager<CAABB>()->createObj();
                entity->add(h);
                aabb = h;
                *aabb = CAABB(zero_v, bbmax-bbmin);
                h = getManager<CTint>()->createObj();
                entity->add(h);
                ctint = h;
                *ctint = tint;
                h = getManager<CTransform>()->createObj();
                entity->add(h);
                CMesh::load("BoxCorner", Handle(entity));
                cmesh = entity->get<CMesh>();
                cmesh->init();
                transform = h;
                transform->setScale(bbmax-bbmin);
                transform->setPosition(bbmin+aabb->getSize()/2);
                h = getManager<CCollider>()->createObj();
                entity->add(h);
                c= h;
                c->setBox(aabb->getSize());
                c->createShape();
                h = getManager<CStaticBody>()->createObj();
                entity->add(h);
                s = h;
                s->createStaticBody();
                break;


			case sceneInfo_t::TAG:

				dp.read(sceneData);
				assert(tag == sceneData.tagTail);

				pos = XMVectorSet(sceneData.pos.x, sceneData.pos.y, sceneData.pos.z, 1);
				rotQ = XMVectorSet(sceneData.rotQ.x, sceneData.rotQ.y, sceneData.rotQ.z, sceneData.rotQ.w);
				tint = (sceneData.r << 24) | (sceneData.g << 16) | (sceneData.b << 8) | (sceneData.a << 0);

				entity = createTransformableEntity(
					pos, rotQ, sceneData.name);
                
                ctint = entity->get<CTint>();
                ctint->set(tint);

				h = getManager<CCollider>()->createObj();
				c = h;
				entity->add(h);

				h = getManager<CStaticBody>()->createObj();
				s = h;
				entity->add(h);

				h = getManager<CTrigger>()->createObj();
				t = h;
				entity->add(h);

				entity->add(getManager<CScene>()->createObj());

				EntityListManager::get(CScene::TAG).add(entity);
				triangleMesh = CollisionMeshLoader::load(sceneData.name);
				s->createStaticMesh(triangleMesh);
				//t->createCollideScene();

				triangleMesh = nullptr;

				break;
              
            case trampolineInfo_t::TAG :

                dp.read(trampolineData);
                assert(tag == trampolineData.tagTail);
                pos = XMVectorSet(
                    trampolineData.pos.x, trampolineData.pos.y,
                    trampolineData.pos.z, 1);
                rotQ = XMVectorSet(
                    trampolineData.rotQ.x, trampolineData.rotQ.y,
                    trampolineData.rotQ.z, trampolineData.rotQ.w);
                
                entity = createTransformableEntity(
                    pos, rotQ, "trampoline");
                h = getManager<CTrampoline>()->createObj();
                trampoline = h;
                trampoline->setRotation(rotQ);
                entity->add(h);
				
				h = getManager<CCollider>()->createObj();
				c = h;
				entity->add(h);
				
				h = getManager<CStaticBody>()->createObj();
				s = h;
				entity->add(h);

				h = getManager<CTrigger>()->createObj();
				t = h;
				entity->add(h);

				EntityListManager::get(CTrampoline::TAG).add(entity);


				c->setBox(XMVectorSet(2, 2, 2, 0));
				c->createShape();

				s->createStaticBody();

				t->createTrigger(Filter::FilterIdentifier::FILTER_TRAMPOLINE);
				

                break;
              
            case cannonInfo_t::TAG :
                dp.read(cannonData);
                assert(tag == cannonData.tagTail);
                pos = XMVectorSet(
                    cannonData.pos.x, cannonData.pos.y,
                    cannonData.pos.z, 1);
                rotQ = XMVectorSet(
                    cannonData.rotQ.x, cannonData.rotQ.y,
                    cannonData.rotQ.z, cannonData.rotQ.w);
                
                entity = createTransformableEntity(
                    pos, rotQ, "cannon");

                h = getManager<CCannon>()->createObj();
                cannon = h;
                cannon->setRotation(rotQ);
                cannon->setFov(cannonData.fovH, cannonData.fovV);
                entity->add(h);

				h = getManager<CCollider>()->createObj();
				c = h;
				entity->add(h);

				h = getManager<CStaticBody>()->createObj();
				s = h;
				entity->add(h);

				h = getManager<CTrigger>()->createObj();
				t = h;
				entity->add(h);
                
                EntityListManager::get(CCannon::TAG).add(entity);

				c->setBox(XMVectorSet(2, 2, 2, 0));
				c->createShape();
				s->createStaticBody();
				t->createTrigger(Filter::FilterIdentifier::FILTER_CANNON);

                break;
              
            case lianaInfo_t::TAG :

				dp.read(ropeBase);

				assert(tag == ropeBase.tagTail);

				pos = XMVectorSet(
					ropeBase.pos.x, ropeBase.pos.y,
					ropeBase.pos.z, 1);
				rotQ = XMVectorSet(
					ropeBase.rotQ.x, ropeBase.rotQ.y,
					ropeBase.rotQ.z, ropeBase.rotQ.w);
				
				createRope(pos, rotQ, 4);

                break;
             
            case creepInfo_t::TAG :
                dp.read(creepData);
                assert(tag == creepData.tagTail);
				pos = XMVectorSet(creepData.pos.x, creepData.pos.y + creepData.height / 2.f, creepData.pos.z, 1);
				//bbmin = XMVectorSet(-creepData.width / 2.f, -creepData.height / 2.f , -creepData.length / 2.f, 1);
                //bbmax = XMVectorSet( creepData.width/2.f, creepData.height/2.f, creepData.length/2.f, 1);

				bbmin = XMVectorSet(-creepData.width / 2.f, -creepData.height / 2.f, -creepData.length / 2.f, 1);		//Problema: bbmin empieza en pos y solo va hacia arriba
				bbmax = XMVectorSet(creepData.width / 2.f, creepData.height / 2.f, creepData.length / 2.f, 1);
				
				/*rotQ = XMVectorSet(
					ropeBase.rotQ.x, ropeBase.rotQ.y,
					ropeBase.rotQ.z, 1);
                
				entity = createTransformableBoxEntity(
					pos, rotQ,
					"BoxCorner",
					bbmin, bbmax,
					0x555555BB, 0x309000FF
					);*/
				
				entity = createTransformableBoxEntity(
					pos, XMQuaternionIdentity(),
                    "BoxCorner",
                    bbmin, bbmax
                    );

				/*entity = createTransformableEntity(
					pos, XMQuaternionIdentity(),
					"BoxCorner", "BoxCorner",
					0x555555BB, 0x309000FF);*/

                h = getManager<CCreep>()->createObj();
                creep = h;
               creep->setup(creepData.width < creepData.height ? xAxis_v : zAxis_v,
                    std::max(creepData.width, creepData.height), creepData.height);
                
				entity->add(h);

				h = getManager<CCollider>()->createObj();
				c = h;
				entity->add(h);

				h = getManager<CStaticBody>()->createObj();
				s = h;
				entity->add(h);

				h = getManager<CTrigger>()->createObj();
				t = h;
				entity->add(h);

				
                EntityListManager::get(CCreep::TAG).add(entity);

				//dbg("CreepData pos X Y Z: %f %f %f\n", creepData.pos.x, creepData.pos.y, creepData.pos.z);
				//dbg("creepData width: %f, height: %f, length: %f\n", creepData.width, creepData.height, creepData.length);
				//c->setBox(XMVectorSet(0.5, 10, 11, 0));
				c->setBox(XMVectorSet(creepData.width, creepData.height, creepData.length, 0));
				c->createShape();
				s->createStaticBody();				//En release peta AQUI
				t->createTrigger(Filter::FilterIdentifier::FILTER_CREEPER);
				break;

            case meshInfo_t::TAG :

                dp.read(meshData);
                assert(tag == meshData.tagTail);

                pos   = XMVectorSet(meshData.pos.x,   meshData.pos.y,  meshData.pos.z,1);
                rotQ  = XMVectorSet(meshData.rotQ.x,  meshData.rotQ.y, meshData.rotQ.z, meshData.rotQ.w);
                bbmin = XMVectorSet(meshData.bbmin.x, meshData.bbmin.y,meshData.bbmin.z,1);
                bbmax = XMVectorSet(meshData.bbmax.x, meshData.bbmax.y,meshData.bbmax.z,1);
                tint  = (meshData.r<<24)|(meshData.g<<16)|(meshData.b<<8)|(meshData.a<<0);

                entity = createTransformableEntity(
                    pos, rotQ, meshData.name);
               /* *((CAABB*)entity->get<CAABB>()) = CAABB(bbmin-pos, bbmax-pos);
                entity->add(getManager<CProp>()->createObj());*/

				h = getManager<CCollider>()->createObj();
				c = h;
				entity->add(h);

				h = getManager<CStaticBody>()->createObj();
				s = h;
				entity->add(h);

				h = getManager<CTrigger>()->createObj();
				t = h;
				entity->add(h);

				entity->add(getManager<CProp>()->createObj());
                
                EntityListManager::get(CProp::TAG).add(entity);

				
				c->setBox(XMVectorSet(2, 2, 2, 0));
				c->createShape();

				s->createStaticBody();
				t->createTrigger(Filter::FilterIdentifier::FILTER_PROP);

                break;

            case levelInfo_t::TAG :
                dp.read(levelData);
                assert(tag == levelData.tagTail);
                pos = XMVectorSet(
                    levelData.playerSpawnPos.x, levelData.playerSpawnPos.y,
                    levelData.playerSpawnPos.z, 1);
                rotQ  = XMVectorSet(
                    levelData.playerSpawnRot.x, levelData.playerSpawnRot.y,
                    levelData.playerSpawnRot.z, levelData.playerSpawnRot.w);
                bbmin = XMVectorSet(
                    levelData.finishBbmin.x, levelData.finishBbmin.y,
                    levelData.finishBbmin.z, 1);
                bbmax = XMVectorSet(
                    levelData.finishBbmax.x, levelData.finishBbmax.y,
                    levelData.finishBbmax.z, 1);
                ret.finish = CAABB(bbmin, bbmax);
                ret.start = pos;
                ret.startRotation = rotQ;
                break;

            case checkpointInfo_t::TAG :
                dp.read(checkpointData);
                assert(tag == checkpointData.tagTail);
                pos   = XMVectorSet(
                    checkpointData.pos.x, checkpointData.pos.y,
                    checkpointData.pos.z, 1);
                bbmin = XMVectorSet(
                    checkpointData.bbmin.x, checkpointData.bbmin.y,
                    checkpointData.bbmin.z, 1);
                bbmax = XMVectorSet(
                    checkpointData.bbmax.x, checkpointData.bbmax.y,
                    checkpointData.bbmax.z, 1);
                ret.checkpoints.push_back(Checkpoint(component::CAABB(bbmin, bbmax), pos));

                break;
         
            case destructibleBoxInfo_t::TAG :
                dp.read(destructibleData);
                assert(tag == destructibleData.tagTail);
                bbmin = XMVectorSet(
                    destructibleData.bbmin.x, destructibleData.bbmin.y,
                    destructibleData.bbmin.z, 1);
                bbmax = XMVectorSet(
                    destructibleData.bbmax.x, destructibleData.bbmax.y,
                    destructibleData.bbmax.z, 1);
                entity = getManager<Entity>()->createObj();
                h = getManager<CAABB>()->createObj();
                entity->add(h);
                aabb = h;
                *aabb = CAABB(bbmin, bbmax);
                *aabb = *aabb + -aabb->getSize();
                h = getManager<CTint>()->createObj();
                entity->add(h);
                ctint = h;
                *ctint = destructibleBoxInfo_t::COLOR;
                CMesh::load("BoxCorner", Handle(entity));
                cmesh = entity->get<CMesh>();
                cmesh->init();
                h = getManager<CTransform>()->createObj();
                entity->add(h);
                transform = h;
                transform->setScale(aabb->getSize());
                transform->setPosition(bbmin);
                transform->setRotation(XMQuaternionIdentity());
                break;

            case pickupInfo_t::TAG :
                dp.read(pickupData);
                assert(tag == pickupData.tagTail);
                pos = XMVectorSet(
                    pickupData.pos.x, pickupData.pos.y,
                    pickupData.pos.z, 1);

                switch(pickupData.type) {
                    case CPickup::HEALTH:
                        entity = PrefabManager::get().prefabricate("pickup-health");
                        break;
                    case CPickup::ENERGY:
                        entity = PrefabManager::get().prefabricate("pickup-energy");
                        break;
                    case CPickup::INVENCIBLE:
                        entity = PrefabManager::get().prefabricate("pickup-invencible");
                        break;
                }
				EntityListManager::get(CPickup::TAG).add(entity);
                ((CPickup*)entity->get<CPickup>())->setup(pos);
                entity->init();
                break;
             
            case enemyInfo_t::TAG :
                dp.read(enemyData);
                assert(tag == enemyData.tagTail);
                pos = XMVectorSet(
                    enemyData.pos.x, enemyData.pos.y+5,			//test
                    enemyData.pos.z, 1);
                rotQ  = XMVectorSet(
                    enemyData.rotQ.x, enemyData.rotQ.y,
                    enemyData.rotQ.z, enemyData.rotQ.w);
                switch (enemyData.type) {
                    case enemyInfo_t::MELEE:
                        entity = PrefabManager::get().prefabricate("melee");
                        break;
                    case enemyInfo_t::FLARE:
                        entity = PrefabManager::get().prefabricate("flare");
                        break;
                    case enemyInfo_t::SNIPER:
                        entity = PrefabManager::get().prefabricate("sniper");
                        break;
                    default:
                        assert(!"Malformed enemy! Is not a known class!");
                        break;
                }
                transform = entity->get<CTransform>();
                transform->setPosition(pos);
                transform->setRotation(rotQ);

				//Creacion del character controller para los enemigos
                aabb = entity->get<CAABB>();
                bbMelee = ((CMesh*)entity->get<CMesh>())->getMesh()->getAABB();
                bbMelee = CAABB(
                    XMVectorSetZ(XMVectorSetX(bbMelee.getMin(), -0.25f), -0.25),
                    XMVectorSetZ(XMVectorSetX(bbMelee.getMax(),  0.25f),  0.25));
                *aabb = bbMelee;
				
				h = getManager<CCharacterController>()->createObj();
				entity->add(h);
				charContrl = h;

				h = getManager<CTrigger>()->createObj();
				t = h;
				entity->add(h);
				
				size = XMVectorAbs(aabb->getSize());
				//Divisones puestas para que las colisiones entre Vinedetta y el enemigo queden bien visualmente
				//Hardcoded! Canviar mas tarde a valores que se cargen.
				charContrl->createCapsuleController(pos,
													0.001f,
													0.01f,
													0.1f,	//0.5f 
													1.f,//XMVectorGetZ(size) / 1.2f,
													1.12f//XMVectorGetY(size) / 4.2f
													);

				t->createTriggerCharacterController();
				
				EntityListManager::get(CEnemy::TAG).add(entity);

				if (entity->has<CAnimationPlugger>())
					animPlugger = entity->get<CAnimationPlugger>();
				
				entity->init();

				if (entity->has<CMelee>())
					animPlugger->loadArchetype("melee");
				
                break;

            case wpHeader_t::TAG:
                dp.read(wpData);
                wps.clear();
                for (unsigned i=0; i< wpData.nknots; i++) {
                    dp.read(point);
                    pos = XMVectorSet(point.x, point.y, point.z, 1);
                    wps.push_back(pos);
                }
                dp.read(tag);
                assert(tag==wpHeader_t::TAG);
                wpsList.push_back(wps);
                break;

            case EOF_TAG :
                notDone = false;
                break;


			default:break;
                utils::fatal("Malformed file");
        }
    }

    return ret;
}

WhiteboxesLevel loadWhiteboxes(const char* name)
{
  char full_name[MAX_PATH];
  sprintf(full_name, "data/whitebox/%s.whitebox", name);
  FileDataProvider fdp(full_name);
  return loadWhiteboxes(fdp);
}
