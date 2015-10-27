#include "mcv_platform.h"
#include "PhysicsManager.h"

#include "handles\entity.h"
#include "handles\objectManager.h"
#include "handles\handle.h"
using namespace component;

#include "PhysX_USER/pxcomponents.h"
#include "filterShader.h"

#include "app.h"

#define DEBUGGER_FLAGS \
    PxVisualDebuggerFlag::eTRANSMIT_CONTACTS | \
    PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS | \
    PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES
#define DEBUGGER_CONNECTION_FLAGS \
    PxVisualDebuggerConnectionFlag::eDEBUG | \
    PxVisualDebuggerConnectionFlag::ePROFILE | \
    PxVisualDebuggerConnectionFlag::eMEMORY

#ifndef PX_PHYSX_CHARACTER_STATIC_LIB 
#error
#endif // !PX_PHYSX_CHARACTER_STATIC_LIB 


using namespace physx;
using namespace DirectX;
using namespace render;

namespace physX_user {

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;

PhysicsManager PhysicsManager::physics_manager;

PhysicsManager::PhysicsManager() : gScene(NULL), timeStep(1.f / 50.f) // 50 Hz
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::init() {

	PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, 
												  gDefaultAllocatorCallback,
												  gDefaultErrorCallback);


	PxTolerancesScale scale = PxTolerancesScale();

	if (foundation == NULL)
		assert("PxCreateFoundation failed!");
	//----------------------------------------------------------------------------------------- 
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, scale);
	if (gPhysicsSDK == NULL)
		assert("PxCreatePhysics failed!");

	PxRegisterParticles(*gPhysicsSDK);

	//-----------------------------------------------------------------------------------------

	#ifdef _TEST_PERFORMANCE_PARTICLES_

		mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(foundation);

		if (!mProfileZoneManager)
			assert("PxProfileZoneManager::createProfileZoneManager failed!");

		PxCudaContextManagerDesc cudaContextManagerDesc;

		#if PX_SUPPORT_GPU_PHYSX

		cudaContextManagerDesc.interopMode = PxCudaInteropMode::D3D11_INTEROP;
		cudaContextManagerDesc.graphicsDevice = Render::getDevice();

		mCudaContextManager = PxCreateCudaContextManager(*foundation, cudaContextManagerDesc, mProfileZoneManager);
		if (mCudaContextManager)
		{
			if (!mCudaContextManager->contextIsValid())
			{
				mCudaContextManager->release();
				mCudaContextManager = NULL;
			}
		}

		#endif 

	#endif 
	//-----------------------------------------------------------------------------------------
	


	PxCookingParams params(scale);
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES | PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, params);
	if (!gCooking)
		assert("PxCreateCooking failed!");

	if (!PxInitExtensions(*gPhysicsSDK)){
		assert("PxInitExtensions failed!");
	}

	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	// Gravedad terrestre
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if (sceneDesc.cpuDispatcher == nullptr){
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}

	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

	// Filtro de colisiones
	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = filter_t::filter;

	sceneDesc.simulationEventCallback = this;
	
	// component data come from XML file
	gScene = gPhysicsSDK->createScene(sceneDesc);

	//------------------------------------------------------------------------------------------

	#ifdef _PARTICLES
		//1-Creating static plane
		PxTransform planePos = PxTransform(PxVec3(0.0f, 0,
			0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
		PxRigidStatic* plane = gPhysicsSDK->createRigidStatic(planePos);
		//Creating material
		PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.01f, 0.01f, 0.01f);
		plane->createShape(PxPlaneGeometry(), *mMaterial);
		gScene->addActor(*plane);
	#endif

    #ifdef _DEBUG
	    // Check if PhysX Visual Debuger is running and listening
        auto connectionManager = gPhysicsSDK->getPvdConnectionManager();
	    ////Visual debugger
	    if (connectionManager != nullptr) {        
	        gScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	        gScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
		    gPhysicsSDK->getVisualDebugger()->setVisualizeConstraints(true);
		    gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlags(DEBUGGER_FLAGS);

	        PxVisualDebuggerConnectionFlags flags = DEBUGGER_CONNECTION_FLAGS;

		    PxVisualDebuggerConnection* gConnection = PxVisualDebuggerExt::createConnection(
                connectionManager,"127.0.0.1", 5425, 1000, flags);
            if (gConnection == nullptr) {
                dbg("PVD not found.\n");
            }
	    } else {
            dbg("Connection manager couldn't be fetched.\n");
        }
    #endif

	manager = PxCreateControllerManager(*gScene);
}

