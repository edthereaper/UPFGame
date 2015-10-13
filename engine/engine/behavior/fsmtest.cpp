#include "mcv_platform.h"
#ifdef _TEST

#include "fsm.h"
using namespace behavior;

#include "utils/utils.h"
using namespace utils;

namespace test {
    
// -------------------------------------------
// HOW TO CREATE A Fsm:

//define an executor
struct _TFsmExecutor {
    /* recommended: STATE_<s> for each state <s>.
       Starting at 0 and no more than the NStates of _TFsm! */
    enum state_e {
        STATE_do0,
        STATE_do1,
        STATE_do2,
        STATE_do3
    };

    /* Data used by the executor */
    int n = 0;

    //states
    fsmState_t do0(float elapsed) {
        n++;
        dbg("do0, n=%d\n", n);
        if (n>5) {return STATE_do1;}
        else {return STATE_do0;}
    }
    fsmState_t do1(float elapsed) {
        n/=2;
        dbg("do1, n=%d\n", n);
        if (n<3) {return STATE_do3;}
        else {return STATE_do1;}
    }
    fsmState_t do2(float elapsed) {
        n+=5;
        dbg("do2, n=%d\n", n);
        return STATE_do2;
    }
    fsmState_t do3(float elapsed) {
        n--;
        dbg("do3, n=%d\n", n);
        return STATE_do2;
    }
    //end states

    fsmState_t init(){return STATE_do0;}
    fsmState_t reset(){return STATE_do0;}
    void update(float elapsed) {}
};
//The complete FSM type
typedef Fsm<_TFsmExecutor> _TFsm;

}

using namespace test;
// ... in the namespace behavior ...
namespace behavior {
// ... we need to instantiate the states array...
_TFsm::container_t _TFsm::states;
// ... and initialize it
void _TFsm::initType()
{   
    //4 different ways of doing the same thing
    SET_FSM_STATE_N(0, do0);
    SET_FSM_STATE_N(_TFsmExecutor::STATE_do1, do1);
    SET_FSM_STATE(do2);
    states[_TFsmExecutor::STATE_do3] = &executor_t::do3;
}
}


namespace test {
// -------------------------------------------
void testFSM(){
    _TFsm::initType();
    _TFsm test1;
    _TFsm test2;
    test1.init();
    test2.init();
    dbg("TEST: testFSM --- BEGIN\n");
    for (auto i=0; i<15; i++) {
        test1.update(0);
        test2.update(0);
    }
    dbg("TEST: testFSM --- END\n");
}

}

#endif
