#include "mcv_platform.h"
#include "props.h"

#include "components/transform.h"
#include "components/color.h"
#include "handles/entity.h"
#include "handles/prefab.h"
using namespace component;

#include "render/mesh/CInstancedMesh.h"
#include "render/mesh/component.h"
#include "render/mesh/mesh.h"
#include "render/renderManager.h"
using namespace render;

using namespace utils;
using namespace DirectX;

#include "gameElements/pickup.h"
#include "gameElements/liana.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

#include "destructible.h"
using namespace behavior;

#include "fmod_User/fmodUser.h"
using namespace fmodUser;

#include "render/texture/tilemap.h"
using namespace render;

#include "Particles/ParticlesManager.h"

    
TransformableFSM::container_t TransformableFSM::states;
void TransformableFSM::initType()
{
    SET_FSM_STATE(notTransformed);
    SET_FSM_STATE(hit);
    SET_FSM_STATE(transforming);
    SET_FSM_STATE(changeMeshOut);
    SET_FSM_STATE(changeMeshIn);
    SET_FSM_STATE(changeCreepOut);
    SET_FSM_STATE(changeCreepIn);
    SET_FSM_STATE(changeMaterial);
    SET_FSM_STATE(changeTint);
    SET_FSM_STATE(transformed);
    SET_FSM_STATE(breathe);
    SET_FSM_STATE(breatheXZ);
	SET_FSM_STATE(spring);
	SET_FSM_STATE(hammerTransform);
    SET_FSM_STATE(hammerTransformed);
}

