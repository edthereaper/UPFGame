#ifndef GAME_ELEMENTS_MESSAGES_H_
#define GAME_ELEMENTS_MESSAGES_H_

#include "handles/message.h"
using namespace component;

namespace gameElements {

struct MsgWeakSpotBreak {
    DECLARE_MSG_ID();
    component::Handle weakSpot;

    MsgWeakSpotBreak(component::Handle weakSpot) : weakSpot(weakSpot) {}
};

struct MsgFlyingMobileEnded {    
    DECLARE_MSG_ID();
};

struct MsgActivateSmokeWarning {    
    DECLARE_MSG_ID();
    uint32_t mask;

    MsgActivateSmokeWarning(uint32_t mask) : mask(mask) {}
};

//Activate smoke if it was previously "heated" by MsgActivateSmokeWarning
struct MsgActivateSmokeIfHot {    
    DECLARE_MSG_ID();
};

struct MsgDeactivateSmoke {
    DECLARE_MSG_ID();
};

/* Player acquired or lost invencibility */
struct MsgPlayerHasInvencible {
	DECLARE_MSG_ID();
	bool state;

	MsgPlayerHasInvencible(bool state) : state(state) {}
};

/* Player acquired or lost megashot */
struct MsgPlayerHasMegashot {
	DECLARE_MSG_ID();
	bool state;

	MsgPlayerHasMegashot(bool state) : state(state) {}
};

/* Setup message: the player entity handler has changed */
struct MsgSetPlayer {
	DECLARE_MSG_ID();
	Handle playerEntity;

	MsgSetPlayer(Handle playerEntity) :
		playerEntity(playerEntity) {}
};

/* Setup message: the cam entity handler has changed */
struct MsgSetCam {
	DECLARE_MSG_ID();
	Handle camEntity;

	MsgSetCam(Handle camEntity) :
		camEntity(camEntity) {}
};

/* Setup message: the bichito entity handler has changed */
struct MsgSetBichito {
	DECLARE_MSG_ID();
	Handle bichitoEntity;

	MsgSetBichito(Handle bichitoEntity) :
		bichitoEntity(bichitoEntity) {}
};

/*set handle to Emitter Particles*/
struct MsgSetParticlesEmitter{
	DECLARE_MSG_ID();
	Handle emitterEntity;

	MsgSetParticlesEmitter(Handle emitterEntity_) :
		emitterEntity(emitterEntity_) {}
};


/* Teleport to player */
struct MsgTeleportToPlayer {
	DECLARE_MSG_ID();
};

/* Flare sync message */
struct MsgFlareSync {
	DECLARE_MSG_ID();
};

/* Player Spawn*/
struct MsgPlayerSpawn {
	DECLARE_MSG_ID();
};

/* Player has shoot */
struct MsgPlayerHasShot {
	DECLARE_MSG_ID();
};

/*set Handle*/

/* Player meets trampoline */
struct MsgPushTrampoline {
	DECLARE_MSG_ID();
	Handle TrampolineEntity;

	MsgPushTrampoline(Handle TrampolineEntity) :
		TrampolineEntity(TrampolineEntity) {}
};

/* Entity begins transformation */
struct MsgTransform {
	DECLARE_MSG_ID();
};

/* Entity begins transformation */
struct MsgRevive {
	DECLARE_MSG_ID();
};

/* SmokeTower begins */
struct MsgSmokeTower {
	DECLARE_MSG_ID();
	unsigned phase;

	MsgSmokeTower(unsigned phase) : phase(phase) {}
};

//----------------------------------------------------------------------
struct MsgCameraCinematicSetPlayer{
    DECLARE_MSG_ID();
    Handle player;
    MsgCameraCinematicSetPlayer(Handle player_) :
		player(player_) {}
};

struct MsgCameraCinematicSetCam{
	DECLARE_MSG_ID();
	const char* cam;
	MsgCameraCinematicSetCam(const char *cam_) :
		cam(cam_) {}
};
//----------------------------------------------------------------------

/* Player achieved checkpoint */
struct MsgPlayerAchievedCheckpoint {
	DECLARE_MSG_ID();
};

/* SmokeTower Reset Phase */
struct MsgSmokeTowerResetPhase {
	DECLARE_MSG_ID();
};

/* Entity is alerted because Vinedetta appeared*/
struct MsgAlert {
	DECLARE_MSG_ID();
};

/* Entity get stunned */
struct MsgStun {
	DECLARE_MSG_ID();
};

/* Entity appear from Boss */
struct MsgAppearFromBoss {
	DECLARE_MSG_ID();
};

/* Entity appear from Boss */
struct MsgFinishMovementFromBoss {
	DECLARE_MSG_ID();
};

/* Entity is defending because Vinedetta is not recheable*/
struct MsgProtect {
	DECLARE_MSG_ID();
};

/* Entity has to stop defense*/
struct MsgStopDefense {
	DECLARE_MSG_ID();
};

/* Player has low HP*/
struct MsgPlayerLowHP {
	DECLARE_MSG_ID();
};

/* Player was hit by a melee attack */
struct MsgMeleeHit {
	DECLARE_MSG_ID();
	unsigned damage;

