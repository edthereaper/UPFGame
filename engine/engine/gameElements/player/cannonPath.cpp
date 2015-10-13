#include "mcv_platform.h"
#include "cannonPath.h"

#include "handles/entity.h"
#include "components/transform.h"
using namespace component;

#include "Physx_USER/pxcomponents.h"
using namespace physX_user;

using namespace utils;
using namespace DirectX;

namespace gameElements {

	const float CCannonPath::GRAVITY = 3.5f * 9.81f;

	void CCannonPath::update(float elapsed)
	{
		Handle h(this);
		Entity* me = h.getOwner();
		CCharacterController* charControl = me->get<CCharacterController>();

		yvel -= GRAVITY*elapsed*yAxis_v;
		charControl->setDisplacement((xzvel + yvel)*elapsed);

		if (charControl->onCeiling() || charControl->onGround() || charControl->onWall()){
            removeFromScene();
		}

		if (ttlTimer.count(elapsed) >= 0) {
            removeFromScene();
		}
	}


	void CCannonPath::removeFromScene(){

		Entity* e = Handle(this).getOwner();
		e->postMsg(MsgDeleteSelf());
	}

	void CCannonPath::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
	{
		float ttl = atts.getFloat("ttl", 1.0f);
		ttlTimer.reset();
		ttlTimer.count(-ttl);
	}

	void CCannonPath::setup(XMVECTOR xzvelocity, XMVECTOR yvelocity)
	{
		xzvel = xzvelocity;
		yvel = yvelocity;
	}

}