#include "mcv_platform.h"
#include "lookatTools.h"

#ifdef LOOKAT_TOOL

#include <sstream>
#include <fstream>

#include "handles/prefab.h"
using namespace component;

using namespace animation;
using namespace DirectX;

namespace antTw_user {
    
extern ArmPointTW* armPointTw[4] = {nullptr, nullptr, nullptr, nullptr};
    
void TW_CALL TWArm_setAllActive(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const bool* b = (const bool*) value;
    tw->setActive(*b);
}
void TW_CALL TWArm_getAllActive(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    bool* ret = (bool*) value;
    *ret = tw->isActive();
}    

void TW_CALL TWArm_setMuted(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const bool* b = (const bool*) value;
    tw->setMuted(*b);
}
void TW_CALL TWArm_getMuted(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    bool* ret = (bool*) value;
    *ret = tw->isMuted();
}

void TW_CALL TWArm_save(void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    tw->save();
}
void TW_CALL TWArm_reload(void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    tw->reload();
}
void TW_CALL TWArm_resetTarget(void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    tw->resetTarget();
}

void TW_CALL TWArm_updateTarget(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const float* ret = (const float*) value;
    tw->setTarget(XMVectorSet(ret[0], ret[1], ret[2], 0));
}
void TW_CALL TWArm_readTarget(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    float* ret = (float*) value;
    XMVECTOR v = tw->getTarget();
    ret[0] = XMVectorGetX(v);
    ret[1] = XMVectorGetY(v);
    ret[2] = XMVectorGetZ(v);
}

void TW_CALL TWArm_updateUpperBone(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const int* ret = (const int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    ap->setUpperArm(*ret);
}
void TW_CALL TWArm_updateUpperTwist(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const int* ret = (const int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    ap->setUpperArmTwist(*ret);
}
void TW_CALL TWArm_updateForeBone(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const int* ret = (const int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    ap->setForeArm(*ret);
}
void TW_CALL TWArm_updateForeTwist(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const int* ret = (const int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    ap->setForeArmTwist(*ret);
}
void TW_CALL TWArm_updateHandBone(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    const int* ret = (const int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    ap->setHand(*ret);
}

void TW_CALL TWArm_readUpperBone(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    int* ret = (int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    *ret = ap->getUpperArm();
}
void TW_CALL TWArm_readUpperTwist(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    int* ret = (int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    *ret = ap->getUpperArmTwist();
}
void TW_CALL TWArm_readForeBone(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    int* ret = (int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    *ret = ap->getForeArm();
}
void TW_CALL TWArm_readForeTwist(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    int* ret = (int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    *ret = ap->getForeArmTwist();
}
void TW_CALL TWArm_readHandBone(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    int* ret = (int*) value;
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    *ret = ap->getHand();
}

void TW_CALL TWArm_updateForeRef(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    const float* ret = (const float*) value;
    XMVECTOR ref = XMVectorSet(ret[0], ret[1], ret[2], 0);
    XMVECTOR skelQ = one_q;
    auto coreSkeleton(skeleton->getModel()->getCoreModel()->getCoreSkeleton());
    skelQ = toXMQuaternion(coreSkeleton->getCoreBone(armPoint->getForeArm())->getRotationAbsolute());
    
    if (tw->useRelFore()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, (camT->getRotation()));
    }
    ref = XMVector3Rotate(ref, skelQ);
    armPoint->setForeArmRef(ref);
}
void TW_CALL TWArm_readForeRef(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    float* ret = (float*) value;
    XMVECTOR ref = armPoint->getForeArmRef();
    XMVECTOR skelQ = one_q;
    auto coreSkeleton(skeleton->getModel()->getCoreModel()->getCoreSkeleton());
    skelQ = toXMQuaternion(coreSkeleton->getCoreBone(armPoint->getForeArm())->getRotationAbsolute());
    ref = XMVector3Rotate(ref, XMQuaternionInverse(skelQ));
    
    if (tw->useRelFore()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, XMQuaternionInverse(camT->getRotation()));
    }

    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}
void TW_CALL TWArm_readForeDir(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    float* ret = (float*) value;
    XMVECTOR ref = armPoint->getForeArmRef();
    XMVECTOR skelQ = one_q;
    auto modelSkeleton(skeleton->getModel()->getSkeleton());
    skelQ = toXMQuaternion(modelSkeleton->getBone(armPoint->getForeArm())->getRotationAbsolute());
    ref = XMVector3Rotate(ref, XMQuaternionInverse(skelQ));
    
    if (tw->useRelFore()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, (camT->getRotation()));
    }

    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}

void TW_CALL TWArm_updateUpperRef(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    const float* ret = (const float*) value;
    XMVECTOR ref = XMVectorSet(ret[0], ret[1], ret[2], 0);
    CTransform* transform(e->get<CTransform>());
    ref = XMVector3Rotate(ref, (transform->getRotation()));
    XMVECTOR skelQ = one_q;
    auto coreSkeleton(skeleton->getModel()->getCoreModel()->getCoreSkeleton());
    skelQ = toXMQuaternion(coreSkeleton->getCoreBone(armPoint->getUpperArm())->getRotationAbsolute());
    
    if (tw->useRelUpper()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, XMQuaternionInverse(camT->getRotation()));
    }
    ref = XMVector3Rotate(ref, (skelQ));
    armPoint->setUpperArmRef(ref);
}
void TW_CALL TWArm_readUpperRef(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    CTransform* transform(e->get<CTransform>());
    float* ret = (float*) value;
    XMVECTOR ref = armPoint->getUpperArmRef();
    XMVECTOR skelQ = one_q;
    auto coreSkeleton(skeleton->getModel()->getCoreModel()->getCoreSkeleton());
    skelQ = toXMQuaternion(coreSkeleton->getCoreBone(armPoint->getUpperArm())->getRotationAbsolute());
    ref = XMVector3Rotate(ref, XMQuaternionInverse(skelQ));
    
    if (tw->useRelUpper()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, (camT->getRotation()));
    }
    ref = XMVector3Rotate(ref, XMQuaternionInverse(transform->getRotation()));

    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}
void TW_CALL TWArm_readUpperDir(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    float* ret = (float*) value;
    XMVECTOR ref = armPoint->getUpperArmRef();
    XMVECTOR skelQ = one_q;
    auto modelSkeleton(skeleton->getModel()->getSkeleton());
    skelQ = toXMQuaternion(modelSkeleton->getBone(armPoint->getUpperArm())->getRotationAbsolute());
    ref = XMVector3Rotate(ref, XMQuaternionInverse(skelQ));
    
    if (tw->useRelUpper()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, (camT->getRotation()));
    }

    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}

void TW_CALL TWArm_readHandDir(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    Entity* e = tw->getEntity();
    CArmPoint* armPoint(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    float* ret = (float*) value;
    XMVECTOR ref = armPoint->getUpperArmRef();
    XMVECTOR skelQ = one_q;
    auto modelSkeleton(skeleton->getModel()->getSkeleton());
    skelQ = toXMQuaternion(modelSkeleton->getBone(armPoint->getHand())->getRotationAbsolute());
    ref = XMVector3Rotate(ref, XMQuaternionInverse(skelQ));
    
    if (tw->useRelUpper()) {
        CTransform* camT = tw->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, (camT->getRotation()));
    }

    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}

void TW_CALL TWArm_updateFadeIn_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.fadeIn = deg2rad(*ret);
    ap->setParams(params, CArmPoint::UPPER_ARM);
}
void TW_CALL TWArm_updateFadeOut_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.fadeOut = deg2rad(*ret);
    ap->setParams(params, CArmPoint::UPPER_ARM);
}
void TW_CALL TWArm_updateMaxXZ_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.maxAngleXZ = deg2rad(*ret);
    ap->setParams(params, CArmPoint::UPPER_ARM);
}
void TW_CALL TWArm_updateMaxY_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.maxAngleY = deg2rad(*ret);
    ap->setParams(params, CArmPoint::UPPER_ARM);
}
void TW_CALL TWArm_updateThreshold_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.threshold = deg2rad(*ret);
    ap->setParams(params, CArmPoint::UPPER_ARM);
}
void TW_CALL TWArm_updateIpolTime_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.ipolTime = *ret;
    ap->setParams(params, CArmPoint::UPPER_ARM);
}
void TW_CALL TWArm_updateStrength_UA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::UPPER_ARM);
    params.strength = *ret;
    ap->setParams(params, CArmPoint::UPPER_ARM);
}

