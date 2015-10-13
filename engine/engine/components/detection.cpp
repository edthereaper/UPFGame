#include "mcv_platform.h"
#include "detection.h"

#include "handles/entity.h"
#include "transform.h"

using namespace utils;
using namespace DirectX;

namespace component {


	void CDetection::loadFromProperties(const std::string& elem, MKeyValue &atts)
	{
		std::string typeStr(atts.getString("type", "sphere"));
		if (typeStr == "line")           { type = LINE; }
		else if (typeStr == "sphere")    { type = SPHERE; }
		else if (typeStr == "cone")      { type = CONE; }
		else if (typeStr == "klaxon")    { type = KLAXON; }
		else if (typeStr == "cutklaxon") { type = CUT_KLAXON; }
		else if (typeStr == "coneaim") { type = CONE_AIM; }

		if (atts.has("sight")) { sight = atts.getFloat("sight"); }
		if (atts.has("fov")) { fov = atts.getFloat("fov"); }
		if (atts.has("hearing")) { hearing = atts.getFloat("hearing"); }
		if (atts.has("margin")) { margin = atts.getFloat("margin"); }
		if (atts.has("fovHear")) { fovHear = atts.getFloat("fovHear"); }
	}

	const float CDetection::NO_SCORE = 1e10;

	float CDetection::score(const Transform* transform, const XMVECTOR& target)
	{

		if (!test(transform, target)) { return NO_SCORE; }
		float s = 0, yaw, pitch;

		switch (type) {
		case SPHERE:
		case LINE:
			s = sqEuclideanDistance(transform->getPosition(), target);
			break;
		case CUT_KLAXON:
		case KLAXON:
			s += sqEuclideanDistance(transform->getPosition(), target);
			if (s < sq(hearing)) {
				break;
			} //fall down
		case CONE:
			s += sqEuclideanDistance(transform->getPosition(), target);
			yaw = XMVectorGetX(XMVector3AngleBetweenVectors(
				transform->getFront(), target - transform->getPosition()));
			s += sq(yaw*sight);
			break;
		case CONE_AIM:
			s = sqEuclideanDistance(transform->getPosition(), target);
			if (s < 10) return NO_SCORE;
			yaw = XMVectorGetX(XMVector3AngleBetweenVectors(
				transform->getFront(), target - transform->getPosition()));
			pitch = XMVectorGetY(XMVector3AngleBetweenVectors(
				transform->getFront(), target - transform->getPosition()));
			s = yaw * pitch;
			break;
		default: break;
		}
		return s;
	}

	bool CDetection::test(const Transform* transform, const XMVECTOR& target)
	{
		switch (type) {
		case LINE:
			return testDistanceSqEu(transform->getPosition(), target, sight)
				&& lineDetection(transform, target, margin);
			break;
		case SPHERE:
			return testDistanceSqEu(transform->getPosition(), target, sight);
			break;
		case CONE_AIM:
		case CONE:
			return coneDetection(transform, target, fov, sight);
			break;
		case KLAXON:
			return klaxonDetection(transform, target, hearing, fov, sight);
			break;
		case CUT_KLAXON:
			return coneDetection(transform, target, fov, sight)
				|| coneDetection(transform, target, fovHear, hearing);
			break;
		default: break;
		}
		return false;
	}

	bool CDetection::testWithSight(const Transform* transform, const XMVECTOR& target, float newSight)
	{
		CDetection copy(*this);
		copy.sight = newSight;
		return copy.test(transform, target);
	}

}