bool PhysicsManager::raycast(PxVec3 origin, PxVec3 unit_dir, PxReal max_distance,
    PxRaycastBuffer &hit, filter_t filter)
{
    auto prevFilterTouch = filterTouch;
    filterTouch = PxQueryHitType::eBLOCK;
	auto ret = gScene->raycast(origin, unit_dir, max_distance, hit, PxHitFlag::eDEFAULT,
        PxQueryFilterData(filter,
            PxQueryFlag::ePREFILTER|PxQueryFlag::eDYNAMIC|PxQueryFlag::eSTATIC),
        &Physics);
    filterTouch = prevFilterTouch;
    return ret;
}

bool PhysicsManager::raycast(
    XMVECTOR origin, XMVECTOR unit_dir,
    PxReal max_distance, PxRaycastBuffer &hit,
    filter_t filter) {
	return raycast(toPxVec3(origin), toPxVec3(unit_dir), max_distance, hit, filter);
}

bool PhysicsManager::sweep(
	XMVECTOR& unitDir, PxReal maxDist, PxGeometry& geom0, PxTransform& pose0,
	PxSweepBuffer& sweepHit, filter_t filter, PxHitFlags hitFlags, PxReal inflation) {
    auto prevFilterTouch = filterTouch;
    filterTouch = PxQueryHitType::eTOUCH;
	auto ret = gScene->sweep(geom0, pose0, toPxVec3(unitDir), maxDist, sweepHit, hitFlags,
        PxQueryFilterData(filter,
            PxQueryFlag::ePREFILTER|PxQueryFlag::eDYNAMIC|PxQueryFlag::eSTATIC),
        &Physics, NULL, inflation);
    filterTouch = prevFilterTouch;
    return ret;
}

class PxOverlapCallback_dummyAdaptor : public PxOverlapCallback{
    private:
        PxOverlapHit hit;

    public:
        PxOverlapCallback_dummyAdaptor() :
            PxOverlapCallback(&hit, 1) {}
        PxAgain processTouches(const PxOverlapHit* buffer, PxU32 nbHits) {
            //Quick check: fail ASAP
            return false;
        }
};

PxShape* PhysicsManager::cloneShape(PxShape* shape)
{
    assert(shape != nullptr);
    PxGeometry* geometry = &shape->getGeometry().any();
    PxMaterial* material;
    shape->getMaterials(&material, 1);
    PxShape* cloned = gPhysicsSDK->createShape(*geometry, *material, true);
    cloned->setSimulationFilterData(shape->getSimulationFilterData());
    cloned->setQueryFilterData(shape->getQueryFilterData());
    cloned->setBaseFlags(shape->getBaseFlags());
    cloned->setContactOffset(shape->getContactOffset());
    cloned->setFlags(shape->getFlags());
    cloned->setLocalPose(shape->getLocalPose());
    cloned->setName(shape->getName());
    cloned->setRestOffset(shape->getRestOffset());
    return cloned ;
}

bool PhysicsManager::overlap(PxShape* shape, PxTransform transform, const PxQueryFilterData& filter) {
    PxOverlapCallback_dummyAdaptor dummy;
    switch (shape->getGeometryType()) {
        case PxGeometryType::eBOX:
            return gScene->overlap(shape->getGeometry().box(), transform, dummy, filter); break;
        case PxGeometryType::eSPHERE:
            return gScene->overlap(shape->getGeometry().sphere(), transform, dummy, filter); break;
        case PxGeometryType::eCAPSULE:
            return gScene->overlap(shape->getGeometry().capsule(), transform, dummy, filter); break;
        case PxGeometryType::eCONVEXMESH:
            return gScene->overlap(shape->getGeometry().convexMesh(), transform, dummy, filter); break;
        default: return false;
    }
	
}

void PhysicsManager::addVelocityObject(PxRigidBody &rigidbody, XMVECTOR force, XMVECTOR rotationTarget, XMVECTOR potition){
	PxTransform transform = PxTransform(
	    toPxVec3(potition),
	    toPxQuat(rotationTarget)
        );

	rigidbody.setGlobalPose(transform);
	rigidbody.setLinearVelocity(toPxVec3(force));
	rigidbody.setAngularVelocity(toPxVec3(force));
	PxRigidBodyExt::updateMassAndInertia(rigidbody, 1);
}

XMVECTOR PhysicsManager::getVelocityObject(PxRigidBody &rigidbody, XMVECTOR potition){
	return toXMVECTOR(PxRigidBodyExt::getVelocityAtPos(rigidbody, toPxVec3(potition)));
}