namespace gameElements {

#define TIME_HIDE_ONSHOT	 0.25f
#define TIME_SHOW_ONSHOT	 0.35f
#define TIME_HIDE_CREEP_ONSHOT	 (TIME_HIDE_ONSHOT*.5f)
#define TIME_SHOW_CREEP_ONSHOT	 (TIME_SHOW_ONSHOT*.5f)
#define TIME_TINT_FADE       0.5f
#define TRANSFORM_ROTATE     deg2rad(360)
#define DEFAULT_TRANSF_MESH "canon"
#define DEFAULT_NONTRANSF_MESH "salida_humo"
    
const float TransformableFSMExecutor:: GLOW_RAMP_UP_TIME = 2.f;
const float TransformableFSMExecutor:: GLOW_RAMP_DOWN_TIME = 1.f;
const float TransformableFSMExecutor:: GLOW_FREQ = 1/0.88f;
const float TransformableFSMExecutor:: GLOW_COS_DEVIATION = 0.25f;
const float TransformableFSMExecutor:: GLOW_COS_MEAN = 1;

const float TransformableFSMExecutor:: GLOW_MARKED_RAMP_UP_TIME = 0.5f;
const float TransformableFSMExecutor:: GLOW_MARKED_RAMP_DOWN_TIME = 0.25f;
const float TransformableFSMExecutor:: GLOW_MARKED_FREQ = 3.f;
const float TransformableFSMExecutor:: GLOW_MARKED_COS_DEVIATION = 0.15f;
const float TransformableFSMExecutor:: GLOW_MARKED_COS_MEAN = 0.95f;

const float TransformableFSMExecutor:: GLOW_SPECIAL_RAMP_UP_TIME = 1.f;
const float TransformableFSMExecutor:: GLOW_SPECIAL_RAMP_DOWN_TIME = 0.5f;
const float TransformableFSMExecutor:: GLOW_SPECIAL_FREQ = 2.f;
const float TransformableFSMExecutor:: GLOW_SPECIAL_COS_DEVIATION = 0.45f;
const float TransformableFSMExecutor:: GLOW_SPECIAL_COS_MEAN = 1;

utils::Counter<float> TransformableFSMExecutor::glowTimer = -GLOW_RAMP_UP_TIME;
float TransformableFSMExecutor::currGlow = 0;
float TransformableFSMExecutor::prevGlow = 0;
bool TransformableFSMExecutor::glowingDown = false;

utils::Counter<float> TransformableFSMExecutor::glowTimer_special = -GLOW_SPECIAL_RAMP_UP_TIME;
float TransformableFSMExecutor::currGlow_special = 0;
float TransformableFSMExecutor::prevGlow_special = 0;
bool TransformableFSMExecutor::glowingDown_special = false;

TransformableFSMExecutor::updateHighlightsFnData TransformableFSMExecutor::generalHLUpdateData =
    updateHighlightsFnData (
        &prevGlow, &currGlow, &glowTimer, &glowingDown,
        GLOW_RAMP_UP_TIME,
        GLOW_RAMP_DOWN_TIME,
        GLOW_FREQ,
        GLOW_COS_DEVIATION,
        GLOW_COS_MEAN
    );
TransformableFSMExecutor::updateHighlightsFnData TransformableFSMExecutor::specialHLUpdateData =
    updateHighlightsFnData (
        &prevGlow_special, &currGlow_special, &glowTimer_special, &glowingDown_special,
        GLOW_SPECIAL_RAMP_UP_TIME,
        GLOW_SPECIAL_RAMP_DOWN_TIME,
        GLOW_SPECIAL_FREQ,
        GLOW_SPECIAL_COS_DEVIATION,
        GLOW_SPECIAL_COS_MEAN
    );

void TransformableFSMExecutor::updateHighlightsFn(float elapsed, bool increase,
    updateHighlightsFnData& data)
{
    *data.prevGlow = *data.currGlow;
    if (increase) {
        if (*data.goingDown) {
            auto a = std::asin(inRange(0.f,*data.currGlow,1.f));
            auto t = data.rampUpTime * (a/M_PI_2f - 1);
            data.glowTimer->set(t);
        }
        *data.goingDown = false;
        auto f = *data.glowTimer;
        if (f < 0) {
            *data.currGlow = std::sin((1 + f/data.rampUpTime) * M_PI_2f);
        } else {
            *data.currGlow = std::cos(f * M_2_PIf * data.growFreq)
                * data.glowCosDeviation + data.glowCosMean;
        }
        data.glowTimer->count(elapsed);
    } else if (!(*data.currGlow == 0 && *data.prevGlow == 0)){
        if (!*data.goingDown) {
            auto a = std::acos(inRange(0.f,*data.currGlow,1.f));
            auto t = -data.rampDownTime * (a/M_PI_2f - 1);
            data.glowTimer->set(t);
        }
        *data.goingDown = true;
        auto f = *data.glowTimer;
        if (f > 0) {
            *data.currGlow = std::cos((1-f/data.rampDownTime) * M_PI_2f);
        } else {
            *data.currGlow = 0;
            data.glowTimer->set(-data.rampUpTime);
            *data.goingDown = false;
        }
        data.glowTimer->count(-elapsed);
    }
}

void TransformableFSMExecutor::updateHighlights(float elapsed, bool increase)
{
    updateHighlightsFn(elapsed, increase, generalHLUpdateData);
}

void TransformableFSMExecutor::updateSpecialHighlights(float elapsed, bool increase)
{
    updateHighlightsFn(elapsed, increase, specialHLUpdateData);
}

const float TransformableFSMExecutor::TRANSFORMED_DIFFUSE_SELFILLUMINATION = 0.2f;
const float TransformableFSMExecutor::GLOW_FACTOR = 0.5f;

void TransformableFSMExecutor::applyDiffuseAsSelfIllumination(float f, bool transformed, bool transforming)
{
	Entity* me(meEntity);
    CSelfIllumination* si = me->get<CSelfIllumination>();
    CMesh* mesh = me->get<CMesh>();
    mesh->setDiffuseAsSelfIllumination(f);
    auto instancedMesh_h = transformed ? instancedMeshYes_h : instancedMeshNot_h;
    if (!transforming && instancedMesh_h.isValid()) {
        CMesh* mesh(instancedMesh_h.getBrother<CMesh>());
        mesh->setDiffuseAsSelfIllumination(f);
    }
}

void TransformableFSMExecutor::applySelfIllumination(const Color& color, float clamp,
    bool transformed, bool transforming)
{
	static auto applyToSingleMesh = [] (Handle h, const Color& color, float clamp) {
        Entity* me(h);
        CSelfIllumination* si = me->get<CSelfIllumination>();
        CMesh* mesh = me->get<CMesh>();
        mesh->setSelfIlluminationClamp(clamp);
        si->set(color);
        RenderManager::updateKeys(h);
    };

    auto instancedMesh_h = transformed ? instancedMeshYes_h : instancedMeshNot_h;
    if (transforming) {
        applyToSingleMesh(meEntity, color, clamp);
    } else if (instancedMesh_h.isValid()) {
        CTransform* t = meEntity.getSon<CTransform>();
        CInstancedMesh* instancedMesh(instancedMesh_h);
        bool isOk = instancedMesh->changeInstanceSelfIllumination(instanceIndex,color);
        assert(isOk &&"Instance not found!");
        CMesh* mesh(instancedMesh_h.getBrother<CMesh>());
        mesh->setSelfIlluminationClamp(clamp);
    } else {
        applyToSingleMesh(meEntity, color, clamp);
    }
    
}

void TransformableFSMExecutor::applyTint(const Color& color, bool transformed, bool transforming)
{
	static auto applyToSingleMesh = [] (Handle h, const Color& color) {
        Entity* me(h);
        CTint* si = me->get<CTint>();
        CMesh* mesh = me->get<CMesh>();
        si->set(color);
        RenderManager::updateKeys(h);
    };

    auto instancedMesh_h = transformed ? instancedMeshYes_h : instancedMeshNot_h;
    if (transforming) {
        applyToSingleMesh(meEntity, color);
    } else if (instancedMesh_h.isValid()) {
        CTransform* t = meEntity.getSon<CTransform>();
        CInstancedMesh* instancedMesh(instancedMesh_h);
        bool isOk = instancedMesh->changeInstanceTint(instanceIndex,color);
        assert(isOk &&"Instance not found!");
    } else {
        applyToSingleMesh(meEntity, color);
    }
}

fsmState_t TransformableFSMExecutor::notTransformed(float elapsed)
{
    float prevG = type != TRANSFORMABLE_HAMMER ? prevGlow : prevGlow_special;
    float currG = type != TRANSFORMABLE_HAMMER ? currGlow : currGlow_special;

    if (prevG != currG || prevMarkedGlow != currMarkedGlow) {
        auto ma = currMarkedGlow*GLOW_FACTOR;
        auto mc = Color(glowMarkedColor).setAf(ma*glowMarkedColor.af());
        auto mm = glowMarkedMax*ma;
        mc.premultiplyAlpha();
        
        auto ga = currG*GLOW_FACTOR-ma;
        auto gc = Color(glowColor).setAf(ga*glowColor.af());
        auto gm = glowMax*ga;
        gc.premultiplyAlpha();
        
        auto color = marked? mc : gc;
        auto currentGlowMax = marked? mm : gm;
        applySelfIllumination(color, currentGlowMax, false);
    }
    return damageDone != 0 ? STATE_hit : STATE_notTransformed;
}

fsmState_t TransformableFSMExecutor::hit(float elapsed)
{
    hits -= damageDone;
    damageDone = 0;
    if (hits < 0) {
        return STATE_transforming;
    } else {
        return STATE_notTransformed; /*blink, etc? (more states)*/
    }
}

fsmState_t TransformableFSMExecutor::transforming(float elapsed)
{
    Entity* me = meEntity;
    CTransform* t = me->get<CTransform>();

    switch (type) {
        case TRANSFORMABLE_LIANA:
        case TRANSFORMABLE_MESH: {
                setupTransforming();
                originalRotation = t->getRotation();
                originalScale = t->getScale();
                return STATE_changeMeshOut;
            } break;
        case TRANSFORMABLE_MATERIAL: {
                setupTransforming();
                return STATE_changeMaterial;
            } break;
        case TRANSFORMABLE_TINT: {
                setupTransforming();
                CTint* tint = me->get<CTint>();
                assert(tint != nullptr);
                previousTint = *tint;
                return STATE_changeTint;
            } break;
        case TRANSFORMABLE_CREEP : {
                setupTransforming();
                originalRotation = t->getRotation();
                originalScale = t->getScale();
                return STATE_changeCreepOut;
            } break;
        case TRANSFORMABLE_HAMMER : {
                originalRotation = t->getRotation();
                originalScale = t->getScale();
                me->sendMsg(MsgTransform());
                
                CMesh* m1 = xtraMesh1.getSon<CMesh>();
                CMesh* m2 = xtraMesh2.getSon<CMesh>();
                m1->setVisible(true);
                m2->setVisible(true);
                
                CTransform* t1 = xtraMesh1.getSon<CTransform>();
                CTransform* t2 = xtraMesh2.getSon<CTransform>();
                t1->setPosition(t->getPosition());
                t2->setPosition(t->getPosition());
                CRestore* r1 = xtraMesh1.getSon<CRestore>();
                CRestore* r2 = xtraMesh2.getSon<CRestore>();
                r1->set(*t1);
                r2->set(*t2);
                t1->setScale(zero_v);
                t2->setScale(zero_v);
                applySelfIllumination(0,1,true,true);

                return STATE_hammerTransform;
            } break;
        default: case TRANSFORMABLE_NONE: {
                setupTransforming();
                setupTransformed();
                me->sendMsg(MsgTransform());
                return STATE_transformed;
            } break;
    }
}

//t in [0,Pi/2]
inline float yTransfOut(float t)
{
    float f = t*std::cos(t)*0.75f + std::sin(t)+std::cos(2*t);
    return f;
}
inline float yTransfIn(float t)
{
    float s = std::cos(t)+t*std::cos(t);
    if (t < M_PI*6/30) {
        s += std::sin(15*(M_PI_2f - t))*0.10f;
    }
    return s;
}

inline float xzTransf(float t)
{
    return std::cos(t)*std::cos(t);
}
inline float angleTransf(float t)
{
    return std::sin(t);
}
inline float zCreepTransf(float t)
{
    float f = std::cos(t)*(1-std::sin(t));
    return f;
}

fsmState_t TransformableFSMExecutor::changeTint(float elapsed)
{
    Entity* me = meEntity;
    CMesh* mesh = me->get<CMesh>();
    if (timer.count(elapsed) >= TIME_TINT_FADE) {
        timer.reset();
        applyTint(targetTint, true);
        applyTint(targetTint, false);
        RenderManager::updateKeys(meEntity);
        
        setupTransformed();
        me->sendMsg(MsgTransform());
        return STATE_transformed;
    } else {
        auto tint = Color(targetTint).setAf(targetTint.af() * timer.get() / TIME_TINT_FADE);
        applyTint(tint, true);
        applyTint(tint, false);
        RenderManager::updateKeys(meEntity);
        return STATE_changeTint;
    }
}

fsmState_t TransformableFSMExecutor::changeCreepIn(float elapsed)
{
	Entity* me(meEntity);
	CTransform* t = me->get<CTransform>();
	if (timer.count(elapsed) >= TIME_SHOW_CREEP_ONSHOT) {
        timer.reset();
		t->setScale(originalScale);
		me->sendMsg(MsgTransform());
		t->setRotation(originalRotation);
        setupTransformed();
		return STATE_transformed;
	} else {
		float s = (1 - timer.get() / TIME_SHOW_CREEP_ONSHOT);
        auto xz(xzTransf(s));
		t->setScale(originalScale * XMVectorSet(xzTransf(s), 1, zCreepTransf(s), 0));
		float angle(angleTransf(s)*TRANSFORM_ROTATE);
		return STATE_changeCreepIn;
	}
}

fsmState_t TransformableFSMExecutor::changeCreepOut(float elapsed)
{
	if (timer.count(elapsed) >= TIME_HIDE_CREEP_ONSHOT) {
		timer.reset();
		Entity* element = meEntity;
        CCreep* creep = element->get<CCreep>();
        creep->setupTransformed();
		return STATE_changeCreepIn;
	} else {
		CTransform* t = meEntity.getSon<CTransform>();
		float s = M_PI_2f*(timer.get() / TIME_HIDE_CREEP_ONSHOT);
		t->setScale(originalScale * XMVectorSet(xzTransf(s), 1, yTransfOut(s), 0));
		float angle(angleTransf(s)*TRANSFORM_ROTATE);
		return STATE_changeCreepOut;
	}
}

fsmState_t TransformableFSMExecutor::changeMeshOut(float elapsed)
{
	if (timer.count(elapsed) >= TIME_HIDE_ONSHOT) {
		timer.reset();
		Entity* element = meEntity;
		CMesh* cmesh = element->get<CMesh>();
        cmesh->removeMesh();
		CMesh::load(resourceName, element, DEFAULT_TRANSF_MESH);
		cmesh->init();
        CAABB* aabb = element->get<CAABB>();
        if (aabb != nullptr) {
            *aabb = cmesh->getMesh()->getAABB();
            CCullingAABB* cullingAABB = element->get<CCullingAABB>();
            if (cullingAABB != nullptr) {
            #ifdef _DEBUG
                cullingAABB->dbgColor = ColorHSL(rand_uniform(1.f));
            #endif
                cullingAABB->setDirty();
            }
        }
		return STATE_changeMeshIn;
	} else {
		CTransform* t = meEntity.getSon<CTransform>();
		float s = M_PI_2f*(timer.get() / TIME_HIDE_ONSHOT);
        auto xz = xzTransf(s);
		t->setScale(originalScale * XMVectorSet(xz, yTransfOut(s), xz, 0));
		float angle(angleTransf(s)*TRANSFORM_ROTATE);
        Transform originalRotTransform;
        originalRotTransform.setRotation(originalRotation);
		t->setRotation(XMQuaternionMultiply(
			originalRotation, XMQuaternionRotationAxis(originalRotTransform.getUp(), angle)));
		return STATE_changeMeshOut;
	}
}

fsmState_t TransformableFSMExecutor::changeMeshIn(float elapsed)
{
	Entity* me(meEntity);
	CTransform* t = me->get<CTransform>();
	if (timer.count(elapsed) >= TIME_SHOW_ONSHOT) {
        timer.reset();
		t->setScale(originalScale);
		me->sendMsg(MsgTransform());
		t->setRotation(originalRotation);
        setupTransformed();
		return STATE_transformed;
	} else {
		float s = M_PI_2f*(1 - timer.get() / TIME_SHOW_ONSHOT);
        auto xz(xzTransf(s));
		t->setScale(originalScale * XMVectorSet(xz, yTransfIn(s), xz, 0));
		float angle(angleTransf(s)*TRANSFORM_ROTATE);
        Transform originalRotTransform;
        originalRotTransform.setRotation(originalRotation);
		t->setRotation(XMQuaternionMultiply(
			originalRotation, XMQuaternionRotationAxis(originalRotTransform.getUp(), angle)));
		return STATE_changeMeshIn;
	}
}

fsmState_t TransformableFSMExecutor::changeMaterial(float elapsed)
{
	Entity* me(meEntity);
	CMesh* cmesh = me->get<CMesh>();
	CTint* self = me->get<CTint>();
	*self = 0xFFFFFFFF;
	cmesh->setMaterial(Material::getManager().getByName(resourceName.c_str()));
	cmesh->init();
    setupTransformed();
	me->sendMsg(MsgTransform());

	return STATE_transformed;
}

#define TIME_SPRING 1.25f

inline float scaSpring(float t)
{
    //sinc
    return std::sin((t+0.1f)*M_PIf*10)/((t+0.1f)*M_PIf*10);
}

inline float yScaSpring(float t)
{
    return std::max(1+10.f*scaSpring(t),0.45f);
}
inline float xzScaSpring(float t)
{
    return std::max(1-1.5f*scaSpring(t),0.45f);
}

fsmState_t  TransformableFSMExecutor::spring(float elapsed)
{
    Entity* me = meEntity;
    CMesh* mesh = me->get<CMesh>();
    if (timer.count(elapsed) >= TIME_SPRING) {
        timer.reset();
        CTransform* t = me->get<CTransform>();
        if (instancedMeshYes_h.isValid()) {
            CInstancedMesh* instancedMeshYes(instancedMeshYes_h);
            t->setScale(one_v);
            bool ok = instancedMeshYes->replaceInstanceWorld(instanceIndex, t->getWorld());
            assert(ok);
        } else {
            t->setScale(one_v);
        }
        return STATE_transformed;
    } else { 
        float time = timer.count(elapsed)/TIME_SPRING;
	    Entity* me(meEntity);
        CTransform* t = me->get<CTransform>();
        CInstancedMesh* instanced = nullptr;
        auto xz = xzScaSpring(time);
        XMVECTOR scale = XMVectorSet(xz, yScaSpring(time), xz, 0);
        if (instancedMeshYes_h.isValid()) {
            CInstancedMesh* instancedMeshYes(instancedMeshYes_h);
            t->setScale(scale);
            bool ok = instancedMeshYes->replaceInstanceWorld(instanceIndex, t->getWorld());
            assert(ok);
        } else {
            t->setScale(scale);
        }
        return STATE_spring;
    }
}

inline float yScaBreath(float t)
{
    return std::sin(t*2.5f+1.04f)*.023f+1;
}
inline float xzScaBreath(float t)
{
    return std::cos(t*2.0f+2.3f)*.02f+1;
}
fsmState_t TransformableFSMExecutor::breathe(float elapsed)
{
    float time = M_PI_2f*.45f*timer.count(elapsed);
	Entity* me(meEntity);
    CTransform* t = me->get<CTransform>();
    CInstancedMesh* instanced = nullptr;
    auto xz = xzScaBreath(time);
    XMVECTOR scale = XMVectorSet(xz, yScaBreath(time), xz, 0);
    if (instancedMeshYes_h.isValid()) {
        CInstancedMesh* instancedMeshYes(instancedMeshYes_h);
        t->setScale(scale);
        bool ok = instancedMeshYes->replaceInstanceWorld(instanceIndex, t->getWorld());
        assert(ok);
    } else {
        t->setScale(scale);
    }
	return STATE_breathe;
}

fsmState_t TransformableFSMExecutor::breatheXZ(float elapsed)
{
    float time = M_PI_2f*2.65f*timer.count(elapsed);
	Entity* me(meEntity);
    CTransform* t = me->get<CTransform>();
    CInstancedMesh* instanced = nullptr;
    auto xz = 1.55f*xzScaBreath(time)-0.2f;
    XMVECTOR scale = XMVectorSet(xz, 1, xz, 1);
    if (instancedMeshYes_h.isValid()) {
        CInstancedMesh* instancedMeshYes(instancedMeshYes_h);
        t->setScale(scale);
        bool ok = instancedMeshYes->replaceInstanceWorld(instanceIndex, t->getWorld());
        assert(ok);
    } else {
        t->setScale(scale);
    }
	return STATE_breatheXZ;
}

void TransformableFSMExecutor::setupTransforming()
{
    Entity* me = meEntity;
    CTransform* t = me->get<CTransform>();
    if (instancedMeshNot_h.isValid()) {
        CInstancedMesh* instancedMeshNot(instancedMeshNot_h);
        bool ok = instancedMeshNot->removeInstance(instanceIndex);
        assert(ok);
        CMesh* mesh = me->get<CMesh>();
        mesh->setVisible(true);
    }
	char cstr[32] = "Prop_transform";
	int randomV = rand_uniform(9, 1);
	char randomC[32] = "";
	sprintf(randomC, "%d", randomV);
	strcat(cstr, randomC);

	fmodUser::fmodUserClass::play3DSingleSound(cstr, t->getPosition());

    applySelfIllumination(0,1,true, true);
    applyDiffuseAsSelfIllumination(TRANSFORMED_DIFFUSE_SELFILLUMINATION, true, true);
}

void TransformableFSMExecutor::setupTransformed()
{
	Entity* me(meEntity);
    if (instancedMeshYes_h.isValid()) {
        CTransform* t = me->get<CTransform>();
        CInstancedMesh* instancedMeshYes(instancedMeshYes_h);
        instanceIndex = instancedMeshYes->addInstance(t->getWorld());
        CMesh* mesh = me->get<CMesh>();
        mesh->setVisible(false);
    }
    if (me->has<CStaticBody>()) {
        CStaticBody* shape = me->get<CStaticBody>();
        shape->removeFilters(filter_t::NONE, filter_t::NONE, filter_t::BULLET);
    }
    applyDiffuseAsSelfIllumination(TRANSFORMED_DIFFUSE_SELFILLUMINATION, true);
}

fsmState_t TransformableFSMExecutor::transformed(float elapsed)
{
    switch (type) {
	    case TRANSFORMABLE_MESH: return STATE_breathe; break;
        case TRANSFORMABLE_LIANA: return STATE_breatheXZ; break;
        case TRANSFORMABLE_HAMMER: return STATE_hammerTransformed; break;
        default: return STATE_transformed;
    }
}

void TransformableFSMExecutor::revive(const MsgRevive&)
{
	Entity* me = meEntity;
    CTransformable* transformable = me->get<CTransformable>();
    bool notTransformed = transformable->isNotTransformed();
    if (notTransformed) {
        applySelfIllumination(0,0,false);
        return;
    }

    bool isTransforming = transformable->isTransforming();

	switch (type) {
	    case TRANSFORMABLE_LIANA:
	    case TRANSFORMABLE_MESH: {
		        me->get<CMesh>().destroy();
		        CMesh::load(originalresourceName, me, DEFAULT_TRANSF_MESH);
		        CMesh* cmesh = me->get<CMesh>();
                cmesh->init();

                CTransform* transf = me->get<CTransform>();
                CRestore* restore = me->get<CRestore>();
                if (restore == nullptr) {
                    transf->setRotation(one_q);
                    transf->setScale(one_v);
                } else {
                    transf->set(*restore);
                }

                //Undo instancing
                CInstancedMesh* instancedMeshYes(instancedMeshYes_h);
		        CInstancedMesh* instancedMeshNot(instancedMeshNot_h);

		        if (instancedMeshYes != nullptr && instancedMeshNot != nullptr) {
                    auto w = isTransforming ? transf->getWorld() :
                        instancedMeshYes->getInstance(instanceIndex).world;
                    if(!isTransforming) {
                        instancedMeshYes->removeInstance(instanceIndex);
                    }
			        instanceIndexNot = instanceIndex = instancedMeshNot->addInstance(w);
                    cmesh->setVisible(false);
		        } else if (instancedMeshYes == nullptr && instancedMeshNot != nullptr) {
                    instanceIndexNot = instanceIndex =
                        instancedMeshNot->addInstance(transf->getWorld());
                    cmesh->setVisible(false);
                } else if (instancedMeshYes != nullptr && instancedMeshNot == nullptr) {
                    if(!isTransforming) {instancedMeshYes->removeInstance(instanceIndex);}
                    cmesh->setVisible(true);
                } else {
                    cmesh->setVisible(true);
                }
	        } break;
	    case TRANSFORMABLE_MATERIAL: {
			    CMesh* cmesh = me->get<CMesh>();
			    cmesh->setMaterial(Material::getManager().getByName(originalresourceName));
	        } break;
	    case TRANSFORMABLE_TINT: {
			    CTint* t = me->get<CTint>();
			    t->set(previousTint);
	        } break;
    }
}

void initPropTypes()
{
    TransformableFSM::initType();
    SUBSCRIBE_MSG_TO_MEMBER(CTransformable, MsgShot, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CTransformable, MsgTransform, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CThrowsPickups, MsgTransform, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CTrampoline, MsgTransform, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CCannon, MsgTransform, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CCannon, MsgFlyingMobileEnded, receive)
    SUBSCRIBE_MSG_TO_MEMBER(CCreep, MsgTransform, receive);

	SUBSCRIBE_MSG_TO_MEMBER(CTrampoline, MsgSetPlayer, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CCannon, MsgSetPlayer, receive);
	SUBSCRIBE_MSG_TO_MEMBER(CCreep, MsgSetPlayer, receive);

    SUBSCRIBE_MSG_TO_MEMBER(CTransformable, MsgRevive, revive);
    SUBSCRIBE_MSG_TO_MEMBER(CThrowsPickups, MsgRevive, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CTrampoline, MsgRevive, revive);
    SUBSCRIBE_MSG_TO_MEMBER(CCannon, MsgRevive, revive);
    SUBSCRIBE_MSG_TO_MEMBER(CCreep, MsgRevive, revive);
    SUBSCRIBE_MSG_TO_MEMBER(CDestructibleRestorer, MsgRevive, revive);
}

void CTransformable::receive(const MsgShot& shot)
{
	if (!isTransformed() && !inert) {
		fsm.getExecutor().damageDone = shot.mega ? 10 : 1;
	}
}

void CTransformable::init()
{
    auto& fsme(fsm.getExecutor());
	fsme.meEntity = Handle(this).getOwner();
	fsm.init();
    if (fsme.type == TRANSFORMABLE_HAMMER) {
        fsme.xtraMesh1 = PrefabManager::get().prefabricate("boss/raiz_01");
        fsme.xtraMesh2 = PrefabManager::get().prefabricate("boss/raiz_02");

        CTransform* t  = fsme.meEntity.getSon<CTransform>();
        CTransform* t1 = fsme.xtraMesh1.getSon<CTransform>();
        CTransform* t2 = fsme.xtraMesh2.getSon<CTransform>();

        t1->applyRotation(t->getRotation());
        t2->applyRotation(t->getRotation());
    }
}

void CTransformable::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (elem == "Transformable") {
	    auto& fsme = fsm.getExecutor();
	    inert = atts.getBool("inert", inert);
	    if (atts.getBool("transformed", fsme.initTransformed)) {
		    fsme.setTransformed();
		    fsm.reset();
	    }

	    setResourceName(atts.getString("resource", DEFAULT_TRANSF_MESH));
	    setOriginalResourceName(atts.getString("originalResource", DEFAULT_NONTRANSF_MESH));
	    std::string typeStr = atts["type"];
	    if (typeStr == "mesh") {
            fsme.type = TRANSFORMABLE_MESH;
        } else if (typeStr == "material") {
            fsme.type = TRANSFORMABLE_MATERIAL;
        } else if (typeStr == "tint") {
            fsme.type = TRANSFORMABLE_TINT;
            bool b = atts.has("tint");
            fsme.targetTint = atts.getHex("tint", fsme.targetTint);
        } else if (typeStr == "creep") {
            fsme.type = TRANSFORMABLE_CREEP;
        } else if (typeStr == "liana") {
            fsme.type = TRANSFORMABLE_LIANA;
        } else if (typeStr == "hammer") {
            fsme.type = TRANSFORMABLE_HAMMER;
        }
    } else if (elem == "glow") {
	    auto& fsme = fsm.getExecutor();
        fsme.glowColor.loadFromProperties(elem, atts);
        fsme.glowMax = atts.getFloat("glowMax", fsme.glowMax);
    } else if (elem == "marked") {
	    auto& fsme = fsm.getExecutor();
        fsme.glowMarkedColor.loadFromProperties(elem, atts);
        fsme.glowMarkedMax = atts.getFloat("glowMax", fsme.glowMax);
    }
}
void CTrampoline::receive(const MsgTransform&)
{
	Entity* me = Handle(this).getOwner();
	if (playerEntity.isValid()) {
	    CTransform* meT = me->get<CTransform>();
        Entity* player = playerEntity;
	    CTransform* playerT = player->get<CTransform>();
	    if (testDistanceSqEu(meT->getPosition(), playerT->getPosition(), 1.5f)){
		    player->sendMsg(MsgPlayerTrampoline(me));
	    }	
    }
	CStaticBody* s = Handle(this).getBrother<CStaticBody>();
	s->setFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);

	particles::CEmitter *emitter = me->get<particles::CEmitter>();
	auto key = emitter->getKey("emitter_0");
	particles::ParticleUpdaterManager::get().setDeleteSelf(key);
}

