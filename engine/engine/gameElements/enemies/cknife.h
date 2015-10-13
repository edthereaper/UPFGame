#ifndef _GAME_ELEMENTS_KNIFE_
#define _GAME_ELEMENTS_KNIFE_

#include "mcv_platform.h"

#include "../gameMsgs.h"

#include "animation/skeleton_manager.h"
#include "components/transform.h"

#include "PhysX_USER/pxcomponents.h"

namespace gameElements {

class CKnife : physX_user::Shape {
    public:
        static void initType();

	private:
		int idBone;
	public:
        CKnife() : Shape(true, false, false){}

		void init();
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed);

        void setBone(std::string boneName);
        inline void setBone(int calBoneId) {idBone = calBoneId;}
        inline int getBone(int calBoneId) {return idBone;}
        
        using Shape::setBox;
        using Shape::setCapsule;
        using Shape::setSphere;
        using Shape::setFilters;
        using Shape::removeFilters;

        void receive(const physX_user::MsgCollisionEvent& msg);
};

}
#endif
