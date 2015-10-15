#include "mcv_platform.h"
#include "vectorUtils.h"

#include <algorithm>

#include "components/transform.h"

using namespace DirectX;
using namespace component;

bool operator==(const XMVECTOR& a, const XMVECTOR& b)
{
    uint32_t r;
    XMVectorEqualR(&r, a, b);
    return XMComparisonAllTrue(r);
}

bool operator==(const XMMATRIX& a, const XMMATRIX& b)
{
    return
        a.r[0] == b.r[0] &&
        a.r[1] == b.r[1] &&
        a.r[2] == b.r[2] &&
        a.r[3] == b.r[3];
}

namespace utils {

float getYawFromVector(XMVECTOR v) {
  float x = XMVectorGetX(v);
  float z = XMVectorGetZ(v);
  return atan2f(x, z);
}

float getPitchFromVector(XMVECTOR v) {
  float x = XMVectorGetX(v);
  float y = XMVectorGetY(v);
  float z = XMVectorGetZ(v);
  return atan2f(y, sqrt(z*z+x*x));
}

XMVECTOR getVectorFromYaw(float yaw) {
  return XMVectorSet(sinf( yaw ), 0.f, cosf( yaw ), 0.f );
}

bool isInFov(const XMVECTOR& origin, const XMVECTOR& direction,
    const XMVECTOR& loc, float fov_rad)
{
  XMVECTOR unit_delta = XMVector3Normalize(loc - origin);
  float cos_angle = XMVectorGetX(XMVector3Dot(direction, unit_delta));
  return(cos_angle > cos(fov_rad * 0.5f));
}

XMVECTOR alignXZ(Transform* t, const XMVECTOR& target, float maxAng_rad)
{
	const XMVECTOR d = XMVectorSetY(target, 0) - XMVectorSetY(t->getPosition(), 0);
    float au = angleBetweenVectors(XMVectorSetY(t->getFront(), 0), d);
    au = std::min(au, maxAng_rad);
    if (testIsBehind(t->getLeft(), d)) {au=-au;}
	const XMVECTOR q = XMQuaternionRotationAxis(t->getUp(), au);
    t->applyRotation(q);
    return q;
}

void align3DByElapsed(Transform* t, const XMVECTOR &target, float speed, float elapsed)
{
	const XMVECTOR d = target - t->getPosition();
	const XMVECTOR front = t->getFront();
	const XMVECTOR left = t->getLeft();
	float a = XMVectorGetX(XMVector3AngleBetweenVectors(front, d));
	if (std::abs(a)<FLT_EPSILON) { return; }

	// Cross product yields a perpendicular vector that is the
	//axis related to the angle returned by XMVector3AngleBetweenVectors.
	const XMVECTOR normal = XMVector3Cross(front, d);
	a = std::min(a, speed * elapsed);
	const XMVECTOR q = XMQuaternionRotationAxis(normal, a);
	t->applyRotation(q);

	// Correct left: always aligned to XZ plane
	float af = XMVectorGetX(XMVector3AngleBetweenNormals(left, yAxis_v)) - deg2rad(90);
	af = std::min(af, speed * elapsed);
	const XMVECTOR qF = XMQuaternionRotationAxis(front, af);
	t->applyRotation(qF);
}

void alignFixedUp(Transform* t, const XMVECTOR& dir, float maxAng_rad)
{
    if (testIsPerpendicular(dir, t->getUp())) {return;}
    //Intersection between the Up plane and the direction-Y plane
    auto desiredFront(XMVector3Cross(t->getUp(), XMVector3Cross(yAxis_v, dir)));

    //Adjust line to desired direction
    if (testIsBehind(dir, desiredFront)) {desiredFront = -desiredFront;}

    float au = angleBetweenVectors(t->getFront(), desiredFront);
    au = std::min(au, maxAng_rad);
    if (testIsBehind(t->getLeft(), desiredFront)) {au=-au;}
	XMVECTOR q = XMQuaternionRotationAxis(t->getUp(), au);
    t->applyRotation(q);
}

void moveDirection(XMVECTOR & vector, const XMVECTOR & direction, float elapsed, float speed)
{
	vector += direction * speed * elapsed;
}

void rotateToTarget(Transform* t, const Transform & target, float elapsed, float speed)
{
    auto myF = t->getFront();
    auto targetF = target.getFront();
    auto angle = angleBetweenVectors(myF, targetF);
    auto axis = XMVector3Cross(myF, targetF);
    if (axis == zero_v || angle == 0) {
        t->setRotation(target.getRotation());
        return;
    }
    auto q = XMQuaternionRotationAxis(XMVector3Normalize(axis),
        std::min(angle, elapsed*speed));
    t->applyRotation(q);
}

void moveDirection(Transform* t, const XMVECTOR & direction, float elapsed, float speed)
{
	moveDirection(t->refPosition(), direction, speed, elapsed);
}

XMVECTOR moveToTarget(Transform* t, const XMVECTOR & target, float elapsed, float speed)
{
    auto prevPosition = t->getPosition();
    auto delta = target - prevPosition;
    moveDirection(t, XMVector3Normalize(delta), elapsed, speed);
    auto nDelta = target - t->getPosition();
    if (testIsBehind(delta, nDelta)) {
        t->setPosition(target);
    }
    return t->getPosition() - prevPosition;
}

bool coneDetection(const Transform* t, const XMVECTOR& target,
    const float fov_deg, const float coneLength)
{
    return testDistanceSqEu(t->getPosition(), target, coneLength )&&
        isInFov(t->getPosition(), t->getFront(), target, deg2rad(fov_deg));
}


bool klaxonDetection(const Transform* t, const XMVECTOR& target,
    const float hear, const float fov_deg, const float coneLength)
{
    float sqEu(sqEuclideanDistance(t->getPosition(), target));
    return sqEu <= sq(hear) ||
        (sqEu <= sq(coneLength) &&
        isInFov(t->getPosition(), t->getFront(), target, deg2rad(fov_deg)));
}

void straightenPartially(Transform* t, float maxAngle, const XMVECTOR& newUP)
{
    auto a(angleBetweenVectors(t->getUp(), newUP));
    if (abs(a) > FLT_EPSILON) {
        auto axis(XMVector3Cross(t->getUp(), newUP));
        rotatePartially(t, a, axis, maxAngle);
    }
}

XMVECTOR rotatePartiallyQ(float angle_rad, const XMVECTOR& axis, float maxAng_rad)
{
    angle_rad = minAbs(angle_rad, maxAng_rad);
	return XMQuaternionRotationAxis(axis, angle_rad);
}
void rotatePartially(Transform* t, float angle_rad, const XMVECTOR& axis, float maxAng_rad)
{
    t->applyRotation(rotatePartiallyQ(angle_rad, axis, maxAng_rad));
}

void align3D(Transform* t, const XMVECTOR &target, float maxAng_rad)
{
	const XMVECTOR d = target - t->getPosition();
    const XMVECTOR front = t->getFront();
    const XMVECTOR left = t->getLeft();
    float a = XMVectorGetX(XMVector3AngleBetweenVectors(front, d));
    if (std::abs(a)<FLT_EPSILON) {return;}

    // Cross product yields a perpendicular vector that is the
    //axis related to the angle returned by XMVector3AngleBetweenVectors.
    const XMVECTOR normal = XMVector3Cross(front,d);
    a = std::min(a, maxAng_rad);
	const XMVECTOR q = XMQuaternionRotationAxis(normal, a);
    t->applyRotation(q);
    
    // Correct left: always aligned to XZ plane
	float af = XMVectorGetX(XMVector3AngleBetweenNormals(left, yAxis_v)) - deg2rad(90);
	const XMVECTOR qF = XMQuaternionRotationAxis(front, af);
    t->applyRotation(qF);
}


bool lineDetection(const Transform* t, const XMVECTOR& target, float threshold)
{
    /*
     * Distance line to point:
     * d^2 = |(l_2 - l_1) x (l_1 - p)|^2 / |l_2 - l_1|^2
     */
    const XMVECTOR l_1  (t->getPosition());
    const XMVECTOR l_2  (l_1 + t->getFront());
    const XMVECTOR l    (l_2 - l_1);
    const XMVECTOR v    (l_1 - target);
    const XMVECTOR num  (XMVector3Cross(l, v));
    const float sqEu    (XMVectorGetX(XMVector3LengthSq(num)) / XMVectorGetX(XMVector3LengthSq(l)));
    return sqEu <= sq(threshold);
}


void lookAt3D(Transform* t, const XMVECTOR& target)
{
  XMMATRIX m = XMMatrixLookAtRH(target, t->getPosition(), XMVectorSet(0, 1, 0, 0));
  XMVECTOR q = XMQuaternionRotationMatrix(m);
  t->setRotation(XMQuaternionInverse(q));
}


void lookAtXZ(Transform* t, const XMVECTOR& target)
{
    XMMATRIX m = XMMatrixLookAtRH(
        XMVectorSetY(target, 0), XMVectorSetY(t->getPosition(), 0), XMVectorSet(0, 1, 0, 0));
    XMVECTOR q = XMQuaternionRotationMatrix(m);
    t->setRotation(XMQuaternionInverse(q));
}


void straighten(Transform* t, const XMVECTOR& newUP)
{
	float al = angleBetweenVectors(t->getUp(), newUP);
	const XMVECTOR qL = XMQuaternionRotationAxis(t->getLeft(), al);
    t->applyRotation(qL);
	float af = angleBetweenVectors(t->getLeft(), newUP) - deg2rad(90);
	const XMVECTOR qF = XMQuaternionRotationAxis(t->getFront(), af);
    t->applyRotation(qF);
}

float angleBetweenVectors(XMVECTOR a, XMVECTOR b)
{
    return XMVectorGetX(XMVector3AngleBetweenVectors(a, b));
}

XMVECTOR rotationBetweenVectors(XMVECTOR a, XMVECTOR b)
{
    XMVECTOR normal = XMVector3Cross(a, b);
    float angle = XMVectorGetX(XMVector3AngleBetweenVectors(a, b));
    return XMQuaternionRotationAxis(normal, angle);
}

bool rotationBetweenVectorsEx(XMVECTOR a, XMVECTOR b, XMVECTOR& result,
    float max, float threshold, float fadeIn, float fadeOut, float factor, float& resultAngle)
{
    XMVECTOR normal = XMVector3Cross(a, b);
    float angle = XMVectorGetX(XMVector3AngleBetweenVectors(a, b));
    float clampAngle = std::max(0.f,std::min(angle, max));
    if (normal == zero_v || angle < FLT_EPSILON || angle >= threshold) {
        return false;
    } else if (angle <= fadeIn) {
        float fading = angle/fadeIn;
        float fadedAngle = clampAngle*sqrt(fading);
        result = XMQuaternionMultiply(result, XMQuaternionRotationAxis(normal, fadedAngle*factor));
        resultAngle = fadedAngle*factor;
        return true;
    } else if (angle > threshold-fadeOut) {
        float fading = (threshold-angle)/fadeOut;
        float fadedAngle = clampAngle*sqrt(fading);
        result = XMQuaternionMultiply(result, XMQuaternionRotationAxis(normal, fadedAngle*factor));
        resultAngle = fadedAngle*factor;
        return true;
    } else {
        result = XMQuaternionMultiply(result, XMQuaternionRotationAxis(normal, clampAngle*factor));
        resultAngle = clampAngle*factor;
        return true;
    }
}

XMVECTOR project(XMVECTOR a, XMVECTOR projection)
{
    auto dir = XMVector3Normalize(projection);
    return XMVector3Dot(a, dir)*dir;
}

XMVECTOR rotateToPlane(XMVECTOR a, XMVECTOR normal)
{
    auto ret = XMVector3Normalize(projectPlane(a, normal)) * XMVector3Length(a);
    if (testIsBehind(a,ret)) {
        return -ret;
    }  else {
        return ret;
    }
}

XMVECTOR projectPlane(XMVECTOR a, XMVECTOR normal)
{
    return a-XMVector3Dot(a, normal)*normal;
}

XMVECTOR projectPlane(XMVECTOR a, XMVECTOR planeX, XMVECTOR planeZ)
{
    return projectPlane(a, XMVector3Normalize(XMVector3Cross(planeX, planeZ)));
}

euler_t quaternionToEuler(XMVECTOR q)
{
    float x = XMVectorGetX(q);
    float y = XMVectorGetY(q);
    float z = XMVectorGetZ(q);
    float w = XMVectorGetW(q);
    euler_t ret;
    ret.yaw   = std::atan2(2*y*w - 2*x*z, 1 - 2*y*y - 2*z*z);
    ret.pitch = std::atan2(2*x*w - 2*y*z, 1 - 2*x*x - 2*z*z);
    ret.roll  = std::asin(2*x*y + 2*z*w);
    return ret;
}

void updateMinMax(XMFLOAT3& min, XMFLOAT3& max, XMFLOAT3 pos)
{
    min.x = std::min(min.x, pos.x);
    min.y = std::min(min.y, pos.y);
    min.z = std::min(min.z, pos.z);
    max.x = std::max(max.x, pos.x);
    max.y = std::max(max.y, pos.y);
    max.z = std::max(max.z, pos.z);
}

bool coplanar(XMVECTOR a, XMVECTOR b, XMVECTOR c)
{
    return XMVectorGetX(XMVector3Dot(XMVector3Cross(a, b), c)) == 0;
}

XMVECTOR deltaMovement(XMVECTOR a, XMVECTOR b, float speed)
{
    XMVECTOR diff = b-a;
    XMVECTOR delta = speed*XMVector3Normalize(diff);
    return testIsBehind(diff, b-(a+delta)) ? diff : delta;
}

XMVECTOR scaleFromMatrix(XMMATRIX m)
{
    return XMVectorSet(
        XMVectorGetX(XMVector3Length(m.r[0])),
        XMVectorGetY(XMVector3Length(m.r[1])),
        XMVectorGetZ(XMVector3Length(m.r[2])),
        1
    );
}

XMVECTOR calculateAimAngle (
	XMVECTOR target, XMVECTOR origin, float offsetY, float speed, float g, bool min)
{
	XMVECTOR d(target - origin);
	const float vpow2 = sq(speed);
	const float z = XMVectorGetZ(d);
	const float x = XMVectorGetX(d);

	const float dy = XMVectorGetY(d) + offsetY;
	const float dz(sqrt(sq(z) + sq(x)));

    const float vpow4(sq(vpow2));
    const float ac4(g*(g*sq(dz) + 2 * dy*vpow2));
	const float r(std::sqrt(std::abs(vpow4 - ac4)));
	const float gdz = g*dz;

    const float sol1 = std::atan2f(vpow2 + r, gdz);
    const float sol2 = std::atan2f(vpow2 - r, gdz);
	float angle = min ? std::min(sol1, sol2) : std::max(sol1, sol2);

	XMFLOAT3 ret;
	float yaw = getYawFromVector(d);
	ret.y = speed * std::sinf(angle);
	ret.x = speed * std::cosf(angle) * std::sinf(yaw);
	ret.z = speed * std::cosf(angle) * std::cosf(yaw);
	return XMVectorSet(ret.x, ret.y, ret.z, 0);
}

}