void CTrampoline::receive(const MsgSetPlayer& msg)
{
	playerEntity = msg.playerEntity;
}

void CCannon::receive(const MsgSetPlayer& msg)
{
	playerEntity = msg.playerEntity;
}

void CCreep::receive(const MsgSetPlayer& msg)
{
	playerEntity = msg.playerEntity;
}

void CCannon::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    fovV = atts.getFloat("fovV", fovV);
    fovH = atts.getFloat("fovH", fovH);
    impulse = atts.getFloat("impulse", impulse);
    rotQ = atts.getQuat("rot", rotQ);
    lookAt = atts.getPoint("lookAt", lookAt);
    bossCannon = atts.getBool("bossCannon", bossCannon);
}

void CCannon::receive(const MsgTransform&)
{
    Entity* me = Handle(this).getOwner();
	CTransform* meT = me->get<CTransform>();
    if (playerEntity.isValid()) {
	    Entity* player = playerEntity;
	    CTransform* playerT = player->get<CTransform>();
	    if (testDistanceSqEu(meT->getPosition(), playerT->getPosition(), 1.5f)){
		    player->sendMsg(MsgPlayerCannnon(me));
	    }
    }
    if (me->has<CStaticBody>()) {
        CStaticBody* s = me->get<CStaticBody>();
        s->setFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);
    }
    if (me->has<CRigidBody>()) {
        CRigidBody* s = me->get<CRigidBody>();
        s->setFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);
    }
}