void PhysicsManager::addForceExternal(PxRigidBody &rigidbody, PxVec3 force, PxVec3 position)
{
	PxRigidBodyExt::addForceAtLocalPos(rigidbody,force, position, PxForceMode::eFORCE, true);
}
void PhysicsManager::addForceExternal(PxRigidBody &rigidbody, XMVECTOR force, XMVECTOR position)
{
	addForceExternal(rigidbody, toPxVec3(force), toPxVec3(position));
}

PxShape* PhysicsManager::createSphereShape(PxReal radius,
		PxReal staticFriction,
		PxReal dynamicFriction,
		PxReal restitution)
        {

	PxShape *shape = gPhysicsSDK->createShape(
        PxSphereGeometry(radius),
        *gPhysicsSDK->createMaterial(
            staticFriction,dynamicFriction,restitution),
        true);

	return shape;
}
PxShape* PhysicsManager::createTriangleMesh(
	PxTriangleMesh* mesh,
	PxReal staticFriction,
	PxReal dynamicFriction,
	PxReal restitution,
    PxReal sca
){
    PxMeshScale scale(PxVec3(sca,sca,sca), PxQuat(1.0f));
    PxTriangleMeshGeometry geometry(mesh,scale);
	return gPhysicsSDK->createShape(geometry, 
		*gPhysicsSDK->createMaterial( staticFriction, dynamicFriction, restitution),
        true);
}

PxShape* PhysicsManager::createCapsule(PxReal h,
	PxReal radius,
	PxReal staticFriction,
	PxReal dynamicFriction,
	PxReal restitution
	){

	PxShape *shape = gPhysicsSDK->createShape(
		PxCapsuleGeometry(radius,h/2),
		*gPhysicsSDK->createMaterial( staticFriction, dynamicFriction, restitution),
		true);

	PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
	shape->setLocalPose(relativePose);

	return shape;
}

PxShape* PhysicsManager::createBoxShape(PxVec3 size,
		PxReal staticFriction,
		PxReal dynamicFriction,
		PxReal restitution)
        {

	PxVec3 halfExtents = size/2;

	PxShape *shape = gPhysicsSDK->createShape(
        PxBoxGeometry(halfExtents),
        *gPhysicsSDK->createMaterial(
            staticFriction,dynamicFriction,restitution),
        true);


	return shape;
}

//--------------------------------------------
PxMaterial* getMaterial(PxShape *collider) {

	PxMaterial* mat;
	collider->getMaterials(&mat, 1);
	return mat;
}

// Returns the material properties as a vector
XMVECTOR getMaterialProperties(PxShape *collider) {
	PxMaterial* mat;
	collider->getMaterials(&mat, 1);
	return XMVectorSet(
		mat->getStaticFriction(),
		mat->getDynamicFriction(),
		mat->getRestitution(),
		0
		);
}

void PhysicsManager::fixedUpdate(float elapsed)
{
    //Simulate the scene
    gScene->simulate(timeStep);
    gScene->fetchResults(true);
    
    // Update PhysX components
    component::getManager<CCharacterController>()->fixedUpdate(elapsed);
    component::getManager<CRigidBody>()->fixedUpdate(elapsed);
}

void PhysicsManager::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; i++) {
		const PxTriggerPair& tp = pairs[i];
		if (tp.status & PxPairFlag::eNOTIFY_TOUCH_FOUND|PxPairFlag::eNOTIFY_TOUCH_LOST){
            
            filter_t f1 = pairs[i].otherShape->getSimulationFilterData();
            filter_t f2 = pairs[i].triggerShape->getSimulationFilterData();
            if (!filter_t::testReport(f1, f2)) {return;}

			Handle h1 = Handle::fromRaw(pairs[i].otherShape->userData);
			Handle h2 = Handle::fromRaw(pairs[i].triggerShape->userData);
            
			if(h1.isValid() && h2.isValid()) {
                bool enter = !(tp.status & PxPairFlag::eNOTIFY_TOUCH_LOST);
			    Entity *e1 = h1;
			    Entity *e2 = h2;
                e1->sendMsg(MsgCollisionEvent(h2, f1, pairs[i].triggerShape, enter));
                e2->sendMsg(MsgCollisionEvent(h1, f2, pairs[i].otherShape,   enter));
            }
		}
	}
}


