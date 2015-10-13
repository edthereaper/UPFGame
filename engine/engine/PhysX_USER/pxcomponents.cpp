#include "mcv_platform.h"
#include "pxcomponents.h"

#include "CollisionMesh.h"

namespace physX_user {

PxMaterial* obtainMaterial(PxShape* shape)
{
    if(shape == nullptr || shape->getNbMaterials() < 1) {
        return Physics.get().gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f);
    } else {
        PxMaterial* ret;
        shape->getMaterials(&ret, 1);
        return ret;
    }
}

void Shape::setUserData(Handle h)
{
    assert(shape != nullptr);
	shape->userData = h.getRawAsVoidPtr();
}

void Shape::createShape()
{
    assert(shape == nullptr);
    switch (shapeDesc.type) {
        case Shape::shape_t::BOX:
            shape = PhysicsManager::get().createBoxShape(
	            PxVec3(
                    shapeDesc.box.x*shapeDesc.scale+shapeDesc.skin,
                    shapeDesc.box.y*shapeDesc.scale+shapeDesc.skin,
                    shapeDesc.box.z*shapeDesc.scale+shapeDesc.skin),
                    materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution
            );
            break;
        case shape_t::SPHERE:
            shape = PhysicsManager::get().createSphereShape(
	            shapeDesc.sphere.radius*shapeDesc.scale+shapeDesc.skin,
                materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution
            );
            break;

		case shape_t::CAPSULE:
			shape = PhysicsManager::get().createCapsule(
				shapeDesc.capsule.height*shapeDesc.scale+shapeDesc.skin,
                shapeDesc.capsule.radius*shapeDesc.scale+shapeDesc.skin,
                materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution
				);
            break;
        case shape_t::TRIANGLE_MESH: {
                shape = PhysicsManager::get().createTriangleMesh(
				    shapeDesc.triangleMesh.mesh,
                    materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution,
                    shapeDesc.scale
                    );
            }
            break;
	}
    assert(shape != nullptr);
    if (shapeDesc.setLocalPose) {
        shape->setLocalPose(toPxTransform(getPose().getWorld()));
    }
    updateFilters();
}

void Shape::setBox(XMVECTOR size)
{
    shapeDesc.type = shape_t::BOX;
    shapeDesc.box.x = XMVectorGetX(size);
    shapeDesc.box.y = XMVectorGetY(size);
    shapeDesc.box.z = XMVectorGetZ(size);
}

void Shape::setSphere(float radius)
{
    shapeDesc.type = shape_t::SPHERE;
    shapeDesc.sphere.radius = radius;
}

void Shape::setCapsule(float radius, float height)
{
	shapeDesc.type = shape_t::CAPSULE;
	shapeDesc.capsule.radius = radius;
	shapeDesc.capsule.height = height;
}

void Shape::setTriangleMesh(PxTriangleMesh *triangleMesh)
{
	shapeDesc.type = shape_t::TRIANGLE_MESH;
	shapeDesc.triangleMesh.mesh = triangleMesh;
}

void Shape::setParticlesSystem(float grid, float maxmotion, float restoffset,
	float contactoffset, float damping, float restitution,
	float dynamic){
	shapeDesc.type = shape_t::PARTICLES_SYSTEM;
	shapeDesc.particles_system.gridsize = grid;
	shapeDesc.particles_system.maxMotion = maxmotion;
	shapeDesc.particles_system.restOffset = restoffset;
	shapeDesc.particles_system.contactOffset = contactoffset;
	shapeDesc.particles_system.damping = damping;
	shapeDesc.particles_system.gridsize = restitution;
	shapeDesc.particles_system.dynamic = dynamic;

}

