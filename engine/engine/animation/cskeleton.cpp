#include "mcv_platform.h"
#include "animation/cskeleton.h"
#include "skeleton_manager.h"
#include "handles/handle.h"
#include "components/transform.h"
#include "font/font.h"
#include "render/renderManager.h"

#include "data/fx/platform/shader_ctes.h"

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

using namespace DirectX;
using namespace component;

//Externs
namespace render {
#if defined(_DEBUG) && defined(RENDER_DEBUG_MESHES)
    extern Mesh axis;
#endif
}

namespace animation {

CSkeleton::~CSkeleton() {SAFE_DELETE(model);}

#if defined(_DEBUG) && defined(RENDER_DEBUG_MESHES)
void CSkeleton::renderBoneAxis(int bone_id, float scale) const
{
    auto bones = model->getSkeleton()->getVectorBone();
    CalBone* bone = bones[bone_id];
    CTransform transform;
    transform.setRotation(toXMQuaternion(bone->getRotationAbsolute()));
    transform.setPosition(toXMVECTOR(bone->getTranslationAbsolute()));
    XMMATRIX world = transform.getWorld();
    world = XMMatrixScaling(scale, scale, scale) * world;
    setObjectConstants(world);
    render::axis.activateAndRender();
}

void CSkeleton::renderDebug3D() const
{
    Technique::getManager().getByName("basic")->activate();
    CoreModel *core = (CoreModel*) model->getCoreModel();
    for (auto bc : core->bone_ids_to_debug) {
        renderBoneAxis(bc, 2.5f);
    }
    
    // Render the bone lines
    auto bones = model->getSkeleton()->getVectorBone();
    for (auto it : bones) {
        auto bone_pos = it->getTranslationAbsolute();
        int parentId = it->getCoreBone()->getParentId();
        // If I have a parent
        if (parentId != -1) {
            CalBone* pParent = bones[parentId];
            auto parent_pos = pParent->getTranslationAbsolute();
        
            XMVECTOR src = DirectX::XMVectorSet(bone_pos.x, bone_pos.y, bone_pos.z, 1);
            XMVECTOR dst = DirectX::XMVectorSet(parent_pos.x, parent_pos.y, parent_pos.z, 1);
            drawLine(src, dst);
          }
    }
}
#endif

void CSkeleton::addBonesToBuffer()
{    
    CalSkeleton* skel = model->getSkeleton();
    
    bone0 = unsigned(getCurrentBoneIndex());
    auto& cal_bones = skel->getVectorBone();
    for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
        CalBone* bone = cal_bones[bone_idx];
        
        const CalMatrix& m = bone->getTransformMatrix();
        const CalVector  p = bone->getTranslationBoneSpace();
    
        //Build a transformation matrix
        addBoneToBuffer( XMMatrixSet(
            m.dxdx, m.dydx, m.dzdx, 0.f,
            m.dxdy, m.dydy, m.dzdy, 0.f,
            m.dxdz, m.dydz, m.dzdz, 0.f,
            p.x,    p.y,    p.z   , 1.f
            ));
    }
}

void CSkeleton::load(std::string name)
{
    CoreModel* core_model = CoreModel::getManager().getByName(name.c_str());
    model = new CalModel(core_model);
    model->getMixer()->blendCycle(0, 1, 0);
}

void CSkeleton::loadFromProperties(const std::string& elem, MKeyValue &atts)
{
    load(atts["name"]);
    if (atts.has("initAnim")) {
        auto id = model->getCoreModel()->getCoreAnimationId(atts.getString("initAnim"));
        if (id>=0) {
            model->getMixer()->blendCycle(id,1,0);
            if (atts.has("initAnimFactor")) {
                auto f = atts.getFloat("initAnimFactor", 1.f);
            
                auto coreAnimation = model->getCoreModel()->getCoreAnimation(id);
                for(auto& anim : model->getMixer()->getAnimationCycle()) {
                        if (anim->getCoreAnimation() == coreAnimation) {
    		            anim->setAsync(0, 0);
    		            anim->setTimeFactor(f);
                    }
                }
            }
        }
    }
}

void CSkeleton::stopAnimations()
{
	for (int i = 1; i != 5; i++) {
		model->getMixer()->removeAction(i, 0.0f);
	}
}

float CSkeleton::globalFactor = 1.0f;

void CSkeleton::bakeCullingMask()
{
    CCullingAABB* aabb = Handle(this).getBrother<CCullingAABB>();
    for (const auto& culler : CCulling::iterateCullers()) {
        if (culler.cull(*aabb)) { 
            cullingMask |= culler.getMask();
        }
    }
}