void CCannon::receive(const MsgFlyingMobileEnded&)
{
    Entity* me = Handle(this).getOwner();
    CTransform* transform = me->get<CTransform>();
    CTransformable* transformable = me->get<CTransformable>();
    transformable->setInert(false);
    if (me->has<CRigidBody>()) {
        auto rigid_h = me->get<CRigidBody>();
        CRigidBody* rigid = rigid_h;
        CStaticBody* body = getManager<CStaticBody>()->createObj();
        auto shape = rigid->getShape();
        auto actor = rigid->getRigidBody();
        actor->detachShape(*shape, false);
        me->add(body);
        body->setShapeDesc(rigid->getShapeDesc());
        body->setShape(shape);
        body->setFilters(rigid->getFilter());
        body->removeFilters(filter_t::SCENE);
        body->setFilters(filter_t::TOOL, filter_t::NONE, filter_t::BULLET);
        body->init();
        if (me->has<CTrigger>()) {
            CTrigger* trigger = me->get<CTrigger>();
            auto shape = trigger->getShape();
            actor->detachShape(*shape, false);
            trigger->setupTrigger();
        }
        rigid_h.destroy();
    } else if (me->has<CStaticBody>()) {
        CStaticBody* body = me->get<CStaticBody>();
        body->removeFilters(filter_t::SCENE);
        body->setFilters(filter_t::TOOL, filter_t::NONE, filter_t::BULLET);
    }
    if (me->has<CTrigger>()) {
        CTrigger* trigger = me->get<CTrigger>();
        trigger->removeFilters(filter_t::SCENE);
        trigger->setFilters(filter_t::TOOL, filter_t::NONE);
    }
    lookAtTarget();
}

