#ifndef COMPONENT_PHYSICS_H_
#define COMPONENT_PHYSICS_H_

#include "handles/message.h"
#include "handles/handle.h"
#include "handles/importXML.h"
#include "handles/entity.h"
#include "handles/objectManager.h"
#include "components/transform.h"
#include "PhysicsManager.h"
#include "render/mesh/component.h"
#include "filterShader.h"

using namespace render;

#define Physics physX_user::PhysicsManager::get()

using namespace DirectX;
using namespace utils;
using namespace physx;
using namespace component;



namespace physX_user {

class Shape {
    public:
        struct shape_t {
            enum type_e {
                NONE,
                BOX,
                SPHERE,
				CAPSULE,
                TRIANGLE_MESH,
				PARTICLES_SYSTEM,
				FLUID
            } type;
            union {
                struct {
                    PxReal x,y,z;
                } box;
                struct {
                    PxReal radius;
                } sphere;

				struct {
					PxReal radius;
					PxReal height;
				} capsule;
                struct {
                    PxTriangleMesh* mesh;
                } triangleMesh;
				struct {
					PxReal gridsize;
					PxReal maxMotion;
					PxReal restOffset;
					PxReal contactOffset;
					PxReal damping;
					PxReal restution;
					PxReal friction;
					PxReal dynamic;

				} particles_system;
				struct {
					PxReal gridsize;
					PxReal maxMotion;
					PxReal restOffset;
					PxReal contactOffset;
					PxReal damping;
					PxReal restution;
					PxReal friction;
					PxReal dynamic;
					PxReal stiffness;
					PxReal viscosity;
				} particles_fluids;

            };
            float skin = 0.f;
            float scale = 1.f;
            bool setLocalPose = true;

            shape_t(): type(NONE) {box.x=0.1f; box.y=0.1f; box.z=0.1f;}
        };
    private:
        static void setupShapeFlags(PxShape* shape,
            PxU32 w0, PxU32 w1, PxU32 w2,
            const Handle& h, bool simulation);

    private:
        void updateFilters();

    protected:
	    PxShape* shape = nullptr;
        component::Transform pose;

        PxFilterData filter;
#ifdef _DEBUG
        bool visualization = true;
#endif
        
		shape_t shapeDesc;
        struct material_t {
            PxReal sFriction=0.5f, dFriction=0.5f, restitution=0.5f;
        } materialDesc;


    protected:


        inline bool hasCCD() const {return (getFilter().has & filter_t::CCD) != 0;}
        inline bool isTrigger() const {return (getFilter().has & filter_t::TRIGGER) != 0;}
        inline bool isStatic() const {return (getFilter().has & filter_t::STATIC) != 0;}
        inline bool isCCT() const {return (getFilter().has & filter_t::CCT) != 0;}
        void setCCD(bool b=true);
        void setTrigger(bool b=true);
        void setStatic(bool b=true);
        void setCCT(bool b=true);

    public:
        Shape(bool trigger=false, bool staticShape=false, bool cct=false) {
            setTrigger(trigger);  
            setStatic(staticShape);
            setCCT(cct);
        }
        Shape(Shape&& move) :
            shape(move.shape),
            materialDesc(move.materialDesc),
            shapeDesc(move.shapeDesc)
            { move.shape = nullptr;}

	    void createShape();
	    inline PxShape* getShape() const {return shape;}
        inline void setShape(PxShape *s) { shape = s; updateFilters(); }
        
        inline shape_t getShapeDesc() const {return shapeDesc;}
        inline void setShapeDesc(shape_t shape) {shapeDesc = shape;}
        void setBox(XMVECTOR size);
        void setSphere(float radius);
        void setCapsule(float radius, float height);
        void setTriangleMesh(PxTriangleMesh *triangleMesh);

		
		void setParticlesSystem(float grid,float maxmotion, float restoffset, 
								float contactoffset, float damping, float restitution,
								float dynamic);

