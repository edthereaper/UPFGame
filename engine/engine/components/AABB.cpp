#include "mcv_platform.h"
#include "handles/entity.h"
#include "aabb.h"
#include "transform.h"

#include "render/mesh/component.h"
using namespace render;

using namespace utils;
using namespace DirectX;

namespace component {

AABB::AABB(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) :
    center((min+max)*0.5f), hSize((max-min)*0.5f) {}

void AABB::expand(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max)
{
    setCorners(XMVectorMin(getMin(), min), XMVectorMax(getMax(), max));
}

void AABB::setCorners(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max)
{
    center = (min+max)*0.5f;
    hSize  = (max-min)*0.5f;
}

AABB AABB::skinAABB(const float& skin)
{
    hSize = XMVector3Normalize(hSize)*(XMVectorGetX(XMVector3Length(hSize))+skin);
    return *this;
}

AABB AABB::skinAABB(const float& skin) const
{
    AABB ret = *this;
    ret.hSize = XMVector3Normalize(hSize)*(XMVectorGetX(XMVector3Length(hSize))+skin);
    return ret;
}

void CAABB::init()
{
    Handle h(const_cast<CAABB*>(this));
    if (h.isValid() && isInvalid() && h.hasBrother<CMesh>()) {
        Entity* me = h.getOwner();
        CMesh* cmesh = me->get<CMesh>();
        Mesh* mesh = cmesh->getMesh();
        if(mesh != nullptr) {
            *this = mesh->getAABB();
            #ifdef _DEBUG
                if (isInvalid()) {
                    dbg("Entity %s has an invalid AABB!", me->getName().c_str());
                }
            #endif
        }
    }
}

XMVECTOR CAABB::getOffset() const
{
    Handle h(const_cast<CAABB*>(this));
    if (!(h.isValid() && h.hasBrother<CTransform>())) {return utils::zero_v;}
    else {
        return ((CTransform*)h.getBrother<CTransform>())->getPosition();
    }
}

}