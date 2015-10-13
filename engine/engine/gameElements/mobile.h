#ifndef GAME_ELEMENTS_MOBILE_H_
#define GAME_ELEMENTS_MOBILE_H_

#include "mcv_platform.h"

namespace gameElements {

class CMobile {
    public:
        enum refType_e {REF_ABSOLUTE, REF_RELATIVE};
        enum moveType_e {
            MOVE_ACCELERATE, //Increase speed gradually p1:= acceleration p2:=initial speed
        };
        enum shakeType_e {
            SHAKE_LINEAR,
            SHAKE_SIN,
            SHAKE_COS,
        };

    private:
        enum {NOTHING, MOVING, SHAKING} currentAction;
        union {
            moveType_e moveType;
            shakeType_e shakeType;
        };
        XMVECTOR v;
        XMVECTOR origin;
        union {
            struct {float accel, speed;};
            struct {float amplitude, frequency;};
        };
        utils::Counter<float> timer;

    public:
        void init();
        inline void loadFromProperties(std::string, utils::MKeyValue){}
        void update(float);

        void startShaking(float amplitude, float frequency,
            shakeType_e method, XMVECTOR direction, refType_e refType);

        void startMoving(XMVECTOR target, refType_e refType,
            moveType_e method, float nAccel, float nSpeed);

        void stop();

        bool isComplete();

};

}

#endif