	MsgMeleeHit() : damage(5) {}
	MsgMeleeHit(unsigned damage) : damage(damage) {}
};

/* Player was hit by a flare shot */
struct MsgFlareHit {
	DECLARE_MSG_ID();
	unsigned damage;

	MsgFlareHit() : damage(10) {}
	MsgFlareHit(unsigned damage) : damage(damage) {}
};

/* Object was shot by a player's bullet */
struct MsgShot {
	DECLARE_MSG_ID();
	bool mega;

	MsgShot() : mega(false) {}
	MsgShot(bool mega) : mega(mega) {}
};

/* Player collided with health pickup */
struct MsgPickupHeal {
	DECLARE_MSG_ID();
	unsigned heal;

	MsgPickupHeal() : heal(5) {}
	MsgPickupHeal(unsigned heal) : heal(heal) {}
};

/* Player collided with energy pickup */
struct MsgPickupEnergy {
	DECLARE_MSG_ID();
	unsigned absorbEnergy;

	MsgPickupEnergy() : absorbEnergy(5) {}
	MsgPickupEnergy(unsigned absorbEnergy) : absorbEnergy(absorbEnergy) {}
};

/* Player collided with invencible pickup */
struct MsgPickupInvincible {
	DECLARE_MSG_ID();
	float invencible;

	MsgPickupInvincible() : invencible(7.5f) {}
	MsgPickupInvincible(float invencible) : invencible(invencible) {}
};

/* Player collided with coin pickup */
struct MsgPickupCoin {
	DECLARE_MSG_ID();
	unsigned absorbCoin;

	MsgPickupCoin() : absorbCoin(1) {}
	MsgPickupCoin(unsigned absorbCoin) : absorbCoin(absorbCoin) {}
};

/* Player collided with collectible pickup */
struct MsgPickupCollectible {
	DECLARE_MSG_ID();
	unsigned absorbColectible;

	MsgPickupCollectible() : absorbColectible(1) {}
	MsgPickupCollectible(unsigned absorbColectible) : absorbColectible(absorbColectible) {}
};


/* Object is affected by an earthquake */
struct MsgEarthquake {
	DECLARE_MSG_ID();
};

/* Object is tackled by a dash */
struct MsgDashTackled {
	DECLARE_MSG_ID();
};

/* Player is dead */
struct MsgPlayerDead {
	DECLARE_MSG_ID();
};

struct MsgPlayerTutorial{
	DECLARE_MSG_ID();

};

struct MsgPlayerTrampoline{
	DECLARE_MSG_ID();
	Handle trampoline;

	MsgPlayerTrampoline(Handle trampoline) : trampoline(trampoline) {}
};

struct MsgPlayerCannnon{
	DECLARE_MSG_ID();
	Handle cannon;
	MsgPlayerCannnon(Handle cannon) : cannon(cannon) {}
};


//--------------------------------------
struct MsgEmitterParticles{
	DECLARE_MSG_ID()
	Handle handle;
	MsgEmitterParticles(Handle h_) : handle(h_){}
};

struct MsgActiveParticles{
	DECLARE_MSG_ID()
	MsgActiveParticles(){}
};

struct MsgActiveParticlesByTime{
	DECLARE_MSG_ID()
	float timer;
	MsgActiveParticlesByTime(float t){ timer = t; }
};

struct MsgInactiveParticlesByTime{
	DECLARE_MSG_ID()
	float timer;
	MsgInactiveParticlesByTime(float t){ timer = t; }
};

struct MsgInactiveParticles{
	DECLARE_MSG_ID()
	MsgInactiveParticles(){}
};

struct MsgResetParticles{
	DECLARE_MSG_ID()
	MsgResetParticles(){}
};

struct MsgResetByTimeParticles{
	DECLARE_MSG_ID()
	float timer;
	MsgResetByTimeParticles(float timer_){}
};

struct MsgKillParticlesInmidiatly{
	DECLARE_MSG_ID()
	MsgKillParticlesInmidiatly(){}
};

struct MsgKillParticlesByTime{
	DECLARE_MSG_ID()
	float timer;
	MsgKillParticlesByTime(float t){ timer = t; }
};
//-----------------------------------



struct MsgPlayerLiana{
	DECLARE_MSG_ID();
};

struct MsgPlayerCreep{
	DECLARE_MSG_ID();
	Handle creep;

	MsgPlayerCreep(Handle creep) : creep(creep) {}
};

/* Player entered to trigger of tutorial */
struct MsgPlayerInTuto {
	DECLARE_MSG_ID();
	unsigned tutoZone;

	MsgPlayerInTuto(unsigned tutoZone) : tutoZone(tutoZone) {}
};

/* Player exit trigger of tutorial*/
struct MsgPlayerOutTuto{
	DECLARE_MSG_ID();
};
}
#endif