#include "mcv_platform.h"
#include "CInstancedMesh.h"

#include "component.h"

#include <algorithm>

#include "handles/entity.h"
using namespace component;

#include "render/render_utils.h"

using namespace DirectX;
using namespace utils;

namespace render {

void CInstancedMesh::updateInstanceData()
{
    assert(created);
    nInstances = dataBuffer->size();
    if (nInstances > 0) {
        instanceData->updateFromCPU(dataBuffer->data(),
            sizeof(instance_t) * nInstances);
    }
    dirty = false;
    culled = false;
}

uint32_t CInstancedMesh::getFreshDataIndex(unsigned index)
{
    if (instanceExtraData == nullptr) {
        instanceExtraData = new std::vector<instanceData_t>;
    }
    for (size_t i = 0; i < instanceExtraData->size(); i++) {
        auto& d = (*instanceExtraData)[i];
        if (!d.used) {
            d.setScale(aabbScale);
            d.setSkin(aabbSkin);
            d.dirty = true;
            d.used = true;
            d.instanceIndex = index;
            return (uint32_t)i;
        }
    }
    instanceData_t d;
    d.setScale(aabbScale);
    d.setSkin(aabbSkin);
    d.dirty = true;
    d.used = true;
    d.instanceIndex = index;
    instanceExtraData->push_back(std::move(d));
    return (uint32_t)instanceExtraData->size()-1;
}


void CInstancedMesh::recalculateAABB()
{
    if (dataBuffer->size() > 0) {
        CAABB* baseAABB = Handle(this).getBrother<CAABB>();
        auto& i = dataBuffer->begin();
        aabb = culling_t::bakeCulling(*baseAABB, i->world);
        for(++i; i != dataBuffer->end(); ++i) {
            auto newAABB = culling_t::bakeCulling(*baseAABB, i->world);
            aabb.expand(newAABB.getMin(), newAABB.getMax());
        }
        doRecalculateAABB = false;
    } else {
        aabb = CullingAABB();
    }
}
unsigned CInstancedMesh::addInstance(instance_t&& instance)
{
    assert(dataBuffer != nullptr);
    ++nInstances;
    if (nInstances > maxInstances) {
        assert(!created);
        maxInstances = nInstances;
    }
    instance.userDataB = getFreshDataIndex(unsigned(nInstances-1));
    dataBuffer->push_back(instance);
    dirty = true;
    changed = true;

    CAABB* baseAABB = Handle(this).getBrother<CAABB>();
    if (!usesGlobalAABB || baseAABB->isInvalid()) {
        doRecalculateAABB = true;
    } else if (!doRecalculateAABB) {
        auto newAABB = culling_t::bakeCulling(*baseAABB, instance.world);
        if (aabb.isInvalid()) {
            aabb = newAABB;
        } else {
            aabb.expand(newAABB.getMin(), newAABB.getMax());
        }
    }

    doRecalculateAABB = true;
    return instance.userDataB;
}

bool CInstancedMesh::createInstanceData()
{
    assert(!created);
    instanceData = new Mesh;
    dataBuffer->resize(maxInstances);
    bool b = instanceData->create(
        unsigned(maxInstances), dataBuffer->data(), 0, nullptr,
        Mesh::POINTS, getVertexDecl<VertexPUNTInstance>(), 1 /*instance stream*/,
        utils::zero_v, utils::zero_v, true);
    dataBuffer->resize(nInstances);
    dirty = false;
    created = true;
    return b;
}

void CInstancedMesh::loadFromProperties(std::string elem, utils::MKeyValue& atts)
{
    if (elem == "InstancedMesh") {
        if (atts.has("name")) {
            load(atts["name"]);
        }
        createBuffer();
        assert(dataBuffer != nullptr);
        usesGlobalAABB = atts.getBool("usesGlobalAABB", usesGlobalAABB);
    } else if (elem == "instance") {
        assert(dataBuffer != nullptr);
        Transform t;
        if (atts.has("pos")) {t.setPosition(atts.getPoint("pos"));}
        if (atts.has("rot")) {t.setRotation(atts.getQuat("rot"));}
        if (atts.has("sca")) {t.setScale(atts.getPoint("sca"));}
        Color tint = uint32_t(atts.getHex("tint", 0));
        Color selfIllumination = uint32_t(atts.getHex("selfIlumination", 0));
        addInstance(instance_t(t.getWorld(), tint, selfIllumination));
    }
}

void CInstancedMesh::load(std::string name)
{
    Handle me_h(Handle(this).getOwner());
    Entity* me(me_h);
    assert(me != nullptr);
    CMesh::load(name, me_h);
    CMesh* cmesh = me->get<CMesh>();
}

bool CInstancedMesh::removeInstance(unsigned i)
{
	assert(dataBuffer != nullptr);
    assert(instanceExtraData != nullptr);
    auto& d = (*instanceExtraData)[i];
    d.used = false;
    unsigned prevInstanceIndex = d.instanceIndex;
    d.instanceIndex = ~0;
    
    //replace this with back
    auto& instance = (*dataBuffer)[prevInstanceIndex];
    instance = dataBuffer->back();
    auto& dn = (*instanceExtraData)[instance.userDataB];
    dn.instanceIndex = prevInstanceIndex;
    --nInstances;

    //remove duplicated back
    dataBuffer->pop_back();

    doRecalculateAABB = true;
    dirty = true;
    changed = true;
    return true;
}

CInstancedMesh::instance_t& CInstancedMesh::getInstance(unsigned i)
{
    assert(dataBuffer != nullptr);
    assert(instanceExtraData != nullptr);
    const auto& d = (*instanceExtraData)[i];
    const auto& index = d.instanceIndex;
    assert(index < nInstances);
    return (*dataBuffer)[index];
}

void CInstancedMesh::setInstance(unsigned i, instance_t&& next)
{
    assert(dataBuffer != nullptr);
    auto& item = getInstance(i);
    next.userDataB = item.userDataB;
    item = next;
    dirty = true;
}

bool CInstancedMesh::changeInstanceTint(
    unsigned i, const component::Color& c)
{
    
    assert(dataBuffer != nullptr);
    auto& item = getInstance(i);
    item.setTint(c);
    dirty = true;
    return true;

}

bool CInstancedMesh::changeInstanceSelfIllumination(
    unsigned i, const component::Color& c)
{
    
    assert(dataBuffer != nullptr);
    auto& item = getInstance(i);
    item.setSelfIllumination(c);
    dirty = true;
    return true;

}

bool CInstancedMesh::replaceInstance(
    const instance_t::world_t& prev,
    instance_t&& next, bool substantialChange)
{
    assert(dataBuffer != nullptr);
    for(auto& i = dataBuffer->begin(); i != dataBuffer->end(); i++) {
        if (i->world == prev) {
            next.userDataB = i->userDataB;
            *i = next;
            dirty = true;
            if (substantialChange) {
                doRecalculateAABB = true;
                changed = true;
            }
            return true;
        }
    }
    return false;
}

bool CInstancedMesh::replaceInstanceWorld(
    unsigned index, instance_t::world_t&& next, bool substantialChange)
{
    assert(dataBuffer != nullptr);
    assert(index < nInstances);
    getInstance(index).world = next;
    dirty = true;
    if (substantialChange) {
        doRecalculateAABB = true;
        changed = true;
    }
    return true;
}

#undef CULLING_METHOD_NEW_VECTOR
#define CULLING_METHOD_PARTITION
#undef CULLING_SKIP

//#define CHECK_CHANGEDVECTOR

size_t CInstancedMesh::cull(const Culling::CullerDelegate& cullerDelegate)
{
    if (usesGlobalAABB) {
        if (doRecalculateAABB) {
            recalculateAABB();
        }
        AABB xaabb(aabb);
        xaabb.skinAABB(aabbSkin);
        xaabb.scaleAABB(aabbScale);
        if (!cullerDelegate.cull(xaabb)) {
            return 0;
        }
    }

#if defined(CULLING_SKIP)
    return dataBuffer->size();
#endif

    Entity* me = Handle(this).getOwner();
    CAABB* aabb = me->get<CAABB>();
    auto culler = Culler(this, cullerDelegate, *aabb, !instancesWillMove);

    
#if defined(CULLING_METHOD_NEW_VECTOR)
    std::vector<instance_t> culledDataBuffer;
    culledDataBuffer.reserve(dataBuffer->size());
    for(const auto& i : *dataBuffer) {
        if (culler(i)) {
            culledDataBuffer.push_back(i);
        }
    }
    nCulled = culledDataBuffer.size();
    if (nCulled > 0) {
        instanceData->updateFromCPU(culledDataBuffer.data(), sizeof(instance_t) * nCulled);
    }
#elif defined(CULLING_METHOD_PARTITION)
    Swapper swapper(this);
    auto begin = dataBuffer->begin();

#if defined(_DEBUG) && defined(CHECK_CHANGEDVECTOR)
    auto dataBufferCopy(*dataBuffer);
#endif

    auto prevNCulled = nCulled;
    bool changedVector = utils::partition_swap(begin, dataBuffer->end(), culler, swapper);
    nCulled = begin - dataBuffer->begin();
    changedVector |= (prevNCulled != nCulled);

#if defined(_DEBUG) && defined(CHECK_CHANGEDVECTOR)
    if (!changedVector && prevNCulled != 0) {
        assert(nCulled == prevNCulled);
        for (auto i = 0; i < nCulled; ++i) {
            auto a = (*dataBuffer)[i];
            auto b = dataBufferCopy[i];
            assert(a == b);
        }
    }
#endif

    if (changedVector && nCulled > 0) {
        instanceData->updateFromCPU(dataBuffer->data(), sizeof(instance_t) * nCulled);
    }

#elif !defined(CULLING_SKIP)
#error No culling method!
#endif

    dirty = false;
    culled = true;
    changed = false;
    return nCulled;
}

void CInstancedMesh::drawAABBs(const Color& c)
{
    if (dataBuffer == nullptr) {return;}
    for (const auto& i : *dataBuffer) {
        instanceExtraData->at(i.userDataB).draw(c);
    }
}

#define ALL_INSTANCES_SAME_AABB_COLOR
void CInstancedMesh::drawAABBs()
{
    if (dataBuffer == nullptr) {return;}

#if defined(ALL_INSTANCES_SAME_AABB_COLOR) && defined(_DEBUG)
    const auto color = dbgColor;
#else
    const auto color = Color::MAGENTA;
#endif

    if (usesGlobalAABB) {
        CullingAABB aabbCopy(aabb);
        aabbCopy.draw(XMVECTOR(color) + one_v*0.45f);
    }

    for (const auto& i : *dataBuffer) {
        instanceExtraData->at(i.userDataB).draw(color);
    }
}


}