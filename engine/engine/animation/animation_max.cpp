#include "mcv_platform.h"
#include "animation_max.h"
#include "gameElements/input.h"
#include "handles/entity.h"
#include "components/transform.h"

using namespace gameElements;
using namespace component;

namespace animation{

	void CMaxAnim::init(){
	
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

	void CMaxAnim::loadFromProperties(const std::string& elem, utils::MKeyValue &atts){
	
		if (elem == "MaxAnim"){
		
			std::string name = atts.getString("name", "INVALID");
			max = atts.getInt("max");
			play = atts.getInt("isOn") ? true : false;
			std::string type_ = atts.getString("type");

			if (type_.compare("PROP") == 0)
				type = MaxAnim::PROP_POS;
			else if (type_.compare("ROT") == 0)
				type = MaxAnim::PROP_ROT;
			else
				type = MaxAnim::PROP_POS_ROT;

			load(name.c_str());
		}

	}

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

	// Return true if the we went beyond the animation duration
	bool CMaxAnim::getTransform(float t) const {


		if (t < 0)
			return true;

		// If they ask for time beyond the animation duration we will clamp
		// and return true

		if (t > float(max)) {
			t = float(max);
			return true;
		}
		else{

			// Convert t to frame key
			// 1.1s => 2.22f
			float frame_number = t * (headerCamera.ntime_keys - 1) / headerCamera.animation_duration;

			// The integer part 2
			unsigned   prev_iframe = static_cast<int>(frame_number);
			// The decimal part = 0.22
			float time_in_next = frame_number - prev_iframe;
			assert(time_in_next >= 0 && time_in_next <= 1.f);
			unsigned   next_iframe = prev_iframe + 1;

			// Caso especial cuando estamos al final de todo
			// Simular una interpolacion al 100% con el next frame
			if (next_iframe == headerCamera.ntime_keys) {
				prev_iframe = next_iframe - 2;
				time_in_next = 1.0f;
				next_iframe = next_iframe - 1;
			}


			if (float(prev_iframe) >= max)
				return true;
			if (float(next_iframe) >= max)
				return true;

			auto prev_key = keys.begin() + prev_iframe;
			auto next_key = keys.begin() + next_iframe;

			Key k;
			k.trans = prev_key->trans + time_in_next * (next_key->trans - prev_key->trans);
			k.rotation = prev_key->rotation + time_in_next * (next_key->rotation - prev_key->rotation);


			Handle h = component::Handle(this).getOwner();
			Entity *me = h;
			CTransform *transform = me->get<CTransform>();

			// set position of camera
			if (type == MaxAnim::PROP_POS)		 transform->setPosition(k.trans);
			else if (type == MaxAnim::PROP_ROT)  transform->setRotation(k.rotation);
			else{
				transform->setPosition(k.trans);
				transform->setRotation(k.rotation);
			}

		}

		return false;
	}

	void CMaxAnim::update(float elapsed){

		if (play) {

			curr_time += elapsed * timing;

			if (!finish) 
				finish = getTransform(curr_time);
			else{

				if (postfinish) play = false;
				else{ 
					finish = false; 
					curr_time = 0;
				}
			}
		}
	}
}