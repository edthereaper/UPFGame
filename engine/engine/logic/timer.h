#ifndef LOGIC_TIMER_H_
#define LOGIC_TIMER_H_

#include "handles/entity.h"

namespace logic {

/* Component TIMER AABB
    Generates events onTimeout

    XML properties:
        xa se verá
 */
class CTimer {
    private:
        utils::Counter<float> timer;
        float timeToLeave;
        bool active;
        char name[32];
    public:
        CTimer()=default;

        inline void init() {}
        inline void update(float elapsed) {
            if(active && timer.count(elapsed) >= 0) {
                onTimeOut();
                active = false;
            }
        }

        inline void start(float ttl) {
            timer.reset();
            timer.count(-ttl);
            timeToLeave = ttl;
            active = true;
        }

        inline float get() const {return timer.get();}
        inline float getTimeToLeave() const {return timeToLeave;}

        void onTimeOut() {
            std::string script("onTimeOut");
            script += name;
            //lua.run(script);
        }

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts){}
};

}

#endif
