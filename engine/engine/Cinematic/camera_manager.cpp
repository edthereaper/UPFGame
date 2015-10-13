#include "mcv_platform.h"
#include "camera_manager.h"
#include "App.h"

using namespace component;

namespace cinematic{

	CameraManager CameraManager::manager;

	void CameraManager::onStartElement(const std::string &elem, utils::MKeyValue &atts){

		if (elem == "cinematic"){

			CameraStream camera;
			camera.defaultStream();

			
			std::string s = atts.getString("name", "none");
			camera.name = s;
			
            std::string s_next = atts.getString("next", "-1");
			camera.nextCam = s_next;
			
			camera.level = atts.getInt("level", -1);
			camera.max = atts.getInt("max", -1);
			camera.idx = atts.getInt("idx", -1);

			camera.play = false; // false = stop
			camera.active = false;
			camera.timing = 1.f;
            camera.finish = false;
            camera.time_lapse_finish = 2.f;
            
			streams.push_back(camera);
		}
	}

	int CameraManager::getIndexByCameraName(std::string name){
		
		int idx = 0;
		for (CameraStream c : streams){
			if (c.name.compare(name)==0)
				return idx;
			idx++;
		}
		return 0;
	}

	bool CameraManager::load(const char* name){
		streams.clear();
		char full_name[MAX_PATH];

		int level = App::get().gamelvl;

		sprintf(full_name, "%s/%s.xml", "data/camera", name);
		return xmlParseFile(full_name);
	}

    void CameraManager::alreadyPlayed(std::string name){
        int idx = getIndexByCameraName(name);
        (streams[idx]).finish = true;
        setPlayerActive(true);
    }
}