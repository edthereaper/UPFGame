#include "mcv_platform.h"
#include "animation_camera.h"
#include "gameElements/input.h"

using namespace gameElements;

namespace cinematic{

	void CCameraAnim::init(){

#if defined(_CINEMATIC_)
		    stream.defaultStream();
		    curr_time = 0;
#else
             
      //      stream.defaultStream();
		    //curr_time = 0;
      //       //--- test, super test---// 
      //      //BORRAR CUANDO YA SE IMPLEMENTE LOS TRIGGERS DE CINEMATICS  
      //      int index = CameraManager::get().getIndexByCameraName("Camera002");
      //      setStream(CameraManager::get().getCameraByIndex(index));
           
#endif
	}

	bool CCameraAnim::load(const char*name) {

		int level = App::get().getLvl();

		char full_name[MAX_PATH];
		sprintf(full_name, "%s/level%i/%s.anim", "data/camera",level, name);

		FileDataProvider fdp(full_name);
		if (!fdp.isValid())
			fatal("NOT LOADED");
		return load(fdp);
	}


	bool CCameraAnim::load(DataProvider& dp) {
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
	bool CCameraAnim::getTransform(float t) const {


		if (t < 0)
			return true;

		// If they ask for time beyond the animation duration we will clamp
		// and return true

		if (t > float(stream.max)) {
			t = float(stream.max);
			return true;
		}
		else{

			// Convert t to frame key
			// 1.1s => 2.22f
			float frame_number = t * headerCamera.ntime_keys / headerCamera.animation_duration;

			unsigned prev_iframe = unsigned(std::floor(frame_number));
			float time_in_next = frame_number - prev_iframe;
			assert(time_in_next >= 0 && time_in_next <= 1.f);
			unsigned next_iframe = prev_iframe + 1;

			// Caso especial cuando estamos al final de todo
			// Simular una interpolacion al 100% con el next frame
			if (t >= headerCamera.animation_duration) {
				time_in_next = 1.0f;
				next_iframe = headerCamera.ntime_keys;
			}

			if (float(prev_iframe) > stream.max) {return true;}
			if (float(next_iframe) > stream.max) {return true;}

			auto prev_key = keys[prev_iframe];
			auto next_key = keys[next_iframe];

			Key k;
			k.trans  = XMVectorLerp(prev_key.trans, next_key.trans, time_in_next);
			k.target = XMVectorLerp(prev_key.target, next_key.target, time_in_next);

			component::CTransform * camT = Handle(this).getBrother<component::CTransform>();

			// set position of camera
			camT->setPosition(k.trans);
			camT->setScale(one_v);

			// align to target to 3d point
			lookAt3D(camT, k.target);
		}

		return false;
	}


	void CCameraAnim::update(float elapsed){

#if defined(_CINEMATIC_)

		if (stream.play) {

			curr_time += elapsed * stream.timing;
			if (!finish) 
				finish = getTransform(curr_time);
			else{
				curr_time = 0;
				finish = false;
			}
		}

#else

		if (stream.play && !stream.finish) {

			curr_time += elapsed * stream.timing;

			if (!finish) {
				finish = getTransform(curr_time);
			}
			else{
				if (counterFinish.count(elapsed) >= stream.time_lapse_finish){
					CameraManager::get().alreadyPlayed(stream.name);
					stream.finish = true;
				}                     
		    }
		}
    
#endif
    }


}