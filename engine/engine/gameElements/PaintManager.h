#ifndef PAINT_MANAGER_H_
#define PAINT_MANAGER_H_

#include "render/shader/vertex_declarations.h"
#include "render/mesh/mesh.h"

#include "handles/entity.h"
#include "components/color.h"
#include "components/aabb.h"

#include "PhysX_USER/PhysicsManager.h"

#include "utils/xmlParser.h"

namespace gameElements {

class CPaintGroup {
    public:
        typedef render::VertexPaintData::instance_t instance_t;
        friend CPaintGroup;

        static void fixedUpdate(float);
        static void tryRemoval();

    private:
        static const float RADIUS_TOLERANCE;
        static const float DECAY_TOLERANCE;
        static const unsigned N_RAYCAST_TESTS_DOWN;
        static const unsigned N_RAYCAST_TESTS_UP;
        static const unsigned N_RAYCAST_TESTS;
        static const unsigned TRY_REMOVAL_THRESHOLD;

        static unsigned nTests;
        static unsigned nGroupRemoval;
        static unsigned nPaintRemoval;

    private:
        static float calculateShaderAmount(float radius, float dist, float decay, float intensity);
        static bool testInside(instance_t s, XMVECTOR pos, float radius);

    private:
        render::Mesh* instanceData = nullptr;
        std::vector<instance_t>* buffer = nullptr;
        std::vector<PxShape*>* shapes;
        physx::PxRigidActor* actor = nullptr;
        physx::PxShape* globalQueryShape = nullptr;
        unsigned maxInstances = 4096;
        unsigned nInstances = 0;
        bool dirty = true;
        component::Color color = component::Color::MOUNTAIN_GREEN;
        float intensity = 0.9f;
        float siIntensity = 1.f;
        float decay = 0.5f;

        component::AABB aabb; 

        component::Handle chain;
        unsigned chainDepth = 0;

    private:
        enum rayTestResult_e {
            NEW_PAINT,
            NO_PAINTABLE,
            REDUNDANT,
        };

        void removeInstance(unsigned n);
        void updateInstanceData();
        bool tryRemoval(unsigned index);
        bool addInstanceAux(const XMVECTOR& pos, float radius);
        void addInstanceImpl(const XMVECTOR& pos, float radius);
        rayTestResult_e testInstance(const XMVECTOR& pos, float radius); //not chained
        rayTestResult_e testRays(const XMVECTOR& pos, float radius, unsigned ignore=~0);
        rayTestResult_e testDirection(const XMVECTOR& pos, const PxVec3& dir, float radius, unsigned ignore=~0);

    public:
        ~CPaintGroup() { 
            SAFE_DELETE(buffer);
            SAFE_DELETE(instanceData);
            nInstances=0;
            if(chain.isValid()) {
                chain.destroy();
            }
            if (globalQueryShape != nullptr) {
                globalQueryShape->release();
                globalQueryShape = nullptr;
            }
        }
        void init();
        inline void update(float){}
        inline void loadFromProperties(const std::string& elem, utils::MKeyValue& atts) {
            if (elem == "PaintGroup") {
                maxInstances = atts.getInt("maxInstances", maxInstances);
                intensity = atts.getFloat("intensity", intensity);
                decay = atts.getFloat("decay", decay);
                siIntensity = atts.getFloat("siIntensity", siIntensity);            
            } else  if (elem == "color"){
                color.loadFromProperties(elem, atts);
            }
        }
        void draw();
        void drawVolume();
        void drawAABB();
        void drawChain();
        void drawVolumeChain();
        
        inline void reset() {
            if (buffer != nullptr) {
                for (unsigned i=0; i<nInstances; i++) {
                    (*buffer)[i] = instance_t();
                    actor->detachShape(*(*shapes)[i]);
                    (*shapes)[i] = nullptr;
                }
            }
            nInstances = 0;
            if(chain.isValid()) {
                chain.destroy();
            }
        }

        inline bool addInstance(const XMVECTOR& pos, float radius) {
            return (testInstance(pos, radius) == NEW_PAINT) && addInstanceAux(pos, radius);
        }

        inline size_t getNInstances() const {return nInstances;}
        
        inline unsigned getMaxInstances() const {return maxInstances;}
        inline float getSIIntensity() const {return siIntensity;}
        inline float getIntensity() const {return intensity;}
        inline float getDecay() const {return decay;}
        inline component::Color getColor() const {return color;}

        inline void setMaxInstances(unsigned max) {maxInstances = max;}
        inline void setIntensity(float f) {intensity = f;}
        inline void setSIIntensity(float f) {siIntensity = f;}
        inline void setDecay(float f) {decay = f;}
        inline void setColor(const component::Color& c) {color = c;}
};

class PaintManager : private utils::XMLParser {
    private:
        typedef std::map<unsigned, component::Handle> map_t;
        static map_t brushes;
        static unsigned currentId;
        static bool aborted;
        static float fireLevel;

        PaintManager()=default;
        void onStartElement(const std::string &elem, utils::MKeyValue &atts);
        void onEndElement(const std::string &elem);

    public:
        static inline CPaintGroup* getBrush(unsigned id) {
            if (aborted) {return nullptr;}
            auto i = brushes.find(id);
            assert(i != brushes.end());
            Entity* e = i->second;
            return e->get<CPaintGroup>();
        }

        static void load() {
            PaintManager ret;
            aborted = false;
            ret.xmlParseFile("data/brushes.xml");
        }

        static void clear() {
            for (auto& i : brushes) {
                i.second.destroy();
            }
            brushes.clear();
        }

        static void reset() {
            fireLevel = -1000000;
            aborted = false;
            for (auto& i : brushes) {
                CPaintGroup* pg(i.second.getSon<CPaintGroup>());
                pg->reset();
            }
        }

        static inline void abort() {aborted = true;}

        static inline void setFireLevel(float f) {fireLevel = f;}
        static inline float getFireLevel() {return fireLevel;}
};


}

#endif