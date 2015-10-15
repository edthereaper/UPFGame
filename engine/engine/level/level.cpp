#include "mcv_platform.h"
#include "level.h"

#include "logic/trigger.h"
using namespace logic;

#include "../gameElements/gameMsgs.h"
using namespace gameElements;

namespace level {

Handle CLevelData::currentLevel;

void CCheckPoint::onEnter(float)
{
    Entity* level_E(levelEntity_h);
    assert(level_E != nullptr);
    CLevelData* level(level_E->get<CLevelData>());
	//Check if the new checkpoint follows the logic of the game, if -1 is ok
	CCheckPoint* currentCheckPoint = level->getCurrentCheckPoint();
	if (level->isBossLevel() || order > currentCheckPoint->order || order == -1){
		level->setCurrentCheckPoint(this);
		level_E->sendMsg(MsgPlayerAchievedCheckpoint());
	}
}

void CLevelData::setSpawnCheckPoint(component::Handle checkPoint_h)
{
    if(spawnCheckPoint.isValid()) {
        ((CCheckPoint*)spawnCheckPoint)->setSpawn(false);
    }
    spawnCheckPoint = checkPoint_h;
    assert(spawnCheckPoint.isValid());
    ((CCheckPoint*)spawnCheckPoint)->setSpawn(true);
}

void CLevelData::setCurrentCheckPoint(component::Handle checkPoint_h)
{
    if(currentCheckPoint.isValid()) {
        ((CCheckPoint*)currentCheckPoint)->setCurrent(false);
    }
    currentCheckPoint = checkPoint_h;
    ((CCheckPoint*)currentCheckPoint)->setCurrent(true);
}

void CCheckPoint::initType()
{
    SUBSCRIBE_MSG_TO_MEMBER(CCheckPoint, gameElements::MsgSetPlayer, receive);
}

void CLevelData::DataReader::onStartElement(
    const std::string &elem, utils::MKeyValue &atts)
{
    assert(lvl != nullptr);
    lvl->loadFromProperties(elem, atts);
}
void CLevelData::loadFromProperties(
    const std::string &elem, utils::MKeyValue &atts)
{
    if (atts.has("song")) {
        fmodUser::FmodStudio::loadBank("Music");
        std::string songName = atts.getString("song", "brote");
        song = fmodUser::FmodStudio::getEventInstance("Music/"+songName);
    }
    bossLevel = atts.getBool("boss", bossLevel);
    zFar = atts.getFloat("zFar", zFar);
}
const float CLevelData::DEFAULT_ZFAR = 200.f;

}