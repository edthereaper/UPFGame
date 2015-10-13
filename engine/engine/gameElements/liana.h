#ifndef __GAMEELEMENTS_LIANA_H__
#define __GAMEELEMENTS_LIANA_H__

#include "mcv_platform.h"
#include "gameMsgs.h"

#include "handles/entity.h"
#include "components/Transform.h"

#include "../PhysX_USER/pxcomponents.h"

//#define DEBUG_LIANA
//#define DEBUG_LIANA_CONTROL_LINK
//#define DEBUG_LIANA_FORCES
//#define DEBUG_LIANA_DAMPING

namespace gameElements {

class CLiana {
    public:
        static const unsigned TAG = 0x07A6209E;

        static const unsigned MAX_LINKS = 40;
        static_assert(MAX_LINKS*4 < 256,
            "setSolverIterationCounts could receive a number greater than 256!");
        
        static const float LINK_LENGTH;
        static const float LINK_MASS;
        static const float LINK_MASS_LOOSE;
        static const float LINEAR_DAMP;
        static const float ANGULAR_DAMP;
        static const float FIXED_LINEAR_DAMP;
        static const float SPRING_DAMP;
        static const float SPRING_STIFF;
        static const float SPRING_DAMP_LOOSE;
        static const float SPRING_STIFF_LOOSE;

    private:
        static const float LOCAL_OFFSET;
        static const physx::PxQuat FRAME_ROT;
        static const physx::PxVec3 FRAME_SEP;

    public:
        static void initType();

    private:
        physX_user::Shape links[MAX_LINKS];
        physX_user::Shape linkTriggers[MAX_LINKS];
        physx::PxRigidDynamic* linkBodies[MAX_LINKS];
        physx::PxJoint* joints[MAX_LINKS];
#ifdef DEBUG_LIANA_FORCES
        physx::PxVec3 linkForces[MAX_LINKS];
        physx::PxVec3 linkBrakes[MAX_LINKS];
#endif

        physX_user::Shape::shape_t linkShapeDesc, linkTriggerShapeDesc;
        physX_user::filter_t linkFilter, linkTriggerFilter;
        unsigned nLinks = MAX_LINKS;
        int controlLink = -1;
        float limitX=deg2rad(60);
        float limitZ=deg2rad(60);
        enum {NONE, FIXED, D6} jointType = NONE;
    private:
        physx::PxShape* createLinkTrigger (physX_user::Shape& link, component::Handle me_h);
        physx::PxShape* createLinkCollider(physX_user::Shape& link, component::Handle me_h);
        void updateJointLimits(int lastNode);
        float getShapeLength() const;
        void createD6Joints();
        void createFixedJoints();

    public:
        CLiana();
        void init();
        void update(float f);
        void loadFromProperties(std::string elem, utils::MKeyValue& atts);

        inline XMVECTOR getLinkVelocity(unsigned n) {
            assert(n <= MAX_LINKS);
            return physX_user::toXMVECTOR(linkBodies[n]->getLinearVelocity());
        }

        inline void setNLinks(unsigned n) {
            assert(n>0 && n <= MAX_LINKS);
            nLinks=n;
            if(controlLink>static_cast<int>(nLinks)) {controlLink=n;}
        }
        inline unsigned getNLinks() const {return nLinks;}
        inline void setControlLink(unsigned n) {
            assert(n < nLinks);
            controlLink=n;
            updateJointLimits(controlLink);
        }
        inline unsigned getControlLink() const {return controlLink;}
        void resetControlLink();

        void setLimits(float x, float z) {
            limitX=x;
            limitZ=z;
            updateJointLimits(controlLink);
        }
        inline float getXLimit() const {return limitX;}
        inline float getZLimit() const {return limitZ;}
        inline float getSmallestLimit() const {return std::min(limitX, limitZ);}
        inline float getLargestLimit() const {return std::max(limitX, limitZ);}
        float getLimit(XMVECTOR dir) const;

