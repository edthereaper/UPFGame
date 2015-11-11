#ifndef RENDER_CINSTANCEDMESH_H_
#define RENDER_CINSTANCEDMESH_H_

#include "mcv_platform.h"

#include "mesh.h"
#include "../camera/culling.h"

#include "components/transform.h"
#include "components/color.h"

#include "../shader/vertex_declarations.h"
#include "app.h"

namespace render {

class CInstancedMesh {
    public:
        typedef VertexPUNTInstance::instance_t instance_t;

    private:
        struct  instanceData_t : public CullingAABB {
            bool used = false;
            unsigned instanceIndex = ~0;
            unsigned cleanTimestamp = 0;
            using CullingAABB::dirty;
            Culling::mask_t cullerMask;
        };

        CullingAABB aabb;
        bool doRecalculateAABB = false;
        bool usesGlobalAABB = true;
        unsigned cleanTimestamp=0;
        void recalculateAABB();

        class Culler {
            private:
                CInstancedMesh*const self;
                const Culling::CullerDelegate& culling;
                const component::AABB baseAABB;
                const bool ignoreWorldChange;
                const Culling::mask_t cullerMask;
                const bool dirty;
                const unsigned cleanTimestamp;

            public:
                Culler(CInstancedMesh*const& self, const Culling::CullerDelegate& culling,
                    const component::AABB& baseAABB, bool ignoreWorldChange) :
                    self(self), culling(culling), baseAABB(baseAABB),
                    ignoreWorldChange(ignoreWorldChange), cullerMask(culling.getMask()),
                    dirty(culling.hasChanged()), cleanTimestamp(self->cleanTimestamp) {}

                bool operator()(instance_t& instance) const{
                    auto& aabb = (*self->instanceExtraData)[instance.userDataB];
                    bool aabbDirty = aabb.dirty;
                    aabb.update(baseAABB, instance.world, ignoreWorldChange);
                    if (dirty || aabbDirty) {
                        aabb.cleanTimestamp = cleanTimestamp;
                        aabb.cullerMask.reset();
                    }
                    if (aabb.cleanTimestamp == cleanTimestamp) {
                        bool b = culling.cull(aabb);
                        if (b) {
                            aabb.cullerMask |= cullerMask;
                            return true;
                        } else {
                            aabb.cullerMask &= ~cullerMask;
                            return false;
                        }
                    } else {
                        return (aabb.cullerMask & cullerMask) != 0;
                    }
                }
        };
        friend Culler;

        class BitMaskChecker {
            private:
                CInstancedMesh* self;
                Culling::mask_t cullerMask;

            public:
                BitMaskChecker(CInstancedMesh*const& self,
                    const Culling::CullerDelegate& culling) :
                    self(self), cullerMask(culling.getMask()) {}

                bool operator()(instance_t& instance) const{
                    auto& aabb = (*self->instanceExtraData)[instance.userDataB];
                    return (aabb.cullerMask & cullerMask) != 0;
                }
        };
        friend BitMaskChecker;

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
        bool dirty = true;
        bool worldDirty = true;
        bool updateCulled = false;
        bool culled = false;
        bool instancesWillMove = true;
        bool changed = true;
        bool isComplete = false;
        float aabbSkin = 0, aabbScale = 1;
        
#ifdef _DEBUG
    public:
        component::Color dbgColor = component::ColorHSL(utils::rand_uniform(1.f));
#endif

    private:
        bool createInstanceData();
        uint32_t getFreshDataIndex(unsigned index);
        instance_t& getInstance_p(unsigned i);

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
        inline void update(float){++cleanTimestamp;}
        void loadFromProperties(std::string elem, utils::MKeyValue& atts);
        void load(std::string name);
        
        void updateInstanceData(size_t count);

        inline Mesh* getData(bool complete = false) {
            if (!complete && culled && (dirty || updateCulled)) {
                updateInstanceData(nCulled);
            } else if (dirty || (complete && !isComplete)) {
                updateInstanceData(nInstances);
            }
            return instanceData;
        }
        inline Mesh* getDirtyData() const {return instanceData;}
        inline size_t getNInstances() const {return nInstances;}
        inline size_t getNCulled() const {return nCulled;}
        
        //partitions according to culler and returns nCulled
        size_t cull(const Culling::CullerDelegate& culler);
        //partitions according to bit masks
        size_t partitionOnBitMask(const Culling::CullerDelegate&);
        //tests the culling withut doing a partition and returns nCulled
        size_t testCull(const Culling::CullerDelegate&);
        //tests culling against the AABB that encapsulates all instances
        bool cullHighLevel(const Culling::CullerDelegate& cullerDelegate);

        inline bool hasChanged() const {
            return changed || (dirty && !instancesWillMove);
        }
        
        inline bool isWorldDirty() const {
            return worldDirty;
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