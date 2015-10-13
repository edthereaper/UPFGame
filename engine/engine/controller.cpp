#include "mcv_platform.h"
#include "controller.h"

#include "PhysX_USER/pxcomponents.h"
#include "PhysX_USER/PhysicsManager.h"
using namespace physX_user;

using namespace DirectX;

#include "Particles/Emitter.h"
using namespace particles;

#include "gameElements/props.h"
#include "gameElements/pickup.h"
#include "gameElements/enemies/flare.h"
#include "gameElements/liana.h"
#include "gameElements/input.h"
using namespace gameElements;

#include "render/components.h"
using namespace render;
#include "antTweakBar_USER/antTW.h"

#include "Cinematic/camera_manager.h"
using namespace cinematic;

#define ROT_CAM_SPEED 45.f
const float CamCannonController::rotation_velocity = deg2rad(ROT_CAM_SPEED);
const float Cam3PController::rotation_velocity = deg2rad(ROT_CAM_SPEED);

void freelyMoveCam(Transform& camT, float rotStep, float elapsed)
{
    const auto& pad = App::get().getPad();
    float camSpeed=20;
    #if defined(_OBJECTTOOL)
    
        camSpeed *= 0.2f;
        rotStep *= 0.5;
    #endif
    if (pad.getState(CONTROLS_MOVE_CAM_FASTER).isPressed()) {
    	camSpeed *= 2.f;
    }
    if (pad.getState(CONTROLS_MOVE_CAM_SLOWER).isPressed()) {
    	camSpeed *= 0.25f;
    	rotStep *= 0.35f;
    }

#if defined(_DEBUG)

		if (pad.getState(CONTROLS_UP).isPressed()) {
			camT.setPosition(camT.getPosition() + camT.getFront() * camSpeed * elapsed);
		}
		if (pad.getState(CONTROLS_DOWN).isPressed()) {
			camT.setPosition(camT.getPosition() - camT.getFront() * camSpeed * elapsed);
		}
		if (pad.getState(CONTROLS_LEFT).isPressed()) {
			camT.setPosition(camT.getPosition() + camT.getLeft() * camSpeed * elapsed);
		}
		if (pad.getState(CONTROLS_RIGHT).isPressed()) {
			camT.setPosition(camT.getPosition() - camT.getLeft() * camSpeed * elapsed);
		}
		if (pad.getState(CONTROLS_MOVE_CAM_UP).isPressed()) {
			camT.setPosition(camT.getPosition() + camT.getUp() * camSpeed * elapsed);
		}
		if (pad.getState(CONTROLS_MOVE_CAM_DOWN).isPressed()) {
			camT.setPosition(camT.getPosition() - camT.getUp() * camSpeed * elapsed);
		}
#endif
}

