#include "mcv_platform.h"
#include "flowerPath.h"

using namespace DirectX;
using namespace utils;

namespace gameElements {

std::vector<XMVECTOR> FlowerPathManager::getCyllinder(XMVECTOR pos, float radius, float h)
{
    float x = XMVectorGetX(pos);
    float y = XMVectorGetY(pos);
    float z = XMVectorGetZ(pos);

    float xMin = x - radius;
    float xMax = x + radius;
    float yMin = y;
    float yMax = y + h;
    float zMin = z - radius;
    float zMax = z + radius;
    
    //Find range in each coordinate
    sproutCoordV_t xRange(
        std::lower_bound(xCoords.begin(), xCoords.end(), xMin),
        std::upper_bound(xCoords.begin(), xCoords.end(), xMax)
    );
    sproutCoordV_t yRange(
        std::lower_bound(yCoords.begin(), yCoords.end(), yMin),
        std::upper_bound(yCoords.begin(), yCoords.end(), yMax)
    );
    sproutCoordV_t zRange(
        std::lower_bound(zCoords.begin(), zCoords.end(), zMin),
        std::upper_bound(zCoords.begin(), zCoords.end(), zMax)
    );

    //Sort ranges by id
    std::sort(xRange.begin(), xRange.end(), &sproutCoord::compById);
    std::sort(yRange.begin(), yRange.end(), &sproutCoord::compById);
    std::sort(zRange.begin(), zRange.end(), &sproutCoord::compById);

    unsigned i=0;
    unsigned j=0;
    unsigned k=0;
    size_t xSize = xRange.size(); 
    size_t ySize = yRange.size(); 
    size_t zSize = zRange.size(); 

    std::vector<XMVECTOR> ret;

    for(; i < xSize; ++i) {
        unsigned xId (xRange[i].id);

        //advance the y and z
        while (j < ySize && yRange[j].id < xId) {j++;}
        if (j >= ySize) {break;}
        while (k < zSize && zRange[k].id < xId) {k++;}
        if (k >= zSize) {break;}

        //We either got a match or not. Also test for already active sprouts
        if (yRange[j].id == xId && zRange[k].id == xId && !active[xId]) {
            //index is in all three vectors => it's within the cyllinder's AABB

            float xVal = xRange[i].val;
            float zVal = zRange[k].val;
            //test circle radius
            if (sq(xVal-x) + sq(zVal-z) <= sq(radius)) {
                ret.push_back(XMVectorSet(xVal, yRange[j].val, zVal, 1.f));
            }
        }
    }
    return ret;
}

}