void Shape::setFluid(float grid, float maxmotion, float restoffset,
	float contactoffset, float damping, float restitution,
	float dynamic, float stiffness, float viscosity){
	shapeDesc.type = shape_t::FLUID;
	shapeDesc.particles_fluids.gridsize = grid;
	shapeDesc.particles_fluids.maxMotion = maxmotion;
	shapeDesc.particles_fluids.restOffset = restoffset;
	shapeDesc.particles_fluids.contactOffset = contactoffset;
	shapeDesc.particles_fluids.damping = damping;
	shapeDesc.particles_fluids.gridsize = restitution;
	shapeDesc.particles_fluids.dynamic = dynamic;
	shapeDesc.particles_fluids.stiffness = stiffness; 
	shapeDesc.particles_fluids.viscosity = viscosity;
}

void Shape::setCCD(bool b)
{
    filter_t f(filter);
    if(b) {f.has = filter_t::traits_t(f.has | filter_t::CCD);}
    else {f.has = filter_t::traits_t(f.has & ~filter_t::CCD);}
    filter = f;
}
void Shape::setTrigger(bool b)
{
    filter_t f(filter);
    if(b) {f.has = filter_t::traits_t(f.has | filter_t::TRIGGER);}
    else {f.has = filter_t::traits_t(f.has & ~filter_t::TRIGGER);}
    filter = f;
}
void Shape::setStatic(bool b)
{
    filter_t f(filter);
    if(b) {f.has = filter_t::traits_t(f.has | filter_t::STATIC);}
    else {f.has = filter_t::traits_t(f.has & ~filter_t::STATIC);}
    filter = f;
}
void Shape::setCCT(bool b)
{
    filter_t f(filter);
    if(b) {f.has = filter_t::traits_t(f.has | filter_t::CCT);}
    else {f.has = filter_t::traits_t(f.has & ~filter_t::CCT);}
    filter = f;
}

void Shape::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    std::string shapeType = atts.getString("type", "<none>");
    if (shapeType == "box") {
        setBox(atts.getPoint("size"));
    } else if (shapeType == "sphere") {
        setSphere(atts.getFloat("radius", 1e-2f));
    } else if (shapeType == "capsule") {
        setCapsule(atts.getFloat("radius", 1e-2f), atts.getFloat("height", 0));
    } else if (shapeType == "triangleMesh") {
        setTriangleMesh(CollisionMeshLoader::load(atts.getString("mesh").c_str()));
	} else if (shapeType == "particles_system"){
		setParticlesSystem(
			atts.getFloat("gridsize",3.0f),
			atts.getFloat("maxmotion", 0.43f),
			atts.getFloat("restoffset", 0.0143f),
			atts.getFloat("contactoffset", 0.0143f * 2),
			atts.getFloat("damping", 0.0f),
			atts.getFloat("restitution", 0.2f),
			atts.getFloat("dynamicfriction", 3.f)
			);
	}else if (shapeType == "fluids"){
		setFluid(
			atts.getFloat("gridsize", 3.0f),
			atts.getFloat("maxmotion", 0.43f),
			atts.getFloat("restoffset", 0.0143f),
			atts.getFloat("contactoffset", 0.0143f * 2),
			atts.getFloat("damping", 0.0f),
			atts.getFloat("restitution", 0.2f),
			atts.getFloat("dynamicfriction", 3.f),
			atts.getFloat("stiffness", 3.f), // 1- 200
			atts.getFloat("viscosity", 5.f) // 5 - 300
			);
	}

    shapeDesc.skin = atts.getFloat("skin", shapeDesc.skin);
    shapeDesc.scale = atts.getFloat("scale", shapeDesc.scale);
    
    materialDesc.sFriction = atts.getFloat("sFriction", materialDesc.sFriction);
    materialDesc.dFriction = atts.getFloat("dFriction", materialDesc.dFriction);
    materialDesc.restitution = atts.getFloat("restitution", materialDesc.restitution);
    
    filter_t f(filter);
    f |= filter_t::getFromStrings(atts.getString("is"),
        atts.getString("supress"), atts.getString("report"));
    f &= ~filter_t::getFromStrings(atts.getString("is_not"),
        atts.getString("supress_not"), atts.getString("report_not"));
    filter = f;

    if (atts.getBool("ccd")) {setCCD();}
    updateFilters();
        
    #ifdef _DEBUG
        visualization = atts.getBool("visualization", visualization);
    #endif
        
    if(atts.has("pos")){pose.setPosition(atts.getPoint("pos"));}
    if(atts.has("rot")){pose.setRotation(atts.getQuat("rot"));}
    shapeDesc.setLocalPose = atts.getBool("setLocalPose", shapeDesc.setLocalPose);
}