void CSkeleton::update(float elapsed)
{
	cullingMask.reset();
	if (model != nullptr && isSpatiallyGood()) {
		bakeCullingMask();
		//Even if the mask is 0, we still calculate the animation, because
		//dummy bones might need to be calculated (thus changing the transform of the entity)

		CTransform* t = Handle(this).getBrother<CTransform>();
		CCharacterController* charControl = Handle(this).getBrother<CCharacterController>();
		CAnimationPlugger* aniP = Handle(this).getBrother<CAnimationPlugger>();

		model->getMixer()->setRootTranslation(toCalVector(t->getPosition()));
		model->getMixer()->setRootRotation(toCalQuaternion(t->getRotation()));

		mixingTimeCreep = false;

		if (aniP != nullptr && prevId != aniP->getActualPlug().plugId){
			prevId = aniP->getActualPlug().plugId;
			if (aniP->getActualPlug().creepControl || aniP->getPreviousPlug().creepControl){
				timerCreep.reset();
				canMixing = true;
			}
		}
		if (aniP != nullptr && timerCreep.count(elapsed) < aniP->getActualPlug().delay && canMixing){
			mixingTimeCreep = true;
		} else{
			canMixing = false;
		}

		/*if (aniP != nullptr && aniP->getActualPlug().footControl){
			CalBone* bone = model->getSkeleton()->getBone(model->getCoreModel()->getBoneId("Bip001 R Foot"));
			CalBone* bone2 = model->getSkeleton()->getBone(model->getCoreModel()->getBoneId("Bip001 L Foot"));
			float ddd1 = 0.f;
			float ddd2 = 0.f;
			XMVECTOR origin = toXMVECTOR(bone->getTranslationAbsolute()) - yAxis_v * ddd;
			XMVECTOR dir = yAxis_v;
			PxReal distance = 1.0f;
			PxRaycastBuffer hit;
		if (PhysicsManager::get().raycast(origin, dir, distance, hit,
			filter_t(
				filter_t::NONE,
				filter_t::id_t(filter_t::PLAYER | filter_t::BULLET | filter_t::CANNONPATH | filter_t::KNIFE | filter_t::FLARESHOT | filter_t::PICKUP),
				filter_t::id_t(filter_t::SCENE)))) {
			ddd1 = hit.block.distance;
		}
		origin = toXMVECTOR(bone2->getTranslationAbsolute()) - yAxis_v * ddd;
		if (PhysicsManager::get().raycast(origin, dir, distance, hit,
			filter_t(
				filter_t::NONE,
				filter_t::id_t(filter_t::PLAYER | filter_t::BULLET | filter_t::CANNONPATH | filter_t::KNIFE | filter_t::FLARESHOT | filter_t::PICKUP),
				filter_t::id_t(filter_t::SCENE)))){
			ddd2 = hit.block.distance;
		}
		if (ddd1 > ddd2)				ddd = ddd1 + 0.3f;
		else							ddd = ddd2 + 0.3f;
		if (ddd < 0.55f)				ddd = 0.55f;
		ddd = 0.55f;
		model->getMixer()->setRootPivot(toCalVector(XMVectorSet(0, ddd, 0, 1)));
		}*/

		if (aniP != nullptr && (aniP->getActualPlug().followDummyPos || mixingTimeCreep)){
			if (aniP->getActualPlug().creepControl || mixingTimeCreep){
				model->getMixer()->setRootPivot(toCalVector(t->getUp() * 0.5f - t->getFront() * 0.1f));
			}
			else{
				model->getMixer()->setRootPivot(toCalVector(t->getUp() * 0.55f - t->getFront() * 0.15f));
			}
			model->getMixer()->setDummyTime(true);
		} else{
			model->getMixer()->setRootPivot(toCalVector(zero_v));
			model->getMixer()->setDummyTime(false);
		}

		if(aniP != nullptr){
			if(aniP->getActualPlug().type == AnimationArchetype::ACTION){
				model->getMixer()->setisCycle(false);
			}
			if(aniP->getActualPlug().type == AnimationArchetype::MAIN_CYCLE || aniP->getActualPlug().fakeCycle){
				model->getMixer()->setisCycle(true);
			}
		}

		model->getMixer()->setTimeFactor(animSpeed);
		model->update(elapsed);

		//Follow Animation Translation
		CalVector deltaLogicTranslation = model->getMixer()->getAndClearDummyLogicTranslation();
		//CalVector deltaLogicTranslation = model->getMixer()->getAndClearLogicTranslation();
		XMVECTOR distBip = toXMVECTOR(deltaLogicTranslation);
		//float distDif = sqrt(XMVectorGetX(distDummy)*XMVectorGetX(distDummy) + XMVectorGetY(distDummy)*XMVectorGetY(distDummy) + XMVectorGetZ(distDummy)*XMVectorGetZ(distDummy));
		float distDif = sqrt(XMVectorGetX(distBip)*XMVectorGetX(distBip) + XMVectorGetY(distBip)*XMVectorGetY(distBip) + XMVectorGetZ(distBip)*XMVectorGetZ(distBip));
		
		/*if(aniP->getAnimationElapsed() > 0.0f && distDif > 0.0f && (aniP->getActualPlug().followDummyPos || mixingTimeCreep)){	
			dbg(" %f / %f , %f\n", aniP->getAnimationElapsed(), aniP->getActualPlugDuration(), distDif);
			dbg("distBip: %f %f %f\n", XMVectorGetX(distBip), XMVectorGetY(distBip), XMVectorGetZ(distBip));
		}*/
		
		if (aniP != nullptr && aniP->getAnimationElapsed() > 0.0f && charControl != nullptr && distDif < aniP->getActualPlug().limitDummyCycle 
			&& distDif > 0.0f && !mixingTimeCreep && aniP->getActualPlug().followDummyPos){
			charControl->setDisplacement(distBip);
		}
	}
}

}