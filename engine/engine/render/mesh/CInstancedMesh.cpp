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

void CInstancedMesh::updateInstanceData(size_t count)
{
    assert(created);
    assert(count <= nInstances);
    if (count > 0) {
        instanceData->updateFromCPU(dataBuffer->data(), sizeof(instance_t) * count);
    }
    dirty = false;
    worldDirty = false;
    updateCulled = false;
    changed = false;
    isComplete = count == nInstances;
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
            d.cullerMask.reset();
            d.used = true;
            d.instanceIndex = index;
            return (uint32_t)i;
        }
    }
    instanceData_t d;
    d.setScale(aabbScale);
    d.setSkin(aabbSkin);
    d.dirty = true;
    d.cullerMask.reset();
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
    worldDirty = true;
    changed = true;
    culled = false;
    updateCulled = false;

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
    updateCulled = false;
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
    worldDirty = true;
    culled = false;
    updateCulled = false;
    changed = true;
    return true;
}

CInstancedMesh::instance_t& CInstancedMesh::getInstance(unsigned i)
{
    assert(dataBuffer != nullptr);
    assert(instanceExtraData != nullptr);
    const auto& d = (*instanceExtraData)[i];
    const auto& index = d.instanceIndex;
    dirty = true;
    worldDirty = true;
    culled = false;
    updateCulled = false;
    assert(index < nInstances);
    return (*dataBuffer)[index];
}

CInstancedMesh::instance_t& CInstancedMesh::getInstance_p(unsigned i)
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
}

bool CInstancedMesh::changeInstanceTint(
    unsigned i, const component::Color& c)
{
    
    assert(dataBuffer != nullptr);
    auto& item = getInstance_p(i);
    item.setTint(c);
    dirty = true;
    return true;

}

bool CInstancedMesh::changeInstanceSelfIllumination(
    unsigned i, const component::Color& c)
{
    
    assert(dataBuffer != nullptr);
    auto& item = getInstance_p(i);
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
            worldDirty = true;
            culled = false;
            updateCulled = false;
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
    if (substantialChange) {
        doRecalculateAABB = true;
        changed = true;
    }
    return true;
}

bool CInstancedMesh::cullHighLevel(const Culling::CullerDelegate& cullerDelegate)
{
    if (usesGlobalAABB) {
        if (doRecalculateAABB) {
            recalculateAABB();
        }
        AABB xaabb(aabb);
        xaabb.skinAABB(aabbSkin);
        xaabb.scaleAABB(aabbScale);
        return cullerDelegate.cull(xaabb);
    } else {
        return true;
    }
}

size_t CInstancedMesh::testCull(const Culling::CullerDelegate& cullerDelegate)
{
    Entity* me = Handle(this).getOwner();
    CAABB* aabb = me->get<CAABB>();
    auto culler = Culler(this, cullerDelegate, *aabb, !instancesWillMove);

    Swapper swapper(this);
    auto begin = dataBuffer->begin();

    nCulled = 0;
    for (auto& i : *dataBuffer) {
        if (culler(i)) {
            ++nCulled;
        }
    }
    return nCulled;
}


size_t CInstancedMesh::cull(const Culling::CullerDelegate& cullerDelegate)
{
    Entity* me = Handle(this).getOwner();
    CAABB* aabb = me->get<CAABB>();
    auto culler = Culler(this, cullerDelegate, *aabb, !instancesWillMove);

    Swapper swapper(this);
    auto begin = dataBuffer->begin();

    auto prevNCulled = nCulled;
    bool changedVector = utils::partition_swap(begin, dataBuffer->end(), culler, swapper);
    nCulled = begin - dataBuffer->begin();
    changedVector |= (prevNCulled != nCulled);
    culled = true;
    updateCulled = changedVector && nCulled > 0;
    return nCulled;
}

size_t CInstancedMesh::partitionOnBitMask(const Culling::CullerDelegate& cullerDelegate)
{
    auto test = BitMaskChecker(this, cullerDelegate);

    Swapper swapper(this);
    auto begin = dataBuffer->begin();

    auto prevNCulled = nCulled;
    bool changedVector = utils::partition_swap(begin, dataBuffer->end(), test, swapper);
    nCulled = begin - dataBuffer->begin();
    changedVector |= (prevNCulled != nCulled);
    culled = true;
    updateCulled = changedVector && nCulled > 0;
    return nCulled;
}

void CInstancedMesh::drawAABBs(const Color& c)
{
    if (dataBuffer == nullptr) {return;}
    for (const auto& i : *dataBuffer) {
        instanceExtraData->at(i.userDataB).draw(c);
    }
}

//#define DIFFERENT_COLOR_PER_MESH
#define DIFFERENT_COLOR_PER_INSTANCE
void CInstancedMesh::drawAABBs()
{
    if (dataBuffer == nullptr) {return;}

#if defined(DIFFERENT_COLOR_PER_MESH) && defined(_DEBUG)
    const auto color = dbgColor;
#else
    const auto color = Color::MAGENTA;
#endif

    if (usesGlobalAABB) {
        CullingAABB aabbCopy(aabb);
        aabbCopy.draw(XMVECTOR( Color(color).setAf(0)) + one_v*0.45f);
    }

    for (const auto& i : *dataBuffer) {
        #if defined(DIFFERENT_COLOR_PER_INSTANCE) && defined(_DEBUG)
            const auto color = (*instanceExtraData)[i.userDataB].dbgColor;
        #endif
        instanceExtraData->at(i.userDataB).draw(color);
    }
}


}