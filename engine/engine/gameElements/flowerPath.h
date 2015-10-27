#ifndef GAMELEMENTS_FLOWER_PATH_H_
#define GAMELEMENTS_FLOWER_PATH_H_

#include "mcv_platform.h"

namespace gameElements {

class FlowerPathManager {
    private:
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
        };

        typedef std::vector<sproutCoord> sproutCoordV_t;

    private:
        sproutCoordV_t xCoords;
        sproutCoordV_t yCoords;
        sproutCoordV_t zCoords;
        std::vector<bool> active;
        
    private:
        std::vector<XMVECTOR> getCyllinder(XMVECTOR pos, float radius, float h);
};

}

#endif