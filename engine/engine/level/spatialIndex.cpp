#include "mcv_platform.h"
#include "spatialIndex.h"

#include "components/aabb.h"
using namespace component;

#include "logic/trigger.h"
using namespace logic;

#include "level.h"

namespace level {
    
void SpatiallyIndexed::findSpatialIndexAux(Handle h)
{
    auto manager = getManager<CSpatialIndex>();
    CTransform* t = h.getBrother<CTransform>();
    auto pos = t->getPosition();
    for (auto m : *manager) {
        Entity* owner = Handle(m).getOwner();
        CTransform* mT = owner->get<CTransform>();
        CAABB* aabb = owner->get<CAABB>();
        if ((*aabb+mT->getPosition()).contains(pos)) {
            setSpatialIndex(m->getSpatialIndex());
            break;
        }
    }
}

bool SpatiallyIndexed::isSpatiallyGood(int threshold) const
{
    if (!CLevelData::currentLevel.isValid()) {return false;}
    auto currentSpatialIndex = ((CLevelData*)CLevelData::currentLevel)->getSpatialIndex();
    return spatialIndex < 0 || currentSpatialIndex < 0 || threshold < 0 ||
        std::abs(spatialIndex - currentSpatialIndex) <= threshold;
}

}
