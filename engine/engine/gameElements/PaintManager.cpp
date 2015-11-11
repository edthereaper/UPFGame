#include "mcv_platform.h"
#include "PaintManager.h"

#include "render/render_utils.h"
using namespace render;
using namespace DirectX;
using namespace component;
using namespace utils;

using namespace physx;
using namespace physX_user;

#define PHYSX (physX_user::PhysicsManager::get().gPhysicsSDK)
//#define DEBUG_PAINT
//#define DEBUG_NTESTS

namespace gameElements {

const float CPaintGroup::RADIUS_TOLERANCE = 0.001f;
const float CPaintGroup::DECAY_TOLERANCE = 0.001f;
const unsigned CPaintGroup::N_RAYCAST_TESTS_DOWN = 60;
const unsigned CPaintGroup::N_RAYCAST_TESTS_UP = 45;
const unsigned CPaintGroup::N_RAYCAST_TESTS = N_RAYCAST_TESTS_DOWN + N_RAYCAST_TESTS_UP;
const unsigned CPaintGroup::TRY_REMOVAL_THRESHOLD = N_RAYCAST_TESTS;
unsigned CPaintGroup::nTests = 0;
unsigned CPaintGroup::nGroupRemoval = 0;
unsigned CPaintGroup::nPaintRemoval = 0;

void CPaintGroup::fixedUpdate(float)
{
    auto nTestsPrev = nTests;
#if defined(_DEBUG) && defined(DEBUG_NTESTS)
    auto nTestsDBG = nTests;
    dbg("%d + ", nTestsDBG);
#endif
    while(nTests <= TRY_REMOVAL_THRESHOLD) {
        tryRemoval();
        if (nTestsPrev == nTests) {break;}
        nTestsPrev = nTests;
    }
    
#if defined(_DEBUG) && defined(DEBUG_NTESTS)
    static auto maxTests = 0;
    if (nTests > maxTests) {maxTests = nTests;}
    dbg("%d = %d raycasts this physx frame (max %d)\n", nTests-nTestsDBG, nTests);

#endif
    nTests = 0;
}

void CPaintGroup::tryRemoval()
{
    auto manager = getManager<CPaintGroup>();
    auto nPaintGroups = manager->getSize();
    if (nPaintGroups > 0) {
        nGroupRemoval = nGroupRemoval % nPaintGroups;
        CPaintGroup* pg = manager->getNth(nGroupRemoval);
        auto nPaints = pg->nInstances;
        if (nPaintRemoval >= nPaints) {
            nPaintRemoval = 0;
            ++nGroupRemoval;
            nGroupRemoval = nGroupRemoval % nPaintGroups;
            pg = manager->getNth(nGroupRemoval);
            nPaints = pg->nInstances;
        }
        if (nPaints > 0) {
            //On removal, the last sphere has replaced the slot of this one: don't advance nPaintRemoval
            //nPaintRemoval > nPaints will be dealt with next frame
            if (!pg->tryRemoval(nPaintRemoval)) {
                ++nPaintRemoval; 
            }
        }
    }
}

void CPaintGroup::removeInstance(unsigned n)
{
    assert(n < nInstances);
    --nInstances;
    auto shape = (*shapes)[n];
    if (n != nInstances) {
        //Last sphere replaces this one
        //(otherwise the last sphere slot is trash)
        (*buffer)[n] = (*buffer)[nInstances];
        (*shapes)[n] = (*shapes)[nInstances];
        (*shapes)[n]->userData = (void*)n;
    }

    (*buffer)[nInstances] = instance_t();
    (*shapes)[nInstances] = nullptr;
    dirty = true;
    
    shape->getActor()->detachShape(*shape);
}

bool CPaintGroup::tryRemoval(unsigned n)
{
    assert(n < nInstances);
    assert(buffer != nullptr);
    auto& s = (*buffer)[n];
    float y = XMVectorGetY(s.getPos()) + s.radius;
    if (y <= PaintManager::getFireLevel()) {
        removeInstance(n);
        return true;
    } else if (testRays(s.getPos(), s.radius, n) == REDUNDANT) {
        removeInstance(n);
        return true;
    } else {
        return false;
    }
}

void CPaintGroup::init()
{
    static const PxMaterial*const PHYSIX_MAT = PHYSX->createMaterial(0.5,0.5,0.5);
    instanceData = new Mesh;

    buffer = new std::vector<instance_t>;
    shapes = new std::vector<PxShape*>;
    buffer->resize(maxInstances);
    shapes->resize(maxInstances);
    for (auto& a : *shapes) {a=nullptr;}
    bool isOK = instanceData->create(
        unsigned(maxInstances), buffer->data(), 0, nullptr,
        Mesh::POINTS, getVertexDecl<VertexPaintData>(), 1 /*instance stream*/,
        utils::zero_v, utils::zero_v, true);
    assert(isOK);
    dirty = false;
     
    auto flags =
#if defined(_DEBUG) && defined(DEBUG_PAINT)
        PxActorFlag::eDISABLE_SIMULATION | PxActorFlag::eVISUALIZATION;
#else 
        PxActorFlag::eDISABLE_SIMULATION;
#endif

    actor = PHYSX->createRigidStatic(PxTransform(PxIdentity));
    physX_user::PhysicsManager::get().gScene->addActor(*actor);
    assert(actor != nullptr);
    actor->setActorFlags(flags);
    actor->userData = Handle(this).getOwner().getRawAsVoidPtr();
#ifdef _DEBUG
    Entity* e = Handle(this).getOwner();
    actor->setName((e->getName()+".PAINT").c_str());
#endif
    
    globalQueryShape = PHYSX->createShape(
        PxSphereGeometry(1), *PHYSIX_MAT, true, PxShapeFlag::eSCENE_QUERY_SHAPE);
}

void CPaintGroup::drawAABB()
{
    if (!aabb.isInvalid()) {
        setObjectConstants(
            XMMatrixAffineTransformation(2*aabb.getHSize(),
            zero_v, one_q, aabb.getCenter()-aabb.getHSize()), color);
        mesh_cube_wire.activateAndRender();
    }
}

bool CPaintGroup::testInside(instance_t s, XMVECTOR pos, float radius)
{
    float d = XMVectorGetX(XMVector3Length(pos-s.getPos()));
    return d+radius <= s.radius + RADIUS_TOLERANCE;
}

void CPaintGroup::drawVolume()
{
    if (nInstances > 0) {
        if (dirty) {updateInstanceData();}
        activatePaintGroup(this);

        static Mesh* const smallIcosaedron = new Mesh;
        static const bool ok = createIcosahedronWireFrame(*smallIcosaedron, 0.01f);
        assert(ok);
        
        //Very low alpha (useful to find multiple spheres generating in the same spot)
        setObjectConstants(XMMatrixIdentity(), Color(Color::RED).setAf(0.015f));
        mesh_icosahedron_wire.renderInstanced(*instanceData, nInstances);
        setObjectConstants(XMMatrixIdentity(), Color(Color::YELLOW).setAf(0.8f));
        smallIcosaedron->renderInstanced(*instanceData, nInstances);
    }
}

void CPaintGroup::drawVolumeChain()
{
    drawVolume();
    if (chain.isValid()) {
        CPaintGroup* chained = chain;
        chained->drawVolume();
    }
}


void CPaintGroup::draw()
{
    if (nInstances > 0) {
        if (dirty) {updateInstanceData();}
        activatePaintGroup(this);
        mesh_icosahedron.renderInstanced(*instanceData, nInstances);
    }
}

void CPaintGroup::drawChain()
{
    draw();
    if (chain.isValid()) {
        CPaintGroup* chained = chain;
        chained->draw();
    }
}

void CPaintGroup::updateInstanceData()
{
    assert(buffer != nullptr);
    if (nInstances > 0) {
        instanceData->updateFromCPU(buffer->data(), sizeof(instance_t) * nInstances);
    }
    dirty = false;
}


void CPaintGroup::addInstanceImpl(const XMVECTOR& pos, float radius)
{
    static const filter_t filter = filter_t(filter_t::PAINT_SPHERE,
        filter_t::ALL_IDS, filter_t::NONE, filter_t::STATIC);
    static const PxShapeFlags flags =
#if defined(_DEBUG) && defined(DEBUG_PAINT)
        PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eVISUALIZATION;
#else 
        PxShapeFlag::eSCENE_QUERY_SHAPE;
#endif

    assert(buffer != nullptr);
    assert(nInstances < buffer->size());

    auto& p = (*buffer)[nInstances];
    p.set(pos, radius);
    dirty = true;
    auto diagonal = XMVectorSet(radius, radius, radius, 1);
    aabb.expand(pos-diagonal, pos+diagonal);

    static const PxMaterial*const PHYSIX_MAT = PHYSX->createMaterial(0.5,0.5,0.5);
    auto shape = PHYSX->createShape(
        PxSphereGeometry(radius), *PHYSIX_MAT, true, PxShapeFlag::eSCENE_QUERY_SHAPE);
    actor->attachShape(*shape);
    assert(shape != nullptr);
    shape->setLocalPose(PxTransform(toPxVec3(pos)));
	shape->setSimulationFilterData(filter);
	shape->setQueryFilterData(filter);
    shape->userData = (void*)nInstances;
    
#ifdef _DEBUG
    Entity* e = Handle(this).getOwner();
    shape->setName((e->getName()+".PAINT"+std::to_string(nInstances)).c_str());
#endif
    (*shapes)[nInstances] = shape;

    ++nInstances;
}

CPaintGroup::rayTestResult_e CPaintGroup::testRays(const XMVECTOR& pos, float radius, unsigned ignore)
{
    //Variation of Saff and Kuijlaars's method of getting evenly distributed points on a sphere
    static const float dl = M_PIf*(3.f - sqrt(5.f));
    static const float dzD = 1.0f / N_RAYCAST_TESTS_DOWN;
    static const float dzU = 1.0f / N_RAYCAST_TESTS_UP;
    float l = 0;
    float z = 1 - 1/N_RAYCAST_TESTS;
    
    auto ret = NO_PAINTABLE;
    for (unsigned i=0; i<N_RAYCAST_TESTS; ++i, l += dl) {
        //This forms a spiral on the surface of a sphere od radius=1z
        //So, we use z as out y coordinate to advance on it
        //Also, we negate n to start looking in the south hemisphere
        float r = sqrt(1-sq(z));
        PxVec3 p(std::cos(l)*r, -z, std::sin(l)*r);
        nTests++;
        switch (testDirection(pos, p, radius, ignore)) {
            case NEW_PAINT: return NEW_PAINT; break;
            case REDUNDANT: ret = REDUNDANT; break;
        }
        //Different speeds in each hemisphere
        z -= (i < N_RAYCAST_TESTS_DOWN) ? dzD : dzU;
    }
    return ret;
}

CPaintGroup::rayTestResult_e CPaintGroup::testInstance(const XMVECTOR& pos, float radius)
{
    if (radius <= 0.f) {return NO_PAINTABLE;}
    static const PxMaterial*const PHYSIX_MAT = PHYSX->createMaterial(0.5,0.5,0.5);
    
    static const auto QUERY =  PxQueryFilterData(
        filter_t(filter_t::NONE, filter_t::NONE, filter_t::SCENE),
        PxQueryFlag::eANY_HIT|PxQueryFlag::eSTATIC);

    globalQueryShape->setGeometry(PxSphereGeometry(radius));
    if( PhysicsManager::get().overlap(globalQueryShape, PxTransform(toPxVec3(pos)), QUERY) ) {
        assert(buffer);
        if (nInstances > 0 && testInside((*buffer)[nInstances-1], pos, radius)) {
            // Trying to create the last sphere again!
            return REDUNDANT;
        } else {
            return testRays(pos, radius);
        }
    } else {
        return NO_PAINTABLE;
    }
}

float CPaintGroup::calculateShaderAmount(float radius, float dist, float decay, float intensity)
{
    return utils::inRange(0.f, ((radius - dist) / (radius * (1 - decay))), 1.f)*intensity;
}

CPaintGroup::rayTestResult_e CPaintGroup::testDirection(
    const XMVECTOR& pos, const PxVec3& dir, float radius, unsigned ignore)
{
    static auto const f = filter_t::id_t(filter_t::SCENE | filter_t::PAINT_SPHERE);
    static auto const qfd = PxQueryFilterData(filter_t(filter_t::NONE, ~f, f),
        PxQueryFlag::eNO_BLOCK | PxQueryFlag::eSTATIC );
    
    static PxRaycastHit buffer[128];
    PxRaycastBuffer hits(buffer, 128);
    auto& physx = PhysicsManager::get();
    
    assert(dir.isNormalized());

    physx.gScene->raycast(toPxVec3(pos), dir, radius+RADIUS_TOLERANCE, hits,
        PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION, qfd);
    
    bool gotStatic = false;
    PxVec3 paintPos(0);
    float paintDist = radius*2;

    struct sphere_t{
        instance_t sphere;
        float intensity;
        float decay;
        unsigned sphereId;
        sphere_t()=default;
        sphere_t(instance_t sphere, float intensity, float decay, unsigned sphereId) :
            sphere(sphere), intensity(intensity), decay(decay), sphereId(sphereId) {}
    };

    std::vector<sphere_t> spheres;

    for (unsigned i=0; i < hits.getNbTouches(); ++i) {
        //find nearest collision
        const auto& hit = hits.getTouch(i);
        const filter_t& filter = hit.shape->getQueryFilterData();
        if (filter.is & filter_t::SCENE) {
            auto newDist = hit.distance;
            if (newDist < paintDist) {
                paintDist = newDist;
                paintPos = hit.position;
                gotStatic = true;
            } 
        } else if (filter.is & filter_t::PAINT_SPHERE) {
            //Add spheres to list
            Entity* e = Handle::fromRaw(hit.actor->userData);
            if (e != nullptr){
                CPaintGroup* paintGroup = e->get<CPaintGroup>();
                unsigned instanceN = unsigned(hit.shape->userData);
                bool ignored = instanceN == ignore;
                if (paintGroup != nullptr && paintGroup == this && !ignored) {
                    const auto& buffer = paintGroup->buffer;
                    if (buffer != nullptr && instanceN < paintGroup->nInstances) {
                        auto s = (*buffer)[instanceN];
                        if (testInside(s, pos, radius)) {
                            return REDUNDANT;
                        } else {
                            spheres.push_back(
                                sphere_t(s, paintGroup->intensity, paintGroup->decay, instanceN));
                        }
                    } 
                }
            }
        }
    }

    if (gotStatic) {
        //How much do I want to paint here? (paint.fx::PSPaint::paintAmount)
        float paintAmount = calculateShaderAmount(radius, paintDist, decay, intensity);

        //See if one of the spheres already paints this spot
        XMVECTOR paintPosXM = toXMVECTOR(paintPos);
        for (const auto& s : spheres) {
            auto spherePos = s.sphere.getPos();
            auto dist = XMVectorGetX(XMVector3Length(paintPosXM-spherePos));
            if (dist < s.sphere.radius) {
                //Are you painting more than me?
                float sPaintDist = XMVectorGetX(XMVector3Length(spherePos - paintPosXM));
                float sPaintAmount = calculateShaderAmount(
                    s.sphere.radius, sPaintDist, s.decay, s.intensity);
                if (sPaintAmount + DECAY_TOLERANCE >= paintAmount) {
                    return REDUNDANT;
                }
            }
        }
        return NEW_PAINT;
    } else {
        //No place to paint
        return NO_PAINTABLE;
    }
}

bool CPaintGroup::addInstanceAux(const XMVECTOR& pos, float radius)
{
    if (nInstances < maxInstances) {
        addInstanceImpl(pos, radius);
        return true;
    } else if (chain.isValid()){
        CPaintGroup* chained = chain;
        return chained->addInstanceAux(pos, radius);
    } else {
        chain = getManager<CPaintGroup>()->createObj();
        CPaintGroup* chained = chain;
        chained->maxInstances = maxInstances;
        chained->color = color;
        chained->decay = decay;
        chained->intensity = intensity;
        chained->siIntensity = siIntensity;
        chained->chainDepth = chainDepth+1;
        chained->init();
        return chained->addInstanceAux(pos, radius);
    }
}

PaintManager::map_t PaintManager::brushes;
unsigned PaintManager::currentId = ~0;

void PaintManager::onStartElement(const std::string &elem, utils::MKeyValue &atts)
{
    if (elem == "paintManager") {
        return;
    } else if (elem == "entry") {
        unsigned i = atts.getInt("id");
        assert(brushes.find(i) == brushes.end());
        currentId = i;
        Entity* e = brushes[i] = getManager<Entity>()->createObj();
        e->add(getManager<CPaintGroup>()->createObj());
    } else {
        assert(brushes.find(currentId) != brushes.end());
        CPaintGroup* pg = getBrush(currentId);
        pg->loadFromProperties(elem, atts);
    }
}

void PaintManager::onEndElement(const std::string &elem)
{
    if (elem == "entry") {
        assert(brushes.find(currentId) != brushes.end());
        Entity* e = brushes[currentId];
        e->init();
    }
}

bool PaintManager::aborted = false;
float PaintManager::fireLevel = -1000000;

}