void CamCannonController::update(Transform& camT, float elapsed, bool allowFreeMovement)
{
    float rotStep(deg2rad(110.f)/1024);
    if (allowFreeMovement) { 
        freelyMoveCam(camT, rotStep, elapsed);
        eye = camT.getPosition();
    }
#if defined(_OBJECTTOOL) || defined(_LIGHTTOOL) || defined (_PARTICLES)
    const auto& app = App::get();
    const auto& pad = app.getPad();
    if (app.clickToTransform && pad.getState(CONTROLS_SHOOT).isHit()){
        POINT p;
        GetCursorPos(&p);
        float xres = (float)App::get().getConfigX();
        float yres = (float)App::get().getConfigY();
        //Entre 0 y 1
        float aa = p.x / xres;
        float bb = p.y / yres;
        //Entre -1 y 1
        aa = (aa - 0.5f) * 2;
        bb = (bb - 0.5f) * 2;
        XMVECTOR origin = camT.getPosition();
        
        // Adjust the points using the projection matrix to account for the aspect ratio of the viewport.
        Handle camH = App::get().getCamera();
        Entity* camE = camH;
        CCamera* camC = camE->get<CCamera>();
        XMMATRIX proj = camC->getProjection();
        XMFLOAT4X4 fView;
        XMStoreFloat4x4(&fView, proj);
        aa = aa / fView._11;
        bb = bb / fView._22;
        
        XMVECTOR dirCam = XMVector3Normalize(camT.getFront() - camT.getLeft() * aa - camT.getUp() * bb);
        
        PxReal distance = 100.0f;
        PxRaycastBuffer hit;
        if (PhysicsManager::get().raycast(origin, dirCam, distance, hit,
        	filter_t(
        	filter_t::NONE,
        	filter_t::id_t(filter_t::PLAYER | filter_t::ENEMY |
        	filter_t::SCENE | filter_t::DESTRUCTIBLE | filter_t::PAINT_SPHERE),
        	filter_t::ALL_IDS))){
        	Handle HitHandle = Handle::fromRaw(hit.block.shape->userData);
        	Entity *eOther = HitHandle;
	    	#if defined(_OBJECTTOOL) || defined(_LIGHTTOOL)
	    	    if (eOther){
	    	    	//If object detected can Transform
	    	    	if (eOther->has<CTransformable>()){
	    	    		eOther->sendMsg(MsgShot(true));
	    	    	}
	    	    }
	    	#endif
	    	#ifdef _LIGHTTOOL
	    	    if (eOther){
	    	    	//If object detected is Light
	    	    	if (eOther->has<CPtLight>()){
	    	    		CPtLight* i = eOther->get<CPtLight>();
	    	    		antTw_user::AntTWManager::selectPointLightTweak(i);
	    	    	}
	    	    	if (eOther->has<CDirLight>()){
	    	    		CDirLight* i = eOther->get<CDirLight>();
	    	    		antTw_user::AntTWManager::selectDirectionalLightTweak(i);
	    	    	}
	    	    	if (eOther->has<CVolPtLight>()){
	    	    		CVolPtLight* i = eOther->get<CVolPtLight>();
	    	    		antTw_user::AntTWManager::selectVolLightTweak(i);
	    	    	}
	    	    	if (eOther->has<CMist>()){
	    	    		CMist* i = eOther->get<CMist>();
	    	    		antTw_user::AntTWManager::selectMistTweak(i);
					}
					if (eOther->has<CEmitter>()){
						CEmitter* i = eOther->get<CEmitter>();
						antTw_user::AntTWManager::selectEmitterTweak(i);
					}
	    	    }
	        #endif

	    }
	}
#endif

	float orbitYaw = .0f, orbitPitch = .0f;

	auto delta = Mouse::getDelta();
	orbitYaw -= delta.x;
	orbitPitch += delta.y;

	if (orbitYaw != 0 || orbitPitch != 0) {
		orbitYaw *= rotStep;
		orbitPitch *= rotStep * invertY;
		float currentPitch = getPitchFromVector(dir);
		float currentYaw = getYawFromVector(dir);
		//Both if are a solution for the jump between 180 to -180 that yaw does.
		if (currentYaw < 0 && limitLeftYaw > deg2rad(180)) {
            currentYaw = deg2rad(180.0f) + (deg2rad(180) - abs(currentYaw));
        }
		if (currentYaw > 0 && limitRightYaw < deg2rad(-180)) {
            currentYaw = deg2rad(-180.0f) - (deg2rad(180) - abs(currentYaw));
        }
		float finalPitch = currentPitch + orbitPitch;
		if (finalPitch < limitDownPitch) { orbitPitch = limitDownPitch - currentPitch; }
		if (finalPitch > limitUpPitch) { orbitPitch = limitUpPitch - currentPitch; }
		float finalYaw = currentYaw + orbitYaw;
		if (finalYaw < limitRightYaw) { orbitYaw = limitRightYaw - currentYaw; }
		if (finalYaw > limitLeftYaw) { orbitYaw = limitLeftYaw - currentYaw; }
		float yawCorrected = M_PIf + currentYaw;
		XMMATRIX transf = XMMatrixIdentity();
		if (orbitPitch == 0) { 
            transf *= XMMatrixRotationRollPitchYaw(0, orbitYaw, 0);
        } else {
			transf *= XMMatrixRotationRollPitchYaw(0, -yawCorrected, 0) *
				XMMatrixRotationRollPitchYaw(orbitPitch, yawCorrected + orbitYaw, 0);
		}
		dir = XMVector3Transform(dir, transf);
	}

	updateEntity(camT);
}

void CamCannonController::updateEntity(Transform& who)
{
    if (dir == zero_v) {dir = zAxis_v;}
	XMMATRIX m(XMMatrixLookAtRH(eye+dir, eye, XMVectorSet(0, 1, 0, 0)));
	XMVECTOR q(XMQuaternionRotationMatrix(m));
#if !defined(_OBJECTTOOL) && !defined(_LIGHTTOOL) && !defined(_PARTICLES) && !defined(_CINEMATIC_)
	who.setPosition(eye);
	if (CameraManager::get().isPlayerCam())
		who.setRotation(XMQuaternionInverse(q));;
#else
	#if defined(_CINEMATIC_)
    if (CameraManager::get().isPlayerCam())
	    who.setRotation(XMQuaternionInverse(q));
	#else
		who.setRotation(XMQuaternionInverse(q));
	#endif
#endif
}

//Inverted
const float Cam3PController::pitchLimitPos = +55.f;
const float Cam3PController::pitchLimitNeg = -89.f;

