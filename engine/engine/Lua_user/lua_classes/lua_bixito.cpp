#include "mcv_platform.h"
#include "lua_callback.h"
#include "gameElements/bichito/bichito.h"


using namespace gameElements;

namespace lua_user{

	Vec BixitoLua::getPosition(){

		Handle h = getHandle();
		if (h.hasSon<CBichito>()){
			Entity *entity = h;
			CTransform *transform = entity->get<CTransform>();
			Vec position = toVEC(transform->getPosition());
			return position;
		}
		assert("position not found");
        return toVEC(zero_v);
	}

	Vec BixitoLua::getPlayerPosition(){
	
		Handle h = getHandle();
		if (h.hasSon<CBichito>()){
			Entity *entity = h;
			CTransform *transform = entity->get<CTransform>();
			Vec position = toVEC(transform->getPosition());
			return position;
		}
		assert("position not found");
        return toVEC(zero_v);
	}
}