void CCannon::lookAtTarget()
{
    CTransform* meT = Handle(this).getBrother<CTransform>();
    component::Transform t = *meT;
    t.lookAt(lookAt);
    t.applyRotation(XMQuaternionRotationAxis(t.getLeft(), deg2rad(90)));
    rotQ = t.getRotation();
}

void CCreep::receive(const MsgTransform&)
{
	CStaticBody* s = Handle(this).getBrother<CStaticBody>();
    s->setFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);
}


void CTrampoline::revive(const MsgRevive&)
{
	Entity* me = Handle(this).getOwner();
	CStaticBody* s = me->get<CStaticBody>();
	s->removeFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);

	particles::CEmitter *emitter = me->get<particles::CEmitter>();
	auto k = emitter->getKey("emitter_0");
    emitter->load("emitter_0", k);
}

void CCannon::revive(const MsgRevive&)
{
    Entity* me = Handle(this).getOwner();

    if (me->has<CStaticBody>()) {
        CStaticBody* s = me->get<CStaticBody>();
        s->removeFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);
    }
    if (me->has<CRigidBody>()) {
        CRigidBody* s = me->get<CRigidBody>();
        s->removeFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);
    }
    if(lookAt != zero_v) {
        lookAtTarget();
    }
}

void CCreep::revive(const MsgRevive&)
{
	CStaticBody* s = Handle(this).getBrother<CStaticBody>();
    s->removeFilters(filter_t::NONE, filter_t::PLAYER, filter_t::NONE);
    generatePosterPlane();
}