void PhysicsManager::onContact(
    const PxContactPairHeader& pairHeader,
    const PxContactPair* pairs,
    PxU32 nbPairs)
{
	for (physx::PxU32 i = 0; i < nbPairs; i++) {
		const PxContactPair& tp = pairs[i];

		if (tp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND ||
            tp.events & PxPairFlag::eNOTIFY_TOUCH_CCD ||
            tp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS ||
            tp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {
			PxShape* shape1 = tp.shapes[0];
            PxShape* shape2 = tp.shapes[1];
            if (shape1->userData == 0 || shape2->userData == 0) {
                continue;
            }

            filter_t f1 = shape1->getSimulationFilterData();
            filter_t f2 = shape2->getSimulationFilterData();
            if (!filter_t::testReport(f1, f2)) {continue;}

			Handle h1 = Handle::fromRaw(shape1->userData);
			Handle h2 = Handle::fromRaw(shape2->userData);

			if(h1.isValid() && h2.isValid()) {
                bool enter = !(tp.events & PxPairFlag::eNOTIFY_TOUCH_LOST);
			    Entity *e1 = h1;
			    Entity *e2 = h2;
                e1->sendMsg(MsgCollisionEvent(h2, f1, shape2, enter));
                e2->sendMsg(MsgCollisionEvent(h1, f2, shape1, enter));
            }

		}
	}
}

void PhysicsManager::onShapeHit(const PxControllerShapeHit& hit)
{
    PxShape* myShape;
    hit.controller->getActor()->getShapes(&myShape, 1);
    PxShape* otherShape = hit.shape;
    assert(myShape != nullptr);
    assert(otherShape != nullptr);
    auto me_h(Handle::fromRaw(myShape->userData));
    auto other_h(Handle::fromRaw(otherShape->userData));
    filter_t f0 = myShape->getSimulationFilterData();
    filter_t f1 = otherShape->getSimulationFilterData();
    if (!filter_t::testReport(f0, f1)) {return;}
    Entity* me(me_h);
	if(me_h.isValid() && other_h.isValid()) {
        me->sendMsg(MsgHitEvent(other_h, f1, otherShape,
            toXMVECTOR(hit.dir), toXMVECTOR(hit.worldNormal),
            toXMVECTOR(hit.worldPos), hit.length));
        Entity* other(other_h);
        other->sendMsg(MsgHitEvent(me_h, f0, myShape,
            toXMVECTOR(-hit.dir), toXMVECTOR(-hit.worldNormal),
            toXMVECTOR(hit.worldPos), hit.length));
    }
}

void PhysicsManager::onControllerHit(const PxControllersHit& hit)
{
    PxShape* myShape;
    hit.controller->getActor()->getShapes(&myShape, 1);
    PxShape* otherShape;
    hit.other->getActor()->getShapes(&otherShape, 1);
    assert(myShape != nullptr);
    assert(otherShape != nullptr);
    filter_t f0 = myShape->getSimulationFilterData();
    filter_t f1 = otherShape->getSimulationFilterData();
    if (!filter_t::testReport(f0, f1)) {return;}

    auto me_h(Handle::fromRaw(myShape->userData));
    auto other_h(Handle::fromRaw(otherShape->userData));
	if(me_h.isValid() && other_h.isValid()) {
        Entity* me(me_h);
        me->sendMsg(MsgHitEvent(other_h, f0, myShape,
            toXMVECTOR(hit.dir), toXMVECTOR(hit.worldNormal),
            toXMVECTOR(hit.worldPos), hit.length));
    }
}


void PhysicsManager::ShutdownPhysX()
{
    if(gScene !=nullptr) {gScene->release(); gScene = nullptr;}
    if(gPhysicsSDK !=nullptr) {gPhysicsSDK->release(); gPhysicsSDK = nullptr;}
}

PxQueryHitType::Enum PhysicsManager::preFilter(
    const PxFilterData & filterData, const PxShape * shape,
    const PxRigidActor * actor, PxHitFlags & queryFlags)
{
    filter_t filter0(filterData);
    filter_t filter1(shape->getQueryFilterData());
    auto ret = filter1.isTrigger() ? PxQueryHitType::eNONE :
        filter_t::filter(filter0, filter1) ? PxQueryHitType::eNONE : filterTouch;
    return ret;
}

bool PhysicsManager::filter(const PxController& a, const PxController& b)
{
    PxShape* shapeA;
    PxShape* shapeB;
    a.getActor()->getShapes(&shapeA,1);
    b.getActor()->getShapes(&shapeB,1);
    return !filter_t::filter(shapeA->getSimulationFilterData(), shapeB->getSimulationFilterData());
}

};
