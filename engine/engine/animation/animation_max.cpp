#include "mcv_platform.h"
#include "animation_max.h"
#include "gameElements/input.h"
#include "handles/entity.h"
#include "components/transform.h"

using namespace gameElements;
using namespace component;

#include "app.h"
#include "gameElements/props.h"
#include "gameElements/gameMsgs.h"
#include "fmod_User/fmodStudio.h"
using namespace fmodUser;
using namespace DirectX;
using namespace utils;

#include "utils/data_provider.h"
#include "gameElements/gameMsgs.h"

namespace animation{

	

	void CMaxAnim::init(){
	
		if (isPiece()){
			animExporter = new AnimationMaxImporter();
		}
		setPivot(XMVectorSet(0,1,0,0));
	}


	void CMaxAnim::setPivot(XMVECTOR pivot){
		
		Handle h = component::Handle(this).getOwner();
		Entity *me = h;
		CTransform *transform = me->get<CTransform>();
		transform->setPivot(pivot);
	}
	XMVECTOR CMaxAnim::getPivot(){
	
		Handle h = component::Handle(this).getOwner();
		Entity *me = h;
		CTransform *transform = me->get<CTransform>();
		return transform->getPivot();
	}

	// prefab
	void CMaxAnim::loadFromProperties(const std::string& elem, utils::MKeyValue &atts){
	
		if (!isPiece()){

			if (elem == "MaxAnim"){

				std::string name = atts.getString("name", "INVALID");

				std::string maxString = atts.getString("max", "INVALID");
				maxString = maxString.substr(0, maxString.size() - 1);
				anim.max = atoi(maxString.c_str());

				anim.play = atts.getInt("isOn") ? true : false;
				std::string type_ = atts.getString("type");

				if (type_.compare("PROP") == 0)
					anim.type = MaxAnim::PROP_POS;
				else if (type_.compare("ROT") == 0)
					anim.type = MaxAnim::PROP_ROT;
				else
					anim.type = MaxAnim::PROP_POS_ROT;

				if (atts.has("tag1"))
					anim.tag1 = atts.getString("tag1", "INVALID");

				if (atts.has("tag2"))
					anim.tag2 = atts.getString("tag2", "INVALID");

				if (atts.has("tag3"))
					anim.tag3 = atts.getString("tag3", "INVALID");


				load(name.c_str());
			}
		}
	}

	void CMaxAnim::loadFromLevel(std::string animationName){
	
		
		
	}

	// ---------------------load file .anim-------------------//

	bool CMaxAnim::load(const char*name) {

		char full_name[MAX_PATH];
		sprintf(full_name, "%s/%s.anim", "data/animmax", name);

		FileDataProvider fdp(full_name);
		assert(fdp.isValid());
		return load(fdp);
	}

	bool CMaxAnim::load(DataProvider& dp) {
		dp.read(headerCamera);
		if (!headerCamera.isValid())
			return false;

		// Read the keys
		int size = (int)headerCamera.ntime_keys;
		keys.resize(size);
		dp.read(&keys[0], keys.size() * sizeof(Key));
		return true;
	}

	// ---------------------load file .anim-------------------//
	//--------------------------------------------------------//
	// ---------------------special cannon -------------------//

	bool CMaxAnim::loadCannon(int tag) {

		char full_name[MAX_PATH];
		sprintf(full_name, "%s/Cannon%i.anim", "data/animmax", tag);

		FileDataProvider fdp(full_name);
		assert(fdp.isValid());
		return load(fdp);
	}

	void CMaxAnim::enableCannon(){

		Entity *me = Handle(this).getOwner();
		CTransform* t = me->get<CTransform>();
		fmodUser::FmodStudio::play3DSingleEvent(fmodUser::FmodStudio::getEventInstance("SFX/boss_canonland"), t->getPosition());
		CMaxAnim* maxAnim = me->get<CMaxAnim>();
		maxAnim->setPostFinish(true);
        me->sendMsg(MsgFlyingMobileEnded());
	}

	// ---------------------special cannon -------------------//

	// Return true if the we went beyond the animation duration
	bool CMaxAnim::getTransform(float t) const {


		if (t < 0)
			return true;

		// If they ask for time beyond the animation duration we will clamp
		// and return true

		if (t > float(anim.max)) {
			t = float(anim.max);
            auto k = keys.back();
			CTransform *transform = component::Handle(this).getBrother<CTransform>();
			if (anim.type == MaxAnim::PROP_POS)		 transform->setPosition(k.trans);
			else if (anim.type == MaxAnim::PROP_ROT)  transform->setRotation(k.rotation);
			else{
				transform->setPosition(k.trans);
				transform->setRotation(k.rotation);
			}
			return true;
		} else {

			// Convert t to frame key
			// 1.1s => 2.22f
			float frame_number = t * headerCamera.ntime_keys / headerCamera.animation_duration;

			// The integer part 2
			unsigned   prev_iframe = unsigned(std::floor(frame_number));
			// The decimal part = 0.22
			float time_in_next = frame_number - prev_iframe;
			assert(time_in_next >= 0 && time_in_next <= 1.f);
			unsigned   next_iframe = prev_iframe + 1;

			// Caso especial cuando estamos al final de todo
			// Simular una interpolacion al 100% con el next frame
			if (t >= headerCamera.animation_duration) {
				time_in_next = 1.0f;
				next_iframe = headerCamera.ntime_keys;
			}

			if (float(prev_iframe) >= anim.max)
				return true;
			if (float(next_iframe) >= anim.max)
				return true;

			auto prev_key = keys[prev_iframe];
			auto next_key = keys[next_iframe];

			Key k;
			k.trans = XMVectorLerp(prev_key.trans, next_key.trans, time_in_next);
            k.rotation = XMQuaternionSlerp(prev_key.rotation,next_key.rotation, time_in_next);

			Handle h = component::Handle(this).getOwner();
			Entity *me = h;
			CTransform *transform = me->get<CTransform>();

			// set position of camera
			if (anim.type == MaxAnim::PROP_POS)		 transform->setPosition(k.trans);
			else if (anim.type == MaxAnim::PROP_ROT)  transform->setRotation(k.rotation);
			else{
				transform->setPosition(k.trans);
				transform->setRotation(k.rotation);
			}

		}

		return false;
	}

	void CMaxAnim::updateObject(float elapsed){
	
		if (anim.play) {

			anim.curr_time += elapsed * anim.timing;

			if (!anim.finish)
				anim.finish = getTransform(anim.curr_time);
			else{
				if (!anim.postfinish)

					anim.play = false;

				Entity *me = Handle(this).getOwner();
				App &app = App::get();

				if (me->has<CCannon>() && app.getLvl() == 4)
					enableCannon();

				else{
					anim.finish = false;
					anim.curr_time = 0;
				}
			}
		}
	}

	void CMaxAnim::updatePiece(float elapsed){
	

	}

	void CMaxAnim::update(float elapsed){

		if (isPiece())
			updatePiece(elapsed);
		else
			updateObject(elapsed);
	}
}