void CCreep::generatePosterPlane()
{
    Entity* me = Handle(this).getOwner();
    
    // Generate the plane for the mesh
	Mesh* mesh = new Mesh;
    const float sX = posterSize/2;
    const float sY = posterSize/2;
	bool isOk = createPlanePUNTWithUV(*mesh,
        posterOffset.x+sX, posterOffset.y+sY,
        posterOffset.x-sX, posterOffset.y-sY,
        -0.089f);
	assert(isOk &&"Failed to create plane");

    CMesh* cmesh = me->get<CMesh>();
    cmesh->setMesh(mesh);
    CMesh::key_t key;
    key.group0 = key.groupf =0;
    key.material=Material::getManager().getByName("creep_no_transform");
    cmesh->addKey(key);
    RenderManager::addKeys(me);
    CAABB* aabb = me->get<CAABB>();
    *aabb = AABB();
    aabb->init();
    CCullingAABB* cullingAABB = me->get<CCullingAABB>();
    if (cullingAABB != nullptr) {
        #ifdef _DEBUG
            cullingAABB->dbgColor = ColorHSL(rand_uniform(1.f));
        #endif
        cullingAABB->setDirty();
    }
}

void CCreep::generateCreepPlane()
{
    static const framedTileMap tilemap = framedTileMap::load("data/creeperTileMap.xml");
    
    Entity* me = Handle(this).getOwner();
	Mesh* mesh = new Mesh;
	bool isOk = createFramedPlanePUNT(*mesh, tilemap,
        width/2, height/2, width/2, height/2, -0.089f);
	assert(isOk &&"Failed to create plane");

    CMesh* cmesh = me->get<CMesh>();
    cmesh->setMesh(mesh);
    CMesh::key_t key;
    key.group0 = key.groupf =0;
    key.material=Material::getManager().getByName("creep_transform");
    cmesh->addKey(key);
    RenderManager::addKeys(me);
    CAABB* aabb = me->get<CAABB>();
    *aabb = AABB();
    aabb->init();
    CCullingAABB* cullingAABB = me->get<CCullingAABB>();
    if (cullingAABB != nullptr) {
        #ifdef _DEBUG
            cullingAABB->dbgColor = ColorHSL(rand_uniform(1.f));
        #endif
        cullingAABB->setDirty();
    }
}


