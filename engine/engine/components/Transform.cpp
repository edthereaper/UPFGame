#include "mcv_platform.h"
#include "Transform.h"

#include "handles/entity.h"

using namespace DirectX;
using namespace utils;

namespace component {

void CTransform::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    position = atts.getPoint("pos", position);
    rotation = atts.getQuat("rot", rotation);
    bool h = atts.has("sca");
    scale    = atts.getPoint("sca", scale);
    pivot    = atts.getPoint("pivot", pivot);
    if (atts.has("lookAt")) {
        lookAt(atts.getPoint("lookAt", position+getFront()));
    }
}

XMMATRIX Transform::getWorld() const
{
    return XMMatrixAffineTransformation(scale, pivot, rotation, position);
}

XMVECTOR Transform::getFront() const
{
    return XMMatrixRotationQuaternion(rotation).r[2];
}

XMVECTOR Transform::getLeft() const
{
    return XMMatrixRotationQuaternion(rotation).r[0];
}

XMVECTOR Transform::getUp() const
{
    return XMMatrixRotationQuaternion(rotation).r[1];
}

bool Transform::isInFront(XMVECTOR loc) const
{
    return XMVectorGetX(XMVector3Dot(getFront(), loc - position)) > 0.f;
}

bool Transform::isInLeft(XMVECTOR loc) const
{
    return XMVectorGetX(XMVector3Dot(getLeft(), loc - position)) > 0.f;
}

bool Transform::isInFov(XMVECTOR loc, float fov_rad) const
{
    return utils::isInFov(loc, getFront(), loc, fov_rad);
}

void Transform::lookAt(XMVECTOR new_target, XMVECTOR upAux)
{
    #ifdef _LIGHTTOOL
        lookAtPos = new_target;
    #endif
    if (XMVectorSetW(new_target,0) != XMVectorSetW(position,0)) {
	    XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), upAux);
        rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
    }
}

void CRestore::init() {
    CTransform* transform = component::Handle(this).getBrother<CTransform>();
    assert(transform != nullptr);
    set(*transform);
}

}