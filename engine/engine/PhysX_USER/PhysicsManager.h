#ifndef _PHYSICS_MANAGER_H_
#define _PHYSICS_MANAGER_H_

#include "mcv_platform.h"
#include "handles/message.h"
#include "filterShader.h"

using namespace physx;
using namespace component;

namespace physX_user {

struct MsgCollisionEvent {
    DECLARE_MSG_ID();
    component::Handle entity;
    bool enter;
    filter_t filter;
    PxShape* shape;
    MsgCollisionEvent()=default;
    MsgCollisionEvent(component::Handle entity, filter_t filter, PxShape* shape, bool enter) :
        entity(entity), filter(filter), shape(shape), enter(enter) {}
};

struct MsgHitEvent {
    DECLARE_MSG_ID();
    component::Handle entity;
    filter_t filter;
    PxShape* shape;
    XMVECTOR dir;
    XMVECTOR worldNormal;
    XMVECTOR worldPos;
    float length;

    MsgHitEvent(component::Handle entity, filter_t filter, PxShape* shape, XMVECTOR dir,
        XMVECTOR worldNormal, XMVECTOR worldPos, float length) :
        entity(entity), filter(filter), shape(shape), dir(dir), 
        worldNormal(worldNormal), length(length) {}
};

class PhysicsManager : public PxSimulationEventCallback,
					   public PxUserControllerHitReport,
                       public PxQueryFilterCallback,
                       public PxControllerFilterCallback,
                       public PxControllerBehaviorCallback 
{

private:
	PhysicsManager();
	virtual ~PhysicsManager();
	static PhysicsManager physics_manager;

    PxQueryHitType::Enum filterTouch = PxQueryHitType::eBLOCK;


public:

	PxPhysics *gPhysicsSDK;
	PxCooking *gCooking;
	PxScene* gScene;
	PxReal timeStep;
	PxControllerManager* manager;
#if PX_SUPPORT_GPU_PHYSX
	PxCudaContextManager*					mCudaContextManager;
#endif
	PxProfileZoneManager*					mProfileZoneManager;

    inline PxVec3 getGravity() const {return gScene->getGravity();}
    static inline PhysicsManager& get() {return physics_manager;}

	//init Properties
	void init();
	void initCollisionManager();

	PxShape* createBoxShape(PxVec3 scale,
		PxReal staticFriction=0,
		PxReal dynamicFriction=0,
		PxReal restitution=0);
    PxShape* createSphereShape(PxReal radius,
		PxReal staticFriction=0,PxReal dynamicFriction=0,PxReal restitution=0);
	PxShape* createCapsule(PxReal h, PxReal radius,
        PxReal staticFriction=0, PxReal dynamicFriction=0, PxReal restitution=0);
    PxShape* createTriangleMesh(PxTriangleMesh* mesh,
        PxReal staticFriction=0, PxReal dynamicFriction=0, PxReal restitution=0,
        PxReal scale = 1.f);

    //Add velocity
	void addVelocityObject(PxRigidBody &rigidbody, XMVECTOR force,
        XMVECTOR rotationTarget,  XMVECTOR position);

	XMVECTOR getVelocityObject(PxRigidBody &rigidbody, XMVECTOR potition);

    void addForceExternal(PxRigidBody &rigidbody, PxVec3 force, PxVec3 position);
	void addForceExternal(PxRigidBody &rigidbody, XMVECTOR force, XMVECTOR position);

	//Raycasts
	bool raycast(PxVec3 origin, PxVec3 unit_dir, PxReal max_distance, 
        PxRaycastBuffer &hit, filter_t filter = filter_t());

	bool raycast(XMVECTOR origin,  XMVECTOR unit_dir,  PxReal max_distance,
        PxRaycastBuffer &hit, filter_t filter = filter_t());

    //Sweeps
	bool sweep(XMVECTOR& unitDir, PxReal maxDist, PxGeometry& geom0, PxTransform& pose0,
        PxSweepBuffer& sweepHit, filter_t filter = filter_t(),
        PxHitFlags hitFlags = PxHitFlag::eDEFAULT,  PxReal inflation = 0.f);

    //overlap
    bool overlap(PxShape* shape, PxTransform transform = PxTransform::createIdentity(),
        const PxQueryFilterData& filter = PxQueryFilterData());

    PxShape* cloneShape(PxShape*);

	// delete all PhysX reference
	void ShutdownPhysX();

	// create a Character Controller
	PxController* createCharacterController(XMVECTOR potition_,
		float contactOffset,
		float climb_,
		float slopeLimit_,
		float radious_,
		float height_,
		PxMaterial* material_,
		PxVec3 upDirection_ = PxVec3(0, 1, 0)
		);
	//filter
	static PxFilterFlags filter(PxFilterObjectAttributes attributes0,
		PxFilterData filterData0,
		PxFilterObjectAttributes attributes1,
		PxFilterData filterData1,
		PxPairFlags& pairFlags,
		const void* constantBlock,
		PxU32 constantBlockSize);

	// Implements PxSimulationEventCallback
	virtual void	onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	virtual void	onTrigger(PxTriggerPair* pairs, PxU32 count);
	virtual void	onConstraintBreak(PxConstraintInfo*, PxU32) {}
	virtual void	onWake(PxActor**, PxU32) {}
	virtual void	onSleep(PxActor**, PxU32){}


	//Implements PxUserControllerHitReport
	virtual void	onControllerHit(const PxControllersHit & 	hit);
	virtual void	onObstacleHit(const PxControllerObstacleHit & 	hit){}
	virtual void	onShapeHit(const PxControllerShapeHit & 	hit);
    
    PxQueryHitType::Enum preFilter(
        const PxFilterData & filterData, const PxShape * shape,
        const PxRigidActor * actor, PxHitFlags & queryFlags);
    PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit) {
        return filterTouch;
    }