#define TIME_TRANSF_HAMMER 2.5f
#define TRANSFORM_ROTATE_HAMMER deg2rad(360)*4

fsmState_t TransformableFSMExecutor::hammerTransform(float elapsed)
{
	if (timer.count(elapsed) >= TIME_TRANSF_HAMMER) {
		timer.reset();
        Entity* r1 = xtraMesh1;
        Entity* r2 = xtraMesh2;
        CTransform* t1 = r1->get<CTransform>();
        CTransform* t2 = r2->get<CTransform>();
        CRestore* t1_or = r1->get<CRestore>();
        CRestore* t2_or = r2->get<CRestore>();
        t1->set(*t1_or);
        t2->set(*t2_or);
        
        CTransform* t = meEntity.getSon<CTransform>();
        savedPosition = t->getPosition();

		return STATE_transformed;
	} else {
        Entity* r1 = xtraMesh1;
        Entity* r2 = xtraMesh2;

        CTransform* t1 = r1->get<CTransform>();
        CTransform* t2 = r2->get<CTransform>();
        CRestore* t1_or = r1->get<CRestore>();
        CRestore* t2_or = r2->get<CRestore>();

        float f = timer.get() / TIME_TRANSF_HAMMER;

        float finv = 1-f;
        float sca_xz = std::pow(1-finv*finv, 1.f/6.f);
		float sca_y_1 = std::pow(1-sinc(std::pow(f, 0.75f)*M_PI_2f*24), 1.95f);
		float sca_y_2 = std::pow(1-sinc(std::pow(f, 0.85f)*M_PI_2f*26), 2.15f);
        float tint = 1-std::pow(1-finv*finv, 1.f/1.35f);

		t1->setScale(t1_or->getScale() * XMVectorSet(sca_xz, sca_y_1, sca_xz, 0));
		t2->setScale(t2_or->getScale() * XMVectorSet(sca_xz, sca_y_2, sca_xz, 0));
		float rot_1(std::pow(std::sin(M_PI_2f*f), 1/9.253f)*TRANSFORM_ROTATE_HAMMER);
		float rot_2(std::pow(std::sin(M_PI_2f*f), 1/9.257f)*TRANSFORM_ROTATE_HAMMER);
		//t1->setRotation(XMQuaternionMultiply( t1_or->getRotation(),
        //    XMQuaternionRotationAxis(t1_or->getUp(), rot_1)));
		//t2->setRotation(XMQuaternionMultiply( t2_or->getRotation(),
        //    XMQuaternionRotationAxis(t2_or->getUp(), rot_2)));

        CTint* tint1 = r1->get<CTint>();
        CTint* tint2 = r2->get<CTint>();
        //When there¡s a texture...
        //tint1->setAf(tint);
        //tint2->setAf(tint);

        tint1->set(Color::BROWN.factor(tint) + Color::MOUNTAIN_GREEN.factor(1-tint));
        tint2->set(Color::BROWN.factor(tint) + Color::MOUNTAIN_GREEN.factor(1-tint));

        RenderManager::updateKeys(r1);
        RenderManager::updateKeys(r2);

		return STATE_hammerTransform;
	}
}

