#ifndef LOGIC_TRIGGER_H_
#define LOGIC_TRIGGER_H_

#include "handles/entity.h"
#include "components/AABB.h"
#include "components/transform.h"
#include "gameElements/gameMsgs.h"

#include "level/spatialIndex.h"

namespace logic {

const uint32_t Trigger_AABB_TAG = 0xAC7AC7FF;

/* Curiously recurring template pattern 
    Trigger duck typing:
    - Implements trigger methods:
        onEnter()
        onExit()
        onStay()
*/
template <class Trigger>
class Trigger_AABB {
    protected:
        bool isPlayerInside = false;
        component::Handle player_h;
        bool active = true;
    public:
        Trigger_AABB()=default;
        Trigger_AABB(component::Handle player_h) : player_h(player_h) {}
        inline void setPlayer(component::Handle entity) {player_h = entity;}


        inline bool test() {
            component::Entity* me(component::Handle(static_cast<Trigger*>(this)).getOwner());
            component::CAABB* aabb = me->get<component::CAABB>();
            if (aabb == nullptr || aabb->isInvalid()) {
                return false;
            } else {
                component::CTransform* t = me->get<component::CTransform>();

                component::Entity* player = player_h;
                component::CAABB* playerAABB = player->get<component::CAABB>();
                component::CTransform* pt = player->get<component::CTransform>();

                return (*aabb + t->getPosition()).intersects(*playerAABB + pt->getPosition());
            }
        }

        inline void init() {
            isPlayerInside = test();
        }

        inline void update(float elapsed) {
            if (active) {
                bool nowInside(test());
                if (!isPlayerInside && nowInside) {
                    static_cast<Trigger*>(this)->onEnter(elapsed);
                } else if (isPlayerInside && !nowInside) {
                    static_cast<Trigger*>(this)->onExit(elapsed);
                } else if (isPlayerInside && nowInside) {
                    static_cast<Trigger*>(this)->onStay(elapsed);
                } else {
                    static_cast<Trigger*>(this)->onAbsent(elapsed);
                }
                isPlayerInside = nowInside;
            }
        }

        inline void setActive(bool b =true) {active = b;}
};

class CScriptTrigger : public Trigger_AABB<CScriptTrigger>{
    public:
        static void initType();
    private:
        component::Handle player_h;
        char script[32];
		char data[32];
    public:
        CScriptTrigger()=default;
        CScriptTrigger(component::Handle player_h, std::string newName, std::string newdata = "") :
            Trigger_AABB(player_h) {
            setScript(newName);
			setArgs(newdata);
        }

        inline void setScript(std::string str) {
            strncpy(script, str.c_str(), 32);
        }
        inline void setArgs(std::string str) {
			strncpy(data, str.c_str(), 32);
        }
        
        inline void onStay(float) {/*Do nothing*/}
        inline void onAbsent(float) {/*Do nothing*/}

        void onEnter(float);
        void onExit(float);

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts){}
        inline void receive (const gameElements::MsgSetPlayer& msg) {setPlayer(msg.playerEntity);}
};

class CSpatialIndex : public Trigger_AABB<CSpatialIndex>, public level::SpatiallyIndexed {
    public:
        static void initType();
    private:
        component::Handle player_h;
    public:
        CSpatialIndex()=default;
        CSpatialIndex(component::Handle player_h, int index) :
            Trigger_AABB(player_h) {spatialIndex = index;}
        
        inline void onStay(float) {/*Do nothing*/}
        inline void onAbsent(float) {/*Do nothing*/}
        inline void onExit(float) {/*Do nothing*/}

        void onEnter(float);

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts){}
        inline void receive (const gameElements::MsgSetPlayer& msg) {setPlayer(msg.playerEntity);}
};

}

#endif