void TW_CALL TWArm_readFadeIn_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::UPPER_ARM).fadeIn);
}
void TW_CALL TWArm_readFadeOut_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::UPPER_ARM).fadeOut);
}
void TW_CALL TWArm_readMaxXZ_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::UPPER_ARM).maxAngleXZ);
}
void TW_CALL TWArm_readMaxY_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::UPPER_ARM).maxAngleY);
}
void TW_CALL TWArm_readThreshold_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::UPPER_ARM).threshold);
}
void TW_CALL TWArm_readIpolTime_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = ap->getParams(CArmPoint::UPPER_ARM).ipolTime;
}
void TW_CALL TWArm_readStrength_UA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = ap->getParams(CArmPoint::UPPER_ARM).strength;
}

void TW_CALL TWArm_updateFadeIn_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.fadeIn = deg2rad(*ret);
    ap->setParams(params, CArmPoint::FOREARM);
}
void TW_CALL TWArm_updateFadeOut_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.fadeOut = deg2rad(*ret);
    ap->setParams(params, CArmPoint::FOREARM);
}
void TW_CALL TWArm_updateMaxXZ_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.maxAngleXZ = deg2rad(*ret);
    ap->setParams(params, CArmPoint::FOREARM);
}
void TW_CALL TWArm_updateMaxY_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.maxAngleY = deg2rad(*ret);
    ap->setParams(params, CArmPoint::FOREARM);
}
void TW_CALL TWArm_updateThreshold_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.threshold = deg2rad(*ret);
    ap->setParams(params, CArmPoint::FOREARM);
}
void TW_CALL TWArm_updateIpolTime_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.ipolTime = *ret;
    ap->setParams(params, CArmPoint::FOREARM);
}
void TW_CALL TWArm_updateStrength_FA(const void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    const float* ret = (const float*) value;
    auto params = ap->getParams(CArmPoint::FOREARM);
    params.strength = *ret;
    ap->setParams(params, CArmPoint::FOREARM);
}

