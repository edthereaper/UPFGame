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

extern LookAtTW* lookAtTw[4] = {nullptr, nullptr, nullptr, nullptr};

void TW_CALL TWBLA_setAllActive(const void *value, void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    const bool* b = (const bool*) value;
    tw->setActive(*b);
}
void TW_CALL TWBLA_setMuted(const void *value, void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    const bool* b = (const bool*) value;
    tw->setMuted(*b);
}
void TW_CALL TWBLA_save(void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    tw->save();
}
void TW_CALL TWBLA_reload(void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    tw->reload();
}
void TW_CALL TWBLA_updateTarget(const void *value, void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    const float* ret = (const float*) value;
    tw->setTarget(XMVectorSet(ret[0], ret[1], ret[2], 0));
}
void TW_CALL TWBLA_resetTarget(void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    tw->resetTarget();
}
void TW_CALL TWBLA_updateLookAtRef(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    XMVECTOR ref = XMVectorSet(ret[0], ret[1], ret[2], 0);
    XMVECTOR skelQ = one_q;
    auto skeleton(((CSkeleton*)e->self->getEntity().getSon<CSkeleton>())->getModel()->getCoreModel()->getCoreSkeleton());
    skelQ = toXMQuaternion(skeleton->getCoreBone(entry.calBone)->getRotationAbsolute());
    if (entry.calSecondBone >= 0) {
        XMVECTOR q2 = toXMQuaternion(skeleton->getCoreBone(entry.calSecondBone)->getRotationAbsolute());
        skelQ = XMQuaternionSlerp(skelQ, q2, 0.5f);
    }
    
    if (e->relativeRef) {
        CTransform* camT = e->self->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, (camT->getRotation()));
    }
    ref = XMVector3Rotate(ref, XMQuaternionInverse(skelQ));
    entry.setRef(ref);
}
void TW_CALL TWBLA_readLookAtRef(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    XMVECTOR ref = entry.getRef();

    XMVECTOR skelQ = one_q;
    auto skeleton(((CSkeleton*)e->self->getEntity().getSon<CSkeleton>())->getModel()->getCoreModel()->getCoreSkeleton());
    skelQ = toXMQuaternion(skeleton->getCoreBone(entry.calBone)->getRotationAbsolute());
    if (entry.calSecondBone >= 0) {
        XMVECTOR q2 = toXMQuaternion(skeleton->getCoreBone(entry.calSecondBone)->getRotationAbsolute());
        skelQ = XMQuaternionSlerp(skelQ, q2, 0.5f);
    }
    ref = XMVector3Rotate(ref, skelQ);
    
    if (e->relativeRef) {
        CTransform* camT = e->self->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, XMQuaternionInverse(camT->getRotation()));
    }
    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}

void TW_CALL TWBLA_updateIpolTime(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    entry.params.ipolTime = *ret;
}
void TW_CALL TWBLA_updateActive(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const bool* ret = (const bool*) value;
    entry.setActive(*ret);
}
void TW_CALL TWBLA_updateDraw(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    const bool* ret = (const bool*) value;
    e->draw = *ret;
}
void TW_CALL TWBLA_updateMaxXZ(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    entry.params.maxAngleXZ = deg2rad(*ret);
}
void TW_CALL TWBLA_updateMaxY(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*)value;
    entry.params.maxAngleY = deg2rad(*ret);
}
void TW_CALL TWBLA_updateStrength(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    entry.params.strength = *ret;
}
void TW_CALL TWBLA_updateThreshold(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    entry.params.threshold = deg2rad(*ret);
}
void TW_CALL TWBLA_updateFadeIn(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    entry.params.fadeIn = deg2rad(*ret);
}
void TW_CALL TWBLA_updateFadeOut(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const float* ret = (const float*) value;
    entry.params.fadeOut = deg2rad(*ret);
}
void TW_CALL TWBLA_updateBone1(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const int* ret = (const int*) value;
    entry.calBone = *ret;
}
void TW_CALL TWBLA_updateBone2(const void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    const int* ret = (const int*) value;
    entry.calSecondBone = *ret;
}
    
