#include "mcv_platform.h"
#include "importLevel.h"

using namespace utils;

#include "handles/prefab.h"
#include "handles/entity.h"
#include "components/transform.h"
#include "components/whitebox.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
#include "PhysX_USER/CollisionMesh.h"
#include "Particles/particlesystem.h"
using namespace physX_user;

#include "render/render_utils.h"
#include "render/mesh/CInstancedMesh.h"
#include "render/environment/mist.h"
using namespace render;

#include "animation/cskeleton.h"
using namespace animation;

#include "logic/trigger.h"
using namespace logic;

#include "gameElements/paintManager.h"
using namespace gameElements;

#include "Particles/ParticlesManager.h"
using namespace particles;

#include "level.h"

#if !defined(_LIGHTTOOL)
#define ENABLE_ENEMIES
#endif
//#define INSTANCE_PER_SPATIAL_INDEX //apparently not a good idea?

namespace level {

Handle LevelImport::currentLevel_h;
Handle LevelImport::currentEntity_h;
Handle LevelImport::previousLevel_h;
Handle LevelImport::playerEntity_h;
Handle LevelImport::bossEntity_h;
LevelImport::instancedPieces_t LevelImport::instancedPieces;
LevelImport::specialCollisions_t LevelImport::specialCollisions;
int LevelImport::spatialIndex;

#define CREEP_CHARACTER_HEIGHT 1.2f

Entity* LevelImport::createNonInstancedPiece(const pieceData_t& p)
{
    currentEntity_h = getManager<Entity>()->createObj();
    assert(currentEntity_h.isValid());
    Entity* currentEntity(currentEntity_h);
    
    Handle h = getManager<CTransform>()->createObj();
    currentEntity->add(h);
    CTransform* t(h);
    t->setPosition(p.pos);
    t->setRotation(p.rot);
    t->setScale(one_v);

    CMesh::load(p.meshName, currentEntity_h);
    return currentEntity;
}

Entity* LevelImport::createNonInstancedNonTransformablePiece(const pieceData_t& p)
{    
    auto currentEntity = createNonInstancedPiece(p);
    //Object doesn't transform -> is a scene
    PrefabManager::get().prefabricateComponents("components/scene", currentEntity_h);
    EntityListManager::get(TAG_SCENE).add(currentEntity);
    
    currentEntity_h = Handle();
    return currentEntity;
}

Entity* LevelImport::createNonInstancedTransformablePiece(const pieceData_t& p)
{
    auto currentEntity = createNonInstancedPiece(p);

    //Object transforms -> is a prop
    PrefabManager::get().prefabricateComponents("components/prop", currentEntity_h);
    
	CStaticBody* s = currentEntity->get<CStaticBody>();
    s->setTriangleMesh(CollisionMeshLoader::load(p.collision.c_str()));

    CTransformable* transformable = currentEntity->get<CTransformable>();
    transformable->setResourceName(p.transformation);
	transformable->setOriginalResourceName(p.meshName);
    transformable->setType(transformType_e::TRANSFORMABLE_MESH);
    transformable->setHits(0);
	CRestore* restore = currentEntity->get<CRestore>();
	restore->setSpatialIndex(p.spatialIndex);
    
	CAABB* aabb = currentEntity->get<CAABB>();
    CMesh* cmesh = currentEntity->get<CMesh>();
    assert(cmesh != nullptr);
    assert(cmesh->getMesh() != nullptr);
	auto maabb = cmesh->getMesh()->getAABB();
	XMMATRIX mat = XMMatrixTransformation(zero_v, zero_v, one_v, zero_v, p.rot, p.pos);
	XMVECTOR newmin = XMVector3Transform(maabb.getMin(), mat);
	XMVECTOR newmax = XMVector3Transform(maabb.getMax(), mat);
    transformable->setCenterAim((newmax+newmin)/2.f);

    EntityListManager::get(CProp::TAG).add(currentEntity);
    currentEntity_h = Handle();	
    return currentEntity;
}

Entity* LevelImport::getInstancedMesh(
    std::string instanceName, std::string meshName, bool transformable)
{
    CInstancedMesh* instancedMesh = nullptr;
    auto it = instancedPieces.find(instanceName);
    if (it == instancedPieces.end()) {
        Handle instancedEntity_h = PrefabManager::get().prefabricate(
            transformable ? "instancedPiece_transformable" : "instancedPiece");
        assert(instancedEntity_h.isValid());
        Entity* instancedEntity(instancedEntity_h);
        CName* name = instancedEntity->get<CName>();
        name->setName(instanceName);

        assert(instancedEntity->has<CInstancedMesh>());
        instancedMesh = instancedEntity->get<CInstancedMesh>();
        instancedMesh->load(meshName);
        instancedMesh->setInstancesWillMove(false);
        instancedMesh->setAABBSkin(0.1f);
        
        instancedPieces[instanceName] = instancedEntity_h;
        return instancedEntity_h;
    } else {
        return it->second;
    }
}
Entity* LevelImport::createInstancedTransformablePiece(const pieceData_t& p)
{
#ifdef INSTANCE_PER_SPATIAL_INDEX
    std::string nameSuffix = '_' + std::to_string(p.spatialIndex);
#else
    std::string nameSuffix = "";
#endif

    Entity* eNot = getInstancedMesh(p.meshName+nameSuffix+"#", p.meshName, true);
    CInstancedMesh* instancedMesh = eNot->get<CInstancedMesh>();
    component::Handle instancedMeshNot_h, instancedMeshYes_h;

    Entity* piece = createNonInstancedTransformablePiece(p);
    CMesh* mesh = piece->get<CMesh>();

    Transform t;
    t.setPosition(p.pos);
    t.setRotation(p.rot);
    t.setScale(one_v);
	CRestore* restore = piece->get<CRestore>();
    assert(restore != nullptr);
	restore->setSpatialIndex(p.spatialIndex);
    unsigned instanceIndex = 0;
    CMesh* mNot = eNot->get<CMesh>();
    if (!mNot->hasRandomMaterials()) {
        instanceIndex = instancedMesh->addInstance(t.getWorld());
        instancedMeshNot_h = instancedMesh;
        mesh->setVisible(false);
    }
    
    Entity* eYes = getInstancedMesh(p.transformation+nameSuffix+"#", p.transformation, true);
    CInstancedMesh* instancedTransformedMesh = eYes->get<CInstancedMesh>();
    instancedTransformedMesh->setAABBScale(1.05f); //Room for the breathe effect
    
    CMesh* mYes = eYes->get<CMesh>();
    if (!mYes->hasRandomMaterials()) {
        instancedMeshYes_h = instancedTransformedMesh;
        instancedTransformedMesh->setMaxInstances(
            instancedTransformedMesh->getMaxInstances()+1);
    }

    CTransformable* transformable = piece->get<CTransformable>();
    transformable->setInstanced(instancedMeshNot_h, instancedMeshYes_h, instanceIndex);

    return piece;
}

component::Entity* LevelImport::createInstancedNonTransformablePiece(const pieceData_t& p)
{
#ifdef INSTANCE_PER_SPATIAL_INDEX
    std::string nameSuffix = '_' + std::to_string(p.spatialIndex);
#else
    std::string nameSuffix = "";
#endif
    Entity* e = getInstancedMesh(p.meshName + nameSuffix, p.meshName, false);
    CMesh* m = e->get<CMesh>();
    if (m->hasRandomMaterials()) {
        return createNonInstancedNonTransformablePiece(p);
    } else {
        CInstancedMesh* instancedMesh = e->get<CInstancedMesh>();
        assert(instancedMesh != nullptr);
        Transform t;
        t.setPosition(p.pos);
        t.setRotation(p.rot);
        t.setScale(one_v);
        instancedMesh->addInstance(t.getWorld());
        return nullptr;
    }

}

Entity* LevelImport::createPiece(const pieceData_t& p)
{
    static const unsigned INSTANCED_THRESHOLD = 3;
    bool instanced = 
        p.instanced &&
        (instancedPieceCount[p.meshName] >= INSTANCED_THRESHOLD) &&
        p.special.empty() ;
    bool transforms = p.transformation != "";
    //If mesh doesn't exist, don't transform
    bool transformationExists =
        PrefabManager::get().getByName("mesh/"+p.transformation) != nullptr;
    Entity* e = nullptr;

#ifdef _DEBUG
    if (transforms && !transformationExists) {
       /*dbg("Mesh %s was supposed to transform into %s but mesh doesn't exist.\n",
            p.meshName.c_str(), p.transformation.c_str());*/
    }
#endif

    if (transforms && transformationExists) {
        if (instanced) {
            e = createInstancedTransformablePiece(p);
        } else {
            e = createNonInstancedTransformablePiece(p);
        }
    } else {
        if (instanced && !transforms) {
            e = createInstancedNonTransformablePiece(p);
        } else {
            e = createNonInstancedNonTransformablePiece(p);
        }
        if (e != nullptr && p.collision != "") {
            auto col = CollisionMeshLoader::load(p.collision.c_str());
            if (col != nullptr) {
	            CStaticBody* s = e->get<CStaticBody>();
                if (s == nullptr && transforms) {
                    e->add(getManager<CStaticBody>()->createObj());
                    s = e->get<CStaticBody>();
                }
                if (s != nullptr) {
                    s->setTriangleMesh(col);
                }
            }
        }
	}
    return e;
}

std::vector<LevelImport::wildcard_t> LevelImport::wildcards;
std::vector<LevelImport::pieceData_t> LevelImport::pieces;
std::map<std::string, unsigned> LevelImport::instancedPieceCount;

void LevelImport::generateLava(const MKeyValue& atts, const wildcard_t& wc)
{
    bool still = wc.tag2 == "still";

    currentEntity_h = PrefabManager::get().prefabricate(still? "smoke_still" : "smoke");
    assert(currentEntity_h.isValid());
    Entity* currentEntity(currentEntity_h);

    if (!still) {
        CSmokeTower* s = currentEntity->get<CSmokeTower>();
        s->setPlayer(playerEntity_h);
    }

    XMVECTOR size = wc.size == zero_v ? XMVectorSet(48,0,48,0): wc.size;
    float x = XMVectorGetX(size);
    float z = XMVectorGetZ(size);

    CTransform* t = currentEntity->get<CTransform>();
    t->setPosition(wc.transform.getPosition());
    t->setRotation(wc.transform.getRotation());

    CMesh* cmesh = currentEntity->get<CMesh>();
    Mesh* mesh = new Mesh;
    bool ok = createPlanePUNT(*mesh,x*.5f,z*.5f,x*.5f,z*.5f,-0.75f, 24.f);
    cmesh->setMesh(mesh);
    assert(ok);
    CMesh::key_t k;
    k.material = Material::getManager().getByName("lava_torre_sube");
    k.group0=0;
    k.groupf=0;
    k.selfIllumination = 0xFFA50000;
    cmesh->addKey(k);
    cmesh->init();

    CMist* mist = currentEntity->get<CMist>();
    mist->setWidth(x);
    mist->setLength(z);

    EntityListManager::get(CSmokeTower::TAG).add(currentEntity);
    currentEntity->init();
}

void LevelImport::onStartElement(const std::string &elem, utils::MKeyValue &atts)
{
    /* LEVEL */
    if (elem == "level") {
		currentLevel_h = getManager<Entity>()->createObj();
        Entity* lvl = currentLevel_h;
        lvl->add(getManager<CTransform>()->createObj());
        lvl->add(getManager<CLevelData>()->createObj());
        CName* name = getManager<CName>()->createObj();
        name->setName("LVL:"+atts.getString("name","<no-name>"));
        lvl->add(name);
        instancedPieces.clear();
	} else if (elem == "collision") {
        auto special = atts.getString("special", "");
        if (special == "") {
            assert(currentLevel_h.isValid());
            Entity* currentLevel(currentLevel_h);
            if (atts.has("mesh")) {
                Handle h = getManager<CStaticBody>()->createObj();
                CStaticBody* s(h);
                currentLevel->add(h);
                s->setTriangleMesh(CollisionMeshLoader::load(atts["mesh"].c_str()));
                s->setFilters(
                    filter_t::SCENE,
                    filter_t::id_t(filter_t::SCENE|filter_t::PROP|
                        filter_t::TOOL|filter_t::DESTRUCTIBLE | filter_t::PAINT_SPHERE),
                    filter_t::id_t(filter_t::BULLET|filter_t::FLARESHOT)
                    );
            }
        } else {
            specialCollisions[special] = atts;
        }
    } else if (elem == "piece") {
        utils::MKeyValue special;
        if (atts.has("special")) {special.setString("special", atts.getString("special"));}
        if (atts.has("hammer")) {special.setString("hammer", atts.getString("hammer"));}
        if (atts.has("weak_spot")) {special.setString("weak_spot", atts.getString("weak_spot"));}
        if (atts.has("spin")) {special.setString("spin", atts.getString("spin"));}
        if (atts.has("id")) {special.setString("id", atts.getString("id"));}

        std::string transformation = 
            atts.has("transformation2") ? atts["transformation2"] :
            atts.has("transformation")  ? atts["transformation"]  :
            "";
        std::string kit = atts["kit"];
        std::string filename = atts["filename"];
        std::string meshName = "PIECE-"+kit+"-"+filename;
        int spatialIndex = atts.getInt("spatialIndex", -1);
        XMVECTOR pos = atts.getPoint("pos");
        XMVECTOR rot = atts.getQuat("rot");
        bool instanced = atts.getBool("instanced", true);
        std::string collision = atts["collision"];
        int hits = atts.getInt("hits", 1);
        pieceData_t p(instanced, meshName, spatialIndex, transformation,
            collision, hits, pos, rot, std::move(special));
        pieces.push_back(p);
        if (instanced && special.empty()) {instancedPieceCount[meshName]++;}
    }
#ifdef ENABLE_ENEMIES
    /* ENEMIES */
	else if (elem == "enemy") {
        spatialIndex = atts.getInt("spatialIndex");
    	currentEntity_h = getManager<Entity>()->createObj();
		assert(currentEntity_h.isValid());
		Entity* currentEntity(currentEntity_h);

		Handle h = getManager<CTransform>()->createObj();
		currentEntity->add(h);
		CTransform* t(h);
		t->setPosition(atts.getPoint("pos"));
		t->setRotation(atts.getQuat("rot"));

        h = getManager<CRestore>()->createObj();
        currentEntity->add(h);
	    CRestore* restore = h;
        assert(restore != nullptr);
        currentEntity->get<CRestore>();
	    restore->setSpatialIndex(atts.getInt("spatialIndex", -1));

		EntityListManager::get(CEnemy::TAG).add(currentEntity_h);

        if (atts.has("tag")) {
            CLevelData* level = currentLevel_h.getSon<CLevelData>();
            level->setTaggedEntity(atts["tag"], currentEntity_h);
        }

	} else if (elem == "sniper") {
		PrefabManager::get().prefabricateComponents("components/sniper", currentEntity_h); //prefab will abort the entity
	} else if (elem == "flare") {
		PrefabManager::get().prefabricateComponents("components/flare", currentEntity_h);
	} else if (elem == "melee") {
		PrefabManager::get().prefabricateComponents("components/melee", currentEntity_h);
	}
#endif
	/* OBJECTS */
	else if (elem == "object") {
		currentEntity_h = getManager<Entity>()->createObj();
		assert(currentEntity_h.isValid());
		Entity* currentEntity(currentEntity_h);

		Handle h = getManager<CTransform>()->createObj();
		currentEntity->add(h);
		CTransform* t(h);
		t->setPosition(atts.getPoint("pos"));
		t->setRotation(atts.getQuat("rot"));

        h = getManager<CRestore>()->createObj();
        currentEntity->add(h);
	    CRestore* restore = h;
        assert(restore != nullptr);
        currentEntity->get<CRestore>();
	    restore->setSpatialIndex(atts.getInt("spatialIndex", -1));

    } else if (elem == "pickup") {
        std::string type = atts.getString("type", "<none>");
        if (type == "Health") {
			PrefabManager::get().prefabricateComponents("pickup/health-stationary", currentEntity_h);
        } else if (type == "Energy") {
			PrefabManager::get().prefabricateComponents("pickup/energy-stationary", currentEntity_h);
        } else if (type == "Invencible") {
			PrefabManager::get().prefabricateComponents("pickup/invincible-stationary", currentEntity_h);
		} else if (type == "Score") {
			PrefabManager::get().prefabricateComponents("pickup/coin-stationary", currentEntity_h);
		} else if (type == "Collectible") {
			PrefabManager::get().prefabricateComponents("pickup/collectable-stationary", currentEntity_h);
		}
        assert(currentEntity_h.isValid());

        Entity* currentEntity(currentEntity_h);
        CPickup* pickup = currentEntity->get<CPickup>();

		CTransform* t = currentEntity->get<CTransform>();        
		pickup->setStrength(atts.getFloat("strength", pickup->getStrength()));
		pickup->setup(t->getPosition(), zero_v, t->getRotation());
        EntityListManager::get(CPickup::TAG).add(currentEntity);
        
        if (atts.has("tag")) {
            CLevelData* level = currentLevel_h.getSon<CLevelData>();
            level->setTaggedEntity(atts["tag"], currentEntity_h);
        }

	} else if (elem == "trampoline") {
        assert(currentEntity_h.isValid());
        PrefabManager::get().prefabricateComponents("components/trampoline", currentEntity_h);
        Entity* currentEntity(currentEntity_h);
        CTrampoline* trampoline = currentEntity->get<CTrampoline>();
        assert(trampoline != nullptr);
        
        // Setup the trampoline
        CTransform* t = currentEntity->get<CTransform>();
        trampoline->setRotation(t->getRotation());
        
		CAABB* aabb = currentEntity->get<CAABB>();
        aabb->init();

		CTransformable* transformable = currentEntity->get<CTransformable>();
		transformable->setCenterAim(aabb->offsetAABB().getCenter());

        EntityListManager::get(CTrampoline::TAG).add(currentEntity);
        
    } else if (elem == "cannon") {
        PrefabManager::get().prefabricateComponents("components/cannon", currentEntity_h);
        assert(currentEntity_h.isValid());
        Entity* currentEntity(currentEntity_h);
        CCannon* cannon = currentEntity->get<CCannon>();
        assert (cannon != nullptr);

        // Setup the cannon
        CTransform* t = currentEntity->get<CTransform>();
        cannon->setRotation(t->getRotation());
        float yaw = getYawFromVector(t->getFront());
        t->setRotation(XMQuaternionRotationAxis(yAxis_v, yaw));
        cannon->setFov(atts.getFloat("fovH"), atts.getFloat("fovV"));
        cannon->setImpulse(atts.getFloat("impulse"));

		CAABB* aabb = currentEntity->get<CAABB>();
        aabb->init();

		CTransformable* transformable = currentEntity->get<CTransformable>();
		transformable->setCenterAim(aabb->offsetAABB().getCenter());

		EntityListManager::get(CCannon::TAG).add(currentEntity);
	
	} else if (elem == "liana") {
        PrefabManager::get().prefabricateComponents("components/liana", currentEntity_h);
        assert(currentEntity_h.isValid());
        Entity* currentEntity(currentEntity_h);
		CLiana* liana = currentEntity->get<CLiana>();
		liana->setNLinks((atts.getInt("nLinks", liana->getNLinks())));
        liana->setLimits(
            deg2rad(atts.getFloat("limitX", rad2deg(liana->getXLimit()))),
            deg2rad(atts.getFloat("limitZ", rad2deg(liana->getZLimit())))
            );

		CTransform* tran = currentEntity->get<CTransform>();
		CTransformable* transformable = currentEntity->get<CTransformable>();
		transformable->setCenterAim(tran->getPosition() - yAxis_v * 0.5f * (liana->getNLinks() / 2.f));
        EntityListManager::get(CLiana::TAG).add(currentEntity);
    } else if (elem == "creep") {
        
        PrefabManager::get().prefabricateComponents("components/creep", currentEntity_h);
        assert(currentEntity_h.isValid());
        Entity* currentEntity(currentEntity_h);
        CCreep* trampoline = currentEntity->get<CCreep>();
        assert(trampoline != nullptr);

        //Setup the creep
        CTransform* t = currentEntity->get<CTransform>();
        CCreep* creep = currentEntity->get<CCreep>();
        float w = atts.getFloat("w",1.f);
        float h = atts.getFloat("h",1.f);
        XMVECTOR normal = XMVector3Normalize(projectPlane(t->getUp(),xAxis_v,zAxis_v));
        float angle = XMVectorGetX(XMVector3AngleBetweenVectors(normal,t->getUp()));
        if (angle != 0) {
            XMVECTOR binormal = XMVector3Cross(normal, t->getUp());
            t->setRotation(XMQuaternionMultiply(t->getRotation(), XMQuaternionRotationAxis(binormal, angle)));
        }
        XMVECTOR knotPos = atts.getPoint("pos", t->getPosition());
        XMVECTOR diff = knotPos - t->getPosition();
        XMVECTOR projectedDiff = projectPlane(diff, normal);
        XMVECTOR projectedPos = t->getPosition() + projectedDiff;
        XMVECTOR relativePos = XMVector3Transform(projectedPos, XMMatrixInverse(nullptr, t->getWorld()));
        assert(abs(XMVectorGetY(relativePos)) <= 1e-3);
        XMFLOAT2 offset;
        XMStoreFloat2(&offset, relativePos);
        offset.x = XMVectorGetX(relativePos);
        offset.y = XMVectorGetZ(relativePos);
        creep->setup(normal, w, h, offset);

        //Creeps are created "onto the ground" then rotated (as specified in the 3DSMax Plane)
        float thick = 0.3f;
        auto sizeCreep(XMVectorSet(
            w-1.4f, //Approximation of 2*character's width,
            thick,
            h-0.5f, //margin to compensate climbing pose,
            0));
        auto sizePoster(XMVectorSet(2, thick, 4, 0));
        CTrigger* trigger(currentEntity->get<CTrigger>());
        trigger->setBox(sizeCreep);
        CStaticBody* staticBody(currentEntity->get<CStaticBody>());
        staticBody->setBox(sizePoster);
        Transform posterT;
        posterT.setPosition(-projectedDiff);
        staticBody->setPose(relativePos);

		CTransformable* transformable = currentEntity->get<CTransformable>();
		transformable->setCenterAim(projectedPos);
        EntityListManager::get(CCreep::TAG).add(currentEntity);

    } else if (elem == "checkPoint") {
        bool isOK = PrefabManager::get().prefabricateComponents("components/checkpoint", currentEntity_h);
        assert(isOK);
        Entity* currentEntity(currentEntity_h);

        Handle checkPoint_h(currentEntity->get<CCheckPoint>());
        CCheckPoint* checkPoint(checkPoint_h);
        checkPoint->setLevel(currentLevel_h);
        checkPoint->setPlayer(playerEntity_h);
		checkPoint->setOrder(atts.getInt("order", -1));

        if (atts.getBool("trigger")) {
            //Create new trigger
            XMVECTOR pos = atts.getPoint("pos");
            float halfw = atts.getFloat("width")/2;
            float halfl = atts.getFloat("length")/2;
            float height = atts.getFloat("height");

            XMVECTOR tPos = ((CTransform*)currentEntity->get<CTransform>())->getPosition();
            XMVECTOR offset = pos - tPos;
            XMVECTOR bbmin = XMVectorSet(-halfw,0,-halfl,0);
            XMVECTOR bbmax = XMVectorSet(halfw,height,halfl,0);

            Handle aabb_h(getManager<CAABB>()->createObj());
            currentEntity->add(aabb_h);
            CAABB* aabb(aabb_h);
            *aabb=CAABB(bbmin, bbmax)+offset;
        }

        if (atts.getBool("isSpawn")) {
            //Set level spawn
            assert(currentLevel_h.isValid());
            Entity* currentLevel(currentLevel_h);
            CLevelData* level(currentLevel->get<CLevelData>());
            level->setSpawnCheckPoint(checkPoint_h);
            level->setCurrentCheckPoint(checkPoint_h);
        } 
        EntityListManager::get(Trigger_AABB_TAG).add(currentEntity);
    } else if(elem == "trigger") {
        currentEntity_h = getManager<Entity>()->createObj();
        assert(currentEntity_h.isValid());
        Entity* currentEntity(currentEntity_h);
        Handle h;
        
        h = getManager<CTransform>()->createObj();
        currentEntity->add(h);
        CTransform* t(h);
        t->setPosition(atts.getPoint("pos"));

        h = getManager<CScriptTrigger>()->createObj();
        currentEntity->add(h);
        CScriptTrigger* script(h);
        script->setScript(atts["script"]);
        script->setArgs(atts["args"]);
        script->setPlayer(playerEntity_h);

        float halfw = atts.getFloat("width")/2;
        float halfl = atts.getFloat("length")/2;
        float height = atts.getFloat("height");
        
        Handle aabb_h(getManager<CAABB>()->createObj());
        currentEntity->add(aabb_h);
        CAABB* aabb(aabb_h);
        *aabb=CAABB(XMVectorSet(-halfw,0,-halfl,0), XMVectorSet(halfw,height,halfl,0));
        
        EntityListManager::get(Trigger_AABB_TAG).add(currentEntity);

    }  else if(elem == "spatial") {
        currentEntity_h = getManager<Entity>()->createObj();
        assert(currentEntity_h.isValid());
        Entity* currentEntity(currentEntity_h);
        Handle h;
        
        h = getManager<CTransform>()->createObj();
        currentEntity->add(h);
        CTransform* t(h);
        t->setPosition(atts.getPoint("pos"));

        h = getManager<CSpatialIndex>()->createObj();
        currentEntity->add(h);
        CSpatialIndex* si(h);
        si->setPlayer(playerEntity_h);
        si->setSpatialIndex(atts.getInt("spatialIndex",-1));

        float halfw = atts.getFloat("width")/2;
        float halfl = atts.getFloat("length")/2;
        float height = atts.getFloat("height");
        
        Handle aabb_h(getManager<CAABB>()->createObj());
        currentEntity->add(aabb_h);
        CAABB* aabb(aabb_h);
        *aabb=CAABB(XMVectorSet(-halfw,0,-halfl,0), XMVectorSet(halfw,height,halfl,0));
        
        EntityListManager::get(Trigger_AABB_TAG).add(currentEntity);

    } else if(elem == "destructible") {

        currentEntity_h = PrefabManager::get().prefabricate("destructible");
        Entity* currentEntity = currentEntity_h;
        CTransform* t = currentEntity->get<CTransform>();
        t->setPosition(atts.getPoint("pos", zero_v));
        t->setRotation(atts.getPoint("rot", one_q));
        float width = std::max(atts.getFloat("width"), 0.01f);
        float length = std::max(atts.getFloat("length"), 0.01f);
        float height = std::max(atts.getFloat("height"), 0.01f);
        XMVECTOR size = XMVectorSet(width, height, length,0);
        t->refPosition() += t->getUp()*height/2;

        CStaticBody* s = currentEntity->get<CStaticBody>();
        CTrigger* trigger = currentEntity->get<CTrigger>();
        s->setBox(size);
        trigger->setBox(size);

        CDestructible* destructible = currentEntity->get<CDestructible>();
        destructible->createBox(size);

    } else if (elem == "smoke") {
        float width = std::max(atts.getFloat("width"), 0.01f);
        float length = std::max(atts.getFloat("length"), 0.01f);
        float height = std::max(atts.getFloat("height"), 0.01f);

        XMVECTOR size = XMVectorSet(width, height, length,0);

        auto index = atts.getInt("index",-1);
        auto subindex = atts.getInt("subindex", -1);

        currentEntity_h = PrefabManager::get().prefabricate(
            index == 0 ? "boss/smoke-feet" : "boss/smoke-panel");

        Entity* currentEntity = currentEntity_h;
        CTransform* t = currentEntity->get<CTransform>();
        t->setPosition(atts.getPoint("pos", zero_v));
        t->setRotation(atts.getQuat("rot", one_q));

		Transform triggerPose;
        triggerPose.refPosition() += t->getUp()*height/2;

        CTrigger* trigger = currentEntity->get<CTrigger>();
        trigger->setBox(size);
		trigger->setPose(triggerPose);

        CSmokePanel* smokePanel = currentEntity->get<CSmokePanel>();
        smokePanel->set(index-1, subindex-1);

        EntityListManager::get(
            index==0 ? CSmokePanel::TAG_ALWAYSHOT : CSmokePanel::TAG
            ).add(currentEntity);
        
    } else if(elem == "whitebox") {
        currentEntity_h = PrefabManager::get().prefabricate("whitebox");
        Entity* currentEntity = currentEntity_h;
            
        CTransform* t = currentEntity->get<CTransform>();
        t->setPosition(atts.getPoint("pos"));
        t->setRotation(atts.getQuat("rot"));
        float width = std::max(atts.getFloat("width"), 0.01f);
        float length = std::max(atts.getFloat("length"), 0.01f);
        float height = std::max(atts.getFloat("height"), 0.01f);
        XMVECTOR size = XMVectorSet(width, height, length,0);
        t->refPosition() += t->getUp()*height/2;
        
        Color tint = atts.has("color")? XMVectorSetW(atts.getPoint("color")/256,1) : Color::WHITE;
        
        CStaticBody* s = currentEntity->get<CStaticBody>();
        s->setBox(size);
        
        CWhiteBox* wb = currentEntity->get<CWhiteBox>();
        wb->createBox(size, tint);
    } else if (elem == "wildcardBox"){
        Transform t;
        t.setPosition(atts.getPoint("pos", t.getPosition()));
        t.setRotation(atts.getQuat("rot", t.getRotation()));
        float width = std::max(atts.getFloat("width"), 0.01f);
        float height = std::max(atts.getFloat("height"), 0.01f);
        float length = std::max(atts.getFloat("length"), 0.01f);
        std::string tag1 = atts.getString("tag1");
        std::string tag2 = atts.getString("tag2");
        std::string tag3 = atts.getString("tag3");
        wildcard_t wc(t, tag1, tag2, tag3, XMVectorSet(width,height,length,0));
        wildcards.push_back(wc);

        if (tag1 == "lava") {generateLava(atts, wc);} 
    }  else if (elem == "wildcard"){
        Transform t;
        t.setPosition(atts.getPoint("pos", t.getPosition()));
        t.setRotation(atts.getQuat("rot", t.getRotation()));
        std::string tag1 = atts.getString("tag1");
        std::string tag2 = atts.getString("tag2");
        std::string tag3 = atts.getString("tag3");
        wildcard_t wc(t, tag1, tag2, tag3);
        wildcards.push_back(wc);
        if (tag1 == "lava") {generateLava(atts, wc);} 
    }
}

void LevelImport::setupHammer(Entity* e, const pieceData_t& p)
{
    PrefabManager::get().prefabricateComponents("boss/hammer", e);
    EntityListManager::get(CBoss::HAMMER_TAG).add(e);
    CAABB* aabb = e->get<CAABB>();
    aabb->init();
    CTransform* t = e->get<CTransform>();
    CTransformable* tr = e->get<CTransformable>();
    tr->setCenterAim(aabb->getCenter() + t->getPosition());
}

void LevelImport::setupWeakSpot(Entity* e, const pieceData_t& p)
{
    PrefabManager::get().prefabricateComponents("boss/weak-spot", e);
    CStaticBody* body = e->get<CStaticBody>();
    assert(body != nullptr);
    body->setTriangleMesh(CollisionMeshLoader::load(p.collision.c_str()));
    EntityListManager::get(CWeakSpot::TAG).add(e);
}

gameElements::CBoss* LevelImport::setupBoss(Entity* e, const pieceData_t& p)
{
    CLevelData* level = currentLevel_h.getSon<CLevelData>();
    level->setBossLevel();

    bossEntity_h = e;
    PrefabManager::get().prefabricateComponents("boss/boss", bossEntity_h);
    CTransform* bossT = e->get<CTransform>();
    CStaticBody* body = e->get<CStaticBody>();
    assert(body != nullptr);
    
    auto i = specialCollisions.find("boss");
    std::string colName;
    XMVECTOR colPos=zero_v;
    XMVECTOR colRot=one_q;
    if (i != specialCollisions.end()) {
        const auto& colAtts = i->second;
        colName = colAtts.getString("mesh");
        colPos = colAtts.getPoint("pos")-bossT->getPosition();
        colRot = XMQuaternionMultiply(colAtts.getQuat("rot"),
            XMQuaternionInverse(bossT->getRotation()));
    } else {
        colName =  p.collision;
    }

    body->setTriangleMesh(CollisionMeshLoader::load(colName.c_str()));

    Transform colT;
    colT.setPosition(colPos);
    colT.setRotation(colRot);
    body->createStaticBody();
    body->getShape()->setLocalPose(toPxTransform(colT.getWorld()));
    CBoss* boss = e->get<CBoss>();
    
    wildcard_t cannonTop[3];
    wildcard_t cannonBottom[3];
    
    for(const auto& w : wildcards) {
        if (w.tag1 == "mark") {
            int n  = atoi(w.tag3.c_str())-1;
            if (w.tag2 == "cannon") {
                assert (n >= 0 && n <3);
                cannonTop[n] = w;
            } else if (w.tag2 == "cannon ground") {
                assert (n >= 0 && n <3);
                cannonBottom[n] = w;
            } else if (w.tag2 == "spawner") {
                if (n > 0 && n <=12) {
                    boss->addSpawn(w.transform, n-1);
                }
            }
        } else if (w.tag1 == "altura" && w.tag2 == "hammer" ) {
            boss->setHammerY(XMVectorGetY(w.transform.getPosition()));
        }
    }
    Transform cannonTopT[3] = {cannonTop[0].transform,
        cannonTop[1].transform, cannonTop[2].transform};
    Transform cannonBottomT[3] = { cannonBottom[0].transform,
        cannonBottom[1].transform, cannonBottom[2].transform};
    boss->setMarks(cannonTopT, cannonBottomT);
    EntityListManager::get(CBoss::TAG).add(e);

	

    return boss;
}

struct bossSpinnerData_t {
    Handle h;
    float spin = 1;

