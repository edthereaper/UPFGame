#include "mcv_platform.h"
#include "ikHandler.h"

using namespace DirectX;

namespace animation {

bool ikHandler_t::operator()(XMVECTOR nA, XMVECTOR nC, XMVECTOR nNormal)
{
    A = nA;
    C = nC;
    normal = nNormal;

    XMVECTOR dir = C-A;
    float AC = XMVectorGetX(XMVector3Length(dir));
    dir = XMVector3Normalize(dir);

    if (AC > AB + BC) {
        state = TOO_FAR;
        B = A + dir * AB;
        return false;
    }

    float num = utils::sq(AB) - utils::sq(BC) - utils::sq(AC);
    float den = -2 * AC;

    if (den == 0) {
        state = UNKNOWN;
        return false;
    }

    float a2 = num / den;
    float a1 = AC - a2;

    // h^2 + a1^2 = AB^2
    float h2 = AB*AB - a1 * a1;
    if (h2 < 0.f) {
        state = TOO_SHORT;
        B = A - dir * AB;
        return false;
    }
    h = sqrtf(h2);

    XMVECTOR perpendicular = XMVector3Normalize(XMVector3Cross(normal, dir));
    B = A + dir * a1 + h * perpendicular;
    state = NORMAL;
    return true;
}


}