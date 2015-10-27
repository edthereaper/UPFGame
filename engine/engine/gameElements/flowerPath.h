#ifndef GAMELEMENTS_FLOWER_PATH_H_
#define GAMELEMENTS_FLOWER_PATH_H_

#include "mcv_platform.h"

#include "utils/random.h"
#include "render/mesh/mesh.h"
#include "render/shader/vertex_declarations.h"

namespace gameElements {
    
//group of flowers that are updated and drawn together
class FlowerGroup {
    private:
        typedef render::VertexFlowerData::instance_t instance_t;
    public:
        static const size_t MAX_SIZE = 2048;
        static const size_t MAX_FRAMES = 16;
        static const float QUAD_SIZE;

        struct flower_t : public instance_t {
            flower_t(const XMVECTOR& p) :
                instance_t(p, utils::die(MAX_FRAMES), DirectX::XMFLOAT2(0,0)) {}
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
        FlowerGroup();

        inline size_t getSize() const {
            return flowers.size();
        }

        inline void add(const std::vector<XMVECTOR>& v) {
            const size_t size(flowers.size() + v.size());
            assert(size < MAX_SIZE);
            flowers.reserve(size);
            flowers.insert(flowers.end(), v.begin(), v.end());
            dirty = true;
        }

        void draw();
        void update(float elapsed);
};

//Manager that creates the new flowers and organizes their operations
class FlowerPathManager {
    private:
        /* spatial index -> FlowerGroup */
        typedef std::multimap<int, FlowerGroup> flowers_t;
        static flowers_t flowers;

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
        typedef std::vector<sproutCoord> sproutCoordV_t;
        typedef std::vector<XMVECTOR> simulation_t;

    private:
        //Optimization: separate into coordinates so we can use lower_bound and upper_bound
        static sproutCoordV_t xCoords;
        static sproutCoordV_t yCoords;
        static sproutCoordV_t zCoords;
        static std::vector<bool> active;
        
    private:
        static std::vector<XMVECTOR> getNewInCyllinder(const XMVECTOR& pos, float radius, float h);
        static simulation_t simulate(const component::AABB& aabb, float density, float step);

    public:
        static void plantCyllinder(const XMVECTOR& pos, float radius, float h, int spatialIndex);
};

inline bool operator<(float a, const FlowerPathManager::sproutCoord& b){
    return a < b.val;
}

}

#endif