#include "mcv_platform.h"
#include "cbonelookat.h"

#include "cskeleton.h"

#include "handles/handle.h"
using namespace component;

using namespace DirectX;

namespace animation {

CBoneLookAt::entry_t::correction_t CBoneLookAt::entry_t::recovery()
{
    if (ipolAccum <= 0) {
        recovering = false;
        return correction_t(true, true);
    } else { 
        XMVECTOR axis;
        float angle;
        XMQuaternionToAxisAngle(&axis, &angle, prevCorrection);
        float f = ipolAccum/prevIpol;
        f= std::sin(f*f*M_PI_2f);
        angle = f*prevAngle;
        
        recovering = true;
        return correction_t(XMQuaternionRotationAxis(axis, angle), true, true);
    }
}

 void CBoneLookAt::entry_t::setActive(bool active)
 {
    if (state == ACTIVE && !active) {
        state = FADEOUT;
    } else if (active) {
        state = ACTIVE;
    }
}

CBoneLookAt::entry_t::correction_t CBoneLookAt::entry_t::calculateCorrection(
    float factor, XMVECTOR desiredDir, float maxAngle, float elapsed) 
{
    ipolAccum.count(elapsed);

    XMVECTOR xmCorrection = XMQuaternionIdentity();
    if (elapsed < 0 || recovering) {
        if(ipolAccum.get()<= 0) {ipolAccum.reset();}
        return recovery();
    } else {
        if(ipolAccum >= params.ipolTime) {
            ipolAccum.set(params.ipolTime);
        }
        float realFactor = params.ipolTime==0 ? 1 : ipolAccum/params.ipolTime;
        realFactor = std::sin(realFactor*realFactor*M_PI_2f);
        realFactor*=params.strength*factor;
        float resultAngle;
        bool rotate = rotationBetweenVectorsEx(referenceDir, desiredDir, xmCorrection,
            maxAngle, params.threshold, params.fadeIn, params.fadeOut, realFactor, resultAngle);
        if (!rotate) {
            if(ipolAccum <= 0) {ipolAccum.reset();}
            //undo the counter, and uncount instead
            ipolAccum.count(-elapsed*2);
            auto ret = recovery();
            return ret;
        } else {
            prevCorrection = xmCorrection;
            prevAngle = resultAngle;
            prevIpol = ipolAccum;
            return correction_t(xmCorrection, false, true);
        }
    }
}

void CBoneLookAt::entry_t::applyCorrection(
    CalBone* bone, CalBone* bone2, bool dual, correction_t data)
{
    if (!data.rotate) {return;}
    CalQuaternion correction = toCalQuaternion(data.rotQ);
    
    // The current bone rotation relative to my parent bone
    CalQuaternion relRot = bone->getRotation();
    
    // Apply the correction and set it to the bone
    CalQuaternion correction1 = correction;
    correction1 *= relRot;
    bone->setRotation(correction1);
    bone->calculateState();

    //Same process for the second bone
    if(dual) {
        CalQuaternion correction2 = correction;
        CalQuaternion relRot2 = bone2->getRotation();
        correction2 *= relRot2;
        bone2->setRotation(correction2);
        bone2->calculateState();
    }
}

XMVECTOR CBoneLookAt::entry_t::getDir(XMVECTOR target, CalBone* bone1, CalBone* bone2, bool dual) const
{
    CalVector bonePos = dual ?
        //if dual, we average the bones
        0.5*(bone1->getTranslationAbsolute() + bone2->getTranslationAbsolute()) :
        bone1->getTranslationAbsolute();
    CalQuaternion absRot = bone1->getRotationAbsolute();
    CalVector desiredDir = toCalVector(target) - bonePos;
    if (dual) {
        //if dual, we average the bones
        XMVECTOR rot1 = toXMQuaternion(absRot);
        XMVECTOR rot2 = toXMQuaternion(bone2->getRotationAbsolute());
        absRot = toCalQuaternion(XMQuaternionSlerp(rot1, rot2, .5f));
    }
    // translate to local coordinates
    absRot.invert();
    desiredDir *= absRot;
    return toXMVECTOR(desiredDir);
}

void CBoneLookAt::entry_t:: setTarget(const XMVECTOR& nTarget, bool ipol)
{
    iPolTarget = currentTarget(0);
    iPolTargetTime = ipol? 0.f : 1.f;
    target = nTarget; 
}

XMVECTOR CBoneLookAt::entry_t::currentTarget(float elapsed)
{
    bool ipolingTarget = iPolTargetTime < 1;
    iPolTargetTime = iPolTargetTime + elapsed;
    float iPolTargetAmount = inRange(0.f, iPolTargetTime/params.ipolTime, 1.f);
    if (ipolingTarget && iPolTargetAmount >= 1) {
        iPolTarget = target;
    }
    auto ans = iPolTargetAmount == 1 ? target :
        XMVectorLerp(iPolTarget, target, iPolTargetAmount);
    return ans;
}

void CBoneLookAt::entry_t::apply(const CalModel* model, float factor, float elapsed)
{
    if (params.ipolTime < FLT_EPSILON) {params.ipolTime = FLT_EPSILON;}
    bool dual = calSecondBone >= 0;
    auto skeleton = model->getSkeleton();
    CalBone* bone = skeleton->getBone(calBone);
    CalBone* bone2 = dual ?  skeleton->getBone(calSecondBone) : nullptr;
#ifdef LOOKAT_TOOL
    if(bone == nullptr) return;
#else
    assert(bone != nullptr);
#endif

    XMVECTOR desiredDir = getDir(currentTarget(elapsed), bone, bone2, dual);

    if (params.maxAngleY != params.maxAngleXZ) {
        auto r1 = calculateCorrection(factor, desiredDir,
            std::max(params.maxAngleXZ, params.maxAngleY), elapsed);
        auto euler = quaternionToEuler(r1.rotQ);
        euler = quaternionToEuler(r1.rotQ);
        float pitch(euler.pitch);
        euler.yaw = minAbs(euler.yaw, params.maxAngleXZ);
        euler.roll = minAbs(euler.roll, params.maxAngleY);
        r1.rotQ = eulerToQuaternion(euler);
        applyCorrection(bone, bone2, dual, r1);
    } else {
        auto r = calculateCorrection(factor, desiredDir, params.maxAngleXZ, elapsed);
        applyCorrection(bone, bone2, dual, r);
    }
}

void CBoneLookAt::update(CalModel*  model, float elapsed)
{
    if (!active) {return;}
    for (auto& e : entries) {
        if (e.used && e.calBone != -1) {
            switch (e.state) {
                case entry_t::ACTIVE:
                    e.apply(model, globalStrength,
                        e.recovering? -elapsed : elapsed);
                    break;
                case entry_t::FADEOUT:
                    e.apply(model, globalStrength, -elapsed);
                    if (e.ipolAccum <= 0) {
                        e.recovering = false;
                        e.ipolAccum.reset();
                        e.state = entry_t::INACTIVE;
                    }
                    break;
                default:break;
            }
        }
    }
}

void CBoneLookAt::update(float elapsed)
{
    if (!active) {return;}
    Handle h(this);
    assert(h.isValid());
    CSkeleton* skeleton = h.getBrother<CSkeleton>();
    CTransform* t = h.getBrother<CTransform>();
    update(skeleton->getModel(), elapsed);
}

CBoneLookAt::~CBoneLookAt() {
    for(auto& e : entries) {
        e.used = false;
    }
}

void CBoneLookAt::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (elem == "BoneLookAt") {
        globalStrength = atts.getFloat("strength", 1.f);
        if (atts.getBool("clear", false)) {
            for (auto& entry : entries) {
                entry.used = false;
            }
        }
        active = atts.getBool("active", active);
    } else if (elem == "entry") {
        XMVECTOR referenceDir = atts.getPoint("referenceDir", yAxis_v);
        referenceDir = XMVector3Normalize(referenceDir);

        CSkeleton* skeleton = Handle(this).getBrother<CSkeleton>();
        auto coreSkeleton = skeleton->getModel()->getSkeleton()-> getCoreSkeleton();
        int bone1 = atts.has("boneName") ?
            coreSkeleton->getCoreBoneId(atts.getString("boneName", "<none>")) :
            atts.getInt("boneId", -1);
        int bone2 = atts.has("bone2Name") ?
            coreSkeleton->getCoreBoneId(atts.getString("bone2Name", "<none>")) :
            atts.getInt("bone2Id", -1);

        entry_t& entry = getEntry(newEntry(bone1, referenceDir, bone2));
        bool active(atts.getBool("active", entry.isActive()));
        entry.params.strength  = atts.getFloat("strength", entry.params.strength);
        if (atts.has("maxAngleXZ")) {entry.params.maxAngleXZ = deg2rad(atts.getFloat("maxAngleXZ"));}
        if (atts.has("maxAngleY"))  {entry.params.maxAngleY  = deg2rad(atts.getFloat("maxAngleY"));}
        if (atts.has("maxAngle")) {entry.setMaxAngle(deg2rad(atts.getFloat("maxAngle")));}
        if (atts.has("threshold")) {entry.params.threshold = deg2rad(atts.getFloat("threshold"));}
        if (atts.has("fadeOut")) {entry.params.fadeOut = deg2rad(atts.getFloat("fadeOut"));}
        if (atts.has("fadeIn")) {entry.params.fadeIn = deg2rad(atts.getFloat("fadeIn"));}
        entry.params.ipolTime  = atts.getFloat("ipolTime", entry.params.ipolTime);
    }
}

CBoneLookAt::entryId_t CBoneLookAt::newEntry(
    int calBone, XMVECTOR reference, int calSecondBone)
{
    for (entryId_t i=0; i<N_LOOKATS; ++i) {
        entry_t& e(entries[i]);
        if (!e.used) {
            e.used = true;
            e.calBone = calBone;
            e.calSecondBone = calSecondBone;
            e.setRef(XMVector3Rotate(reference, utils::skeletonRef_q));
            e.ipolAccum.reset();
            return i;
        }
    }
    return ~0;
}

CBoneLookAt::entryId_t CBoneLookAt::newEntry(
    const std::string& calBone, XMVECTOR reference, const std::string& calSecondBone)
{
    CSkeleton* skeleton = Handle(this).getBrother<CSkeleton>();
    auto coreSkeleton = skeleton->getModel()->getSkeleton()-> getCoreSkeleton();
    return newEntry(
        coreSkeleton->getCoreBoneId(calBone), reference,
        coreSkeleton->getCoreBoneId(calSecondBone));
}

}