void Shape::removeFilters(filter_t::id_t is, filter_t::id_t supress, filter_t::id_t report)
{
    filter = filter_t(filter) & ~filter_t(is, supress, report);
    updateFilters();
}

void Shape::setFilters(filter_t::id_t is, filter_t::id_t supress, filter_t::id_t report)
{
    filter = filter_t(filter) | filter_t(is, supress, report);
    updateFilters();
}

void Shape::updateFilters()
{
    if (shape != nullptr) {
	    shape->setSimulationFilterData(filter);
	    shape->setQueryFilterData(filter);

        // Triggers aren't included in CCD! The filterShader will know (filter_t::has)
        // but disabling both flags makes the shape collide anyway (So no CCD for triggers)
        
        if ((getFilter().has & filter_t::TRIGGER) == 0) {
            shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        } else {
            shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
            shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true); 
        }
        #ifdef _DEBUG
            if (visualization) {
                shape->setFlag(PxShapeFlag::eVISUALIZATION, true);
            } else {
                shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
            }
        #endif

    }
}

CRigidBody::~CRigidBody() {
	if (rigidBody != nullptr) {
        Physics.gScene->removeActor(*rigidBody);
        rigidBody->release();
        rigidBody = nullptr;
    }
}

CRigidBody::CRigidBody(CRigidBody&& move) :
    temp_mass(move.temp_mass),
    temp_density(move.temp_density),
    temp_is_kinematic(move.temp_is_kinematic),
    temp_use_gravity(move.temp_use_gravity),
    temp_linear_damping(move.temp_linear_damping),
    rigidBody(move.rigidBody)
{
    move.rigidBody = nullptr;
    setTrigger(false);
}

void CRigidBody::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (elem == "RigidBody") {
	    temp_mass = atts.getFloat("mass", temp_mass);
	    temp_use_gravity = atts.getBool("gravity", temp_use_gravity);
	    temp_is_kinematic = atts.getBool("kinematic", temp_is_kinematic);
	    temp_density = atts.getFloat("density", temp_density);
	    temp_linear_damping = atts.getFloat("damping", temp_linear_damping);
	    temp_angular_damping = atts.getFloat("angular_damping", temp_angular_damping);
    } else if (elem == "shape") {
        Shape::loadFromProperties(elem, atts);
    }
}

void CRigidBody::updateBodyProperties()
{
    setMass(temp_mass);
	rigidBody->setLinearDamping(temp_linear_damping);
	rigidBody->setAngularDamping(temp_angular_damping);
    rigidBody->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, Shape::hasCCD());
    setKinematic(temp_is_kinematic);
    setUseGravity(temp_use_gravity);
}

void CRigidBody::createRigidBody()
{
	Entity* e = Handle(this).getOwner();
	CTransform * t = e->get<CTransform>();
    assert(t != nullptr);
    
    if (shape == nullptr) {
        Shape::createShape();
    }
    assert(shape != nullptr);

    if (temp_is_kinematic) {
	    rigidBody = PxCreateKinematic(
		    *Physics.gPhysicsSDK,
		    PxTransform(
                toPxVec3(t->getPosition()),
                toPxQuat((t->getRotation() == zero_v) ? XMQuaternionIdentity() : t->getRotation())),
		    *shape, temp_density);
    } else {
	    rigidBody = PxCreateDynamic(
		    *Physics.gPhysicsSDK,
		    PxTransform(toPxVec3(t->getPosition()), toPxQuat((t->getRotation() == zero_v) ? XMQuaternionIdentity() : t->getRotation())),
		    *shape, temp_density);
    }
    updateBodyProperties();
#ifdef _DEBUG
    std::string name = e->getName();
    PHYSX_SET_NAME(rigidBody, name+(temp_is_kinematic?".KYN":".DYN"));
    PHYSX_SET_NAME(shape, name+".collider");
#endif

	Physics.gScene->addActor(*rigidBody);
	setUseGravity(temp_use_gravity);
}

