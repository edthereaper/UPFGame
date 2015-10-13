#ifndef INC_DOOM_CONTROLLER_H_
#define INC_DOOM_CONTROLLER_H_

#include "whiteboxes.h"

#include "handles/entity.h"

#include "components\Transform.h"

using namespace utils;

#if defined(_OBJECTTOOL) || defined(_LIGHTTOOL) || defined (_PARTICLES) || defined(_CINEMATIC_) 
    #define CAMERA_CONTROLLER_ALLOWFREEMOVEMENT true
#else
    #define CAMERA_CONTROLLER_ALLOWFREEMOVEMENT false
#endif

//--------------------------------
class CamCannonController {
    public:
        static const float rotation_velocity;
    private:
        XMVECTOR eye = DirectX::XMVectorSet(0,1,-1,0);
        XMVECTOR dir = DirectX::XMVectorSet(0,0,1,0);
        float limitLeftYaw = deg2rad(50);
        float limitRightYaw = deg2rad(-50);
        float limitUpPitch = deg2rad(50);
        float limitDownPitch = deg2rad(-50);
        float invertY = -1.f;
    public:
		inline void setup( XMVECTOR newEye, XMVECTOR newDir = zAxis_v,
            float limitH = M_4PIf, float limitV = M_4PIf)
		{
            eye= newEye;
            dir = newDir;
            float yaw = getYawFromVector(dir);
            float pitch = getPitchFromVector(dir);
            limitLeftYaw = yaw + limitH;
            limitRightYaw = yaw - limitH;
            limitUpPitch = pitch + limitV;
			if (limitUpPitch >= deg2rad(90.0f)) limitUpPitch = deg2rad(89.0f);
			limitDownPitch = pitch - limitV;
			if (limitDownPitch < deg2rad(-90.0f)) limitDownPitch = deg2rad(-89.0f);
        }
        
        void update(component::Transform& camEntity, float elapsed,
            bool moveFreely=CAMERA_CONTROLLER_ALLOWFREEMOVEMENT);
        void updateEntity(component::Transform& camEntity);
};


// -------------------------------
class Cam3PController {
    private:
        static const float pitchLimitNeg;
        static const float pitchLimitPos;
        static const float rotation_velocity;
    private:
		XMVECTOR eyeOffset = zero_v;
        XMVECTOR targetOffset = zero_v;
        float invertY = -1.f;
    public:
        void update(component::Transform& who, const component::Transform& target, float elapsed);
        void updateEntity(component::Transform& who, const component::Transform& targetEntity);
        inline void setEye(XMVECTOR eye) {eyeOffset = eye;}
        inline void setTarget(XMVECTOR target) {targetOffset = target;}
        inline void setInvertY(bool invert) {invertY = invert ? -1.f : 1.f;}

};

#endif
