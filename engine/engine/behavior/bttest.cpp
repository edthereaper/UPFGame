#include "mcv_platform.h"
#ifdef _TEST

#include "bt.h"
using namespace behavior;
using namespace utils;

namespace test {

// HOW TO CREATE A Bt:

//define an executor
struct _TBtExecutor {
    public:
        utils::Counter<float> cooldownPistol;
        void init(){cooldownPistol.reset();}
        void reset(){cooldownPistol.reset();}
        void update(float elapsed) {
            cooldownPistol.count(elapsed);
        }

        enum state_e {
            SOLDIER = 5,
                ESCAPE,
                COMBAT,
                    PURSUIT,
                    ALERT,
                        ATTACK,
                            PCOOLDOWN,
                                PISTOL,
                            GRENADE,
                IDLE,
                    SMOKE,
                    STRETCH,
                    YELL,
        };

        //actions
    	ret_e idle(float elapsed) {
            dbg("IDLE ");
            return ret_e::DONE;
        }
		ret_e escape(float elapsed){
            dbg("ESCAPE ");
            return chance(1,5)? ret_e::DONE : ret_e::STAY;
        }
		ret_e alert(float elapsed){
            dbg("ALERT ");
            return ret_e::DONE;
        }
		ret_e attack(float elapsed){
            dbg("ATTACK ");
            return ret_e::DONE;
        }
		ret_e grenade(float elapsed){
            dbg("GRENADE ");
            return ret_e::DONE;
        }
		ret_e pistol(float elapsed){
            cooldownPistol.reset();
            cooldownPistol.count(-20.f); //cooldown Time
            dbg("PISTOL ");
            return ret_e::DONE;
        }
		ret_e pursuit(float elapsed){
            dbg("PURSUIT ");
            return ret_e::DONE;
        }
		ret_e smoke(float elapsed){
            dbg("SMOKE ");
            return ret_e::DONE;
        }
		ret_e stretch(float elapsed){
            dbg("STRETCH ");
            return ret_e::DONE;
        }
		ret_e yell(float elapsed){
            dbg("YELL ");
            return ret_e::DONE;
        }

        //conditions
		bool canEscape(float elapsed) const {
            bool escaped = chance(2,10);
            dbg("escape?%c ", escaped?'y':'n');
            return escaped;
        }

        bool isOnRange(float elapsed) const {
            bool onRange = chance(4,10);
            dbg("onRange?%c ", onRange?'y':'n');
            return onRange;
        }

        bool isMotivated(float elapsed) const {
            bool motivated = chance(7,10);
            dbg("motivated?%c ", motivated?'y':'n');
            return motivated;
        }

        bool noDanger(float elapsed) const {
            bool safe = chance(7,10);
            dbg("safe?%c ", safe?'y':'n');
            return safe;
        }
        
        bool inDanger(float elapsed) const {
            return !noDanger(elapsed);
        }
        
        bool testCDPistol(float elapsed) const {
            bool ok = cooldownPistol.get() >= 0;
            dbg("pistol-cooldown?%c ", ok?'y':'n');
            return ok;
        }
};

//The complete BT type
typedef Bt<_TBtExecutor> _TBt;

}

using namespace test;
// ... in the namespace behavior ...
namespace behavior {
// ... we need to instantiate the static root node...
_TBt::nodeId_t _TBt::rootNode = _TBt::INVALID_NODE;
// ... and the states array...
_TBt::container_t _TBt::nodes;
// ... and initialize it
void _TBt::initType()
{
    /* Can also use methods createRoot and createChild. They're more versatile,
       cut diving into classes to get the enums or actions can be ugly to read*/

    /*                  node-id     parent-id   node-type       condition       action      */
    BT_CREATEROOT    (  SOLDIER,                PRIORITY                                    );  
    BT_CREATECHILD_CA(  ESCAPE,     SOLDIER,    LEAF,           canEscape,      escape      );
    BT_CREATECHILD_C (  COMBAT,     SOLDIER,    SEQUENCE,       inDanger                    );  
    BT_CREATECHILD_A (  PURSUIT,    COMBAT,     LEAF,                           pursuit     );
    BT_CREATECHILD_A (  ALERT,      COMBAT,     CHAIN,                          alert       );
    BT_CREATECHILD_A (  ATTACK,     ALERT,      PRIORITY,                       attack      );
    BT_CREATECHILD_C (  PCOOLDOWN,  ATTACK,     CHAIN,          testCDPistol                );
    BT_CREATECHILD_CA(  PISTOL,     PCOOLDOWN,  LEAF,           isOnRange,      pistol      );
    BT_CREATECHILD_CA(  GRENADE,    ATTACK,     LEAF,           isMotivated,    grenade     );
    BT_CREATECHILD_CA(  IDLE,       SOLDIER,    SEQUENCE_WHILE, noDanger,       idle        );
    BT_CREATECHILD_A (  SMOKE,      IDLE,       LEAF,                           smoke       );
    BT_CREATECHILD_A (  STRETCH,    IDLE,       LEAF,                           stretch     );
    BT_CREATECHILD_CA(  YELL,       IDLE,       LEAF,           isMotivated,    yell        );
}

}

namespace test {
// -------------------------------------------
void testBT(){
    _TBt::initType();
    _TBt test1;
    test1.init();
    dbg("TEST: testBt ---- BEGIN\n");
    for (auto i=0; i<50; i++) {
        test1.update(1.f);
        dbg("\n");
    }
    dbg("TEST: testBt ---- END\n");
}

}

#endif