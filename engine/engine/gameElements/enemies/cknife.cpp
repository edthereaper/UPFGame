#include "mcv_platform.h"
#include "cknife.h"

#include "animation/cskeleton.h"
#include "animation/skeleton_manager.h"
using namespace animation;

#include "handles/handle.h"
#include "components/transform.h"
using namespace component;

#include "melee.h"

using namespace DirectX;
using namespace physX_user;
using namespace utils;

namespace gameElements {

void CKnife::update(float elapsed)
{
	Entity* e = Handle(this).getOwner();
	CSkeleton * skeleton = e->get<CSkeleton>();
    auto model = skeleton->getModel();
    if (model != nullptr) {
        CTransform* t = e->get<CTransform>();
        auto pxt = shape->getActor()->getGlobalPose();
        auto actorTi = toXMMATRIX(pxt.getInverse());
        CalBone *bone = model->getSkeleton()->getBone(idBone);
        auto diff = zero_v;toXMVECTOR(pxt.p) - t->getPosition();
        auto pos = XMVector3Transform(toXMVECTOR(bone->getTranslationAbsolute()) + diff,actorTi);
        auto rot = XMQuaternionMultiply(toXMQuaternion(bone->getRotationAbsolute()), t->getRotation());
        shape->setLocalPose(PxTransform(toPxVec3(pos), toPxQuat(rot)));
    }
}

void CKnife::init()
{
    if(shape == nullptr) {Shape::createShape();}
    assert(shape != nullptr);
    Entity* e(Handle(this).getOwner());
    
    #ifdef _DEBUG        
        std::stringstream ss;
        PHYSX_SET_NAME(shape, e->getName()+".knifeTrigger");
    #endif
    if(e->has<CStaticBody>()) {
        CStaticBody* body = e->get<CStaticBody>();
        body->init();
        body->getStaticBody()->attachShape(*shape);
    } else if(e->has<CRigidBody>()) {
        CRigidBody* body = e->get<CRigidBody>();
        body->init();
        body->getRigidBody()->attachShape(*shape);
    } else if(e->has<CCharacterController>()) {
        CCharacterController* body = e->get<CCharacterController>();
        body->init();
        body->getCharacterController()->getActor()->attachShape(*shape);
    } else {
        dbg("No actor found\n");
    }
	shape->userData = Handle(this).getOwner().getRawAsVoidPtr();
    update(0.f);
}

void CKnife::setBone(std::string boneName)
{
	Entity* e = Handle(this).getOwner();
	CSkeleton * skeleton = e->get<CSkeleton>();
    idBone = skeleton->getModel()->getCoreModel()->getCoreSkeleton()->getCoreBoneId(boneName);
}

void CKnife::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (elem == "Knife") {
        setBone(atts["bone"]);
    } else if (elem == "shape") {
        Shape::loadFromProperties(elem, atts);    
    }
}

void CKnife::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CKnife, MsgCollisionEvent, receive);
}

void CKnife::receive(const MsgCollisionEvent& msg)
{
	component::Entity* e(msg.entity);
    CMelee* melee = Handle(this).getBrother<CMelee>();
	if (melee->getDamage() > 0) {
        e->sendMsg(MsgMeleeHit(melee->getDamage()));
    }
}

}