    bossSpinnerData_t(Handle h, float spin = 1) : h(h), spin(spin) {}
};

void LevelImport::onEndElement (const std::string &elem)
{
    /* LEVEL */
    if (elem == "level") {
        assert(currentLevel_h.isValid());
        
        // Create the pieces
        Handle weak_spots[3];
        Handle hammers[3];
        std::vector<bossSpinnerData_t> bossSpinners;

        for(const auto& p : pieces) {
            Entity* e = createPiece(p);

            //Do special things
            if (p.special.getString("special") == "weak spot") {
                int n = p.special.getInt("id");
                assert(n > 0 && n <= 3);
                setupWeakSpot(e, p);
                weak_spots[n-1] = e;
                e->init();
                if (bossEntity_h.isValid()) {
                    CBoss* boss = bossEntity_h.getSon<CBoss>();
                    assert(boss != nullptr);
                    boss->setWeakSpot(e, n-1);
                }
            } else if (p.special.getString("special") == "hammer") {
                int n = p.special.getInt("id");
                assert(n > 0 && n <= 3);
                setupHammer(e, p);
                hammers[n-1] = e;
                e->init();
                if (bossEntity_h.isValid()) {
                    CBoss* boss = bossEntity_h.getSon<CBoss>();
                    assert(boss != nullptr);
					boss->setHammer(e, n - 1);
                }
            } else if (p.special.getString("special") == "boss") {
                CBoss* boss = setupBoss(e, p);
                for(int i=0; i<3; ++i) {
                    if (weak_spots[i].isValid()) {boss->setWeakSpot(weak_spots[i], i);}

                    if (hammers[i].isValid()) {
						boss->setHammer(hammers[i], i);
					}
                }
                for (const auto& s : bossSpinners) {
                    boss->addSpinner(s.h, s.spin);
                }
                e->init();
            } else if (p.special.has("spin")) {
                float spin = p.special.getFloat("spin");
                bossSpinners.push_back(bossSpinnerData_t(e,spin));
                if (bossEntity_h.isValid()) {
                    CBoss* boss = bossEntity_h.getSon<CBoss>();
                    assert(boss != nullptr);
                    boss->addSpinner(e, spin);
                }
                e->init();
            } else if (e != nullptr) {
                e->init();
            }
        }

        // Commit the instanced pieces
        for(const auto& p : instancedPieces) {
            Entity* instancedEntity(p.second);
            assert(instancedEntity != nullptr);
            CInstancedMesh* instancedMesh = instancedEntity->get<CInstancedMesh>();
            assert(instancedMesh != nullptr);
            if (instancedMesh->getNInstances() == 0) {
                CMesh* mesh = instancedEntity->get<CMesh>();
                if (mesh->hasRandomMaterials()) {
                    instancedEntity->destroy();
                    continue;
                }
            }
            instancedMesh->commitInstances();
            instancedEntity->init();
            EntityListManager::get(TAG_SCENE).add(instancedEntity);
        }

        instancedPieces.clear();
        pieces.clear();
        wildcards.clear();
        instancedPieceCount.clear();
        
        currentLevel_h.init();
        previousLevel_h = currentLevel_h;
        currentLevel_h = Handle();
    }
    
#ifdef ENABLE_ENEMIES
	/* ENEMIES */
	else if (elem == "enemy") {
		if(currentEntity_h.isValid()) {
		    Entity* e(currentEntity_h);

		    //Move up the enemy to avoid collisions with the ground
		    CTransform* transform = e->get<CTransform>();
		    transform->setPosition(transform->getPosition() + XMVectorSet(0, 0.45f, 0, 0));
            
		    CSkeleton* skel = e->get<CSkeleton>();
            skel->setSpatialIndex(spatialIndex);
            spatialIndex=-1;

            currentEntity_h.init();
        }
        currentEntity_h = Handle();
    }
#endif    
    /* OBJECTS */
    else if (elem == "object"
         ||  elem == "trigger"
         ||  elem == "whitebox"
         ||  elem == "destructible"
         ||  elem == "smoke"
         ) {
        assert(currentEntity_h.isValid());
        currentEntity_h.init();
        currentEntity_h = Handle();
        spatialIndex=-1;
    }
}

component::Handle LevelImport::load(const char* name, Handle player_h)
{
    playerEntity_h = player_h;
    currentLevel_h = Handle();
    currentEntity_h = Handle();
    previousLevel_h = Handle();
    bossEntity_h = Handle();
    spatialIndex=-1;
    std::stringstream ss;
    ss << "data/levels/" << name << ".xml";
    LevelImport importer;
    bool isOk = importer.xmlParseFile(ss.str());
    assert(isOk && previousLevel_h.isValid());
    ((Entity*) previousLevel_h)->init();
    MessageManager::dispatchPosts();
    CLevelData* ld = previousLevel_h.getSon<CLevelData>();
    assert(ld != nullptr);
    CLevelData::currentLevel = ld;

    return previousLevel_h;
}

}
