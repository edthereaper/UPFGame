#if 0
#ifndef WHITEBOXES_H_
#define WHITEBOXES_H_

#include "utils/data_provider.h"
#include "components/AABB.h"

struct Checkpoint {
    component::CAABB bb;
    XMVECTOR pos;

    Checkpoint() = default;
    Checkpoint(component::CAABB bb, XMVECTOR pos) : bb(bb), pos(pos) {}
};

struct WhiteboxesLevel {
    
    std::vector<Checkpoint> checkpoints;

    XMVECTOR start, startRotation;
    component::CAABB finish;
};

WhiteboxesLevel loadWhiteboxes(const char* name);
WhiteboxesLevel loadWhiteboxes(utils::DataProvider& dp);
void createRope(XMVECTOR position, XMVECTOR rotation, int numNodes);

#endif
#endif