behavior::fsmState_t TransformableFSMExecutor::hammerTransformed(float elapsed)
{
    Entity* me = meEntity;
    Entity* r1 = xtraMesh1;
    Entity* r2 = xtraMesh2;

    CTransform* t  = me->get<CTransform>();
    CTransform* t1 = r1->get<CTransform>();
    CTransform* t2 = r2->get<CTransform>();


    //We want to maintain the distance between AABB tops constant
    //Stretch the roots to grab the hammer at its current position

    // Problem data... {
    float meY0 = XMVectorGetY(savedPosition);
    float meYt = XMVectorGetY(t->getPosition());
    float r1Y = XMVectorGetY(t1->getPosition());
    float r2Y = XMVectorGetY(t2->getPosition());

    CAABB* meAABB = me->get<CAABB>();
    float meAABBtop_rel = XMVectorGetY(meAABB->getHSize());
    float meAABBtop0 = meAABBtop_rel + meY0;
    float meAABBtopt = meAABBtop_rel + meYt;
    
    CAABB* r1AABB = r1->get<CAABB>();
    float r1AABBtop_rel = XMVectorGetY(r1AABB->getHSize());

    CAABB* r2AABB = r2->get<CAABB>();
    float r2AABBtop_rel = XMVectorGetY(r2AABB->getHSize());
    // }

    //Distances between AABB tops
    float k1 = meAABBtop0 - r1AABBtop_rel;
    float k2 = meAABBtop0 - r2AABBtop_rel;

    float ht_1 = meAABBtopt - k1;
    float ht_2 = meAABBtopt - k2;

    float scaY_1 = ht_1 / r1AABBtop_rel;
    float scaY_2 = ht_2 / r2AABBtop_rel;
    t1->setScale(XMVectorSet(1,scaY_1,1,1));
    t2->setScale(XMVectorSet(1,scaY_2,1,1));

    //if (scaY_1 != 1) {dbg("%.3f && %.3f \n", scaY_1, scaY_2);} // about 3
    
    t1->setPosition(XMVectorSetY(t->getPosition(), r1Y));
    t2->setPosition(XMVectorSetY(t->getPosition(), r2Y));

    return STATE_hammerTransformed;
}

}
