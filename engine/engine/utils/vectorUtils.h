#ifndef UTILS_VECTORUTILS_H_
#define UTILS_VECTORUTILS_H_

#include "mcv_platform.h"

#include "numberUtils.h"

namespace component{class Transform;}

/* Apparently these were too much effort to implement for poor DirectX team */
bool operator==(const XMVECTOR& a, const XMVECTOR& b);
inline bool operator!=(const XMVECTOR& a, const XMVECTOR& b) {return !(a==b);}
bool operator==(const XMMATRIX& a, const XMMATRIX& b);
inline bool operator!=(const DirectX::XMMATRIX& a, const DirectX::XMMATRIX& b) {return !(a==b);}

namespace utils {

/* some vector constants */
const XMVECTOR zero_v       = DirectX::XMVectorSet(0,0,0,0);
const XMVECTOR one_v        = DirectX::XMVectorSet(1,1,1,1);
const XMVECTOR xAxis_v      = DirectX::XMVectorSet(1,0,0,0);
const XMVECTOR yAxis_v      = DirectX::XMVectorSet(0,1,0,0);
const XMVECTOR zAxis_v      = DirectX::XMVectorSet(0,0,1,0);
const XMVECTOR wAxis_v      = DirectX::XMVectorSet(0,0,0,1);

const XMVECTOR one_q        = DirectX::XMQuaternionIdentity();
const XMVECTOR x90Rot_q     = DirectX::XMQuaternionRotationAxis(xAxis_v, deg2rad(90));
const XMVECTOR y90Rot_q     = DirectX::XMQuaternionRotationAxis(yAxis_v, deg2rad(90));
const XMVECTOR z90Rot_q     = DirectX::XMQuaternionRotationAxis(zAxis_v, deg2rad(90));
const XMVECTOR x90RotInv_q     = DirectX::XMQuaternionRotationAxis(xAxis_v, deg2rad(-90));
const XMVECTOR y90RotInv_q     = DirectX::XMQuaternionRotationAxis(yAxis_v, deg2rad(-90));
const XMVECTOR z90RotInv_q     = DirectX::XMQuaternionRotationAxis(zAxis_v, deg2rad(-90));
const XMVECTOR x45Rot_q     = DirectX::XMQuaternionRotationAxis(xAxis_v, deg2rad(45));
const XMVECTOR y45Rot_q     = DirectX::XMQuaternionRotationAxis(yAxis_v, deg2rad(45));
const XMVECTOR z45Rot_q     = DirectX::XMQuaternionRotationAxis(zAxis_v, deg2rad(45));
const XMVECTOR x45RotInv_q     = DirectX::XMQuaternionRotationAxis(xAxis_v, deg2rad(-45));
const XMVECTOR y45RotInv_q     = DirectX::XMQuaternionRotationAxis(yAxis_v, deg2rad(-45));
const XMVECTOR z45RotInv_q     = DirectX::XMQuaternionRotationAxis(zAxis_v, deg2rad(-45));

const XMVECTOR skeletonRef_q = DirectX::XMQuaternionMultiply(
    DirectX::XMQuaternionRotationAxis(zAxis_v, deg2rad(-90)),
    DirectX::XMQuaternionRotationAxis(xAxis_v, deg2rad(-90))
    );
const XMVECTOR skeletonRefInv_q = DirectX::XMQuaternionInverse(skeletonRef_q);

const XMVECTOR xAxisSkel_v  = DirectX::XMVector3Rotate(xAxis_v, skeletonRef_q);
const XMVECTOR yAxisSkel_v  = DirectX::XMVector3Rotate(yAxis_v, skeletonRef_q);
const XMVECTOR zAxisSkel_v  = DirectX::XMVector3Rotate(zAxis_v, skeletonRef_q);



inline XMVECTOR toXMVECTOR(const float v[]) {return DirectX::XMVectorSet(v[0], v[1], v[2], v[3]);}

float getYawFromVector(XMVECTOR front);
float getPitchFromVector(XMVECTOR front);

XMVECTOR getVectorFromYaw(float yaw);

/* Orientate the transform to a target*/
void lookAt3D(component::Transform* who, const XMVECTOR& target);

/* Orientate the transform to a target*/
void lookAtXZ(component::Transform* who, const XMVECTOR& target);

/* Orientate the transform to a target*/
void straighten(component::Transform* who, const XMVECTOR& newUP = yAxis_v);

/* Orientate the transform to a target, limiting the angle */
void align3D(component::Transform* who, const XMVECTOR& target, float maxAng_rad);
void align3DByElapsed(component::Transform* t, const XMVECTOR &target, float speed, float elapsed);

/* Orientate the transform to a target rotating on Up axis */
XMVECTOR alignXZ(component::Transform* who, const XMVECTOR& target, float maxAng_rad);

/* Orientate the transform rotating it around its up vector, so the new front is 
    coplanar with dir and the yAxis, a maximum angle */
void alignFixedUp(component::Transform* t, const XMVECTOR& dir, float maxAng_rad);
/* Orientate the transform rotating it around its up vector, so the new front is 
    coplanar with dir and the yAxis*/
inline void alignFixedUp(component::Transform* t, const XMVECTOR& dir) {
    alignFixedUp(t, dir, M_4PIf);
}

/* Rotate a step*/
XMVECTOR rotatePartiallyQ(float angle_rad, const XMVECTOR& axis, float maxAng_rad);
void rotatePartially(component::Transform* who, float angle_rad, const XMVECTOR& axis, float maxAng_rad);
inline void rotate(component::Transform* who, float angle_rad, const XMVECTOR& axis) {
    rotatePartially(who, angle_rad, axis, M_4PIf);
}

/* Straighten a step */
void straightenPartially(component::Transform* who, float maxAngle, const XMVECTOR& newUP = yAxis_v);

/* Move an entity in a direction. direction must be unitary*/
void moveDirection(component::Transform* who, const XMVECTOR & direction, float elapsed, float speed);

/* Move a vector in a direction. Direction must be unitary*/
void moveDirection(XMVECTOR& v, const XMVECTOR & direction, float elapsed, float speed);

/* Move an entity towards a target */
void moveToTarget(component::Transform* who, const XMVECTOR & target, float elapsed, float speed);


/* move rotation towards a target */
void rotateToTarget(component::Transform* t, const component::Transform & target, float elapsed, float speed);

/* Square euclidean distance */
inline float sqEuclideanDistance(const XMVECTOR & a, const XMVECTOR & b) {
    return DirectX::XMVectorGetX(
        DirectX::XMVector3LengthSq(DirectX::operator-(a,b)));
}

/* Square */
template<typename T> inline T sq(T base) {return base*base;}

/* Distance test using square distance. */
inline bool testDistanceSqEu(const XMVECTOR & a, const XMVECTOR & b, float radius) {
    return sqEuclideanDistance(a, b) <= sq(radius);
}

inline bool testCyllinder(const XMVECTOR & a, const XMVECTOR & b, float radius, float height) {
    return std::abs(DirectX::XMVectorGetY(DirectX::operator-(a,b))) <= height && testDistanceSqEu(a,b,radius);
}

/* Vectors are so close we can say they're practically the same*/
inline bool touching(const XMVECTOR & a, const XMVECTOR & b) {
    return testDistanceSqEu(a,b, 0.05f);
}

/* test whether a target is behind a vector */
inline bool testIsBehind(const XMVECTOR& direction, const XMVECTOR& delta) {
    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, delta)) < 0.f;
}

