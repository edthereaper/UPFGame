#include "mcv_platform.h"
#include "flowerPath.h"

#include "utils/utils.h"
#include "render/render_utils.h"
#include "PhysX_USER/PhysicsManager.h"

using namespace DirectX;
using namespace utils;
using namespace render;

using namespace physx;
using namespace physX_user;

#define PHYSX physX_user::PhysicsManager::get()

namespace gameElements {

const float FlowerGroup::QUAD_SIZE = 0.5f;

FlowerPathManager::sproutCoordV_t FlowerPathManager::xCoords;
FlowerPathManager::sproutCoordV_t FlowerPathManager::yCoords;
FlowerPathManager::sproutCoordV_t FlowerPathManager::zCoords;
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
            }
        }
    }
    return ret;
}

void FlowerPathManager::plantCyllinder(const XMVECTOR& pos, float radius, float h, int spatialIndex)
{
    auto newFlowers = getNewInCyllinder(pos, radius, h);
    auto newSize = newFlowers.size();

    FlowerGroup* group = nullptr;
    for(auto& i : range_t<flowers_t::iterator>(flowers.equal_range(spatialIndex))) {
        if (i.second.getSize() + newSize < FlowerGroup::MAX_SIZE) {
            group = &i.second;
            break;
        }
    }
    if (group == nullptr) {
        group = &flowers.emplace(spatialIndex, FlowerGroup())->second;
    }

    group->add(newFlowers);
}


inline float yScaBreath(float t)
{
    return std::sin(t*2.5f+1.04f)*.023f;
}
inline float xScaBreath(float t)
{
    return std::cos(t*2.0f+2.3f)*.02f;
}
inline float yScaGrow(float t)
{
    return t >= (1.f/3.f) ? 1 :
        std::sin(t*(1.f/3.f)*M_PI_2f)+0.45f*std::sin(t*(1.f/3.f)*M_PIf);
}
inline float xScaGrow(float t)
{
    return t >= (1.f/3.f) ? 1 :
        std::sin(t*(1.f/3.f)*M_PI_2f);
}

void FlowerGroup::update(float elapsed)
{
    for(auto& f : flowers) {
        f.life += elapsed;
        f.sca.x = (xScaGrow(f.life) + xScaBreath(f.life))*QUAD_SIZE;
        f.sca.y = (yScaGrow(f.life) + yScaBreath(f.life))*QUAD_SIZE;
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

FlowerGroup::FlowerGroup()
{
    instanceData = new Mesh;
    flowers.resize(MAX_SIZE);
    bool isOK = instanceData->create(
        unsigned(MAX_SIZE), flowers.data(), 0, nullptr,
        Mesh::POINTS, getVertexDecl<VertexFlowerData>(), 1 /*instance stream*/,
        utils::zero_v, utils::zero_v, true);
    assert(isOK);
    dirty = false;
}

FlowerPathManager::simulation_t FlowerPathManager::simulate(
    const component::AABB& aabb, float density, float step)
{
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

    float yPrev = yMin;
    for(float y=yPrev; yPrev<yMax; y+=step) {
        yPrev = y;
        PxRaycastBuffer hit;
        for(unsigned i=0; i<nTests; i++) {
            static const auto ignore = filter_t::TOOL | filter_t::PROP;
            static const auto f = filter_t::SCENE | filter_t::TOOL | filter_t::PROP;
            static const auto filter = filter_t(filter_t::NONE, ~f, f);
            PxVec3 p(rand_uniform(xMax, xMin), y, rand_uniform(zMax, zMin));
            if (PHYSX.raycast(p, PxVec3(0,-1, 0), step, hit, filter)) {
                filter_t hitFilter = hit.block.shape->getSimulationFilterData();
                if (!((hitFilter.is & ignore) != 0)) {
                    ret.push_back(toXMVECTOR(hit.block.position));
                }
            }
        }
    }
}

}