void Cam3PController::update(Transform& who, const Transform& targetEntity, float elapsed)
{
    float rotStep(deg2rad(110.f)/1024);

#ifdef _DEBUG
    if (isKeyPressed(VK_CONTROL)) {return;}
#endif

#ifdef _OBJECTTOOL
	if (Mouse::getWheel() > 0){
		eyeOffset = eyeOffset * 0.9f;
	}
	if (Mouse::getWheel() < 0){
		eyeOffset = eyeOffset / 0.9f;
	}
#endif

	float orbitYaw = .0f, orbitPitch = .0f;
    
	auto delta = Mouse::getDelta();
	orbitYaw -= delta.x;
	orbitPitch += delta.y;

	if (orbitYaw != 0 || orbitPitch != 0) {
		orbitYaw *= rotStep;
		orbitPitch *= rotStep * invertY;

        XMVECTOR target = targetEntity.getPosition() + targetOffset;
        XMVECTOR eye = targetEntity.getPosition() + eyeOffset;
        XMVECTOR dir = target - eye;
        float yaw = M_PIf + getYawFromVector(dir);	
        float currentPitch = getPitchFromVector(dir) ; 
		float finalPitch = currentPitch + orbitPitch;
        if (finalPitch < deg2rad(pitchLimitNeg)) {orbitPitch = deg2rad(pitchLimitNeg) - currentPitch;}
        if (finalPitch > deg2rad(pitchLimitPos)) {orbitPitch = deg2rad(pitchLimitPos) - currentPitch;}
        
		XMMATRIX transf = XMMatrixTranslationFromVector(-targetOffset);
		if (orbitPitch == 0) { transf *= XMMatrixRotationRollPitchYaw(0, orbitYaw, 0); }
		else {
			transf *= XMMatrixRotationRollPitchYaw(0, -yaw, 0) *
				XMMatrixRotationRollPitchYaw(orbitPitch, yaw + orbitYaw, 0);
		}
		transf *= XMMatrixTranslationFromVector(targetOffset);
		eyeOffset = XMVector3Transform(eyeOffset, transf);
	}
    updateEntity(who, targetEntity);
}

void Cam3PController::updateEntity(Transform& who, const Transform& targetEntity)
{
	
#ifdef _OBJECTTOOL
	XMVECTOR eyePos(eyeOffset);
	XMVECTOR targetPos(targetOffset);

	XMMATRIX m(XMMatrixLookAtRH(targetPos, eyePos, XMVectorSet(0, 1, 0, 0)));
	XMVECTOR q(XMQuaternionRotationMatrix(m));

	who.setPosition(eyePos);
	who.setRotation(XMQuaternionInverse(q));
#else
	XMVECTOR eyePos(eyeOffset + targetEntity.getPosition());
	XMVECTOR targetPos(targetOffset + targetEntity.getPosition());

	XMMATRIX m(XMMatrixLookAtRH(targetPos, eyePos, XMVectorSet(0, 1, 0, 0)));
	XMVECTOR q(XMQuaternionRotationMatrix(m));

	//Raycast (player -> cam): To obtain many hits in raycast
	float x = XMVectorGetX(targetPos) - XMVectorGetX(eyePos);
	float y = XMVectorGetY(targetPos) - XMVectorGetY(eyePos);
	float z = XMVectorGetZ(targetPos) - XMVectorGetZ(eyePos);
	PxReal distCamPlayer = sqrt(x*x + y*y + z*z);												// Distance between the cam and the player (sqEulerDistance did wrong here)
	XMVECTOR vNormCamPlayer = XMVector3Normalize(eyePos - targetPos);							// Normalized vector direction
	PxSphereGeometry a;
	a.radius = 0.01f;
	PxTransform b;
	b.p = toPxVec3(targetPos);
	b.q = toPxQuat(XMQuaternionIdentity());
	const PxU32 bufferSize = 4;
	PxSweepHit hitBuffer[bufferSize];
	PxSweepBuffer aaa(hitBuffer, bufferSize);
	if (PhysicsManager::get().sweep(vNormCamPlayer, distCamPlayer, a, b, aaa,
        filter_t(
            filter_t::NONE,
            ~filter_t::id_t(filter_t::SCENE|filter_t::TOOL|filter_t::DESTRUCTIBLE),
            filter_t::id_t(filter_t::SCENE|filter_t::TOOL|filter_t::DESTRUCTIBLE)),
            PxHitFlag::eDEFAULT, 0.1f)) {
		for (int i = 0; i != aaa.nbTouches; i++){
			if (aaa.touches[i].distance > 0.1f){											  //Filter of distance in order to avoid errors
				eyePos = toXMVECTOR(aaa.touches[i].position + (aaa.touches[i].normal)*0.05f); //Set the params.
			}
		}
	}

	who.setPosition(eyePos);
	who.setRotation(XMQuaternionInverse(q));
#endif
}
