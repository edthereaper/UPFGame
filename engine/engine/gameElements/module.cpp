#include "mcv_platform.h"
#include "module.h"
#include "gameMsgs.h"

namespace gameElements {

void initGameElements()
{
    initPropTypes();
	SUBSCRIBE_MSG_TO_MEMBER(CPickup, MsgSetPlayer, receive);
	CSmokeTower::initType();
    CLiana::initType();
    CEnemy::initType();
    CMelee::initType();
    CKnife::initType();
    CFlare::initType();
    CFlareShot::initType();
    CBoss::initType();
    CWeakSpot::initType();
    CSmokeTower::initType();
    CPlayerMov::initType();
    CPlayerAttack::initType();
    CPlayerStats::initType();
    CBullet::initType();
	CBichito::initType();
    CSmokePanel::initType();
    WeakSpotFSM::initType();
    FlyingMobileFSM::initType();
    SmokePanelFSM::initType();
	particles::CParticleSystem::initType();
	CTextHelper::initType();
}

}