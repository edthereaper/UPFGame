#include "mcv_platform.h"
#include "renderManager.h"
#include "app.h"

using namespace utils;

#include "animation/cskeleton.h"
using namespace animation;

#include "components/transform.h"
#include "handles/handle.h"
#include "handles/entity.h"
using namespace component;

#include "mesh/CInstancedMesh.h"
#include "mesh/component.h"
#include "render_utils.h"
#include "deferredRender.h"
#include "texture/material.h"
#include "shader/shaders.h"
#include "camera/camera.h"

#include "illumination/shadow.h"
#include "illumination/cubeshadow.h"

namespace render {


// Static attribute members
RenderManager::keyContainer_t RenderManager::keys;
RenderManager::keyByEntity_t RenderManager::keysByEntity;
RenderManager::shadowKeyContainer_t RenderManager::shadowKeys;
bool RenderManager::sortRequired = true;
bool RenderManager::shadowsSortRequired = true;
RenderManager::tech2techMap_t RenderManager::instancedVersion;


void RenderManager::init()
{
    auto& man = Technique::getManager();
    instancedVersion[man.getByName("deferred_gbuffer")] = man.getByName("deferred_gbuffer_instanced");
    instancedVersion[man.getByName("gen_shadows")] = man.getByName("gen_shadows_instanced");
}

bool RenderManager::compMesh(const shadowKey_t& k1, const shadowKey_t& k2)
{
    const Mesh* m1 = ((CMesh*)k1.mesh)->getMesh();
    const Mesh* m2 = ((CMesh*)k2.mesh)->getMesh();
    return m1 < m2;
}

bool RenderManager::compTechMesh(const shadowKey_t& k1, const shadowKey_t& k2)
{
    auto tech1 = techniqueOf(k1);
    auto tech2 = techniqueOf(k2);
    if (tech1 == tech2) {
        return compMesh(k1, k2);
    } else {
        return tech1 < tech2;
    }
}

bool RenderManager::compTechMatMesh(const key_t& k1, const key_t& k2)
{
    auto tech1 = techniqueOf(k1);
    auto tech2 = techniqueOf(k2);
    if (tech1 == tech2) {
        return compMatMesh(k1, k2);
    } else {
        return tech1 < tech2;
    }
}

bool RenderManager::compMatMesh(const key_t& k1, const key_t& k2)
{
    if (k1.material != k2.material) {
        return k1.material < k2.material;
    } else {
        const Mesh* m1 = ((CMesh*)k1.mesh)->getMesh();
        const Mesh* m2 = ((CMesh*)k2.mesh)->getMesh();
        if (m1 != m2) {
            return m1 < m2;
        } else {
            return k1.group0 < k2.group0;
        }
    }
}

void RenderManager::renderKey(const key_t& k,
    const Culling::CullerDelegate& culler,
    const Technique*& technique_prev, const Mesh*& mesh_prev,
    const Material*& material_prev, bool& bones, bool& first,
    std::vector<key_t>* glassKeys)
{
    #if defined(_DEBUG) && defined (DEBUG_TRACK_MESHKEY)
        if (k.__debug) {
            auto name(((Entity*)k.entity)->getName().c_str());
            TraceScoped trace(name);
            dbg("Debugged key %s:%d\n", name, k.group);
        }
    #endif

    //Visibility
    CMesh* cmesh = k.mesh;
    if (!cmesh->isVisible()) {return;}
    if (k.specialClass == SKINNED) {
        const CSkeleton* skeleton = k.special;
        if (skeleton->disabled()) {return;}
    }

    //Alpha
    const Material* material = k.material;
    if (material->isGlass() && glassKeys!=nullptr) {
        //Postpone
        glassKeys->push_back(k);
        return;
    }
    
    // Technique
    const Technique* technique = techniqueOf(k);
    if (technique != technique_prev) {
        technique_prev = technique;
        technique->activate();
        activateObjectConstants();
        bones = technique->usesBones();
    }

    //Mesh
    const Mesh* mesh = cmesh->getMesh();
    if (mesh != mesh_prev || first) {
        mesh->activate();
        mesh_prev = mesh;
    }

    //Material
    if (material != material_prev || first || material->isRandom()) {
        material_prev = material;
        material->activateTextures(k.entity.getRaw());
    }
    
    //Transform
    assert(k.transform.isValid());
    CTransform* transform = k.transform;
    
    switch (k.specialClass) {
        case SKINNED:
            if (bones) {
                //Skinned submesh
                const CSkeleton* skeleton = k.special;
                assert(skeleton != nullptr);
                if ((skeleton->getCullingMask() & culler.getMask())!=0) {
                    setObjectConstants(transform->getWorld(), k.tint, k.selfIllumination,
                        cmesh, skeleton->getBone0(), material);
                    mesh->renderGroups(k.group0, k.groupf);
                }
            } break;
        default:
        case NORMAL: {
                Entity* e(k.entity);
                CCullingAABB* aabb = e->get<CCullingAABB>();
                if (aabb==nullptr || culler.cull(*aabb)) {
                    //Submesh
                    setObjectConstants(transform->getWorld(),
                        k.tint, k.selfIllumination, cmesh, 0, material);
                    mesh->renderGroups(k.group0, k.groupf);
                }
            } break;
        case INSTANCED: {
                //Instances
                setObjectConstants(transform->getWorld(),
                    k.tint, k.selfIllumination, cmesh, 0, material);
                CInstancedMesh* instances = k.special;
                assert(instances != nullptr);

                switch (App::get().instanceCulling) {
                    case App::IC_NO:
                    case App::IC_HIGHLEVEL:
                        mesh->renderInstanced(k.group0, k.groupf,
                            *instances->getData(true), instances->getNInstances());
                        break;
                    default:
                        size_t nCulled = instances->cull(culler);
                        if (nCulled > 0) {
                            mesh->renderInstanced(k.group0, k.groupf, *instances->getData(), nCulled);
                        }
                }
            }
            break;
    }
    
    first = false;
}

void RenderManager::renderAll(component::Handle cameraE_h,
    const DeferredRender& renderer, Culling::cullDirection_e dir)
{
    if (sortRequired) {
        std::sort(keys.begin(), keys.end(), compTechMatMesh);
        sortRequired = false;
        keysByEntity.clear();
        size_t i = 0;
        for(const auto& k : keys) {
            keysByEntity[k.entity].push_back(i);
            i++;
        }
    }

    bool first=true;
    bool bones = false;
    const Technique* technique_prev = nullptr;
    const Mesh* mesh_prev = nullptr;
    const Material* material_prev = nullptr;
    
    const Culling::CullerDelegate culler(cameraE_h, dir);

    std::vector<key_t> glassKeys;
    activateBlendConfig(BLEND_CFG_DEFAULT);
    {
        TraceScoped _("renderAll.regular");
        for (const auto& k : keys) {
            renderKey(k, culler, technique_prev, mesh_prev,
                material_prev, bones, first,
                &glassKeys);
        }
    }
    if (!glassKeys.empty()) {
        TraceScoped scope("renderAll.glass");
        activateZConfig(zConfig_e::ZCFG_TEST_LT);
        activateBlendConfig(BLEND_GLASS);
        first = true;
        
        renderer.initGBuffer(~(DeferredRender::SPACE|DeferredRender::NORMALS|DeferredRender::NORMALS_PAINTED));

        bones = false;
        technique_prev = nullptr;
        mesh_prev = nullptr;
        for (auto k=glassKeys.begin(); k!=glassKeys.end(); ++k) {
            renderKey(*k, culler, technique_prev, mesh_prev,
                material_prev, bones, first, nullptr);
        }
    }
}

void RenderManager::deleteKeys(Handle entity_h)
{
    keysByEntity.erase(entity_h);
    keys.erase(std::remove_if(keys.begin(), keys.end(),
        [=] (const key_t& k) {return k.entity == entity_h;}
        ), keys.end());
    shadowKeys.erase(std::remove_if(shadowKeys.begin(), shadowKeys.end(),
        [=] (const shadowKey_t& k) {return k.entity == entity_h;}
        ), shadowKeys.end());
    sortRequired = true;
}

void RenderManager::updateKeys(component::Handle entity_h)
{
    Entity* e(entity_h);
    Color tint = e->has<CTint>() ? *((CTint*)e->get<CTint>()) : CMesh::key_t::NO_COLOR;
    Color selfi = e->has<CSelfIllumination>() ?
        *((CSelfIllumination*)e->get<CSelfIllumination>()) : CMesh::key_t::NO_SELFI;
    
    if (sortRequired) {
        //keysByEntity's contents aren't trustworthy => run the O(n) search
        for (auto& k : keys) {
            if(k.entity == entity_h) {
                CMesh* mesh = k.mesh;
                for (auto& i : mesh->iterateKeys()) {
                    if (i.group0 == k.group0) {
                        k.tint = i.tint == CMesh::key_t::NO_COLOR ?
                            tint : i.tint;
                        k.selfIllumination = i.selfIllumination == CMesh::key_t::NO_SELFI ?
                            selfi : i.selfIllumination;
                    }
                }
            }
        }
    } else {
        //keysByEntity should contain correct information => Search is solved in O(1)
        const auto& it = keysByEntity.find(entity_h);
        if (it != keysByEntity.end()) {
            for(auto& i : it->second) {
                auto& k = keys[i];
                assert(k.entity == entity_h);
                CMesh* mesh = k.mesh;
                for (auto& i : mesh->iterateKeys()) {
                    if (i.group0 == k.group0) {
                        k.tint = i.tint == CMesh::key_t::NO_COLOR ?
                            tint : i.tint;
                        k.selfIllumination = i.selfIllumination == CMesh::key_t::NO_SELFI ?
                            selfi : i.selfIllumination;
                    }
                }
            }
        }
    }
}

void RenderManager::addKey(
    const Material* material,
    Handle mesh,
    Mesh::groupId_t group0, Mesh::groupId_t groupf,
    Handle entity,
    Color tint, Color selfIllumination,
    bool debug
    )
{
    Entity* e = entity;
    /* Get special cases */
    Handle special;
    specialKey_e specialClass = NORMAL;
    if (e->has<CSkeleton>()) {
        specialClass = SKINNED;
        special = e->get<CSkeleton>();
    } else if (e->has<CInstancedMesh>()) {
        assert(!special.isValid());
        specialClass = INSTANCED;
        special = e->get<CInstancedMesh>();
    }
    Handle transform = e->get<CTransform>();
    keys.push_back(key_t(material, entity, mesh, specialClass, special,
        transform, group0, groupf, tint, selfIllumination, debug));
    keysByEntity[entity].push_back(keys.size()-1);
    sortRequired = true;
    
    if (material->castsShadows()) {
        shadowKeys.push_back(shadowKey_t(entity, mesh, transform, specialClass, special));
        shadowsSortRequired = true;
    }
}

void RenderManager::addKeys(Handle entity_h)
{ 
    Entity* e = entity_h;
    Handle mesh_h = e->get<CMesh>();
    CMesh* mesh = mesh_h;
    Color tint = e->has<CTint>() ? *((CTint*)e->get<CTint>()) : CMesh::key_t::NO_COLOR;
    Color selfIllumination = e->has<CSelfIllumination>() ?
        *((CSelfIllumination*)e->get<CSelfIllumination>()) : CMesh::key_t::NO_SELFI;

    for (auto i : mesh->iterateKeys()) {
        if (i.material == nullptr || i.group0 == CMesh::key_t::BAD_ID) {continue;}
        Color subTint = i.tint == CMesh::key_t::NO_COLOR ? tint : i.tint;
        Color subSelfIllumination = i.selfIllumination == CMesh::key_t::NO_SELFI ?
            selfIllumination : i.selfIllumination;
        Mesh::groupId_t groupf = i.groupf == CMesh::key_t::BAD_ID ? i.group0 : i.groupf;
        addKey(i.material, mesh_h, i.group0, groupf, e, subTint, subSelfIllumination
            #if defined(_DEBUG) && defined (DEBUG_TRACK_MESHKEY)
                , i.__debug
            #endif
            );
    }
}

void RenderManager::renderShadowKeys(
    component::Handle light_h,
    Culling::cullDirection_e cullingType,
    const Culling::CullerDelegate& culler,
    shadowKeyContainer_t keys,
    Technique* normalTech,
    Technique* skinnedTech,
    Technique* instancedTech)
{
    static Technique*const defNormalTech =
        Technique::getManager().getByName("gen_shadows");
    static Technique*const defSkinnedTech =
        Technique::getManager().getByName("gen_shadows_skinned");
    static Technique*const defInstancedTech =
        Technique::getManager().getByName("gen_shadows_instanced");
    if (normalTech == nullptr) {normalTech = defNormalTech;}
    if (skinnedTech == nullptr) {skinnedTech = defSkinnedTech;}
    if (instancedTech == nullptr) {instancedTech = defInstancedTech;}
    
    Entity* light_e(light_h);
    CCamera* light = light_e->get<CCamera>();
    
    //Setup camera and culler
    if (cullingType != Culling::NORMAL) {
        CCubeShadow* shadow = light_e->get<CCubeShadow>();
        auto data(shadow->getCachedCamData(cullingType));
        light->setup(data.view, data.viewProjection, data.front, data.right, data.up);
    }
    
    activateLight(*light);
    activateCamera(*light);

    Technique* currentTech = normalTech;
    currentTech->activate();
    bool skinned = false;
    for (auto k : keys) {
        CTransform* t = k.transform;
        assert(t != nullptr);
        CMesh* mesh(k.mesh);
        switch (k.specialClass) {
            case SKINNED: {
                    const CSkeleton* skeleton = k.special;
                    if (currentTech != skinnedTech) {
                        skinnedTech->activate();
                        currentTech = skinnedTech;
                    }
                    assert(skeleton != nullptr);
                    setObjectConstants(t->getWorld(),0,0,nullptr,skeleton->getBone0());
                    const auto m(mesh->getMesh());
                    if (m != Mesh::current_active_mesh) {m->activate();}
                    m->render();
                } break;
            default:
            case NORMAL: {
                    Entity* e(k.entity);
                    if (currentTech != normalTech) {
                        normalTech->activate();
                        currentTech = normalTech;
                    }
                    setObjectConstants(t->getWorld());
                    const auto m(mesh->getMesh());
                    if (m != Mesh::current_active_mesh) {m->activate();}
                    m->render();
                } break;
            case INSTANCED: {
                    setObjectConstants(t->getWorld());
                    if (currentTech != instancedTech) {
                        instancedTech->activate();
                        currentTech = instancedTech;
                    }
                    CInstancedMesh* instances = k.special;
                    const auto m(mesh->getMesh());
                    assert(instances != nullptr);

                    switch (App::get().instanceCulling) {
                        case App::IC_BEFORE_W_O_PARTITION:
                            instances->partitionOnBitMask(culler);
                            // fall thru
                        case App::IC_BEFORE_W_PARTITION:
                            m->renderInstanced(*instances->getData(), instances->getNCulled());
                            break;
                        case App::IC_AFTER : {
                                size_t nCulled = instances->cull(culler);
                                if (nCulled > 0) {
                                    m->renderInstanced(*instances->getData(), nCulled);
                                }
                            } break;
                        default:
                        case App::IC_HIGHLEVEL :
                        case App::IC_NO :
                            m->renderInstanced(*instances->getData(), instances->getNInstances());
                            break;
                    }
                } break;
        }
        
    }

}

bool RenderManager::renderShadows(component::Handle light_h,
    RenderedTextureCube& shadowCubeMap,
    Culling::cullDirection_e cullingType,
    Technique* normalTech,
    Technique* skinnedTech,
    Technique* instancedTech)
{
    if (shadowsSortRequired) {
        std::sort(shadowKeys.begin(), shadowKeys.end(), compTechMesh);
        shadowsSortRequired = false;
    }
    Culling::CullerDelegate culler(light_h, cullingType);
    shadowKeyContainer_t keys;
    bool changed = testShadowKeys(culler, keys);
    if (changed && !keys.empty()) {
        // Start rendering in the rt of the depth buffer
        auto face = cullingType;
        assert(face>=0);
        shadowCubeMap.clearRenderTargetView(face, utils::BLACK);
        shadowCubeMap.clearDepthBuffer(face);
        shadowCubeMap.activateFace(face);
        renderShadowKeys(light_h, cullingType, culler, keys,
            normalTech, skinnedTech, instancedTech);
    }
    return changed;
}

bool RenderManager::renderShadows(component::Handle light_h,
    RenderedTexture& shadowMap,
    Culling::cullDirection_e cullingType,
    Technique* normalTech,
    Technique* skinnedTech,
    Technique* instancedTech)
{
    if (shadowsSortRequired) {
        std::sort(shadowKeys.begin(), shadowKeys.end(), compTechMesh);
        shadowsSortRequired = false;
    }
    Culling::CullerDelegate culler(light_h, cullingType);
    shadowKeyContainer_t keys;
    bool changed = testShadowKeys(culler, keys);
    if (changed && !keys.empty()) {
//#ifdef _DEBUG
//        App::get().captureFrame();
//#endif

        // Start rendering in the rt of the depth buffer
        shadowMap.clearDepthBuffer();
        shadowMap.activate();
        renderShadowKeys(light_h, cullingType, culler, keys,
            normalTech, skinnedTech, instancedTech);
    }
    return changed;
}

bool RenderManager::testShadowKeys(
    const Culling::CullerDelegate& culler,
    shadowKeyContainer_t& keys)
{
    bool changed = culler.hasChanged();

    for (auto k : shadowKeys) {
        CTransform* t = k.transform;
        assert(t != nullptr);
        CMesh* mesh(k.mesh);
        if (mesh->isVisible()) {
            switch (k.specialClass) {
                case SKINNED: {
                        const CSkeleton* skeleton = k.special;
                        if (!skeleton->disabled() &&
                            (skeleton->getCullingMask() & culler.getMask())!=0) {
                            changed |= k.nonStatic;
                            keys.push_back(k);
                        }
                    }
                    break;
                default:
                case NORMAL: {
                        Entity* e(k.entity);
                        CCullingAABB* aabb = e->get<CCullingAABB>();
                        if (aabb==nullptr || culler.cull(*aabb)) {
                            changed |= k.nonStatic;
                            keys.push_back(k);
                        }
                    }
                    break;
                case INSTANCED: {
                        CInstancedMesh* instances = k.special;
                        assert(instances != nullptr);
                        if (instances->getNInstances() > 0 && instances->cullHighLevel(culler) ) {
#ifdef _DEBUG
                            switch (App::get().instanceCulling) {
                                case App::IC_BEFORE_W_PARTITION:
#endif
                                    if (instances->cull(culler) > 0) {
                                        changed |= k.nonStatic || instances->isWorldDirty();
                                        keys.push_back(k);
                                    }
#ifdef _DEBUG
                                    break;
                                case App::IC_BEFORE_W_O_PARTITION:
                                    if (instances->testCull(culler) > 0) {
                                        changed |= k.nonStatic || instances->isWorldDirty();
                                        keys.push_back(k);
                                    }
                                    break;
                                case App::IC_HIGHLEVEL :
                                    if (instances->cullHighLevel(culler)) {
                                        changed |= k.nonStatic || instances->isWorldDirty();
                                        keys.push_back(k);
                                    }
                                    break;
                                case App::IC_NO :
                                case App::IC_AFTER :
                                default:
                                    changed |= k.nonStatic || instances->isWorldDirty();
                                    keys.push_back(k);
                                    break;
                            }
#endif
                        }
                    } break;
            }
        }
    }
    return changed;
}

const Technique* RenderManager::techniqueOf(const shadowKey_t& k)
{
    auto gen_shadows = Technique::getManager().getByName("gen_shadows");
    switch (k.specialClass) {
        default:
        case NORMAL: return gen_shadows; break;
        case INSTANCED: return instancedVersionOf(gen_shadows); break;
        case SKINNED: return Technique::getManager().getByName("gen_shadows_skinned"); break;
    }
}
const Technique* RenderManager::techniqueOf(const key_t& k)
{
    if (k.specialClass == INSTANCED) {
        return instancedVersionOf(k.material->getTechnique());
    } else {
        return k.material->getTechnique();
    }
}
const Technique* RenderManager::instancedVersionOf(const Technique* technique)
{
    auto it = instancedVersion.find(technique);
    assert(it != instancedVersion.end() &&"No instanced version of this technique!");
    return it->second;
}

void CTagNonStaticShadow::ensure(component::Handle e_h) {
    component::Entity* e(e_h);
    if (!e->has<CTagNonStaticShadow>()) {
        e->add(component::getManager<CTagNonStaticShadow>()->createObj());
    }
}

}