        inline void setShapeDesc(physX_user::Shape::shape_t shape,
            bool colliders=true, bool triggers=true) {
            if (colliders) {linkShapeDesc = shape;}
            if (triggers) {linkTriggerShapeDesc = shape;}
        }
        void setCapsule(float radius, float height, bool colliders=true, bool triggers=true);
        void setShapeModifiers(float skin, float scale=1.0f, bool colliders=true, bool triggers=true);
        inline physX_user::Shape::shape_t getColliderShapeDesc() const {return linkShapeDesc;}
        inline physX_user::Shape::shape_t getTriggerShapeDesc() const {return linkTriggerShapeDesc;}

        void setFilters(physX_user::filter_t filter, bool colliders=true, bool triggers=true);
        void removeFilters(physX_user::filter_t filter, bool colliders=true, bool triggers=true);
        inline physX_user::filter_t getColliderFilters() const {return linkFilter;}
        inline physX_user::filter_t getTriggerFilters() const {return linkTriggerFilter;}

        int getNodeIndex(PxShape*);

        component::Transform getTransform(unsigned nodeIndex) const;

        /* localHeightRatio [-1, 1] => [bottom, top] */
        void applyForceToLink(unsigned nodeIndex,
            const XMVECTOR& force, float localHeightRatio);
        /* localHeightRatio [-1, 1] => [bottom, top] */
        inline void propagateForceUp(unsigned nodeIndex,
            const XMVECTOR& force, float localHeightRatio,
            float factor=0.f, float diff=0.001f) {
            propagateForce(nodeIndex, -1, force, localHeightRatio, factor, diff);
        }
        /* localHeightRatio [-1, 1] => [bottom, top] */
        inline void propagateForceDown(unsigned nodeIndex,
            const XMVECTOR& force, float localHeightRatio=0.f,
            float factor=1.f, float diff=0.001f) {
            propagateForce(nodeIndex, 1, force, localHeightRatio, factor, diff);
        }
        void propagateForce(unsigned nodeIndex, int step,
            const XMVECTOR& force, float localHeightRatio,
            float factor=0.f, float diff=0.001f);
        
        /* localHeightRatio [-1, 1] => [bottom, top] 
           distFactor multiplies the distance to the straight position
           returns wether the distance was greater than the tolerance
           */
        bool applyBrakeToLink(unsigned nodeIndex,
            float distFactor, float localHeightRatio=0.f,
            float aimOffsetRatio = 0.f, float tolerance=0.01f);
        /* localHeightRatio [-1, 1] => [bottom, top] 
           distFactor multiplies the distance to the straight position
           diff is substracted from factor */
        inline void propagateBrakeUp(unsigned nodeIndex,
            float distFactor, float localHeightRatio=0.f,
            float factor=1.f, float diff=0.001f,
            float aimOffsetRatio = 0.f, float offsetFactor = 1.f,
            float tolerance = 0.01f) {
            propagateBrake(nodeIndex, -1, distFactor, localHeightRatio,
                factor, diff, aimOffsetRatio, offsetFactor, tolerance);
        }
        /* localHeightRatio [-1, 1] => [bottom, top] 
           distFactor multiplies the distance to the straight position
           diff is substracted from factor */
        inline void propagateBrakeDown(unsigned nodeIndex,
            float distFactor, float localHeightRatio=0.f,
            float factor=1.f, float diff=0.001f,
            float aimOffsetRatio = 0.f, float offsetFactor = 1.f,
            float tolerance = 0.01f) {
            propagateBrake(nodeIndex, 1, distFactor, localHeightRatio,
                factor, diff, aimOffsetRatio, offsetFactor, tolerance);
        }
        /* localHeightRatio [-1, 1] => [bottom, top] 
           distFactor multiplies the distance to the straight position
           diff is substracted from factor */
        void propagateBrake(unsigned nodeIndex, int step,
            float distFactor, float localHeightRatio=0.f,
            float factor=1.f, float diff=0.001f,
            float aimOffsetRatio = 0.f, float offsetFactor = 1.f,
            float tolerance = 0.01f);

        XMVECTOR getLinkPosition(unsigned nodeIndex, float localHeightRatio = 1.f);

        inline PxRigidActor* getLinkBody(unsigned nodeIndex) {
            assert(nodeIndex < nLinks);
            return linkBodies[nodeIndex];
        }

        void receive(const MsgTransform&);
        void revive(const MsgRevive& m = MsgRevive());
		void reset();
};

}

#endif