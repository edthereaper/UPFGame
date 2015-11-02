#ifndef GAMELEMENTS_FLOWER_PATH_H_
#define GAMELEMENTS_FLOWER_PATH_H_

#include "mcv_platform.h"

#include "utils/random.h"
#include "render/mesh/mesh.h"
#include "render/shader/vertex_declarations.h"
#include "level/level.h"

namespace gameElements {
    
//group of flowers that are updated and drawn together
class FlowerGroup {
    private:
        typedef render::VertexFlowerData::instance_t instance_t;
    public:
        static const size_t MAX_FRAMES = 16;
        static const float QUAD_SIZE;
        static const float GROW_TIME;
        static const float GROW_VAR;

        struct flower_t : public instance_t {
            private:
                static uint16_t index;
            public:
                flower_t() : instance_t(utils::zero_v, ~0, 0) {}
                flower_t(const XMVECTOR& p) :
                    instance_t(p, index++, utils::die(MAX_FRAMES*2), DirectX::XMFLOAT2(0,0)) {
                    life = rand_uniform(GROW_VAR, -GROW_VAR);
                }
                friend FlowerGroup;
        };
        static_assert(sizeof(flower_t) == sizeof(render::VertexFlowerData::instance_t),
            "flower_t can't declare new member objects");

    private:
        typedef std::vector<flower_t> flowers_t;
        render::Mesh* instanceData = nullptr;
        flowers_t flowers;
        bool dirty;

        void updateInstanceData();

    public:
        FlowerGroup(size_t maxSize);
        FlowerGroup(FlowerGroup&& move) : 
            instanceData(move.instanceData), flowers(std::move(move.flowers)),
            dirty(move.dirty)
        {
            move.instanceData = nullptr;
        }

        ~FlowerGroup();

        inline size_t getSize() const {
            return flowers.size();
        }

        inline void add(const std::vector<XMVECTOR>& v) {
            const size_t size(flowers.size() + v.size());
            flower_t::index = uint16_t(flowers.size());
            flowers.reserve(size);
            flowers.insert(flowers.end(), v.begin(), v.end());
            dirty = true;
        }

        void draw();
        void drawPoints(const component::Color& color = component::Color::YELLOW);
        void update(float elapsed);
        void zSort();
};

//Manager that creates the new flowers and organizes their operations
class FlowerPathManager {
    private:
        static const float COS_ANGLE_THRESHOLD;
        static const size_t MAX_GROUP_SIZE = 2048;

    public:
        struct sproutCoord {
            static inline bool compById(const sproutCoord& a, const sproutCoord& b) {
                return a.id < b.id;
            };

            float val=0;
            uint32_t id=0;

            sproutCoord()=default;
            sproutCoord(float val) : val(val) {}
            sproutCoord(float val, uint32_t id) : val(val), id(id) {}

            inline bool operator<(const sproutCoord& c) const {
                return val < c.val;
            }
            inline bool operator<(float c) const {
                return val < c;
            }
        };

    private:
        struct point_t {
            XMVECTOR pos = utils::zero_v;
            int spatialIndex = -1;
            point_t()=default;
            point_t(XMVECTOR pos, int spatialIndex) : pos(pos), spatialIndex(spatialIndex) {}
        };
        typedef std::vector<sproutCoord> sproutCoordV_t;
        typedef std::vector<point_t> simulation_t;

    private:
        //Optimization: separate into coordinates so we can use lower_bound and upper_bound
        struct parallelCoord_t {
            sproutCoordV_t xCoords;
            sproutCoordV_t yCoords;
            sproutCoordV_t zCoords;
        };
        //Optimization: separate per spatial index.
        static std::map<int, parallelCoord_t> coords;
        static std::vector<bool> active;
        static component::Transform lastTest;
        static bool lastTestActive;

        /* spatial index -> FlowerGroup */
        typedef std::multimap<int, FlowerGroup> flowers_t;
        static flowers_t flowers;
        static FlowerGroup* simulationHolder;

    private:
        static FlowerGroup* generateSimulationHolder();
        static std::vector<XMVECTOR> getNewInCyllinder(const XMVECTOR& pos, float radius, float h,
            int spatialIndex);
        static std::vector<XMVECTOR> getNewInCyllinder(const XMVECTOR& pos, float radius, float h,
            const parallelCoord_t& c);
        static simulation_t simulate(const component::AABB& aabb, float density, float step);
        static simulation_t loadSimulationFile(const std::string& name);
        static void writeSimulationFile(const std::string& name, const simulation_t&);

    public:
        static void plantCyllinder(const XMVECTOR& pos, float radius, float h, int spatialIndex);
        static void buildSimulationData(Handle levelE, float density, float step);

        static void drawSimulation(const component::Color& color = component::Color::RED);
        static void drawLastTest(const component::Color& color = component::Color::CYAN);
        static void drawSproutedPoints(const component::Color& color= component::Color::YELLOW);
        
        static void updateFlowers(float elapsed);
        static void drawFlowers();

        static void clear();
        static void removeFlowers();
};

inline bool operator<(float a, const FlowerPathManager::sproutCoord& b){
    return a < b.val;
}

}

#endif