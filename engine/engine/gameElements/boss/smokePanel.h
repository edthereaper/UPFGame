#ifndef GAME_ELEMENTS_SMOKEPANEL
#define GAME_ELEMENTS_SMOKEPANEL

#include "mcv_platform.h"

#include "../gameMsgs.h"
#include "behavior/fsm.h"
#include "PhysX_USER/PhysicsManager.h"

namespace gameElements {class SmokePanelFSMExecutor; class CSmokePanel;}
namespace behavior { typedef Fsm<gameElements::SmokePanelFSMExecutor> SmokePanelFSM; }

namespace gameElements {

class SmokePanelFSMExecutor {
    public:
		friend behavior::SmokePanelFSM;
		friend CSmokePanel;

		enum states {
			STATE_cold,
			STATE_starting,
			STATE_heating,
			STATE_firing,
            STATE_cooling,
		};

    private:
        component::Handle meEntity;
        component::Handle playerEntity;
        bool signal = false;
        bool alwaysHot = false;
        utils::Counter<float> timer;

    private:
        //states
        behavior::fsmState_t cold(float elapsed);
        behavior::fsmState_t starting(float elapsed);
		behavior::fsmState_t heating(float elapsed);
        behavior::fsmState_t firing(float elapsed);
        behavior::fsmState_t cooling(float elapsed);

    public:
		inline behavior::fsmState_t init() { return reset(); }
		inline behavior::fsmState_t reset() {
            timer.reset();
            if (alwaysHot) {signal = true;}
            return alwaysHot?STATE_heating:STATE_cold;
        }
		inline void update(float elapsed) {}

        void heat(const behavior::fsmState_t& state) {if (state == STATE_cold) {signal = true;}}
        void fire(const behavior::fsmState_t& state) {if (state == STATE_heating) {signal = true;}}
        void cool(const behavior::fsmState_t& state) {if (state == STATE_firing) {signal = true;}}

        inline static bool isHot(const behavior::fsmState_t& state) {return state == STATE_firing;}
};

class CSmokePanel {
    public:
        static const unsigned TAG = 0x4321B000;
        static const unsigned TAG_ALWAYSHOT = 0xB0BAFEE7;
        static void initType();

    private:
        unsigned index=0;
        unsigned subindex=0;
        bool playerInside = false;
		utils::Counter<float> timer;

    public:
        behavior::SmokePanelFSM fsm;

        void reset();
        void init();
        void update(float);
        void loadFromProperties(std::string elem, utils::MKeyValue atts);

        inline bool isHot() const {
            return fsm.getExecutor().isHot(fsm.getState());
        }

        inline void receive(const MsgActivateSmokeWarning& msg) {
            if (msg.mask & (1<<subindex)) {fsm.getExecutor().heat(fsm.getState());}
        }
        inline void receive(const MsgActivateSmokeIfHot& msg) {
            fsm.getExecutor().fire(fsm.getState());
        }
        inline void receive(const MsgDeactivateSmoke& msg) {
            fsm.getExecutor().cool(fsm.getState());
        }
        inline void receive(const MsgSetPlayer& msg) {
            fsm.getExecutor().playerEntity = msg.playerEntity;
        }
        inline void receive(const physX_user::MsgCollisionEvent& msg) {
            assert(fsm.getExecutor().playerEntity.isValid());
            if (fsm.getExecutor().playerEntity == msg.entity) {
                playerInside = msg.enter;
            }
        }
        inline void receive(const MsgRevive&) {
            reset();
        }

        inline void set(unsigned nindex, unsigned nsubindex) {
            index = nindex;
            subindex = nsubindex;
        }
};

}

#endif