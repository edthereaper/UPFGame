#ifndef RENDER_CINSTANCEDMESH_H_
#define RENDER_CINSTANCEDMESH_H_

#include "mcv_platform.h"

#include "mesh.h"
#include "../camera/culling.h"

#include "components/transform.h"
#include "components/color.h"

#include "../shader/vertex_declarations.h"

namespace render {

class CInstancedMesh {
    public:
        typedef VertexPUNTInstance::instance_t instance_t;

    private:
        struct  instanceData_t : public CullingAABB {
            bool used = false;
            unsigned instanceIndex = ~0;
            using CullingAABB::dirty;
        };

        CullingAABB aabb;
        bool doRecalculateAABB = false;
        bool usesGlobalAABB = true;
        void recalculateAABB();

        class Culler {
            private:
                CInstancedMesh* self;
                const Culling::CullerDelegate& culling;
                const component::AABB baseAABB;
                bool ignoreWorldChange;
                Culling::mask_t cullerMask;

            public:
                Culler(CInstancedMesh*const& self, const Culling::CullerDelegate& culling,
                    const component::AABB& baseAABB, bool ignoreWorldChange) :
                    self(self), culling(culling),
                    baseAABB(baseAABB),
                    ignoreWorldChange(ignoreWorldChange) {}

                bool operator()(const instance_t& instance) const{
                    auto& aabb = (*self->instanceExtraData)[instance.userDataB];
                    aabb.update(baseAABB, instance.world, ignoreWorldChange);   
                    return culling.cull(aabb);
                }
        };
        friend Culler;

        class Swapper {
            private:
                CInstancedMesh* self;

            public:
                Swapper(CInstancedMesh*const& self) : self(self) {}

                void operator()(const instance_t& a, const instance_t& b) const{
                    auto& aData = (*self->instanceExtraData)[a.userDataB];
                    auto& bData = (*self->instanceExtraData)[b.userDataB];
                    std::swap(aData.instanceIndex, bData.instanceIndex);
                }
        };
        friend Swapper;

    private:
        Mesh* instanceData = nullptr;
        std::vector<instanceData_t>* instanceExtraData = nullptr;
        std::vector<instance_t>* dataBuffer = nullptr;
        size_t nInstances = 0;
        size_t nCulled = 0;
        size_t maxInstances = 0;
        bool created = false;
        bool dirty = false;
        bool culled = false;
        bool instancesWillMove = true;
        bool changed = true;
        float aabbSkin = 0, aabbScale = 1;
        
#ifdef _DEBUG
    public:
        component::Color dbgColor = component::ColorHSL(utils::rand_uniform(1.f));
#endif

    private:
        bool createInstanceData();
        uint32_t getFreshDataIndex(unsigned index);

    public:
        ~CInstancedMesh(){SAFE_DELETE(instanceData); SAFE_DELETE(dataBuffer);}
        CInstancedMesh()=default;
        CInstancedMesh(const CInstancedMesh& copy)=default;
        CInstancedMesh(CInstancedMesh&& move) : 
            instanceData(move.instanceData), dataBuffer(move.dataBuffer), instanceExtraData(move.instanceExtraData),
            nInstances(move.nInstances), nCulled(move.nCulled), maxInstances(move.maxInstances),
            created(move.created), dirty(move.dirty), instancesWillMove(move.instancesWillMove),
            aabbSkin(move.aabbSkin), aabbScale(move.aabbScale) {
            move.dataBuffer = nullptr; 
            move.instanceExtraData = nullptr; 
            move.instanceData = nullptr; 
        }

        inline void createBuffer() {
            assert(dataBuffer == nullptr && !created);
            dataBuffer = new std::vector<instance_t>(0);
        }
        inline void setMaxInstances(size_t max) {maxInstances = max;} 
        inline size_t getMaxInstances() const {return maxInstances;} 
        unsigned addInstance(instance_t&& instance);
        inline unsigned addInstance(const component::Transform& t,
            component::Color tint = component::Color(0),
            component::Color selfIllumination = component::Color(0)
            ) {
            return addInstance(instance_t(t.getWorld(),tint,selfIllumination));
        }
        inline unsigned addInstance(const instance_t::world_t& world,
            component::Color tint = component::Color(0),
            component::Color selfIllumination = component::Color(0)
            ) {
            return addInstance(instance_t(world,tint,selfIllumination));
        }
        bool removeInstance(unsigned index);
        bool replaceInstance(const instance_t::world_t& prev,
            instance_t&& next, bool substantialChange = false);
        bool replaceInstanceWorld(unsigned index, instance_t::world_t&& next,
            bool substantialChange=false);
        bool changeInstanceTint(unsigned index, const component::Color& c);
        bool changeInstanceSelfIllumination(unsigned index,  const component::Color& c);
        void setInstance(unsigned i, instance_t&& next);
        instance_t& getInstance(unsigned i);
        inline bool commitInstances() {
            assert(dataBuffer != nullptr);
            bool isOk = createInstanceData();
            return isOk;
        }

        inline void init(){}
        inline void update(float){}
        void loadFromProperties(std::string elem, utils::MKeyValue& atts);
        void load(std::string name);
        
        void updateInstanceData();

        inline Mesh* getData() {
            if (dirty || culled) {updateInstanceData();}
            return instanceData;
        }
        inline Mesh* getDirtyData() const {return instanceData;}
        inline size_t getNInstances() const {return nInstances;}
        inline size_t getNCulled() const {return nCulled;}

        //returns nCulled
        size_t cull(const Culling::CullerDelegate&);

        inline bool hasChanged() const {
            return changed || (dirty && !instancesWillMove);
        }

        inline bool getInstancesWillMove() const { return instancesWillMove;}
        inline void setInstancesWillMove(bool b=true) {instancesWillMove = b;}

        inline float getAABBSkin() const {return aabbSkin;}
        inline void setAABBSkin(float s) {aabbSkin = s;}
        inline float getAABBScale() const {return aabbScale;}
        inline void setAABBScale(float s) {aabbScale = s;}

        void drawAABBs();
        void drawAABBs(const component::Color& c);
};

}

#endif