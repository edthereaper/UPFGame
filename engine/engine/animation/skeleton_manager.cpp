#include "mcv_platform.h"
#include "skeleton_manager.h"

using namespace DirectX;

namespace animation {

CalVector toCalVector(XMVECTOR v)
{
    return CalVector(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));
}

XMVECTOR toXMVECTOR(CalVector q)
{
    return XMVectorSet(q.x, q.y, q.z, 1.0f);
}

CalQuaternion toCalQuaternion(XMVECTOR v)
{
    return CalQuaternion(
        XMVectorGetX(v), XMVectorGetY(v),
        XMVectorGetZ(v), -XMVectorGetW(v));
}

XMVECTOR toXMQuaternion(CalQuaternion q)
{
    return XMVectorSet(q.x, q.y, q.z, -q.w);
}

CoreModel::Manager CoreModel::manager;

}