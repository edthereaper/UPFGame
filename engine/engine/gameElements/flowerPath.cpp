#include "mcv_platform.h"
#include "flowerPath.h"

#include "utils/utils.h"
#include "utils/data_provider.h"
#include "utils/data_saver.h"
#include "level/level.h"
#include "render/render_utils.h"
#include "PhysX_USER/PhysicsManager.h"

using namespace DirectX;
using namespace utils;
using namespace level;
using namespace render;
using namespace component;

using namespace physx;
using namespace physX_user;

#define PHYSX physX_user::PhysicsManager::get()

namespace gameElements {

const float FlowerGroup::QUAD_SIZE = 0.4f;
const float FlowerPathManager::COS_ANGLE_THRESHOLD = std::cos(deg2rad(10));

FlowerPathManager::sproutCoordV_t FlowerPathManager::xCoords;
FlowerPathManager::sproutCoordV_t FlowerPathManager::yCoords;
FlowerPathManager::sproutCoordV_t FlowerPathManager::zCoords;
Transform FlowerPathManager::lastTest;
bool FlowerPathManager::lastTestActive = false;
FlowerPathManager::flowers_t FlowerPathManager::flowers;
std::vector<bool> FlowerPathManager::active;

std::vector<XMVECTOR> FlowerPathManager::getNewInCyllinder(const XMVECTOR& pos, float radius, float h)
{
    float x = XMVectorGetX(pos);
    float y = XMVectorGetY(pos);
    float z = XMVectorGetZ(pos);

    float xMin = x - radius;
    float xMax = x + radius;
    float yMin = y;
    float yMax = y + h;
    float zMin = z - radius;
    float zMax = z + radius;
    
    //Find range in each coordinate
    sproutCoordV_t xRange(
        std::lower_bound(xCoords.begin(), xCoords.end(), xMin),
        std::upper_bound(xCoords.begin(), xCoords.end(), xMax)
    );
    sproutCoordV_t yRange(
        std::lower_bound(yCoords.begin(), yCoords.end(), yMin),
        std::upper_bound(yCoords.begin(), yCoords.end(), yMax)
    );
    sproutCoordV_t zRange(
        std::lower_bound(zCoords.begin(), zCoords.end(), zMin),
        std::upper_bound(zCoords.begin(), zCoords.end(), zMax)
    );

    //Sort ranges by id
    std::sort(xRange.begin(), xRange.end(), &sproutCoord::compById);
    std::sort(yRange.begin(), yRange.end(), &sproutCoord::compById);
    std::sort(zRange.begin(), zRange.end(), &sproutCoord::compById);

    unsigned i=0;
    unsigned j=0;
    unsigned k=0;
    size_t xSize = xRange.size(); 
    size_t ySize = yRange.size(); 
    size_t zSize = zRange.size();

    std::vector<XMVECTOR> ret;

    for(; i < xSize; ++i) {
        unsigned xId (xRange[i].id);

        //advance the y and z
        while (j < ySize && yRange[j].id < xId) {j++;}
        if (j >= ySize) {break;}
        while (k < zSize && zRange[k].id < xId) {k++;}
        if (k >= zSize) {break;}

        //We either got a match or not. Also test for already active sprouts
        if (yRange[j].id == xId && zRange[k].id == xId && !active[xId]) {
            //index is in all three vectors => it's within the cyllinder's AABB
            float xVal = xRange[i].val;
            float zVal = zRange[k].val;
            //test circle radius
            if (sq(xVal-x) + sq(zVal-z) <= sq(radius)) {
                ret.push_back(XMVectorSet(xVal, yRange[j].val, zVal, 1.f));
                active[xId] = true;
            }
        }
    }

    lastTest.setPosition(pos);
    lastTest.setScale(XMVectorSet(radius, h, radius, 1));
    lastTestActive = true;
    return ret;
}

void FlowerPathManager::plantCyllinder(const XMVECTOR& pos, float radius, float h, int spatialIndex)
{
    auto newFlowers = getNewInCyllinder(pos, radius, h);
    auto newSize = newFlowers.size();
#ifdef _DEBUG
    if (newSize > 0) {
        dbg("%d new flowers\n", newSize);
    }
#endif

    FlowerGroup* group = nullptr;
    for(auto& i : range_t<flowers_t::iterator>(flowers.equal_range(spatialIndex))) {
        if (i.second.getSize() + newSize < MAX_GROUP_SIZE) {
            group = &i.second;
            break;
        }
    }
    if (group == nullptr) {
        group = &flowers.emplace(spatialIndex, FlowerGroup(MAX_GROUP_SIZE))->second;
    }

    group->add(newFlowers);
}


inline float yScaFlowerBreath(float t)
{
    return std::sin(t*2.5f+1.04f)*.023f;
}
inline float xScaFlowerBreath(float t)
{
    return std::cos(t*2.0f+2.3f)*.02f;
}
inline float yScaFlowerGrow(float t)
{
    return t >= (1.f/3.f) ? 1 :
        std::sin(t*(1.f/3.f)*M_PI_2f)+0.45f*std::sin(t*(1.f/3.f)*M_PIf);
}
inline float xScaFlowerGrow(float t)
{
    return t >= (1.f/3.f) ? 1 :
        std::sin(t*(1.f/3.f)*M_PI_2f);
}

void FlowerGroup::update(float elapsed)
{
    for(auto& f : flowers) {
        f.life += elapsed;
        f.sca.x = (xScaFlowerGrow(f.life) + xScaFlowerBreath(f.life))*QUAD_SIZE;
        f.sca.y = (yScaFlowerGrow(f.life) + yScaFlowerBreath(f.life))*QUAD_SIZE;
    }
    dirty=true;
}

void FlowerGroup::draw()
{
    if (flowers.size() > 0) {
        if (dirty) {updateInstanceData();}
        mesh_textured_quad_xy.renderInstanced(*instanceData, flowers.size());
    }
}

void FlowerGroup::updateInstanceData()
{
    if (flowers.size() > 0) {
        instanceData->updateFromCPU(flowers.data(),
            sizeof(instance_t) * flowers.size());
    }
    dirty = false;
}

FlowerGroup::FlowerGroup(size_t maxSize)
{
    instanceData = new Mesh;
    flowers.reserve(maxSize);
    decltype(flowers) zeroes;
    zeroes.resize(maxSize);
    bool isOK = instanceData->create(
        unsigned(maxSize), zeroes.data(), 0, nullptr,
        Mesh::POINTS, getVertexDecl<VertexFlowerData>(), 1 /*instance stream*/,
        utils::zero_v, utils::zero_v, true);
    assert(isOK);
    dirty = false;
}

FlowerPathManager::simulation_t FlowerPathManager::simulate(
    const component::AABB& aabb, float density, float step)
{
    dbg("flower simulation start\n");
    const XMVECTOR min = aabb.getMin();
    const XMVECTOR max = aabb.getMax();
    float xMin = XMVectorGetX(min);
    float xMax = XMVectorGetX(max);
    float yMin = XMVectorGetY(min);
    float yMax = XMVectorGetY(max);
    float zMin = XMVectorGetZ(min);
    float zMax = XMVectorGetZ(max);

    float area = (xMax-xMin)*(zMax-zMin);
    unsigned nTests = unsigned(density*area);

    FlowerPathManager::simulation_t ret;

    uintmax_t nTestTotal = 0;

    float yPrev = yMin;
    for(float y=yPrev; yPrev<yMax; y+=step) {
        yPrev = y;
        PxRaycastBuffer hit;
        for(unsigned i=0; i<nTests; i++) {

            static const auto ignore = filter_t::TOOL | filter_t::PROP;
            static const auto f = filter_t::SCENE | filter_t::TOOL | filter_t::PROP;
            static const auto filter = filter_t(filter_t::NONE, ~f, filter_t::SCENE);
            PxVec3 p(rand_uniform(xMax, xMin), y, rand_uniform(zMax, zMin));
            ++nTestTotal;
            if (PHYSX.raycast(p, PxVec3(0,-1, 0), step, hit, filter)) {
                filter_t hitFilter = hit.block.shape->getQueryFilterData();
                if (((hitFilter.is & ignore) == 0) && 
                    (hit.block.normal.dot(PxVec3(0,1,0)) >= COS_ANGLE_THRESHOLD)) {
                    ret.push_back(toXMVECTOR(hit.block.position));
                }
            }
        }
    }
    dbg("flower simulation ended (%lld/%lld)\n", ret.size(), nTestTotal);
    return ret;
}

FlowerPathManager::simulation_t FlowerPathManager::loadSimulationFile(const std::string& name)
{
    FlowerPathManager::simulation_t ret;
    FileDataProvider file("data/flowerSim/"+name+".points");
    if (file.isValid()) {
        size_t nPoints;
        file.readVal(nPoints);
        ret.resize(nPoints);
        for (size_t i=0;i<nPoints;++i) {
            float x, y, z;
            file.readVal(x);
            file.readVal(y);
            file.readVal(z);
            ret[i] = XMVectorSet(x,y,z,1);
        }
    }
    return ret;
}

void FlowerPathManager::writeSimulationFile(const std::string& name, const simulation_t& sim)
{
    FlowerPathManager::simulation_t ret;
    MemoryDataSaver file;
    size_t nPoints = sim.size();
    file.writePOD(nPoints);
    for (auto& p : sim) {
        file.writePOD(XMVectorGetX(p));
        file.writePOD(XMVectorGetY(p));
        file.writePOD(XMVectorGetZ(p));
    }
    file.saveToFile(("data/flowerSim/"+name+".points").c_str());
}

void FlowerPathManager::buildSimulationData(Handle levelE, float density, float step)
{
    Entity* level(levelE);
    CStaticBody* scene = level->get<CStaticBody>();
    physx::PxTriangleMeshGeometry geo;
    bool ok = scene->getShape()->getTriangleMeshGeometry(geo);
    assert(ok);
    auto pxaabb = geo.triangleMesh->getLocalBounds();
    std::string name = level->getName();
    auto points = loadSimulationFile(name);
    if (points.size() == 0) {
        points = simulate(AABB(toXMVECTOR(pxaabb.minimum), toXMVECTOR(pxaabb.maximum)), density, step);
        writeSimulationFile(name, points);
    }
    auto nPoints = points.size();
    dbg("flower loading start (%lld points)\n", nPoints);
    xCoords.resize(nPoints);
    yCoords.resize(nPoints);
    zCoords.resize(nPoints);
    active.resize(nPoints, false);
    for(uint32_t i = 0; i < nPoints; ++i) {
        const auto& p(points[i]);
        xCoords[i] = sproutCoord(XMVectorGetX(p), i);
        yCoords[i] = sproutCoord(XMVectorGetY(p), i);
        zCoords[i] = sproutCoord(XMVectorGetZ(p), i);
    }
    std::sort(xCoords.begin(), xCoords.end());
    std::sort(yCoords.begin(), yCoords.end());
    std::sort(zCoords.begin(), zCoords.end());
    dbg("flower loading end\n");
}

void FlowerGroup::drawPoints(const component::Color& color)
{
    if (flowers.size() > 0) {
        if (dirty) {updateInstanceData();}

        static Mesh* const star = new Mesh;
        static const bool ok = createStar(*star, QUAD_SIZE);
        assert(ok);

        setObjectConstants(XMMatrixIdentity(), color);
        star->renderInstanced(*instanceData, flowers.size());
    }
}

FlowerGroup* FlowerPathManager::simulationHolder = nullptr;

FlowerGroup* FlowerPathManager::generateSimulationHolder()
{
    if (simulationHolder != nullptr) {delete simulationHolder;}
    simulationHolder = new FlowerGroup(active.size());
    std::vector<XMVECTOR> points;
    points.reserve(active.size());
    std::sort(xCoords.begin(), xCoords.end(), &sproutCoord::compById);
    std::sort(yCoords.begin(), yCoords.end(), &sproutCoord::compById);
    std::sort(zCoords.begin(), zCoords.end(), &sproutCoord::compById);
    for(size_t i = 0; i < active.size(); ++i) {
        if (!active[i]) {
            points.push_back(XMVectorSet(xCoords[i].val,
                yCoords[i].val, zCoords[i].val, 1.f));
        }
    }
    simulationHolder->add(points);
    std::sort(xCoords.begin(), xCoords.end());
    std::sort(yCoords.begin(), yCoords.end());
    std::sort(zCoords.begin(), zCoords.end());
    return simulationHolder;
}

void FlowerPathManager::drawSimulation(const component::Color& color)
{
    if (simulationHolder == nullptr) {generateSimulationHolder();}
    simulationHolder->drawPoints(color);
}

void FlowerPathManager::drawLastTest(const component::Color& color)
{
    if (lastTestActive || App::get().isDebugPaused()) {
        static Mesh* const star = new Mesh;
        static const bool ok = createStar(*star, 0.2f);
        setObjectConstants(lastTest.getWorld(), color);
        mesh_cyllinder.activateAndRender();
        setObjectConstants(lastTest.getWorld(), color);
        star->activateAndRender();
        lastTestActive = false;
    }
}

void FlowerPathManager::drawSproutedPoints(const component::Color& color)
{
    for(auto& i : flowers) {
        i.second.drawPoints(color);
    }
}

}