		void setFluid(float grid, float maxmotion, float restoffset,
					  float contactoffset, float damping, float restitution,
					  float dynamic, float stiffness, float viscosity);

	    void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

        inline filter_t getFilter() const {return filter;}
        void setFilters(filter_t::id_t is, filter_t::id_t supress= filter_t::NONE,
            filter_t::id_t reports= filter_t::NONE);
        inline void setFilters(const filter_t& f) {setFilters(f.is, f.supress, f.report);}
        void removeFilters(filter_t::id_t is, filter_t::id_t supress= filter_t::NONE,
            filter_t::id_t reports= filter_t::NONE);
        inline void removeFilters(const filter_t& f) {removeFilters(f.is, f.supress, f.report);}

        void setUserData(component::Handle h);

		inline component::Transform getPose() const { return pose; }
		inline void setPose(const component::Transform& t) { pose.set(t); }
};


class CRigidBody : public Shape {
	private:
		PxRigidDynamic* rigidBody = nullptr;

		// Just for rigidBody creation
		float temp_mass = 0.f;
		float temp_density = 1.f;
		bool temp_is_kinematic = false;
		bool temp_use_gravity = true;
		float temp_linear_damping = 0.f;
        float temp_angular_damping = 0.05f;

	public:
        CRigidBody() : Shape(false, false, false) {}
        CRigidBody(CRigidBody&& move);
        ~CRigidBody();

		inline PxRigidDynamic* getRigidBody() {
            if (rigidBody == nullptr) {createRigidBody();}
            return rigidBody;
        }

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

        void init();
        void createRigidBody();

		void fixedUpdate(float elapsed) {
			Entity* e = Handle(this).getOwner();
            if (e) {
			    CTransform * t = e->get<CTransform>();
                if (t) {
                    if (rigidBody->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC) {
                        rigidBody->setKinematicTarget(toPxTransform(t->getWorld()));
                    } else {
			            t->setPosition(toXMVECTOR(rigidBody->getGlobalPose().p));
			            t->setRotation(toXMQuaternion(rigidBody->getGlobalPose().q));
                    }
                }
            }
		}

		inline void setKinematic(bool is_kinematic) {
            assert(rigidBody != nullptr);
			rigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, is_kinematic);
		}

		inline bool isKinematic() {
            assert(rigidBody != nullptr);
			return rigidBody->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
		}

		inline void setUseGravity(bool use_gravity) {
            assert(rigidBody != nullptr);
			rigidBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !use_gravity);
		}

		inline void setSimulationEnabled(bool b) {
            assert(rigidBody != nullptr);
			rigidBody->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !b);
		}

		inline bool isUsingGravity() {
            assert(rigidBody != nullptr);
			return rigidBody->getActorFlags().isSet(PxActorFlag::eDISABLE_GRAVITY);
		}

		inline void update(float elapsed) {}

		inline void setMass(float kg){rigidBody->setMass(kg);}

		void addVelocityObject(XMVECTOR force, XMVECTOR rotationTarget, XMVECTOR potition){
			PhysicsManager::get().addVelocityObject(*rigidBody, force, rotationTarget, potition);
		}

		inline XMVECTOR getVelocityObject(XMVECTOR potition){
			return PhysicsManager::get().getVelocityObject(*rigidBody, potition);
		}

		inline void addForceAtObject(XMVECTOR force, XMVECTOR potition){
			PhysicsManager::get().addForceExternal(*rigidBody, force, potition);
		}

        void updateBodyProperties();

        using Shape::setBox;
        using Shape::setCapsule;
        using Shape::setSphere;
        using Shape::setTriangleMesh;
        using Shape::setShapeDesc;
        using Shape::getShapeDesc;
        using Shape::setFilters;
        using Shape::getFilter;
        using Shape::removeFilters;
        using Shape::getShape;
        using Shape::setShape;
};

class CStaticBody : public Shape {

