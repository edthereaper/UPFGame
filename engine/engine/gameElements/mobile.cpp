#include "mcv_platform.h"
#include "mobile.h"

#include "handles/handle.h"
#include "handles/entity.h"
#include "components/transform.h"
using namespace component;

using namespace DirectX;
using namespace utils;

namespace gameElements {

void CMobile::init()
{
    CTransform* t = Handle(this).getBrother<CTransform>();
    origin = t->getPosition();
}

bool CMobile::isComplete()
{
    switch (currentAction) {
        case MOVING: {
            CTransform* t = Handle(this).getBrother<CTransform>();
            return XMVectorSetW(t->getPosition(), 0) == XMVectorSetW(v, 0);
        } break;
        default: return true; break;
    }
}

void CMobile::stop()
{
    switch (currentAction) {
        case SHAKING: {
            CTransform* t = Handle(this).getBrother<CTransform>();
            t->setPosition(origin);
        } break;
        default: break;
    }
    currentAction = NOTHING;
    slaveRenew = true;
    v = utils::zero_v;
}

void CMobile::startShaking(float nAmplitude, float nFrequency,
    shakeType_e method, XMVECTOR direction, refType_e refType)
{
    stop();
    currentAction = SHAKING;
    shakeType = method;
    amplitude = nAmplitude;
    frequency = nFrequency;
    CTransform* t = Handle(this).getBrother<CTransform>();
    origin = t->getPosition();
    switch (refType) {
        case REF_ABSOLUTE: v = direction; break;
        case REF_RELATIVE: v = XMVector3Rotate(direction, t->getRotation());
    }
    v = XMVector3Normalize(v);
    slaveRenew = true;
}

void CMobile::startMoving(XMVECTOR target, refType_e refType,
    moveType_e method, float nAccel, float nSpeed)
{
    stop();
    currentAction = MOVING;
    moveType = method;
    accel = nAccel;
    speed = nSpeed;
    CTransform* t = Handle(this).getBrother<CTransform>();
    origin = t->getPosition();
    switch (refType) {
        case REF_ABSOLUTE: v = target; break;
        case REF_RELATIVE: {
            v = XMVector3Transform(target, t->getWorld());
            }
    }
    timer.reset();
}

void CMobile::update(float elapsed)
{
    CTransform* t = Handle(this).getBrother<CTransform>();
    switch (currentAction) {
        case MOVING: {
            switch (moveType) {
                case MOVE_ACCELERATE:
                    speed += accel*elapsed;
                    auto delta = moveToTarget(t, v, elapsed, speed);
                    if (slave.isValid()) {
                        CTransform* st = slave.getSon<CTransform>();
                        st->refPosition() += delta;
                    }
                    break;
            }
        } break;
        case SHAKING: {
            float prevTimer = timer;
            float f = timer.count(elapsed)*frequency;
            switch (shakeType) {
                case SHAKE_LINEAR:
                    f = f-std::floor(f)*2.f-1.f;
                    break;
                case SHAKE_COS:
                    f = std::cos(f);
                    break;
                case SHAKE_SIN:
                    f = std::sin(f);
                    break;
                default: break;
            }
            t->setPosition(origin+v*f*amplitude);
            
            if (slave.isValid()) {
                CTransform* st = slave.getSon<CTransform>();
                float prevF = prevTimer*frequency;
                switch (shakeType) {
                    case SHAKE_LINEAR:
                        prevF = prevF-std::floor(prevF)*2.f-1.f;
                        break;
                    case SHAKE_COS:
                        prevF = std::cos(prevF);
                        break;
                    case SHAKE_SIN:
                        prevF = std::sin(prevF);
                        break;
                    default: break;
                }
                XMVECTOR prevOff = slaveRenew ? zero_v : v*prevF*amplitude;
                st->refPosition() -= prevOff;
                st->refPosition() += v*f*amplitude;
                slaveRenew = false;
            }
            } break;
        default: break;
        case NOTHING:
            origin = t->getPosition();
            break;
    }
}

}