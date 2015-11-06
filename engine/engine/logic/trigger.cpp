#include "mcv_platform.h"
#include "trigger.h"

#include "gameElements/module.h"
using namespace gameElements;

#include "level/level.h"
using namespace level;

#include "Cinematic/animation_camera.h"
using namespace cinematic;

namespace logic {

void CScriptTrigger::onEnter(float)
{
    std::string scriptName("onEnter");
    scriptName += script;
    //utils::dbg("ENTERED %s\n", script);

	//All the level changes goes here
    //Todo hardcodeado??? Lua está para algo...
	App &app = App::get();

	if (app.getLvl() == 1){
	
		if (strcmp(script, "tolevel2") == 0){
			Entity* player = app.getPlayer();
			CPlayerStats* playerS = player->get<CPlayerStats>();
			app.setGlobalPoints(playerS->getPoints());
			app.setGlobalHealth(playerS->getHealth());
			app.setGlobalEnergy(playerS->getEnergy());
			app.setLvl(2);
		}
		if (strcmp(script, "tutorial1") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(1));
			levi->sendMsg(MsgPlayerInTuto(1));
		}
		if (strcmp(script, "tutorial2") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(2));
			levi->sendMsg(MsgPlayerInTuto(2));
		}
		if (strcmp(script, "tutorial3") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(3));
			levi->sendMsg(MsgPlayerInTuto(3));
		}
		if (strcmp(script, "tutorial4") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(4));
			levi->sendMsg(MsgPlayerInTuto(4));
		}
		if (strcmp(script, "tutorial5") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(5));
			levi->sendMsg(MsgPlayerInTuto(5));
		}
		if (strcmp(script, "tutorial6") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(6));
			levi->sendMsg(MsgPlayerInTuto(6));
		}
		if (strcmp(script, "tutorial7") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(7));
			levi->sendMsg(MsgPlayerInTuto(7));
		}
		if (strcmp(script, "tutorial8") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(8));
			levi->sendMsg(MsgPlayerInTuto(8));
		}
		if (strcmp(script, "tutorial9") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(9));
			levi->sendMsg(MsgPlayerInTuto(9));
		}
	}
	if (app.getLvl() == 2){
		if (strcmp(script, "tolevel3") == 0){
			Entity* player = app.getPlayer();
			CPlayerStats* playerS = player->get<CPlayerStats>();
			app.setGlobalPoints(playerS->getPoints());
			app.setGlobalHealth(playerS->getHealth());
			app.setGlobalEnergy(playerS->getEnergy());
			app.setLvl(3);
		}
	}
	if (app.getLvl() == 3){
		if (strcmp(script, "tolevel4") == 0){
			Entity* player = app.getPlayer();
			CPlayerStats* playerS = player->get<CPlayerStats>();
			app.setGlobalPoints(playerS->getPoints());
			app.setGlobalHealth(playerS->getHealth());
			app.setGlobalEnergy(playerS->getEnergy());
			app.setLvl(4);
		}
		if (strcmp(script, "initsmoke") == 0){
			Entity* levi = app.getBichito();
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerInTuto(10));
			levi->sendMsg(MsgPlayerInTuto(10));
			EntityListManager::get(CSmokeTower::TAG).broadcast(MsgSmokeTower(0));
		}
		if (strcmp(script, "smokephase1") == 0){
			EntityListManager::get(CSmokeTower::TAG).broadcast(MsgSmokeTower(1));
		}
		if (strcmp(script, "smokephase2") == 0){
			EntityListManager::get(CSmokeTower::TAG).broadcast(MsgSmokeTower(2));
		}
		if (strcmp(script, "smokephase3") == 0){
			EntityListManager::get(CSmokeTower::TAG).broadcast(MsgSmokeTower(3));
		}
		if (strcmp(script, "smokephase4") == 0){
			EntityListManager::get(CSmokeTower::TAG).broadcast(MsgSmokeTower(4));
		}
	}
    
	if (strcmp(script, "cinematic") == 0){

		if (strcmp(data, "") != 0){

			int idx = CameraManager::get().getIndexByCameraName(data);
			CameraManager::CameraStream stream = CameraManager::get().getCameraByIndex(idx);

			if (!stream.finish){
				component::Handle camera_h = App::get().getCamera();
				Entity *camera(camera_h);
				CCameraAnim *camanim = camera->get<CCameraAnim>();
				camanim->setStream(stream);
			}
		}
	}
}

void CScriptTrigger::onExit(float)
{
    std::string scriptName("onExit");
    scriptName += script;
    //utils::dbg("EXITED %s\n", script);
	App &app = App::get();
	if (app.getLvl() == 1){
		if (strcmp(script, "tutorial1") == 0 || strcmp(script, "tutorial2") == 0 || strcmp(script, "tutorial3") == 0 ||
			strcmp(script, "tutorial4") == 0 || strcmp(script, "tutorial5") == 0 || strcmp(script, "tutorial6") == 0 ||
			strcmp(script, "tutorial7") == 0 || strcmp(script, "tutorial8") == 0 || strcmp(script, "tutorial9") == 0){
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerOutTuto());
		}
	}
	if (app.getLvl() == 3){
		if (strcmp(script, "initsmoke") == 0){
			Entity* player = app.getPlayer();
			player->sendMsg(MsgPlayerOutTuto());
		}
	}
}
    
void CScriptTrigger::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CScriptTrigger, gameElements::MsgSetPlayer, receive);
}

/*
void CSpatialIndex::onEnter(float)
{
    if (CLevelData::currentLevel.isValid()) {
        CLevelData* level = CLevelData::currentLevel;
        level->setSpatialIndex(spatialIndex);
    }
}
*/

}