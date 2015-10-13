#ifndef GAME_ELEMENTS_WEAKSPOT
#define GAME_ELEMENTS_WEAKSPOT

#include "mcv_platform.h"

#include "components/transform.h"
#include "handles/handle.h"
#include "handles/entity.h"

#include "PhysX_USER/PhysicsManager.h"

#include "../gameMsgs.h"

#include "behavior/fsm.h"

namespace gameElements {class WeakSpotFSMExecutor; class CWeakSpot;}
namespace behavior { typedef Fsm<gameElements::WeakSpotFSMExecutor> WeakSpotFSM; }

namespace gameElements {
    
class WeakSpotFSMExecutor {
    public:
        friend CWeakSpot;
        friend behavior::WeakSpotFSM;

		enum states {
			STATE_waiting,
			STATE_active,
			STATE_broken,
			STATE_fightOver,
		};
        
        static inline bool isBroken(behavior::fsmState_t s) {
            return s == STATE_broken;
        }

    private:
        bool signaled = false;
		component::Handle meEntity;
        component::Handle callback;

    private:
        //states
        behavior::fsmState_t waiting(float elapsed);
		behavior::fsmState_t active(float elapsed);
        behavior::fsmState_t broken(float elapsed);
        behavior::fsmState_t fightOver(float elapsed);
        
    public:
		inline behavior::fsmState_t init() { return reset(); }
		inline behavior::fsmState_t reset() { signaled=false; return STATE_waiting;}
		inline void update(float elapsed) {}


        
};

class CWeakSpot {
    public:
        static void initType();
        static const unsigned TAG = 0x00EE3351;
    private:
        behavior::WeakSpotFSM fsm;
    public:
        
        void reset();
        inline void init(){
            fsm.getExecutor().meEntity = component::Handle(this).getOwner();
            fsm.init();
        }
        inline void update(float e) {fsm.update(e);}
        inline void loadFromProperties(std::string, utils::MKeyValue){}

        inline void open(component::Handle callback = Handle()) {
            if (fsm.getState() == WeakSpotFSMExecutor::STATE_waiting) {
                auto& fsme = fsm.getExecutor();
                fsme.callback = callback;
                fsme.signaled = true;
            }
        }
        
        void receive(const physX_user::MsgHitEvent& msg);
        void receive(const MsgDeactivateSmoke&);
        inline void receive(const MsgRevive& msg) {
            reset();
        }

        inline bool isBroken() const {
            return fsm.getExecutor().isBroken(fsm.getState());
        }
};
}

#endif