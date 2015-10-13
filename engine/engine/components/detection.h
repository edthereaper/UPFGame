#ifndef COMPONENT_DETECTION_H_
#define COMPONENT_DETECTION_H_

#include "mcv_platform.h"

namespace component {

	/* Component DETECTION
	Defines an strategy to do a detection test

	XML PROPERTIES
	type    = line|sphere|cone|klaxon|cutklaxon
	sight   = the distance of detection
	fov     = the angle in a cone or klaxon detection in degrees
	hearing = the spherical range in a klaxon detection
	margin  = the margin in a line detection
	fovHear = the inner angle in a cut klaxon detection in degrees
	*/
	struct CDetection {
		enum type_e {
			LINE,
			SPHERE,
			CONE,
			KLAXON,
			CUT_KLAXON,
			CONE_AIM,
		} type = SPHERE;
		float sight = 0.f;
		float fov = 0.f;
		float hearing = 0.f;
		float fovHear = 90.f;
		float margin = 0.f;
		static const float NO_SCORE;

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		inline void update(float elapsed) {}
		inline void init() {}

		CDetection() = default;
		CDetection(type_e type, float sight = 0.f, float fov = 0.f, float hearing = 0.f, float margin = 0.f) :
			type(type), sight(sight), hearing(hearing), fov(fov), margin(margin) {}

		bool test(const Transform* transform, const XMVECTOR& target);
		bool testWithSight(const Transform* transform, const XMVECTOR& target, float newSight);
		float score(const Transform* transform, const XMVECTOR& target);
	};

}

#endif