CStaticBody::~CStaticBody()
{
	if (staticBody != nullptr) {
        Physics.gScene->removeActor(*staticBody);
        staticBody->release();
        staticBody = nullptr;
    }
}

void CStaticBody::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (elem == "StaticBody") {
        /*nothing*/
    } else if (elem == "shape") {
        Shape::loadFromProperties(elem, atts);
    }
}

void CStaticBody::createStaticBody()
{
	Entity* e = Handle(this).getOwner();
	CTransform* t = e->get<CTransform>();
	assert(t!=nullptr);

    if (shape == nullptr) {
        Shape::createShape();
    }
    assert(shape != nullptr);

	staticBody = PxCreateStatic(
        *Physics.gPhysicsSDK, PxTransform(
			toPxVec3(t->getPosition()),
			toPxQuat(t->getRotation())),
        *shape);
    
#ifdef _DEBUG
    std::string name = e->getName();
    PHYSX_SET_NAME(staticBody, name+".STA");
    PHYSX_SET_NAME(shape, name+".collider");
#endif
	Physics.gScene->addActor(*staticBody);
}

CCharacterController::~CCharacterController()
{
	if (controller != nullptr) {
		controller->release();
		controller = nullptr;
	}
}

void CCharacterController::createController(
    XMVECTOR position, PxControllerDesc* desc,
	float contactOffset, float climb, float slopeLimit,
    float maxjumpheight,
	PxMaterial* material)
{
    assert(controller == nullptr);

	//initial position
	desc->position = toPxExtendedVec3(position);
	//controller skin within which contacts generated
	desc->contactOffset = contactOffset;
	desc->stepOffset = climb;
	desc->maxJumpHeight = maxjumpheight;

	desc->slopeLimit = std::cosf(deg2rad(slopeLimit));
	desc->upDirection = toPxVec3(utils::yAxis_v);
    desc->nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	desc->material = material;
	desc->reportCallback = &Physics;
	desc->density = 1e-6f;
        
    controllerFilters = PxControllerFilters();
    controllerFilters.mFilterData = &(Shape::filter);
    controllerFilters.mFilterCallback = &Physics;
    controllerFilters.mCCTFilterCallback = &Physics;
    controllerFilters.mFilterFlags = PxQueryFlag::ePREFILTER|PxQueryFlag::eSTATIC|PxQueryFlag::eDYNAMIC;

    desc->behaviorCallback = &Physics;

	assert(desc->isValid());

	controller = Physics.manager->createController(*desc);
    assert(controller != nullptr);
	PxRigidDynamic* actor = controller->getActor();
    
    assert(actor != nullptr);
	moved = toPxVec3(zero_v);
    
#ifdef _DEBUG
    std::string name = ((Entity*) Handle(this).getOwner())->getName();
    PHYSX_SET_NAME(actor, name+".CCT");
#endif
	if (actor->getNbShapes() > 0) {
        PxShape* nShape;
	    actor->getShapes(&nShape, 1);
#ifdef _DEBUG
        PHYSX_SET_NAME(nShape, name+".collider");
#endif
        Shape::setShape(nShape);
	}
}

void CCharacterController::createBoxController(XMVECTOR position,
	float contactOffset, float climb, float slopeLimit, PxVec3 size,
    float maxJumpHeight,
	PxMaterial* material)
{
	PxBoxControllerDesc desc;
	PxVec3 halfSize = size / 2;
	desc.halfForwardExtent = halfSize.z;
	desc.halfHeight = halfSize.y;
	desc.halfSideExtent = halfSize.x;
    createController(position, &desc, contactOffset, climb, slopeLimit, maxJumpHeight, material);
    Shape::setBox(toXMVECTOR(size));
}