void TW_CALL TWArm_readFadeIn_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::FOREARM).fadeIn);
}
void TW_CALL TWArm_readFadeOut_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::FOREARM).fadeOut);
}
void TW_CALL TWArm_readMaxXZ_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::FOREARM).maxAngleXZ);
}
void TW_CALL TWArm_readMaxY_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::FOREARM).maxAngleY);
}
void TW_CALL TWArm_readThreshold_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = rad2deg(ap->getParams(CArmPoint::FOREARM).threshold);
}
void TW_CALL TWArm_readIpolTime_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = ap->getParams(CArmPoint::FOREARM).ipolTime;
}
void TW_CALL TWArm_readStrength_FA(void *value, void *clientData)
{ 
    ArmPointTW* tw(static_cast<ArmPointTW*>(clientData));
    CArmPoint* ap(tw->getEntity().getSon<CArmPoint>());
    float* ret = (float*) value;
    *ret = ap->getParams(CArmPoint::FOREARM).strength;
}

void ArmPointTW::setTarget(XMVECTOR v)
{
    target = v;
    CArmPoint* armPoint(entity_h.getSon<CArmPoint>());
    armPoint->setTarget(v);
}

void ArmPointTW::createBoneEnum()
{
    Entity* e(entity_h);
    std::stringstream ss;
    ss << "eBONE_AP_" << e->getName();
    CSkeleton* skeleton(e->get<CSkeleton>());
    auto model = skeleton->getModel()->getCoreModel();
    auto boneVector = model->getCoreSkeleton()->getVectorCoreBone();
    int nBones = static_cast<int>(boneVector.size());
    TwEnumVal* boneEnumLabels;
    boneEnumLabels = new TwEnumVal[nBones+1];
    for (int i=0; i<nBones; i++) {
        boneEnumLabels[i].Label = boneVector[i]->getName().c_str();
        boneEnumLabels[i].Value = i;
    }
    boneEnum = TwDefineEnum(ss.str().c_str(), boneEnumLabels, nBones);
    delete boneEnumLabels;
}

void ArmPointTW::resetTarget()
{
    CTransform* t = entity_h.getSon<CTransform>();
    setTarget(XMVector3Transform(DirectX::XMVectorSet(0,2,2,0),t->getWorld()));
}

