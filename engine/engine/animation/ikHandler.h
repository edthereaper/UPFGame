#ifndef INC_IK_HANDLER_H_
#define INC_IK_HANDLER_H_

#include "mcv_platform.h"

namespace animation {

struct ikHandler_t {
    public:
        enum state_e {
            NORMAL ,
            TOO_FAR ,
            TOO_SHORT ,
            UNKNOWN
        };

    public:
        //Given
        float    AB;        // Obtained from Bone
        float    BC;        // Obtained from Bone

        //Found
        XMVECTOR A, B, C;
        float    h;
        XMVECTOR normal;
        state_e   state = UNKNOWN;

    public:
        ikHandler_t() = default;
        ikHandler_t(float AB, float BC) :
            AB(AB), BC(BC), A(A),C(C), normal(normal) {}
        bool operator()(XMVECTOR A, XMVECTOR C, XMVECTOR normal);
};

}

#endif