	private:
		PxRigidStatic* staticBody = nullptr;

	public:
        CStaticBody() : Shape(false, true, false) {}
        CStaticBody(CStaticBody&& move) : staticBody(move.staticBody)
            { move.staticBody = nullptr; setTrigger(false);}

		~CStaticBody();
        
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed) {}
		void fixedUpdate(float elapsed) {}

		inline PxRigidStatic* getStaticBody() {
            if (staticBody == nullptr) {createStaticBody();}
            return staticBody;
        }

		void init();
        void createStaticBody();

		inline void setSimulationEnabled(bool b)
            {staticBody->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !b);}

        using Shape::setBox;
        using Shape::setCapsule;
        using Shape::setSphere;
        using Shape::setTriangleMesh;
        using Shape::setShapeDesc;
        using Shape::getShapeDesc;
        using Shape::setFilters;
        using Shape::getFilter;
        using Shape::removeFilters;
        using Shape::getShape;
        using Shape::setShape;
};

class CCharacterController : public Shape {
	private:
		PxController*       controller = nullptr;
        PxControllerFilters controllerFilters;
		PxVec3		        moved = PxVec3(0, 0, 0);
		PxExtendedVec3      pos = PxExtendedVec3(0, 0, 0);
		uint32_t            collisionFlags = 0;

		float				limitTimer = 0.0f;
		bool				enableTimer = false;
		Counter<float>		counter;

        PxRigidActor*       master = nullptr;

        struct charCoDesc_t {
            float contactOffset = 0.01f;
            float climb         = 0.f;
            float slopeLimit    = 90.f; // angle of slope in degrees
            float maxJumpHeight = 0.f;
        } charCoDesc;

    private:
        void createController(
            XMVECTOR position, PxControllerDesc* desc,
	        float contactOffset, float climb, float slopeLimit, float maxjumpheight,
	        PxMaterial* material = Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f));
		void createBoxController(XMVECTOR position,
			float contactOffset, float climb, float slopeLimit, PxVec3 size, float maxjumpheight,
			PxMaterial* material = Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f));

		void createCapsuleController(XMVECTOR potition,
            float contactOffset, float climb, float slopeLimit,
            float radius, float height, float maxjumpheight,
            PxMaterial* material = Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f));

	public:
		CCharacterController() : Shape(false, false, true) {}
		CCharacterController(CCharacterController&& move) :
            controller(move.controller), moved(move.moved), pos(move.pos)
            {move.controller = nullptr; setTrigger(false);}
		~CCharacterController();

		inline PxController* getCharacterController(){ return controller; }


        void teleport(const XMVECTOR& position, bool checked=true);

		inline void setDisplacement(XMVECTOR movement){
			moved += toPxVec3(movement);
		}

        inline void enslave(PxRigidActor*const& nMaster) {
            assert(nMaster != nullptr);
            master = nMaster;
        }
        inline PxRigidActor* getMaster() const {return master;}
        inline void liberate() {
            master = nullptr;
            controller->setUpDirection(PxQuat(M_PI_2f, PxVec3(0,0,1)).rotate(PxVec3(1,0,0)));
        }

        inline void move(XMVECTOR movement, float elapsed) {
            controllerFilters.mFilterData = &(Shape::filter);
			collisionFlags = controller->move(toPxVec3(movement), 0.01f, elapsed, controllerFilters);
	        pos = controller->getFootPosition();
        }

		inline XMVECTOR getDisplacement(){
			return toXMVECTOR(moved);
		}

		inline void fixedUpdate(float elapsed) {
            controllerFilters.mFilterData = &(Shape::filter);
            if (master != nullptr) {
                auto globalPose(master->getGlobalPose());
                controller->setUpDirection(globalPose.q.rotate(PxVec3(0,1,0)));
                auto p (globalPose.p);
                controller->setPosition(PxExtendedVec3(p.x, p.y, p.z));
            } else {
			    collisionFlags = controller->move(moved, 0.01f, elapsed, controllerFilters);
            }
	        pos = controller->getFootPosition();
            moved = toPxVec3(zero_v);
		}

		inline bool onGround() const {
			return (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) != 0;
		}
        inline bool onWall() const {
			return (collisionFlags & PxControllerCollisionFlag::eCOLLISION_SIDES) != 0;
		}
        inline bool onCeiling() {
			return (collisionFlags & PxControllerCollisionFlag::eCOLLISION_UP) != 0;
        }

		inline void setSimulationEnabled(bool b);

		inline void setSimulationByTimer(bool b, float limit) {
			enableTimer = b;
			limitTimer = limit;
		}

        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

        void init();
        void createController();
        void update(float elapsed);

        using Shape::setBox;
        using Shape::setCapsule;
        using Shape::setSphere;
        using Shape::removeFilters;
        using Shape::getShape;
        using Shape::getFilter;
        using Shape::setFilters;
        using Shape::setShape;
        using Shape::setShapeDesc;
        using Shape::getShapeDesc;

	    inline PxTransform getShapePose() const {
            auto ret = controller->getActor()->getGlobalPose();
            ret.q *= PxQuat(M_PI_2f, PxVec3(0,0,1));
            return ret;
        }

        inline XMVECTOR getFootOffset() {
            assert(controller!= nullptr);
            return toXMVECTOR(controller->getPosition() - controller->getFootPosition());
        }
};

