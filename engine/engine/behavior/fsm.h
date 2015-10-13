#ifndef BEHAVIOR_FSM_H_
#define BEHAVIOR_FSM_H_

#include "mcv_platform.h"
#include <map>

namespace behavior {

typedef uint32_t fsmState_t;

/* FINITE STATE MACHINE (AKA AUTOMATON)
 * T is the "executor" class. It has all the data, and also declares the state execution functions
 *
 * Duck typing for T:
 *      fsmState_t init()   -> returns the initialized state
 *      fsmState_t reset()  -> returns the initialized state
 *      void update(float elapsed)
 */
template<class T>
class Fsm
{
    public:
        typedef fsmState_t state_t;
        static void initType();
        /* returns the next state */
        typedef state_t (T::*handler_t)(float /*elapsed*/);
    private:
        typedef T executor_t;
        typedef std::map<state_t, handler_t> container_t;
        static container_t states;

    private:
        T executor;
	    state_t state = 0;

    public:
        inline void init() {changeState(executor.init());}
        inline void reset() {changeState(executor.reset());}
        inline const state_t getState() const {return state;}
        inline void update(float elapsed) {
            executor.update(elapsed);
            assert (states.find(state) != states.end());
            handler_t handler(states[state]);
            assert (handler != nullptr);
            /* call the function on the executor variable*/
            changeState((executor.*handler)(elapsed));
        }

        inline void changeState(state_t newState) {
            assert (states.find(state) != states.end());
            state = newState;
        }

        inline T& getExecutor() {return executor;}
        inline const T& getExecutor() const {return executor;}

        /* use this in Fsm<..>::initTypes() to initialize the state array */
        #define SET_FSM_STATE_N(n, fm) states[n] = &executor_t::fm
        /* use this in Fsm<..>::initTypes() to initialize the state array              
           only works if you declare enumerated STATE_fm values
           (see fsmtest.cpp for an example) */
        #define SET_FSM_STATE(fm) SET_FSM_STATE_N(executor_t::STATE_##fm, fm)
};
}


#ifdef _TEST
namespace test {
void testFSM();
}
#endif

#endif