    bool filter(const PxController& a, const PxController& b);

    inline PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) {
        return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
    }
    inline PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) {
        return PxControllerBehaviorFlag::eCCT_SLIDE;
    }
    inline PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle) {
        return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
    }

    void fixedUpdate(float elapsed);

};

inline PxVec3 toPxVec3(XMVECTOR v) {
    return PxVec3(
        DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), DirectX::XMVectorGetZ(v)
        );
}

inline PxVec3 toPxVec3(XMFLOAT3 v) {
	return PxVec3(v.x,v.y,v.z);
}

inline PxExtendedVec3 toPxExtendedVec3(XMVECTOR v) {
    return PxExtendedVec3(
        DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), DirectX::XMVectorGetZ(v)
        );
}

inline XMVECTOR toXMVECTOR(PxVec3 v) {
    return DirectX::XMVectorSet((float)v.x, (float)v.y, (float)v.z, 0.f);
}

inline XMFLOAT3 toXMFLOAT3(PxVec3 v) {
	return XMFLOAT3((float)v.x, (float)v.y, (float)v.z);
}

inline XMVECTOR toXMVECTOR(PxExtendedVec3 v) {
    return DirectX::XMVectorSet((float)v.x, (float)v.y, (float)v.z, 0.f);
}


inline XMVECTOR toXMVECTOR(PxQuat q){
	return DirectX::XMVectorSet((float)q.x, (float)q.y, (float)q.z, (float)q.w);
}

inline XMVECTOR toXMVECTOR(PxVec4 v) {
    return DirectX::XMVectorSet((float)v.x, (float)v.y, (float)v.z, (float)v.w);
}
inline PxVec4 toPxVec4(XMVECTOR v){
    return PxVec4(
        DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v),
        DirectX::XMVectorGetZ(v), DirectX::XMVectorGetW(v)
        );
}

inline PxQuat toPxQuat(XMVECTOR quat)
{
	return PxQuat(
		DirectX::XMVectorGetX(quat), DirectX::XMVectorGetY(quat),
		DirectX::XMVectorGetZ(quat), DirectX::XMVectorGetW(quat)
		);
}

inline XMVECTOR toXMQuaternion(PxQuat quat) {
	return DirectX::XMVectorSet(quat.x, quat.y, quat.z, quat.w);
}


inline PxTransform toPxTransform(XMMATRIX m) {
    PxMat44 pxM(toPxVec4(m.r[0]), toPxVec4(m.r[1]), toPxVec4(m.r[2]), toPxVec4(m.r[3]));
    return PxTransform(pxM);
}
inline XMMATRIX toXMMATRIX(PxTransform m) {
    PxMat44 pxM(m);
    return XMMATRIX(toXMVECTOR(pxM.column0), toXMVECTOR(pxM.column1),
        toXMVECTOR(pxM.column2), toXMVECTOR(pxM.column3));
}

inline PxTransform PxMatrixAffineTransformation(
    XMVECTOR scale, XMVECTOR rotOrigin,
    XMVECTOR rotQ, XMVECTOR position
    ) {
    return toPxTransform(DirectX::XMMatrixAffineTransformation(
        scale, rotOrigin, rotQ, position));
}


}

#endif