void TW_CALL TWBLA_getAllActive(void *value, void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    bool* ret = (bool*) value;
    *ret = tw->isActive();
}    
void TW_CALL TWBLA_getMuted(void *value, void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    bool* ret = (bool*) value;
    *ret = tw->isMuted();
}
void TW_CALL TWBLA_readTarget(void *value, void *clientData)
{ 
    LookAtTW* tw(static_cast<LookAtTW*>(clientData));
    float* ret = (float*) value;
    XMVECTOR v = tw->getTarget();
    ret[0] = XMVectorGetX(v);
    ret[1] = XMVectorGetY(v);
    ret[2] = XMVectorGetZ(v);
}
void TW_CALL TWBLA_readBoneDir(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    XMVECTOR ref = entry.getRef();

    XMVECTOR skelQ = one_q;
    auto skeleton(((CSkeleton*)e->self->getEntity().getSon<CSkeleton>())->getModel()->getSkeleton());
    skelQ = toXMQuaternion(skeleton->getBone(entry.calBone)->getRotationAbsolute());
    if (entry.calSecondBone >= 0) {
        XMVECTOR q2 = toXMQuaternion(skeleton->getBone(entry.calSecondBone)->getRotationAbsolute());
        skelQ = XMQuaternionSlerp(skelQ, q2, 0.5f);
    }
    ref = XMVector3Rotate(ref, skelQ);
    
    if (e->relativeRef) {
        CTransform* camT = e->self->getCamera().getSon<CTransform>();
        ref = XMVector3Rotate(ref, XMQuaternionInverse(camT->getRotation()));
    }
    ret[0] = XMVectorGetX(ref);
    ret[1] = XMVectorGetY(ref);
    ret[2] = XMVectorGetZ(ref);
}
void TW_CALL TWBLA_readIpolTime(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = entry.params.ipolTime;
}
void TW_CALL TWBLA_readStrength(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = entry.params.strength;
}
void TW_CALL TWBLA_readActive(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    bool* ret = (bool*) value;
    *ret = !entry.isInactive();
}
void TW_CALL TWBLA_readMaxXZ(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = rad2deg(entry.params.maxAngleXZ);
}
void TW_CALL TWBLA_readMaxY(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = rad2deg(entry.params.maxAngleY);
}
void TW_CALL TWBLA_readThreshold(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = rad2deg(entry.params.threshold);
}
void TW_CALL TWBLA_readFadeIn(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = rad2deg(entry.params.fadeIn);
}
void TW_CALL TWBLA_readFadeOut(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    float* ret = (float*) value;
    *ret = rad2deg(entry.params.fadeOut);
}
void TW_CALL TWBLA_readBone1(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    int* ret = (int*) value;
    *ret = entry.calBone;
}
void TW_CALL TWBLA_readBone2(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    CBoneLookAt* lookAt(e->handle.getSon<CBoneLookAt>());
    auto& entry = lookAt->getEntry(e->id);
    int* ret = (int*) value;
    *ret = entry.calSecondBone;
}
void TW_CALL TWBLA_readDraw(void *value, void *clientData)
{ 
    LookAtTW::entryTw_t* e(static_cast<LookAtTW::entryTw_t*>(clientData));
    bool* ret = (bool*) value;
    *ret = e->draw;
}

void LookAtTW::setTarget(XMVECTOR v)
{
    target = v;
    CBoneLookAt* lookAt(entity_h.getSon<CBoneLookAt>());
    lookAt->setTarget(v);
}

