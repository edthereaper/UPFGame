#ifndef RENDER_RENDER_MANAGER_H_
#define RENDER_RENDER_MANAGER_H_

#include "handles/handle.h"
#include "components/color.h"
#include "texture/material.h"
#include "mesh/component.h"
#include "mesh/mesh.h"
#include "camera/culling.h"


namespace render {

class DeferredRender;

class RenderManager {
    public:
    
        enum specialKey_e {
            NORMAL      =0,
            SKINNED     =1,
            INSTANCED   =2,
        };

        struct key_t {
            const Material* material;
            component::Handle entity;
            component::Handle mesh;
            specialKey_e specialClass;
            component::Handle special;
            component::Handle transform;
            Mesh::groupId_t group0;
            Mesh::groupId_t groupf;
            component::Color tint;
            component::Color selfIllumination;
            #if defined(_DEBUG) && defined (DEBUG_TRACK_MESHKEY)
                bool __debug;
            #endif

            key_t()=default;
            key_t(
                const Material* material,
                component::Handle entity,
                component::Handle mesh,
                specialKey_e specialClass,
                component::Handle special,
                component::Handle transform,
                Mesh::groupId_t group0,
                Mesh::groupId_t groupf,
                component::Color tint,
                component::Color selfIllumination,
                bool __debug = false
                ) :
                    material(material),
                    entity(entity),
                    mesh(mesh),
                    specialClass(specialClass),
                    special(special),
                    transform(transform),
                    group0(group0),
                    groupf(groupf),
                    tint(tint),
                    selfIllumination(selfIllumination)
                    #if defined(_DEBUG) && defined (DEBUG_TRACK_MESHKEY)
                        ,__debug(__debug)
                    #endif
                {}
        };

        struct shadowKey_t {
            public:
                component::Handle entity;
                component::Handle mesh;
                component::Handle transform;
                specialKey_e specialClass;
                component::Handle special;
                bool nonStatic = false;

            public:
                shadowKey_t(component::Handle entity, component::Handle mesh,
                    component::Handle transform, specialKey_e specialClass,
                    component::Handle special) :
                    entity(entity), mesh(mesh), transform(transform),
                    specialClass(specialClass), special(special) {}
        };
    private:
        RenderManager()=delete;
        typedef std::vector<key_t> keyContainer_t;
        typedef std::map<component::Handle, std::vector<size_t>> keyByEntity_t;
        typedef std::vector<shadowKey_t> shadowKeyContainer_t;
        typedef std::map<const Technique*, const Technique*> tech2techMap_t;
        static tech2techMap_t instancedVersion;
    private:
        static keyByEntity_t keysByEntity;
        static keyContainer_t keys;
        static shadowKeyContainer_t shadowKeys;
        static bool sortRequired;
        static bool shadowsSortRequired;

        static bool compMatMesh(const key_t&, const key_t&);
        static bool compTechMatMesh(const key_t&, const key_t&);
        static bool compMesh(const shadowKey_t&, const shadowKey_t&);
        static bool compTechMesh(const shadowKey_t&, const shadowKey_t&);

        static const Technique* techniqueOf(const shadowKey_t& k);
        static const Technique* techniqueOf(const key_t& k);
        static const Technique* instancedVersionOf(const Technique* technique);

        static void renderKey(const key_t& k, const Culling::CullerDelegate& culler,
            const Technique*& technique_prev, const Mesh*& mesh_prev,
            const Material*& material_prev,
            bool& bones, bool& first, std::vector<key_t>* alphaKeys = nullptr);

        static void renderShadowKeys(
            component::Handle light_h,
            Culling::cullDirection_e cullingType,
            const Culling::CullerDelegate& culler,
            shadowKeyContainer_t keys,
            Technique* normalTech,
            Technique* skinnedTech,
            Technique* instancedTech);

        static bool testShadowKeys(
            const Culling::CullerDelegate& culler,
            shadowKeyContainer_t& returnKeys);
    public:
        static void init();
        static void renderAll(component::Handle cameraE_h,
            const DeferredRender& renderer, Culling::cullDirection_e dir=Culling::NORMAL);
        static void renderShadows(component::Handle light_h,
            Culling::cullDirection_e cullingType = Culling::NORMAL,
            Technique* normalTech = nullptr,
            Technique* skinnedTech = nullptr,
            Technique* instancedTech = nullptr);

        static void addKey(
            const Material* material,
            component::Handle mesh,
            Mesh::groupId_t group0, Mesh::groupId_t groupf,
            component::Handle entity,
            component::Color tint = 0,
            component::Color selfIllumination = 0,
            bool debug = false
        );
        static void addKeys(component::Handle entity_h);
        static void deleteKeys(component::Handle entity_h);
        static void updateKeys(component::Handle entity_h);
};


}

#endif