void CCharacterController::createCapsuleController(XMVECTOR position,
    float contactOffset, float climb, float slopeLimit,
    float radius, float height, float maxJumpHeight,
    PxMaterial* material)
{
    PxCapsuleControllerDesc desc; // character controller for each character in game
	desc.radius = radius; //radius of the capsule
	desc.height = height; //height of the controller
    createController(position, &desc, contactOffset, climb, slopeLimit, maxJumpHeight, material);
    Shape::setCapsule(radius, height);
}

void CCharacterController::teleport(const XMVECTOR& position, bool checked)
{
    XMVECTOR pos = position + getFootOffset();
    bool tp = true;
    if (checked) {
        PxShape* shape;
        controller->getActor()->getShapes(&shape,1);
        PxTransform transform = controller->getActor()->getGlobalPose();
        transform.p = toPxVec3(pos);
        if (Physics.overlap(shape, transform)) {
            tp = false;
        }
    }
    if (tp) {
        controller->setPosition(toPxExtendedVec3(pos));
        CTransform* transform(Handle(this).getBrother<CTransform>());
        transform->setPosition(pos);
    }
}

void CCharacterController::setSimulationEnabled(bool b)
{
	controller->getActor()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !b);
}

void CCharacterController::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{   
    if (elem == "CharacterController") {
        charCoDesc.contactOffset = atts.getFloat("contactOffset", charCoDesc.contactOffset);
        charCoDesc.climb         = atts.getFloat("climb", charCoDesc.climb);
        charCoDesc.slopeLimit    = atts.getFloat("slopeLimit", charCoDesc.slopeLimit);
        charCoDesc.maxJumpHeight = atts.getFloat("maxJumpHeight", charCoDesc.maxJumpHeight);
    } else if (elem == "shape") {
        Shape::loadFromProperties(elem, atts);
    }
}
    
void CCharacterController::createController()
{
    Handle h(this);
    CTransform* t = h.getBrother<CTransform>();
    pos = toPxExtendedVec3(t->getPosition());

    auto type = shapeDesc.type;
    assert(type != shape_t::TRIANGLE_MESH && type != shape_t::NONE);
    switch(type) {
        case shape_t::BOX:
            createBoxController(t->getPosition(), charCoDesc.contactOffset,
                charCoDesc.climb, charCoDesc.slopeLimit,
                physx::PxVec3(shapeDesc.box.x, shapeDesc.box.y, shapeDesc.box.z),
                charCoDesc.maxJumpHeight,
                Physics.gPhysicsSDK->createMaterial(
                    materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution));
            break;
        case shape_t::CAPSULE:
            createCapsuleController(t->getPosition(), charCoDesc.contactOffset,
                charCoDesc.climb, charCoDesc.slopeLimit,
                shapeDesc.capsule.radius, shapeDesc.capsule.height,
                charCoDesc.maxJumpHeight,
                Physics.gPhysicsSDK->createMaterial(
                    materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution));
            break;
        case shape_t::SPHERE:
            createCapsuleController(t->getPosition(), charCoDesc.contactOffset,
                charCoDesc.climb, charCoDesc.slopeLimit,
                shapeDesc.sphere.radius, shapeDesc.sphere.radius*2,
                charCoDesc.maxJumpHeight,
                Physics.gPhysicsSDK->createMaterial(
                    materialDesc.sFriction, materialDesc.dFriction, materialDesc.restitution));
            break;
        default: assert(false);
    }
    assert(controller != nullptr);
    controller->setPosition(pos);
}

void CCharacterController::update(float elapsed)
{
	Handle h(this);
	CTransform* t = h.getBrother<CTransform>();
	t->setPosition(toXMVECTOR(pos));

	if (enableTimer){
		if (counter.count(elapsed) >= limitTimer) {
			setSimulationEnabled(true);
			counter.reset();
			enableTimer = false;
			limitTimer = 0.0;
		}
	}
}

