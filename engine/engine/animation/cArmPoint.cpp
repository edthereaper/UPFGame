#include "mcv_platform.h"
#include "cArmPoint.h"

#include "skeleton_manager.h"
#include "cskeleton.h"
#include "ikHandler.h"

#include "handles/handle.h"
using namespace component;

using namespace DirectX;

namespace animation {

void CArmPoint::setUpperArm(int calBone) {
    calBoneId[0] = calBone;
    lookAt.getEntry(armLookAt).calBone = calBone;
}

void CArmPoint::setUpperArmTwist(int calBone) {
    calBoneTwist[0] = calBone;
}

void CArmPoint::setForeArm(int calBone) {
    calBoneId[1] = calBone;
    lookAt.getEntry(forearmLookAt).calBone = calBone;
}

void CArmPoint::setForeArmTwist(int calBone) {
    calBoneTwist[1] = calBone;
}

void CArmPoint::setHand(int calBone) {
    calBoneId[2] = calBone;
}

void CArmPoint::setForeArmRef(XMVECTOR v) {
    lookAt.getEntry(forearmLookAt).setRef(v);
}

XMVECTOR CArmPoint::getForeArmRef() const {
    return lookAt.getEntry(forearmLookAt).getRef();
}

void CArmPoint::setUpperArmRef(XMVECTOR v) {
    lookAt.getEntry(armLookAt).setRef(v);
}

XMVECTOR CArmPoint::getUpperArmRef() const {
    return lookAt.getEntry(armLookAt).getRef();
}

void CArmPoint::setBones(
    int armId, int forearmId, int handId,
    XMVECTOR armReference, XMVECTOR forearmReference,
    int armTwistId, int foreTwistId)
{
    calBoneId[0] = armId;
    calBoneId[1] = forearmId;
    calBoneId[2] = handId;
    calBoneTwist[0] = armTwistId;
    calBoneTwist[1] = foreTwistId;
    lookAt.freeEntry(armLookAt);
    lookAt.freeEntry(forearmLookAt);
    armLookAt = lookAt.newEntry(calBoneId[0], armReference);
    forearmLookAt = lookAt.newEntry(calBoneId[1], forearmReference);
}

void CArmPoint::setActive(bool b)
{
    active=b;
    lookAt.setActive(b);
}
void CArmPoint::setAllActive(bool b)
{
    lookAt.setAllActive(b);
}

void CArmPoint::loadFromProperties(std::string elem, utils::MKeyValue atts)
{
    if (elem == "ArmPoint") {
        setActive(atts.getBool("active", active));
        setGlobalStrength(atts.getFloat("strength", getGlobalStrength()));
    } else if (elem == "upper") {
        auto coremodel = ((CSkeleton*) Handle(this).getBrother<CSkeleton>())->getModel()->getCoreModel();
        lookAt.freeEntry(armLookAt);
        armLookAt = lookAt.newEntry(-1);
        setUpperArm(coremodel->getBoneId(atts.getString("bone", "<none>")));
        setUpperArmTwist(coremodel->getBoneId(atts.getString("twist", "<none>")));
        setUpperArmRef(atts.getPoint("referenceDir", getUpperArmRef()));
        parameters_t params;
        params.maxAngleXZ = deg2rad(atts.getFloat("angleXZ", rad2deg(params.maxAngleXZ)));
        params.maxAngleY = deg2rad(atts.getFloat("angleY", rad2deg(params.maxAngleY)));
        params.threshold = deg2rad(atts.getFloat("threshold", rad2deg(params.threshold)));
        params.fadeIn = deg2rad(atts.getFloat("fadeIn", rad2deg(params.fadeIn)));
        params.fadeOut = deg2rad(atts.getFloat("fadeOut", rad2deg(params.fadeOut)));
        params.strength = atts.getFloat("strength", params.strength);
        params.ipolTime = atts.getFloat("ipolTime", params.ipolTime);
        setParams(params, UPPER_ARM);
    } else if (elem == "forearm") {
        lookAt.freeEntry(forearmLookAt);
        forearmLookAt = lookAt.newEntry(-1);
        auto coremodel = ((CSkeleton*) Handle(this).getBrother<CSkeleton>())->getModel()->getCoreModel();
        setForeArm(coremodel->getBoneId(atts.getString("bone", "<none>")));
        setForeArmTwist(coremodel->getBoneId(atts.getString("twist", "<none>")));
        setForeArmRef(atts.getPoint("referenceDir", getForeArmRef()));
        parameters_t params;
        params.maxAngleXZ = deg2rad(atts.getFloat("angleXZ", rad2deg(params.maxAngleXZ)));
        params.maxAngleY = deg2rad(atts.getFloat("angleY", rad2deg(params.maxAngleY)));
        params.threshold = deg2rad(atts.getFloat("threshold", rad2deg(params.threshold)));
        params.fadeIn = deg2rad(atts.getFloat("fadeIn", rad2deg(params.fadeIn)));
        params.fadeOut = deg2rad(atts.getFloat("fadeOut", rad2deg(params.fadeOut)));
        params.ipolTime = atts.getFloat("ipolTime", params.ipolTime);
        params.strength = atts.getFloat("strength", params.strength);
        setParams(params, FOREARM);
    } else if (elem == "hand") {
        auto coremodel = ((CSkeleton*) Handle(this).getBrother<CSkeleton>())->getModel()->getCoreModel();
        setHand(coremodel->getBoneId(atts.getString("bone", "<none>")));
    }
}

void CArmPoint::setBones(
    std::string armName, std::string forearmName, std::string handName,
    XMVECTOR armReference, XMVECTOR forearmReference,
    std::string armTwistName, std::string foreTwistName)
{
    auto coremodel = ((CSkeleton*) Handle(this).getBrother<CSkeleton>())->getModel()->getCoreModel();
    setBones(
        coremodel->getBoneId(armName),
        coremodel->getBoneId(forearmName),
        coremodel->getBoneId(handName),
        armReference, forearmReference,
        coremodel->getBoneId(armTwistName),
        coremodel->getBoneId(foreTwistName)
        );
}

XMVECTOR CArmPoint::getPosHand() const
{
	CalModel* model = ((CSkeleton*)Handle(this).getBrother<CSkeleton>())->getModel();
	auto calSkeleton = model->getSkeleton();
	CalBone* bone = model->getSkeleton()->getBone(calBoneId[2]);
	return toXMVECTOR(bone->getTranslationAbsolute());
}

/*
    Algorithm: to point at target t with bones A,F,H (arm ,forearm, hand)
    arm is flexed with a factor of f

    o := A.pos              //shoulder
    v = ||t-o||
    w := o + v(|A|+|F|)*f   //wrist
    e := IK(A,B,C,w)        //elbow
    A.pos := o
    F.pos := e
    H.pos := w
*/
void CArmPoint::update(float elapsed)
{
    if (!active) {return;}
#ifdef LOOKAT_TOOL
    if(calBoneId[0]<0 || calBoneId[1]<0 || calBoneId[2]<0) return;
#else
    assert(!(calBoneId[0]<0 || calBoneId[1]<0 || calBoneId[2]<0));
#endif
    CalModel* model = ((CSkeleton*) Handle(this).getBrother<CSkeleton>())->getModel();
    auto calSkeleton = model->getSkeleton();
    auto upperArm = calSkeleton->getBone(calBoneId[0]);
    auto forearm = calSkeleton->getBone(calBoneId[1]);
    auto hand = calSkeleton->getBone(calBoneId[2]);

    CalVector shoulder = upperArm->getTranslationAbsolute();
    XMVECTOR xmShoulder = toXMVECTOR(shoulder);
    CalVector currElbow = forearm->getTranslationAbsolute();
    CalVector currWrist = hand->getTranslationAbsolute();

    CalVector currUpperArmVector(currElbow-shoulder);
    CalVector currForearmVector(currWrist-currElbow);
    CalVector currS2WVector(currWrist-shoulder);
    XMVECTOR normal(XMVector3Normalize(
            XMVector3Cross(toXMVECTOR(currForearmVector), toXMVECTOR(currUpperArmVector))
        ));
    float upperArmLength = currUpperArmVector.length();
    float forearmLength = currForearmVector.length();
    float armLength = upperArmLength + forearmLength;
    float flex = currS2WVector.length()/armLength; // current animation

    XMVECTOR dir = XMVector3Normalize((target-xmShoulder));
    XMVECTOR wrist = xmShoulder + ((upperArmLength+forearmLength)*flex)*dir;

    ikHandler_t ik;
    ik.AB = upperArmLength;
    ik.BC = forearmLength;

    if (ik(xmShoulder, wrist, normal)) {
        lookAt.getEntry(armLookAt).setTarget(ik.B);
        lookAt.getEntry(forearmLookAt).setTarget(target);
        lookAt.update(model, elapsed);

        //Twists
        if(calBoneTwist[1] > 0) {
            auto twist = calSkeleton->getBone(calBoneTwist[1]);
            twist->setRotation(forearm->getRotation());
            twist->calculateState();
        }
        if(calBoneTwist[0] > 0) {
            auto twist = calSkeleton->getBone(calBoneTwist[0]);
            twist->setRotation(upperArm->getRotation());
            twist->calculateState();
        }
    }
}

}