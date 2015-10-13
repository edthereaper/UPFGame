#ifndef GAME_ELEMENTS_FLYINGMOBILE_H_
#define GAME_ELEMENTS_FLYINGMOBILE_H_

#include "mcv_platform.h"

#include "components/transform.h"
#include "handles/handle.h"
#include "handles/entity.h"

#include "behavior/fsm.h"

namespace gameElements {class FlyingMobileFSMExecutor; class CFlyingMobile;}
namespace behavior { typedef Fsm<gameElements::FlyingMobileFSMExecutor> FlyingMobileFSM; }
namespace gameElements {

class FlyingMobileFSMExecutor {
    public:
		friend CFlyingMobile;
		friend behavior::FlyingMobileFSM;

		enum states {
			STATE_waiting,
			STATE_running,
			STATE_done,
		};

    private:
		component::Handle meEntity;
        component::Transform origin;
        component::Transform target;
        float speed=0;
        float rotSpeed=0;
        bool signaled=false;

    private:
        //states
        behavior::fsmState_t waiting(float elapsed);
		behavior::fsmState_t running(float elapsed);
        behavior::fsmState_t done(float elapsed);

    public:
		inline behavior::fsmState_t init() { return reset(); }
		inline behavior::fsmState_t reset() { signaled=false; return STATE_waiting;}
		inline void update(float elapsed) {}

        bool isComplete();
};

// TODO: export animation
class CFlyingMobile {
    public:
        friend FlyingMobileFSMExecutor;
    private:
		behavior::FlyingMobileFSM fsm;
    public:
        inline void init(){
            fsm.getExecutor().meEntity = component::Handle(this).getOwner();
            fsm.init();
        }
        inline void loadFromProperties(std::string, utils::MKeyValue){}
        inline void update(float elapsed) {fsm.update(elapsed);}

        void stop();

        inline bool isComplete() {return fsm.getExecutor().isComplete();}

        inline void setParameters(const component::Transform& t,
            float speed, float rotSpeed, bool signal=false) {
            auto& fsme = fsm.getExecutor();
            fsme.target.set(t);
            fsme.speed = speed;
            fsme.rotSpeed = rotSpeed;
            fsme.signaled = signal;
        }

        inline void signal(bool b = true) {fsm.getExecutor().signaled = b;}
};

}

#endif