void CStaticBody::init()
{
    if (staticBody == nullptr) {
        createStaticBody();
    }
    setUserData(Handle(this).getOwner());
}
void CCharacterController::init()
{
    if(controller == nullptr) {
        createController();
    }
    setUserData(Handle(this).getOwner());
}
void CRigidBody::init()
{
    if(rigidBody == nullptr) {
        createRigidBody();
    }
    setUserData(Handle(this).getOwner());
}
void CTrigger::init()
{
    if(shape == nullptr) {
        setupTrigger();
    }
    setUserData(Handle(this).getOwner());
}

void CTrigger::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{   
    if (elem == "Trigger") {
        /*Nothing as of yet*/
    } else if (elem == "shape") {
        Shape::loadFromProperties(elem, atts);
    }
}

void CTrigger::setupTrigger()
{
    if (shape == nullptr) {
        Shape::createShape();
    }
    assert(shape != nullptr);
    Entity* e(Handle(this).getOwner());
#ifdef _DEBUG
    std::string name = e->getName();
    PHYSX_SET_NAME(shape, name+".trigger");
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
        shape->setLocalPose(body->getShape()->getLocalPose());
        body->getCharacterController()->getActor()->attachShape(*shape);
    } else {
        assert(actor == nullptr);
	    CTransform* t = e->get<CTransform>();
	    assert(t!=nullptr);

	    actor = PxCreateStatic(
            *Physics.gPhysicsSDK, PxTransform(
			    toPxVec3(t->getPosition()),
			    toPxQuat(t->getRotation())),
            *shape);
    
        #ifdef _DEBUG
            PHYSX_SET_NAME(actor, name+".ACTOR");
        #endif
	    Physics.gScene->addActor(*actor);
    }
}

Shape* CExtraShapes::getActorShape()
{
    Entity* e(Handle(this).getOwner());
    if(e->has<CStaticBody>()) {
        CStaticBody* s = e->get<CStaticBody>();
        return s;
    } else if(e->has<CRigidBody>()) {
        CRigidBody* s = e->get<CRigidBody>();
        return s;
    } else if(e->has<CCharacterController>()) {
        CCharacterController* s = e->get<CCharacterController>();
        return s;
    } else if(e->has<CTrigger>()) {
        CTrigger* s = e->get<CTrigger>();
        return s;
    } else {
        return nullptr;
    }
}

PxRigidActor* CExtraShapes::getActor()
{
    Entity* e(Handle(this).getOwner());
    if(e->has<CStaticBody>()) {
        CStaticBody* body = e->get<CStaticBody>();
        return body->getStaticBody();
    } else if(e->has<CRigidBody>()) {
        CRigidBody* body = e->get<CRigidBody>();
        return body->getRigidBody();
    } else if(e->has<CCharacterController>()) {
        CCharacterController* body = e->get<CCharacterController>();
        return body->getCharacterController()->getActor();
    } else if(e->has<CTrigger>()) {
        CTrigger* t = e->get<CTrigger>();
        return t->getActor();
    } else {
        return nullptr;
    }
}

void CExtraShapes::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (elem == "shape") {
        assert(nShapes < MAX_SHAPES);
        shapes[nShapes] = new Shape;
        shapes[nShapes]->loadFromProperties(elem, atts);
        nShapes++;
    }
}

void CExtraShapes::attachShapes()
{
    auto actor = getActor();
    auto h = Handle(this).getOwner();
    for(unsigned i = 0; i < MAX_SHAPES; ++i) {
        auto& s = shapes[i];
        if (s != nullptr) {
            auto shape = s->getShape();
            if (shape == nullptr) {
                s->createShape();
                shape = s->getShape();
                #ifdef _DEBUG
                    Entity* e(h);
                    std::string name = e->getName();
                    PHYSX_SET_NAME(shape, name+".shape"+std::to_string(i));
                #endif
                s->setUserData(h);
            }
            if (shape != nullptr) {
                if (actor != nullptr) {
                    actor->attachShape(*shape);
                }
            }
        }
    }
}

}

#ifdef _DEBUG
_PxDbgStringManager _PxDbgStringManager::instance;
#endif