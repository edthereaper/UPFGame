#include "mcv_platform.h"
#include "Cinematic/camera_manager.h"
#include "handles/entity.h"
#include "Cinematic/animation_camera.h"
#include "antTW.h"
#include "App.h"

using namespace utils;
using namespace cinematic;

namespace antTw_user {

#ifdef _CINEMATIC_

	TwBar *barCamera;
	std::string stringname;
	component::Entity *camera;
	int currentLevel = 1;
	std::string levelsId[] = {"< Choose level >","level1", "level2", "level3", "level4" };

	//timing
	static void TW_CALL read_speed(void *value, void *clientData)
	{
		if (!CameraManager::get().isPlayerCam()){
			CCameraAnim *cameraAnim = camera->get<CCameraAnim>();
			*static_cast<float *>(value) = cameraAnim->getTiming();
		}
	}

	static void TW_CALL update_speed(const void *value, void *clientData)
	{
		if (!CameraManager::get().isPlayerCam()){
			CCameraAnim *cameraAnim = camera->get<CCameraAnim>();
			float time = *static_cast<const float *>(value);
			cameraAnim->setTiming(time);
		}
	}

	//play
	static void TW_CALL read_play(void *value, void *clientData)
	{
		if (!CameraManager::get().isPlayerCam()){
			CCameraAnim *cameraAnim = camera->get<CCameraAnim>();
			*static_cast<bool *>(value) = cameraAnim->getPlay();
			
		}
	}

	static void TW_CALL update_play(const void *value, void *clientData)
	{
		if (!CameraManager::get().isPlayerCam()){
			CCameraAnim *cameraAnim = camera->get<CCameraAnim>();
			cameraAnim->setPlay(*static_cast<const bool *>(value));
			if (cameraAnim->getPlay()) CameraManager::get().setFreeCam(false);
			else CameraManager::get().setFreeCam(true);
			
		}
	}

	//free
	static void TW_CALL read_free(void *value, void *clientData)
	{
		if (!CameraManager::get().isPlayerCam())
			*static_cast<bool *>(value) = CameraManager::get().isFreeCam();
		
	}

	static void TW_CALL update_free(const void *value, void *clientData)
	{
		if (!CameraManager::get().isPlayerCam())
			CameraManager::get().setFreeCam(*static_cast<const bool *>(value));
	}

	//player
	static void TW_CALL read_player(void *value, void *clientData)
	{
		*static_cast<bool *>(value) = CameraManager::get().isPlayerCam();
	}

	static void TW_CALL update_player(const void *value, void *clientData)
	{
		CameraManager::get().setPlayerActive(*static_cast<const bool *>(value));
	}

	//name
	static void TW_CALL read_camera_name(void *value, void *clientData)
	{
		CameraManager::CameraStream stream = static_cast<const CCameraAnim *>(clientData)->getStream();
		int idx = CameraManager::get().getIndexByCameraName(stream.name);
		*static_cast<int *>(value) = idx;
	}

	static void TW_CALL update_camera_name(const void *value, void *clientData)
	{
		int index = *static_cast<const int *>(value);
		CameraManager::CameraStream stream = CameraManager::get().getCameraByIndex(index);
		stream.play = false;
		static_cast<CCameraAnim *>(clientData)->setStream(stream);

	}

	//level
	static void TW_CALL read_level(void *value, void *clientData)
	{
		*static_cast<int *>(value) = currentLevel;
	}

	static void TW_CALL update_level(const void *value, void *clientData)
	{
		int index = *static_cast<const int *>(value);

		if (currentLevel != index && index > 0){
			App &app = App::get();
			app.changelvl = true;
			app.gamelvl = index;
			currentLevel = index;

			int count = CameraManager::get().count();

			if (count > 0){
				CameraManager::CameraStream stream = CameraManager::get().getCameraByIndex(0);
				stream.play = false;
				static_cast<CCameraAnim *>(clientData)->setStream(stream);
			}

		}
	}

	void AntTWManager::createCameraTweak() {

		barCamera = TwNewBar("camera_manager");
		TwDefine("camera_manager position = '10 20' size = '250 100'");

		CameraManager::CameraStreamList storage = CameraManager::get().getCamerasCinematic();

		auto camera_h = App::get().getCamera();
		camera = camera_h;
		
		CCameraAnim *cameraAnim = camera->get<CCameraAnim>();
		cameraAnim->init();

		std::string defineLevel = "";
		TwType levelType;
		for (int i = 0; i < 5; i++){
			if (i == 4) defineLevel += levelsId[i];
			else defineLevel += levelsId[i] + ",";
		}

		levelType = TwDefineEnumFromString("level", defineLevel.c_str());
		TwAddVarCB(barCamera, "level:", levelType,
			update_level, read_level,
			NULL, "");

		int idx = 0;
		std::string defineEnum = "";
		TwType densityType;

		for (auto &ps : storage){
			if (idx == storage.size() - 1) defineEnum += ps.name;
			else defineEnum += ps.name + ",";
		}

		densityType = TwDefineEnumFromString("storage", defineEnum.c_str());
		TwAddVarCB(barCamera, "load cam:", densityType,
			update_camera_name, read_camera_name,
			cameraAnim, "");
		TwAddVarCB(barCamera, "timing", TW_TYPE_FLOAT, update_speed, read_speed, cameraAnim, "min=-10 max=10 step=0.01");
		TwAddVarCB(barCamera, "play", TW_TYPE_BOOL8, update_play, read_play, cameraAnim, "true=1 false=0");


		int count = CameraManager::get().count();

		if (count > 0){
			CameraManager::CameraStream stream = CameraManager::get().getCameraByIndex(0);
			stream.play = false;
			cameraAnim->setStream(stream);
		}

	}

#endif

}