class CTrigger : public Shape {
    private:
        PxRigidActor* actor = nullptr;
	public:
        ~CTrigger() {
            if (actor != nullptr) {
                if (actor->isReleasable()) {actor->release();}
                actor = nullptr;
            }
        }
        CTrigger() : Shape(true, false, false) {}
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        void init();

        void setupTrigger();

		inline void update(float elapsed) {}
		void fixedUpdate(float elapsed) {}

        inline PxRigidActor* getActor() const {return actor;}
        
        using Shape::setShapeDesc;
        using Shape::getShapeDesc;
        using Shape::setBox;
        using Shape::setCapsule;
        using Shape::setSphere;
        using Shape::setFilters;
        using Shape::getFilter;
        using Shape::removeFilters;
        using Shape::getShape;
        using Shape::setShape;
};

class CExtraShapes {
    public:
    static const unsigned MAX_SHAPES = 8;
    private:
        Shape* shapes[MAX_SHAPES];
        component::Transform transforms[MAX_SHAPES];
        unsigned nShapes = 0;
	public:
        CExtraShapes(){for(auto& a:shapes){a=nullptr;}}

        ~CExtraShapes() {
            for(auto& s:shapes){
                if(s!=nullptr) {
                    auto shape = s->getShape();
                    if (shape != nullptr) {
                        auto actor = shape->getActor();
                        if (actor != nullptr) {
                            shape->getActor()->detachShape(*shape);
                        }
                    }
                }
            }
        }
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        void init() { attachShapes();}
        void attachShapes();
        PxRigidActor* getActor();
        Shape* getActorShape();

		inline void update(float elapsed) {}
		void fixedUpdate(float elapsed) {}
};

}

#ifdef _DEBUG
class _PxDbgStringManager {
    private:
        static _PxDbgStringManager instance;

    private:
        _PxDbgStringManager() = default;
        std::vector<std::string*> strs;
        const char* operator()(const std::string& str) {
            auto nstr = new std::string(str);
            strs.push_back(nstr);
            return nstr->c_str();
        }
    public:
        ~_PxDbgStringManager() {for(auto& str : strs) {delete str;} strs.clear(); strs.resize(0);}
        static inline const char* store(std::string str) {return instance(str);}
};
#define PHYSX_SET_NAME(obj, str) ((obj)->setName(_PxDbgStringManager::store(str)))
#else
#define PHYSX_SET_NAME(obj, str)
#endif

#endif
