#include "mcv_platform.h"
#include "liana.h"

#ifdef _DEBUG
#include <iomanip>
#endif
#ifdef DEBUG_LIANA
#include "player/playerMov.h"
#endif

#include "props.h"

using namespace physX_user;

//#define TEST_FREEFALL //Liana spawns straight in the -z direction

#include "render/mesh/CInstancedMesh.h"
#include "render/render_utils.h"
using namespace render;

namespace gameElements {
    
const float CLiana::LINK_MASS = 0.0001f;
const float CLiana::LINK_MASS_LOOSE = LINK_MASS*0.01f;
const float CLiana::FIXED_LINEAR_DAMP = 1000.f;
const float CLiana::LINEAR_DAMP = 2.0f;
const float CLiana::ANGULAR_DAMP = 0.5f;
const float CLiana::LINK_LENGTH = 1.25f;
const float CLiana::LOCAL_OFFSET = 0.1f;
const float CLiana::SPRING_STIFF = 1e4f;
const float CLiana::SPRING_DAMP = 1e2f;
const float CLiana::SPRING_STIFF_LOOSE = 1e2f;
const float CLiana::SPRING_DAMP_LOOSE = 1e1f;

CLiana::CLiana()
{
    for(unsigned i=0;i<MAX_LINKS;i++) {
        links[i] = Shape();
        links[i].setShapeDesc(Shape::shape_t());
        linkTriggers[i] = Shape(true);
        linkTriggers[i].setShapeDesc(Shape::shape_t());
        linkBodies[i] = nullptr;
        joints[i] = nullptr;
    }
}

void CLiana::update(float elapsed)
{
    Entity* me(Handle(this).getOwner());
    CTransform* t = me->get<CTransform>();
    auto mePos = t->getPosition();
    CInstancedMesh* iMesh = me->get<CInstancedMesh>();
    CTransformable* transformable = me->get<CTransformable>();
    bool transformed = transformable->isTransformed();
    auto world = linkBodies[nLinks/2]->getGlobalPose();
    auto jointWorld = joints[nLinks/2]->getLocalPose(PxJointActorIndex::eACTOR1);
    auto center = toXMMATRIX(world * jointWorld);
    transformable->setCenterAim(center.r[3]);
    for (unsigned i=0; i<nLinks; i++) {
        auto world = linkBodies[i]->getGlobalPose();
        
        auto normal = XMVector3Normalize(mePos - toXMVECTOR(world.p));
        auto angleY = angleBetweenVectors(normal, yAxis_v);
        angleY = inRange(0.f,((angleY*0.9f)*M_PI_2f)/deg2rad(5), M_PI_2f);
        const auto& limit = getLimit(normal);
        const auto f = std::abs(cos(angleY));
        float lDamp = 
            jointType == D6 ? (
                controlLink < 0 ? (f*0.15f+1)*LINEAR_DAMP :
                i > unsigned(controlLink) ? f*LINEAR_DAMP*0.05f :
                    LINEAR_DAMP+f*LINEAR_DAMP*0.15f
                ) : FIXED_LINEAR_DAMP;
        linkBodies[i]->setLinearDamping(lDamp);

        #if defined(DEBUG_LIANA_FORCES)
            //Please do not remove, even if the liana rendering is changed (skeletal, etc)
            //allow both modes through an #if directive
            //This is useful for debugging
            auto aF = linkForces[i].magnitude()/
                (elapsed*LINK_MASS*nLinks*PlayerMovBtExecutor::LIANA_MAX_IMPULSE);
            auto aB = linkBrakes[i].magnitude()/
                (elapsed*LINK_MASS*nLinks*PlayerMovBtExecutor::LIANA_BRAKE);
            aB = sin(aB * M_PI_2f);
            aF = sin(aF * M_PI_2f);
            auto siF = Color(Color::ORANGE).setAf(aF);
            auto siB = Color(Color::STEEL_BLUE).setAf(aB);
            Color selfIll = siF + siB;
            linkForces[i] = PxVec3(0);
            linkBrakes[i] = PxVec3(0);
        #elif defined(DEBUG_LIANA_DAMPING)
            Color selfIll = 
                lDamp >= FIXED_LINEAR_DAMP ? Color::RED :
                lDamp > LINEAR_DAMP*1.25f? Color(Color::CERULEAN).setAf((lDamp-LINEAR_DAMP*2.f)/LINEAR_DAMP) :
                lDamp > LINEAR_DAMP      ? Color(Color::SPRING_GREEN).setAf((lDamp-LINEAR_DAMP)/LINEAR_DAMP) :
                                           Color(Color::BURGUNDY).setAf(lDamp/LINEAR_DAMP);
        #endif
        auto jointWorld = joints[i]->getLocalPose(PxJointActorIndex::eACTOR1);
        jointWorld.q = PxQuat(PxIdentity);
        //jointWorld.p += PxVec3(0, -LINK_LENGTH*.5f, 0);

        if (iMesh != nullptr) {
            iMesh->setInstance(i, CInstancedMesh::instance_t(
                XMMatrixScalingFromVector(t->getScale()) * toXMMATRIX(world * jointWorld)
                #if defined(DEBUG_LIANA_CONTROL_LINK)
                    , i == controlLink ? Color::RED : Color::WHITE
                #else
                    , transformed ? Color(0) : Color(0x999966FF)
                #endif
                #if defined(DEBUG_LIANA_FORCES) || defined(DEBUG_LIANA_DAMPING)
                    , selfIll
                #endif
                ));
        }
    }
}

float CLiana::getShapeLength() const
{
    assert(linkShapeDesc.type == Shape::shape_t::CAPSULE);
    return linkShapeDesc.scale*(linkShapeDesc.capsule.height + linkShapeDesc.capsule.radius*2)
         + linkShapeDesc.skin*2.f;
}

PxShape* CLiana::createLinkCollider(Shape& link, Handle me_h)
{
    link.setFilters(linkFilter);
    link.setShapeDesc(linkShapeDesc);
    link.createShape();
    link.setUserData(me_h);
    return link.getShape();
}
    
PxShape* CLiana::createLinkTrigger(Shape& link, Handle me_h)
{
    link.setFilters(linkTriggerFilter);
    link.setShapeDesc(linkTriggerShapeDesc);
    link.createShape();
    link.setUserData(me_h);
    return link.getShape();
}

const PxQuat CLiana::FRAME_ROT = PxQuat(-M_PI_2f, PxVec3(0,0,1));
const PxVec3 CLiana::FRAME_SEP = PxVec3(0, -LINK_LENGTH, 0);

void CLiana::createD6Joints()
{
    Entity* me = Handle(this).getOwner();
    CTransform* transform = me->get<CTransform>();
    PxVec3 pos = toPxVec3(transform->getPosition());
    PxQuat rot = toPxQuat(transform->getRotation());

    PxQuat prevFrameRot(rot*FRAME_ROT);
    PxVec3 localFrameOff(0, getShapeLength()/2 + LOCAL_OFFSET, 0);

    PxTransform prevLocalFrame(PxTransform(pos, prevFrameRot));
    PxTransform nextLocalFrame(PxTransform(localFrameOff+FRAME_SEP, FRAME_ROT));
    PxTransform currLocalFrame(PxTransform(localFrameOff, FRAME_ROT));
    #ifdef _DEBUG
        std::string name = me->getName();
    #endif

    PxRigidDynamic* previousLink = nullptr;
    
    for(unsigned i=0;i<nLinks;i++) {
        auto& joint(joints[i]);
        if (joint != nullptr) {joint->release(); joint=nullptr;}
        auto& linkBody(linkBodies[i]);

        PxD6Joint* d6Joint = PxD6JointCreate(*Physics.gPhysicsSDK,
            previousLink, prevLocalFrame, linkBody, currLocalFrame);
        d6Joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
        d6Joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
        
        d6Joint->setProjectionAngularTolerance(deg2rad(0.5));
        d6Joint->setProjectionLinearTolerance(0.01f);
        d6Joint->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
        d6Joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
        d6Joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
        d6Joint->setConstraintFlag(PxConstraintFlag::eIMPROVED_SLERP, true);
        d6Joint->setConstraintFlag(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
        joint = d6Joint;
#ifdef _DEBUG 
        std::stringstream ss;
        ss << name << "[" << std::setfill('0') << std::setw(2) << i << "]";
        PHYSX_SET_NAME(joint, ss.str()+".D6J");
#endif
        previousLink = linkBody;
        prevLocalFrame = nextLocalFrame;
    }
    jointType = D6;
    updateJointLimits(controlLink);
}

void CLiana::createFixedJoints()
{
    Entity* me = Handle(this).getOwner();
    CTransform* transform = me->get<CTransform>();
    PxVec3 pos = toPxVec3(transform->getPosition());
    PxQuat rot = toPxQuat(transform->getRotation());

    PxQuat prevFrameRot(rot*FRAME_ROT);
    PxVec3 localFrameOff(0, getShapeLength()/2 + LOCAL_OFFSET, 0);

    PxTransform prevLocalFrame(PxTransform(pos, prevFrameRot));
    PxTransform nextLocalFrame(PxTransform(localFrameOff+FRAME_SEP, FRAME_ROT));
    PxTransform currLocalFrame(PxTransform(localFrameOff, FRAME_ROT));
    #ifdef _DEBUG
        std::string name = me->getName();
    #endif

    PxRigidDynamic* previousLink = nullptr;
    
    for(unsigned i=0;i<nLinks;i++) {
        auto& joint(joints[i]);
        if (joint != nullptr) {joint->release(); joint=nullptr;}
        auto& linkBody(linkBodies[i]);

        PxFixedJoint* fixedJoint = PxFixedJointCreate(*Physics.gPhysicsSDK,
            previousLink, prevLocalFrame, linkBody, currLocalFrame);
        
        fixedJoint->setProjectionAngularTolerance(deg2rad(2));
        fixedJoint->setProjectionLinearTolerance(0.01f);
        fixedJoint->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
        fixedJoint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
        fixedJoint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
        //fixedJoint->setConstraintFlag(PxConstraintFlag::eIMPROVED_SLERP, true);
        //fixedJoint->setConstraintFlag(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
        joint = fixedJoint;
#ifdef _DEBUG 
        std::stringstream ss;
        ss << name << "[" << std::setfill('0') << std::setw(2) << i << "]";
        PHYSX_SET_NAME(joint, ss.str()+".FXJ");
#endif
        previousLink = linkBody;
        prevLocalFrame = nextLocalFrame;
    }
    jointType = FIXED;
}

void CLiana::init()
{
    Entity* me_h = Handle(this).getOwner();
    Entity* me = me_h;
    CTransform* transform = me->get<CTransform>();
#ifdef _DEBUG
    std::string name = me->getName();
#endif
    PxVec3 pos = toPxVec3(transform->getPosition());
    PxQuat linkRot = toPxQuat(transform->getRotation());
    PxVec3 localFrameOff(0, getShapeLength()/2 + LOCAL_OFFSET, 0);
    PxVec3 step(FRAME_SEP);
    pos -= localFrameOff;

    PxTransform cMass(PxTransform(localFrameOff, FRAME_ROT));
    //cMass.p = (currLocalFrame.p + nextLocalFrame.p)/2.f;

    assert(nLinks<=MAX_LINKS);
    unsigned i=0;
    for(;i<nLinks;i++) {
        auto& link(links[i]);
        auto& linkTrigger(linkTriggers[i]);
        auto& linkBody(linkBodies[i]);
        assert(linkBody == nullptr);
        assert(link.getShape() == nullptr);
        assert(linkTrigger.getShape() == nullptr);

        PxShape* linkShape = createLinkCollider(link, me_h);
        PxShape* linkTriggerShape = createLinkTrigger(linkTrigger, me_h);
        PxTransform linkTransform(pos, linkRot);
	    linkBody = PxCreateDynamic( *Physics.gPhysicsSDK, linkTransform, *linkShape, 1.f);
	    Physics.gScene->addActor(*linkBody);
        linkBody->attachShape(*linkTriggerShape);
        linkBody->setMass(LINK_MASS);
        linkBody->setLinearDamping(LINEAR_DAMP);
        linkBody->setAngularDamping(ANGULAR_DAMP);
        linkBody->setCMassLocalPose(cMass);
        linkBody->setActorFlag(PxActorFlag::eVISUALIZATION, true);
        linkBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
        linkBody->setSolverIterationCounts(nLinks*4, nLinks);

#ifdef _DEBUG        
        std::stringstream ss;
        ss << name << "[" << std::setfill('0') << std::setw(2) << i << "]";
        PHYSX_SET_NAME(linkBody, ss.str()+".DYN");
        PHYSX_SET_NAME(linkShape, ss.str()+".collider");
        PHYSX_SET_NAME(linkTriggerShape, ss.str()+".trigger");
#endif

        pos += step;
    }
    for(;i<MAX_LINKS;i++) {
        links[i].setShapeDesc(Shape::shape_t());
        linkBodies[i] = nullptr;
        joints[i] = nullptr;
    }
    createFixedJoints();
    updateJointLimits(controlLink);

#ifdef DEBUG_LIANA
    //Please do not remove, even if the liana rendering is changed (skeletal, etc
    //allow both modes through an #if directive
    //This is useful for debugging
    if(me->has<CInstancedMesh>()) {
        me->get<CInstancedMesh>()->destroy();
    }
    if(me->has<CMesh>()) {
        me->get<CMesh>()->destroy();
    }
    me->add(getManager<CInstancedMesh>()->createObj());
    me->add(getManager<CMesh>()->createObj());
    CInstancedMesh* iMesh = me->get<CInstancedMesh>();
    CMesh* cmesh = me->get<CMesh>();
    if (iMesh != nullptr && cmesh != nullptr) {
        iMesh->setAABBSkin(1);
        iMesh->setAABBScale(1);
        iMesh->createBuffer();
        for (unsigned i=0; i<nLinks; i++) {
            iMesh->addInstance(toXMMATRIX(linkBodies[i]->getGlobalPose()));
        }
        iMesh->commitInstances();
        auto size = XMVectorSet(0.2f, LINK_LENGTH, 0.2f, 1);
        auto min = XMVectorSetY(-size/2, 0);
        auto max = XMVectorSetY(size/2, -XMVectorGetY(size));
        Mesh* mesh = new Mesh;
        createPUNTBox(*mesh, min, max);
        cmesh->setMesh(mesh);
        CMesh::key_t key;
        key.group0 = key.groupf =0;
        key.material=Material::getManager().getByName("white");
        cmesh->addKey(key);
        cmesh->init();

        CAABB* aabb = me->get<CAABB>();
        if (aabb != nullptr) {aabb->init();}
    }

//#define VERIFY_CREATION
#if defined(VERIFY_CREATION) && defined(_DEBUG)
    for (unsigned i = 1; i<nLinks-1; ++i) {
        auto a0 = linkBodies[i]->getGlobalPose();
        auto j0 = joints[i]->getLocalPose(PxJointActorIndex::eACTOR0);
        auto a1 = linkBodies[i+1]->getGlobalPose();
        auto j1 = joints[i+1]->getLocalPose(PxJointActorIndex::eACTOR1);
        auto g0 = a0*j0;  
        auto g1 = a1*j1;
        assert((g0.p - g1.p).magnitude() < 0.001f); //error margin
    }
#endif

#else
    CInstancedMesh* iMesh = me->get<CInstancedMesh>();
    assert(iMesh != nullptr);
    iMesh->setAABBSkin(1);
    iMesh->setAABBScale(1);
    for (unsigned i=0; i<nLinks; i++) {
        iMesh->addInstance(toXMMATRIX(linkBodies[i]->getGlobalPose()));
    }
    iMesh->commitInstances();

    CAABB* aabb = me->get<CAABB>();
    if (aabb != nullptr) {aabb->init();}
#endif

    updateJointLimits(controlLink);
};

float CLiana::getLimit(XMVECTOR dir) const
{
    //Radius of an ellipse
    const auto& a = limitX;
    const auto& b = limitZ;
    const auto angle = getYawFromVector(dir);
    const auto cosine = std::cos(angle);
    const auto sine   = std::sin(angle);
    return (a*b)/sqrt(a*a*sine*sine+b*b*cosine*cosine);
}

void CLiana::updateJointLimits(int lastLink)
{
    if (jointType != D6) {return;}
    assert(lastLink < static_cast<int>(nLinks));
    if (lastLink < 0 ) {lastLink = nLinks-1;} 
    unsigned i=0;
    const auto constraint = PxSpring(
        SPRING_STIFF*(lastLink+1)*LINK_MASS,
        SPRING_DAMP*(lastLink+1)*LINK_MASS);
    for (;i<=static_cast<unsigned>(lastLink);++i) {
        auto& joint = (PxD6Joint*&)(joints[i]);
        if(joint != nullptr) {
            joint->setSwingLimit(PxJointLimitCone(limitX/lastLink, limitZ/lastLink, constraint));
        }
        auto& link = linkBodies[i];
        if(link != nullptr) {
            link->setMass(LINK_MASS);
        }
    }
    
    const auto restLinks = (nLinks-i+1);
    const auto freeConstraint = PxSpring(
        SPRING_STIFF_LOOSE*restLinks*LINK_MASS,
        SPRING_DAMP_LOOSE*restLinks*LINK_MASS);
    float freeLimitFactor = restLinks > 3 ? 1.25f : restLinks > 2 ? 0.75f : 0.25f;
    const float l = freeLimitFactor*M_PI_2f/restLinks;
    for(;i<nLinks;++i) {
        assert(nLinks-i != 0);
        auto& joint = (PxD6Joint*&)(joints[i]);
        if(joint != nullptr) {
            joint->setSwingLimit(PxJointLimitCone(l, l, freeConstraint));
        }
        auto& link = linkBodies[i];
        if(link != nullptr) {
            link->setMass(LINK_MASS_LOOSE);
        }
    }
}

void CLiana::setFilters(physX_user::filter_t filter, bool colliders, bool triggers)
{
    static const auto obligatory1stFilter = filter_t(filter_t::NONE, filter_t::SCENE, filter_t::NONE);
    if(colliders) {
        linkFilter |= filter;
        for(auto& shape : links) {shape.setFilters(filter);}
    }
    if(triggers) {
        linkTriggerFilter |= filter;
        for(auto& shape : linkTriggers) {shape.setFilters(filter);}
    }
    links[0].setFilters(obligatory1stFilter);
    linkTriggers[0].setFilters(obligatory1stFilter);
}
void CLiana::removeFilters(physX_user::filter_t filter, bool colliders, bool triggers)
{
    static const auto obligatory1stFilter = filter_t(filter_t::NONE, filter_t::SCENE, filter_t::NONE);
    if(colliders) {
        linkFilter &=~filter;
        for(auto& shape : links) {shape.removeFilters(filter);}
    }
    if(triggers) {
        linkTriggerFilter &=~filter;
        for(auto& shape : linkTriggers) {shape.removeFilters(filter);}
    }
    links[0].setFilters(obligatory1stFilter);
    linkTriggers[0].setFilters(obligatory1stFilter);
}

void CLiana::setCapsule(float radius, float height, bool colliders, bool triggers)
{
    if (colliders) {
	    linkShapeDesc.type = Shape::shape_t::CAPSULE;
	    linkShapeDesc.capsule.radius = radius;
	    linkShapeDesc.capsule.height = height;
        for(auto& shape : links) {shape.setCapsule(radius, height);}
    }
    if (triggers) {
	    linkTriggerShapeDesc.type = Shape::shape_t::CAPSULE;
	    linkTriggerShapeDesc.capsule.radius = radius;
	    linkTriggerShapeDesc.capsule.height = height;
        for(auto& shape : linkTriggers) {shape.setCapsule(radius, height);}
    }
}

void CLiana::setShapeModifiers(float skin, float scale, bool colliders, bool triggers)
{
    if (colliders) {
	    linkShapeDesc.scale = scale;
	    linkShapeDesc.skin = skin;
        for(auto& shape : links) {
            auto desc = shape.getShapeDesc();
            desc.skin = skin;
            desc.scale = scale;
            shape.setShapeDesc(desc);
        }
    }
    if (triggers) {
	    linkTriggerShapeDesc.scale = scale;
	    linkTriggerShapeDesc.skin = skin;
        for(auto& shape : linkTriggers) {
            auto desc = shape.getShapeDesc();
            desc.skin = skin;
            desc.scale = scale;
            shape.setShapeDesc(desc);
        }
    }
}

int CLiana::getNodeIndex(PxShape* shape)
{
    for(unsigned i=0; i<nLinks; i++) {
        if (linkTriggers[i].getShape() == shape) {return i;}
        if (links[i].getShape()        == shape) {return i;}
    }
    return -1;
}

Transform CLiana::getTransform(unsigned nodeIndex) const
{
    assert(nodeIndex < nLinks);
    const auto& linkT = linkBodies[nodeIndex]->getGlobalPose();
    Transform ret;
    ret.setPosition(toXMVECTOR(linkT.p));
    ret.setRotation(toXMQuaternion(linkT.q));
    return ret;
}


XMVECTOR CLiana::getLinkPosition(unsigned nodeIndex, float localHeightRatio)
{
    assert(nodeIndex < nLinks);
    const float& linkH = LINK_LENGTH;
    return XMVector3Transform(
        -yAxis_v*inRange(-1.f,localHeightRatio,1.f)*LINK_LENGTH/2,
        toXMMATRIX(linkBodies[nodeIndex]->getGlobalPose()));
}

bool CLiana::applyBrakeToLink(unsigned nodeIndex,
    float distFactor, float localHeightRatio, float aimOffsetRatio, float tolerance)
{
    assert(nodeIndex < nLinks);
    float f(distFactor);
    const float& linkH = LINK_LENGTH;
    float offset = linkH*aimOffsetRatio;
    float h = inRange(-1.f,localHeightRatio,1.f)*0.5f*linkH;
    float hReal = (linkH*0.5f-h) + offset;
    auto hRelativeOffset = yAxis_v*(h - offset);
    auto hRelative = yAxis_v*h;
    CTransform* lianaT = Handle(this).getBrother<CTransform>();
    auto straightPos = lianaT->getPosition() - yAxis_v*(linkH*(float)nodeIndex+hReal);
    auto currentPos  = XMVector3Transform(hRelative, toXMMATRIX(linkBodies[nodeIndex]->getGlobalPose()));
    auto distance = (straightPos - currentPos);
    auto distLen = XMVectorGetX(XMVector3Length(distance));
    if (distLen > tolerance+offset) {
        auto force = distance * distFactor;
        #ifdef DEBUG_LIANA_FORCES
            linkBrakes[nodeIndex] += toPxVec3(force);
        #endif
        Physics.get().addForceExternal(*linkBodies[nodeIndex], force, hRelative);
        return true;
    } else {
        return false;
    }
}

void CLiana::propagateBrake(unsigned nodeIndex, int step,
    float distFactor, float localHeightRatio, float factor, float diff,
    float aimOffsetRatio, float offsetFactor, float tolerance)
{
    assert(nodeIndex < nLinks);
    assert(diff >= 0.f);
    float f(distFactor);
    float offRatio(aimOffsetRatio);
    const float& linkH = LINK_LENGTH;
    float h = inRange(-1.f,localHeightRatio,1.f)*0.5f*linkH;
    auto hRelative = yAxis_v*h;
    CTransform* lianaT = Handle(this).getBrother<CTransform>();
    for (int i = nodeIndex; i>=0 && f>0 && i<static_cast<int>(nLinks); i+=step) {
        float offset = linkH*offRatio;
        float hReal = (linkH*0.5f-h) + offset;
        auto hRelativeOffset = yAxis_v*(h - offset);
        auto straightPos = lianaT->getPosition() - yAxis_v*(linkH*(float)i+hReal);
        auto currentPos  = XMVector3Transform(hRelative, toXMMATRIX(linkBodies[i]->getGlobalPose()));
        auto distance = (straightPos - currentPos);
        auto distLen = XMVectorGetX(XMVector3Length(distance));
        if (distLen > tolerance+offset) {
            auto force = distance * f;
            #ifdef DEBUG_LIANA_FORCES
                linkBrakes[i] += toPxVec3(force);
            #endif
            Physics.get().addForceExternal(*linkBodies[i], force, hRelative);
        }
        f *= factor;
        f -= std::min(f, diff);
        offRatio*=offsetFactor;
    }
}

void CLiana::applyForceToLink(unsigned nodeIndex, const XMVECTOR& force, float localHeightRatio)
{
    assert(nodeIndex < nLinks);
    float h = inRange(-1.f,localHeightRatio,1.f)*LINK_LENGTH/2;
    auto pxForce(toPxVec3(force));
#ifdef DEBUG_LIANA_FORCES
        linkForces[nodeIndex] += pxForce;
#endif
    Physics.get().addForceExternal(*linkBodies[nodeIndex], pxForce, PxVec3(0,h,0));
}

void CLiana::propagateForce(unsigned nodeIndex, int step,
    const XMVECTOR& force, float localHeightRatio, float factor, float diff)
{
    auto currentForce(toPxVec3(force));
    assert(nodeIndex < nLinks);
    assert(diff >= 0.f);
    PxReal m = 1.f;
    float h = inRange(-1.f,localHeightRatio,1.f)*getShapeLength()/2;
    for (int i = nodeIndex; i>=0 && m>0 && i<static_cast<int>(nLinks); i+=step) {
#ifdef DEBUG_LIANA_FORCES
        linkForces[i] += currentForce;
#endif
        Physics.get().addForceExternal(*linkBodies[i], currentForce, PxVec3(0,h,0));
        currentForce *= factor;
        if (diff != 0.f) {
            m = currentForce.normalize();
            m -= std::min(m, diff);
            currentForce *= m;
        }
    }
}

void CLiana::resetControlLink()
{
    controlLink = -1;
    updateJointLimits(controlLink);
}

void CLiana::loadFromProperties(std::string elem, MKeyValue& atts)
{
    if (elem == "Liana") {
        limitX = deg2rad(atts.getFloat("limitX", rad2deg(limitX)));
        limitZ = deg2rad(atts.getFloat("limitZ", rad2deg(limitZ)));
        nLinks = atts.getInt("nLinks", nLinks);
    } else if (elem == "collider") {
        Shape reader;
        reader.loadFromProperties(elem, atts);
        linkShapeDesc = reader.getShapeDesc();
        linkFilter = reader.getFilter();
    } else if (elem == "trigger") {
        Shape reader;
        reader.loadFromProperties(elem, atts);
        linkTriggerShapeDesc = reader.getShapeDesc();
        linkTriggerFilter = reader.getFilter();
    }
}

void CLiana::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CLiana, MsgTransform, receive);
    SUBSCRIBE_MSG_TO_MEMBER(CLiana, MsgRevive, revive);
}

void CLiana::receive(const MsgTransform&)
{
    filter_t filter1(filter_t::NONE, filter_t::PLAYER|filter_t::PLAYERCANNON, filter_t::NONE);
    filter_t filter2(filter_t::NONE, filter_t::BULLET, filter_t::NONE);
    setFilters   (filter2,true ,false);
    removeFilters(filter1,false,true );
    createD6Joints();
}

void CLiana::revive(const MsgRevive&)
{
    filter_t filter1(filter_t::NONE, filter_t::PLAYER|filter_t::PLAYERCANNON, filter_t::NONE);
    filter_t filter2(filter_t::NONE, filter_t::BULLET, filter_t::NONE);
    removeFilters(filter1 | filter2, true ,false);
    setFilters   (filter1          , false,true );
    createFixedJoints();
}

void CLiana::reset()
{
	filter_t filter1(filter_t::NONE, filter_t::PLAYER | filter_t::PLAYERCANNON, filter_t::NONE);
	filter_t filter2(filter_t::NONE, filter_t::BULLET, filter_t::NONE);
	setFilters   (filter1 | filter2, true, false);
	removeFilters(filter2          , false, true);
    createFixedJoints();
}

}