/* test whether a target is behind a vector */
inline bool testIsPerpendicular(const XMVECTOR& direction, const XMVECTOR& delta) {
    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, delta)) == 0.f;
}

/* test whether a target is behind a vector */
inline bool testIsBehindOrPerpendicular(const XMVECTOR& direction, const XMVECTOR& delta) {
    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, delta)) <= 0.f;
}

/* Whether a point is in a certain fov. direction must be an unitary vector */
bool isInFov(const XMVECTOR& origin, const XMVECTOR& direction,
    const XMVECTOR& loc, float fov_rad);

/* Detection test */
bool coneDetection(const component::Transform* who, const XMVECTOR& target,
    const float fov_deg, const float coneLength);

/* Detection test */
bool klaxonDetection(const component::Transform* who, const XMVECTOR& target,
    const float hear, const float fov_deg, const float coneLength);

/* Test if target is in LOS using distance to the line of sight*/
bool lineDetection(const component::Transform* who, const XMVECTOR& target, float threshold);

/* Get the rotation between two vectors and multiply it by result
    max: maximum rotation
    threshold: for angles higher than this, do not rotate
    fadeIn and fadeOut: for angles between threshold-fade and threshold, rotate progressively less
    factor: multiply the angle before calculating the quaternion

    returns wether the desired angle was below the threshold
*/
bool rotationBetweenVectorsEx(XMVECTOR a, XMVECTOR b, XMVECTOR& result,
    float max, float threshold, float fadeIn, float fadeOut, float factor, float& resultAngle);

/* Get the rotation quaternion between two vectors */
XMVECTOR rotationBetweenVectors(XMVECTOR a, XMVECTOR b);

XMVECTOR project(XMVECTOR a, XMVECTOR projection);
XMVECTOR projectPlane(XMVECTOR a, XMVECTOR planeX, XMVECTOR planeZ);
XMVECTOR projectPlane(XMVECTOR a, XMVECTOR normal);

XMVECTOR rotateToPlane(XMVECTOR a, XMVECTOR normal);

float angleBetweenVectors(XMVECTOR a, XMVECTOR b);

struct euler_t {
    float yaw, pitch, roll;
};

/* Extract euler angles from quaternion */
euler_t quaternionToEuler(XMVECTOR q);
inline XMVECTOR eulerToQuaternion(const euler_t& e) {
    return DirectX::XMQuaternionRotationRollPitchYaw(e.pitch, e.yaw, e.roll);
}

void updateMinMax(XMFLOAT3& min, XMFLOAT3& max, XMFLOAT3 pos);

inline float module(XMVECTOR v) {return DirectX::XMVectorGetX(DirectX::XMVector3Length(v));}
inline float moduleSq(XMVECTOR v) {return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v));}

// coplanar := vectors are in the same plane
bool coplanar(XMVECTOR a, XMVECTOR b, XMVECTOR c);

XMVECTOR deltaMovement(XMVECTOR a, XMVECTOR b, float speed);

}


#endif