void LookAtTW::createBoneEnum()
{
    Entity* e(entity_h);
    std::stringstream ss;
    ss << "eBONE_LA_" << e->getName();
    CSkeleton* skeleton(e->get<CSkeleton>());
    auto model = skeleton->getModel()->getCoreModel();
    auto boneVector = model->getCoreSkeleton()->getVectorCoreBone();
    int nBones = static_cast<int>(boneVector.size());
    TwEnumVal* boneEnumLabels;
    boneEnumLabels = new TwEnumVal[nBones+1];
    boneEnumLabels[0].Label = "<NO BONE>";
    boneEnumLabels[0].Value = -1;
    for (int i=0; i<nBones; i++) {
        boneEnumLabels[i+1].Label = boneVector[i]->getName().c_str();
        boneEnumLabels[i+1].Value = i;
    }
    validBoneEnum = TwDefineEnum((ss.str()+"_valid").c_str(), &boneEnumLabels[1], nBones);
    boneEnum = TwDefineEnum(ss.str().c_str(), boneEnumLabels, nBones+1);
    delete boneEnumLabels;
}

void LookAtTW::resetTarget()
{
    CTransform* t = entity_h.getSon<CTransform>();
    setTarget(XMVector3Transform(DirectX::XMVectorSet(0,2,2,0),t->getWorld()));
}

void LookAtTW::reload()
{
    std::string str = filename.substr(0,filename.find_last_of('.'));
    bool found = false;
    for (auto pos = size_t(0);
        pos != std::string::npos && found == false;
        pos = str.find("/"), str = str.substr(pos+1)) {
        found = PrefabManager::get().prefabricateComponents(str.c_str(), entity_h, true);
    }
}

void LookAtTW::save()
{
    std::ofstream os(filename);
    Entity* e(entity_h);
    CBoneLookAt* lookAt(e->get<CBoneLookAt>());
    CSkeleton* skeleton(e->get<CSkeleton>());
    auto calSkeleton = skeleton->getModel()->getCoreModel()->getCoreSkeleton();
    MKeyValue mapLA;
    mapLA.setFloat("strength", lookAt->getGlobalStrength());
    mapLA.setBool("clear", true);
    mapLA.writeStartElement(os, "BoneLookAt", "", " ", "");
    for (unsigned i=0; i<CBoneLookAt::N_LOOKATS; ++i) {
        auto& entry(lookAt->getEntry(i));
        MKeyValue mapE;
        mapE.setString("boneName", calSkeleton->getCoreBone(entry.calBone)->getName());
        if (entry.calSecondBone >= 0) {
            mapE.setString("bone2Name", calSkeleton->getCoreBone(entry.calSecondBone)->getName());
        }
        const auto& params(entry.getParameters());
        mapE.setFloat("strength", params.strength);
        mapE.setFloat("maxAngleXZ", rad2deg(params.maxAngleXZ));
        mapE.setFloat("maxAngleY", rad2deg(params.maxAngleY));
        mapE.setFloat("fadeIn", rad2deg(params.fadeIn));
        mapE.setFloat("fadeOut", rad2deg(params.fadeOut));
        mapE.setFloat("threshold", rad2deg(params.threshold));
        mapE.setFloat("ipolTime", params.ipolTime);
        mapE.setPoint("referenceDir", XMVector3Rotate(entry.getRef(), utils::skeletonRefInv_q), 2);
        mapE.writeSingle(os, "entry", "\t", "\n", "\t\t");
    }
    mapLA.writeEndElement(os, "BoneLookAt");

}