void ArmPointTW::reload()
{
    std::string str = filename.substr(0,filename.find_last_of('.'));
    bool found = false;
    for (auto pos = size_t(0);
        pos != std::string::npos && found == false;
        pos = str.find("/"), str = str.substr(pos+1)) {
        found = PrefabManager::get().prefabricateComponents(str.c_str(), entity_h, true);
    }
}

void ArmPointTW::save()
{
    std::ofstream os(filename);
    Entity* e(entity_h);
    CArmPoint* ap(e->get<CArmPoint>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    auto calSkeleton = skeleton->getModel()->getCoreModel()->getCoreSkeleton();
    MKeyValue mapAP, mapUA, mapFA, mapH;

    auto uaParams = ap->getParams(CArmPoint::UPPER_ARM);
    auto faParams = ap->getParams(CArmPoint::FOREARM);

    mapAP.setFloat("strength", ap->getGlobalStrength());

    mapUA.setString("bone", calSkeleton->getCoreBone(ap->getUpperArm())->getName());
    mapUA.setString("twist", calSkeleton->getCoreBone(ap->getUpperArmTwist())->getName());
    mapUA.setPoint("referenceDir", XMVector3Normalize(ap->getUpperArmRef()), 2);
    mapUA.setFloat("angleXZ", rad2deg(uaParams.maxAngleXZ));
    mapUA.setFloat("angleY", rad2deg(uaParams.maxAngleY));
    mapUA.setFloat("fadeIn", rad2deg(uaParams.fadeIn));
    mapUA.setFloat("fadeOut", rad2deg(uaParams.fadeOut));
    mapUA.setFloat("threshold", rad2deg(uaParams.threshold));
    mapUA.setFloat("ipolTime", uaParams.ipolTime);
    mapUA.setFloat("strength", uaParams.strength);
    
    mapFA.setString("bone", calSkeleton->getCoreBone(ap->getForeArm())->getName());
    mapFA.setString("twist", calSkeleton->getCoreBone(ap->getForeArmTwist())->getName());
    mapFA.setPoint("referenceDir", XMVector3Normalize(ap->getForeArmRef()), 2);
    mapFA.setFloat("angleXZ", rad2deg(faParams.maxAngleXZ));
    mapFA.setFloat("angleY", rad2deg(faParams.maxAngleY));
    mapFA.setFloat("fadeIn", rad2deg(faParams.fadeIn));
    mapFA.setFloat("fadeOut", rad2deg(faParams.fadeOut));
    mapFA.setFloat("threshold", rad2deg(faParams.threshold));
    mapFA.setFloat("ipolTime", faParams.ipolTime);
    mapFA.setFloat("strength", faParams.strength);

    mapH.setString("bone", calSkeleton->getCoreBone(ap->getHand())->getName());

    mapAP.writeStartElement(os, "ArmPoint", "", " ", "");
    mapUA.writeSingle(os, "upper", "\t", "\n", "\t\t");
    mapFA.writeSingle(os, "forearm", "\t", "\n", "\t\t");
    mapH.writeSingle(os, "hand", "\t", " ", "");
    mapAP.writeEndElement(os, "ArmPoint", "");
}

ArmPointTW::ArmPointTW(Handle entity_h, Handle camera_h, std::string filename, Color c)
    : entity_h(entity_h), camera_h(camera_h), filename(filename), color(c)
{
    Entity* e(entity_h);
    CSkeleton* skeleton(e->get<CSkeleton>());
    CArmPoint* armPoint(e->get<CArmPoint>());

    if (!e->has<CArmPoint>()) {
        valid = false;
        return;
    } else {
        valid = true;
    }
    resetTarget();
    setMuted(false);
    setActive(true);
    createBoneEnum();
    std::stringstream ss;
    ss.str(""); ss << "\"" << int(c.r()) << " " << int(c.g()) << " " << int(c.b()) << "\"";
    std::string colorStr = ss.str();
    ss.str(""); ss << "ArmPoint - " << e->getName();
	bar = TwNewBar(ss.str().c_str());
    std::string barName = TwGetBarName(bar);
    ss.str(""); ss << "\"" <<barName << "\" color=" << colorStr << " ";
    const ColorHSL hsl(c);
    if (hsl.l() > 0.5f) {ss<<" text=dark ";} else {ss<<" text=light ";}
    TwDefine(ss.str().c_str());

    TwAddButton(bar, "save", TWArm_save, this, " label='SAVE' ");
    TwAddButton(bar, "reload", TWArm_reload, this, " label='RELOAD' ");

    TwAddVarCB(bar, "mute", TW_TYPE_BOOLCPP, TWArm_setMuted, TWArm_getMuted, this,
        " label='MUTE' help='mute armPoint' ");
    TwAddVarCB(bar, "activate", TW_TYPE_BOOLCPP, TWArm_setAllActive, TWArm_getAllActive, this,
        " label='ACTIVE' help='activate armPoint' ");
    
    ss.str(""); ss << "label='Move target:' help='Move the armPoint target around.' axisx=-x axisz=-z "
        << "arrowcolor=" << colorStr << " ";
	TwAddVarCB(bar, "target", TW_TYPE_DIR3F, TWArm_updateTarget, TWArm_readTarget, this,
        ss.str().c_str());
    TwAddButton(bar, "resetTarget", TWArm_reload, this, " label='Reset target' ");

    TwAddSeparator(bar,NULL,NULL);
    {   //UPPER ARM
        std::stringstream groupSS;
        TwAddVarCB(bar, "boneUpper", boneEnum,
            TWArm_updateUpperBone, TWArm_readUpperBone, this,
            " label='Bone:' help='ArmPoint upper arm bone' group=\"Upper arm\" ");
        TwAddVarCB(bar, "boneUpperTwist", boneEnum,
            TWArm_updateUpperTwist, TWArm_readUpperTwist, this,
            " label='Twist:' help='ArmPoint upper arm twist bone' group=\"Upper arm\" ");
	    TwAddVarRW(bar, "drawUpper", TW_TYPE_BOOLCPP, &drawUpper,
            " label='Draw:' help='Display the reference direction and bone axis' group=\"Upper arm\" ");
	    TwAddVarCB(bar, "refRelUpper", TW_TYPE_DIR3F, NULL, TWArm_readUpperDir, this,
            " label='Front:' opened=false help='Reference relative to bone.' readonly=true"
            " arrowcolor='127 127 127' axisx=-x axisz=-z showval=false group=\"Upper arm\" ");
	    TwAddVarCB(bar, "refUpper", TW_TYPE_DIR3F, TWArm_updateUpperRef, TWArm_readUpperRef, this,
            " label='Reference :' axisx=-x axisz=-z opened=true "
            " help='Define reference vector for upper armarmPoint.' "
            " group=\"Upper arm\" ");
        groupSS << " \"" << barName << "\"/refUpper " << " arrowcolor=" << colorStr << " \n";
    
	    TwAddVarRW(bar, "useRelUpper", TW_TYPE_BOOLCPP, &relativeUpperRef,
            " label='Show relative:' help='Display the arrow relative to camera.' group=\"Upper arm\" ");

        TwAddVarCB(bar, "angleXZ_UA", TW_TYPE_FLOAT, TWArm_updateMaxXZ_UA, TWArm_readMaxXZ_UA, this,
            " label='Yaw:' help='Armpoint upper arm yaw' group=\"Upper arm\" min=0 max=360 ");
        TwAddVarCB(bar, "angleY_UA", TW_TYPE_FLOAT, TWArm_updateMaxY_UA, TWArm_readMaxY_UA, this,
            " label='Pitch:' help='Armpoint upper arm pitch' group=\"Upper arm\" min=0 max=360 ");
        TwAddVarCB(bar, "angleTH_UA", TW_TYPE_FLOAT, TWArm_updateThreshold_UA, TWArm_readThreshold_UA, this,
            " label='Threshold:' help='Armpoint upper arm threshold' group=\"Upper arm\" min=0 max=360 ");
        TwAddVarCB(bar, "fadeIn_UA", TW_TYPE_FLOAT, TWArm_updateFadeIn_UA, TWArm_readFadeIn_UA, this,
            " label='Fade in:' help='Armpoint upper arm fade in' group=\"Upper arm\" min=0 max=360 ");
        TwAddVarCB(bar, "fadeOut_UA", TW_TYPE_FLOAT, TWArm_updateFadeOut_UA, TWArm_readFadeOut_UA, this,
            " label='Fade out:' help='Armpoint upper arm fade out' group=\"Upper arm\" min=0 max=360 ");
        TwAddVarCB(bar, "Ipol_UA", TW_TYPE_FLOAT, TWArm_updateIpolTime_UA, TWArm_readIpolTime_UA, this,
            " label='Ipol time:' help='Armpoint upper arm interpolation time'"
            " min=0 step=0.05 max=5 group=\"Upper arm\" ");
        TwAddVarCB(bar, "strenght_UA", TW_TYPE_FLOAT, TWArm_updateStrength_UA, TWArm_readStrength_UA, this,
            " label='Strength:' help='Armpoint upper arm strength time'"
            " min=0 step=0.05 max=1 group=\"Upper arm\" ");

        TwDefine(groupSS.str().c_str());
    }
    {   //FOREARM
        std::stringstream groupSS;
        TwAddVarCB(bar, "boneFore", boneEnum,
            TWArm_updateForeBone, TWArm_readForeBone, this,
            " label='Bone:' help='ArmPoint forearm bone' group=\"Forearm\" ");
        TwAddVarCB(bar, "boneForeTwist", boneEnum,
            TWArm_updateForeTwist, TWArm_readForeTwist, this,
            " label='Twist:' help='ArmPoint forearm twist bone' group=\"Forearm\" ");
	    TwAddVarRW(bar, "drawFore", TW_TYPE_BOOLCPP, &drawFore,
            " label='Draw:' help='Display the reference direction and bone axis' group=\"Forearm\" ");
	    TwAddVarCB(bar, "refRelFore", TW_TYPE_DIR3F, NULL, TWArm_readForeDir, this,
            " label='Front:' opened=false help='Reference relative to bone.' readonly=true"
            " arrowcolor='127 127 127' axisx=-x axisz=-z showval=false group=\"Forearm\" ");
	    TwAddVarCB(bar, "refFore", TW_TYPE_DIR3F, TWArm_updateForeRef, TWArm_readForeRef, this,
            " label='Reference :' axisx=-x axisz=-z opened=true "
            " help='Define reference vector for forearmarmPoint.' "
            " group=\"Forearm\" ");
        groupSS << " \"" << barName << "\"/refFore " << " arrowcolor=" << colorStr << " \n";
    
	    TwAddVarRW(bar, "useRelFore", TW_TYPE_BOOLCPP, &relativeForeRef,
            " label='Show relative:' help='Display the arrow relative to camera.' group=\"Forearm\" ");

        TwAddVarCB(bar, "angleXZ_FA", TW_TYPE_FLOAT, TWArm_updateMaxXZ_FA, TWArm_readMaxXZ_FA, this,
            " label='Yaw:' help='Armpoint forearm yaw' group=\"Forearm\" min=0 max=360 ");
        TwAddVarCB(bar, "angleY_FA", TW_TYPE_FLOAT, TWArm_updateMaxY_FA, TWArm_readMaxY_FA, this,
            " label='Pitch:' help='Armpoint forearm pitch' group=\"Forearm\" min=0 max=360 ");
        TwAddVarCB(bar, "angleTH_FA", TW_TYPE_FLOAT, TWArm_updateThreshold_FA, TWArm_readThreshold_FA, this,
            " label='Threshold:' help='Armpoint forearm threshold' group=\"Forearm\" min=0 max=360 ");
        TwAddVarCB(bar, "fadeIn_FA", TW_TYPE_FLOAT, TWArm_updateFadeIn_FA, TWArm_readFadeIn_FA, this,
            " label='Fade in:' help='Armpoint forearm fade in' group=\"Forearm\" min=0 max=360 ");
        TwAddVarCB(bar, "fadeOut_FA", TW_TYPE_FLOAT, TWArm_updateFadeOut_FA, TWArm_readFadeOut_FA, this,
            " label='Fade out:' help='Armpoint forearm fade out' group=\"Forearm\" min=0 max=360 ");
        TwAddVarCB(bar, "Ipol_FA", TW_TYPE_FLOAT, TWArm_updateIpolTime_FA, TWArm_readIpolTime_FA, this,
            " label='Ipol time:' help='Armpoint forearm interpolation time'"
            " min=0 step=0.05 max=5 group=\"Forearm\" ");
        TwAddVarCB(bar, "strenght_FA", TW_TYPE_FLOAT, TWArm_updateStrength_FA, TWArm_readStrength_FA, this,
            " label='Strength:' help='Armpoint forearm strength time'"
            " min=0 step=0.05 max=1 group=\"Forearm\" ");

        TwDefine(groupSS.str().c_str());
    }
    {   //HAND
        TwAddVarCB(bar, "boneHand", boneEnum,
            TWArm_updateHandBone, TWArm_readHandBone, this,
            " label='Bone:' help='ArmPoint hand bone' group=\"Hand\" ");
	    TwAddVarCB(bar, "refRelHand", TW_TYPE_DIR3F, NULL, TWArm_readHandDir, this,
            " label='Front:' opened=false help='Reference relative to bone.' "
            " arrowcolor='127 127 127' axisx=-x axisz=-z showval=false group=\"Hand\" ");
	    TwAddVarRW(bar, "useRelHand", TW_TYPE_BOOLCPP, &relativeHandRef,
            " label='Show relative:' help='Display the arrow relative to camera.'"
            " readonly=true group=\"Hand\" ");
    }
}

void ArmPointTW::setActive(bool b)
{
    Entity* e(entity_h);
    CArmPoint* armPoint(e->get<CArmPoint>());
    active = b;
    armPoint->setAllActive(b);
}
void ArmPointTW::setMuted(bool b)
{
    Entity* e(entity_h);
    CArmPoint* armPoint(e->get<CArmPoint>());
    muted = b;
    armPoint->setActive(!b);
}

void ArmPointTW::draw() const
{
    if (isMuted()) {return;}
    setObjectConstants(XMMatrixAffineTransformation(one_v * 0.04f, zero_v, x90Rot_q, target), color);
    mesh_icosahedron_wire.activateAndRender();

    Entity* e = entity_h;
    CSkeleton* sk = e->get<CSkeleton>();
    auto calSk = sk->getModel()->getSkeleton();
    CArmPoint* la = e->get<CArmPoint>();

    struct {
        const bool& draw;
        int (CArmPoint::*getBone)();
        XMVECTOR (CArmPoint::*getRef)()const;
    } entries[2] = {
        {drawUpper, &CArmPoint::getUpperArm, &CArmPoint::getUpperArmRef},
        {drawFore, &CArmPoint::getForeArm, &CArmPoint::getForeArmRef},
    };

    for (const auto& e : entries) {
            if (e.draw) {
            auto b1 = calSk->getBone((la->*e.getBone)());
            auto pos = toXMVECTOR(b1->getTranslationAbsolute());
            auto rot = toXMQuaternion(b1->getRotationAbsolute());
            float len = XMVectorGetX(XMVector3Length(target-pos))*1.1f;
            auto ref = (la->*e.getRef)();
        
            XMMATRIX m = XMMatrixIdentity();
            XMVectorSetW(pos, 1.f);
            m.r[3] = pos;

            ref = XMVector3Rotate(ref, (rot));
            XMVectorSetW(ref, 0.f);
            m.r[2] = XMVector3Normalize(ref);
            m = XMMatrixMultiply(XMMatrixScalingFromVector(len*one_v), m);
            setObjectConstants(m, color);
            mesh_line.activateAndRender();

            ref = XMVector3Rotate(yAxis_v, rot);
            XMVectorSetW(ref, 0.f);
            m.r[2] = XMVector3Normalize(ref);
            m = XMMatrixMultiply(XMMatrixScalingFromVector(0.5f*one_v), m);
            setObjectConstants(m, Color::BLUE);
            mesh_line.activateAndRender();
            
            ref = XMVector3Rotate(xAxis_v, rot);
            XMVectorSetW(ref, 0.f);
            m.r[2] = XMVector3Normalize(ref);
            m = XMMatrixMultiply(XMMatrixScalingFromVector(0.5f*one_v), m);
            setObjectConstants(m, Color::RED);
            mesh_line.activateAndRender();

            setObjectConstants(XMMatrixAffineTransformation(one_v*0.5f, zero_v, rot, pos), Color::GREEN);
            mesh_line.activateAndRender();
        }
    }
}

}

#endif