LookAtTW::LookAtTW(Handle entity_h, Handle camera_h, std::string filename, Color c)
    : entity_h(entity_h), camera_h(camera_h), filename(filename), color(c)
{
    Entity* e(entity_h);
    if (!e->has<CBoneLookAt>()) {
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
    ss.str(""); ss << "LookAt - " << e->getName();
	bar = TwNewBar(ss.str().c_str());

    ss.str(""); ss << "\"" << TwGetBarName(bar) << "\" "
        "color=\"" << int(c.r()) << " " << int(c.g()) << " " << int(c.b()) << "\" ";
    const ColorHSL hsl(c);
    if (hsl.l() > 0.5f) {ss<<" text=dark ";} else {ss<<" text=light ";}
    TwDefine(ss.str().c_str());

    TwAddButton(bar, "save", TWBLA_save, this, " label='SAVE' ");
    TwAddButton(bar, "reload", TWBLA_reload, this, " label='RELOAD' ");
    TwAddVarCB(bar, "mute", TW_TYPE_BOOLCPP, TWBLA_setMuted, TWBLA_getMuted, this,
        " label='MUTE' help='mute lookat' ");
    TwAddVarCB(bar, "activate", TW_TYPE_BOOLCPP, TWBLA_setAllActive, TWBLA_getAllActive, this,
        " label='ACTIVATE' help='activate lookat' ");
    
    ss.str(""); ss << "label='Move target:' help='Move the lookat target around.' axisx=-x axisz=-z "
        << "arrowcolor=\"" << int(c.r()) << " " << int(c.g()) << " " << int(c.b()) << "\" ";
	TwAddVarCB(bar, "target", TW_TYPE_DIR3F, TWBLA_updateTarget, TWBLA_readTarget, this,
        ss.str().c_str());
    TwAddButton(bar, "resetTarget", TWBLA_resetTarget, this, " label='Reset target' ");

    TwAddSeparator(bar,NULL,NULL);
    TwAddButton(bar, NULL, NULL, NULL, " label='Influences: (Order matters)' ");

    for (unsigned i = 0; i<ARRAYSIZE(entryTw); i++) {
        createEntry(i);
    }
}

void LookAtTW::createEntry(CBoneLookAt::entryId_t i)
{
    entryTw[i].handle = entity_h;
    entryTw[i].id = i;
    entryTw[i].self = this;
    void* dataPtr = &entryTw[i];
    Entity* e(entity_h);
    CSkeleton* skeleton(e->get<CSkeleton>());
    auto nBones = skeleton->getModel()->getCoreModel()->getCoreSkeleton()->getVectorCoreBone().size();
    CBoneLookAt* lookAt(e->get<CBoneLookAt>());
    auto& entry = lookAt->getEntry(i);

    std::stringstream ss;
    std::stringstream groupSS;
    ss << "Influence " << int(i);
    std::string group = ss.str();
    std::string varName;
    std::string barName = TwGetBarName(bar);
    
    ss.str(""); ss << "boneA" << int(i); varName = ss.str().c_str();
    TwAddVarCB(bar, varName.c_str(), validBoneEnum, TWBLA_updateBone1, TWBLA_readBone1, dataPtr,
        " label='Bone:' help='LookAt Bone'");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "boneB" << int(i); varName = ss.str().c_str();
    TwAddVarCB(bar, varName.c_str(), boneEnum, TWBLA_updateBone2, TWBLA_readBone2, dataPtr,
        " label='Dual bone:' help='If a dual bone exists, both bones are affected and the calculations"
        "take the average of both bones'");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";

    ss.str(""); ss << "active" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_BOOLCPP, TWBLA_updateActive, TWBLA_readActive, dataPtr,
        " label='Active:' help='Affects only this viewer.' ");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";

    ss.str(""); ss << "draw" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_BOOLCPP, TWBLA_updateDraw, TWBLA_readDraw, dataPtr,
        " label='Draw:' help='Draw the bone front.' ");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "refRelBone" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_DIR3F, NULL, TWBLA_readBoneDir, dataPtr,
        " label='Bone front:' opened=false help='Reference relative to bone.' readonly=true"
        " arrowcolor='127 127 127' axisx=-x axisz=-z showval=false");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";

    ss.str(""); ss << "ref" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_DIR3F, TWBLA_updateLookAtRef, TWBLA_readLookAtRef, dataPtr,
        " label='Reference :' axisx=-x axisz=-z opened=true help='Define reference vector for lookat.' ");
    groupSS << " \"" << barName << "\"/\"" << varName << "\"" <<
        " arrowcolor=\"" << int(color.r()) << " " << int(color.g()) << " " << int(color.b()) << "\" \n";
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "relativeRef" << int(i); varName = ss.str().c_str();
	TwAddVarRW(bar, varName.c_str(), TW_TYPE_BOOLCPP, &entryTw[i].relativeRef,
        " label='Show relative:' help='Display the arrow relative to camera.' ");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";

    ss.str(""); ss << "pitch" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateMaxY, TWBLA_readMaxY, dataPtr,
        " label='Max pitch:' help='Max angle in ZY plane' min=0 max=360 step=1");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "yaw" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateMaxXZ, TWBLA_readMaxXZ, dataPtr,
        " label='Max Yaw:' help='Max angle in XZ plane' min=0 max=360 step=1");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "threshold" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateThreshold, TWBLA_readThreshold, dataPtr,
        " label='Threshold:' help=\"LookAt doesn't take effect beyond threshold\" min=0 max=360 step=1");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";

    ss.str(""); ss << "fadeIn" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateFadeIn, TWBLA_readFadeIn, dataPtr,
        " label='Fade in:' help='Fade in angle' min=0 max=360 step=1");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "fadeOut" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateFadeOut, TWBLA_readFadeOut, dataPtr,
        " label='Fade out:' help='Fade out angle' min=0 max=360 step=1");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    ss.str(""); ss << "strength" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateStrength, TWBLA_readStrength, dataPtr,
        " label='Strength:' help='Strength of the lookAt' min=0 max=1 step=0.05");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";

    ss.str(""); ss << "iPol" << int(i); varName = ss.str().c_str();
	TwAddVarCB(bar, varName.c_str(), TW_TYPE_FLOAT, TWBLA_updateIpolTime, TWBLA_readIpolTime, dataPtr,
        " label='Ipol time:' help='Interpolation time' min=0 step=0.05");
    groupSS << " \"" << barName << "\"/\"" << varName << "\" group=\"" << group << "\" \n";
    
    groupSS << " \"" << barName << "\"/\"" << group << "\" opened=false \n ";
    TwDefine(groupSS.str().c_str());
}
void LookAtTW::setActive(bool b)
{
    Entity* e(entity_h);
    CBoneLookAt* lookAt(e->get<CBoneLookAt>());
    active = b;
    lookAt->setAllActive(b);
}
void LookAtTW::setMuted(bool b)
{
    Entity* e(entity_h);
    CBoneLookAt* lookAt(e->get<CBoneLookAt>());
    muted = b;
    lookAt->setActive(!b);
}

void LookAtTW::draw() const
{
    if (isMuted()) {return;}

    setObjectConstants(XMMatrixAffineTransformation(one_v * 0.05f, zero_v, one_q, target), color);
    mesh_icosahedron_wire.activateAndRender();

    Entity* e = entity_h;
    CSkeleton* sk = e->get<CSkeleton>();
    auto calSk = sk->getModel()->getSkeleton();
    CBoneLookAt* la = e->get<CBoneLookAt>();

    for (const auto& e : entryTw) {
            if (e.draw) {
            const auto& entry = la->getEntry(e.id);
            auto b1 = calSk->getBone(entry.calBone);
            auto pos = toXMVECTOR(b1->getTranslationAbsolute());
            auto rot = toXMQuaternion(b1->getRotationAbsolute());
            if (entry.calSecondBone >= 0) {
                auto b2 = calSk->getBone(entry.calSecondBone);
                pos = (pos + toXMVECTOR(b2->getTranslationAbsolute()))/2;
                rot = XMQuaternionSlerp(rot, toXMQuaternion(b2->getRotationAbsolute()), 0.5f);
            }
            float len = XMVectorGetX(XMVector3Length(target-pos))*1.1f;
            auto ref = entry.getRef();
        
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