#include "mcv_platform.h"
#include "playerstats.h"

#include "playerMov.h"

using namespace component;

#include "animation/animationPlugger.h"
using namespace animation;

#include "gameElements\Ambientsound.h"

#define TIME_FALLING_LEAF			1.0f
#define TIME_HEALING_LEAF			1.0f
#define TIME_MOVING_LEAF			0.5f
#define DISTANCE_FALL_LEAF			60
#define SPEED_SIN_MOVING_LEAF		31.4f
#define FORCE_SIN_MOVING_LEAF		10
#define SPEED_SPIN_FALLING_LEAF		480

namespace behavior {

	MegashotFSM::container_t MegashotFSM::states;
	InvencibleFSM::container_t InvencibleFSM::states;

	void MegashotFSM::initType()
	{
		SET_FSM_STATE(check);
		SET_FSM_STATE(activate);
		SET_FSM_STATE(during);
		SET_FSM_STATE(ending);
		SET_FSM_STATE(ended);
	}

	void InvencibleFSM::initType()
	{
		SET_FSM_STATE(check);
		SET_FSM_STATE(activate);
		SET_FSM_STATE(during);
		SET_FSM_STATE(ending);
		SET_FSM_STATE(ended);
	}

}

//Tiempo total es: TIME_ACTIVATE_MEGASHOT + TIME_MEGASHOT + TIME_ENDED_MEGASHOT

#define TIME_ACTIVATE_MEGASHOT          0.4f
#define TIME_ENDED_MEGASHOT             0.3f
#define TIME_ACTIVATE_INVENCIBLE        TIME_ACTIVATE_MEGASHOT
#define TIME_ENDED_INVENCIBLE           TIME_ENDED_MEGASHOT

namespace gameElements {

	//////////

	fsmState_t MegashotFSMExecutor::check(float elapsed)
	{
		if (powerUpTime <= 0) {
			return STATE_check;
		} else {
			active = true;
			return STATE_activate;
		}
	}

	fsmState_t MegashotFSMExecutor::activate(float elapsed)
	{
		if (timer.count(elapsed) < TIME_ACTIVATE_MEGASHOT) {
			return STATE_activate;
		} else {
            regularSaturation = RenderConstsMirror::ResolveSaturation;
            regularBrightness = RenderConstsMirror::ResolveBrightness;
            regularContrast = RenderConstsMirror::ResolveContrast;
            CMesh* mesh = meEntity.getSon<CMesh>();
            regularSelfIll = mesh->getDiffuseAsSelfIllumination();

			timer.reset();
			((Entity*)meEntity)->sendMsg(MsgPlayerHasMegashot(true));
			return STATE_during;
		}
	}
    
    #define SATURATION_INC 0.0075f
    #define BRIGHTNESS_INC 0.035f
    #define CONTRAST_INC 0.1f
    #define SELFILL_INC 0.04f

    #define RAMP_UP 0.1f
    #define RAMP_DOWN (1-0.025f)
    #define SINE_THRESHOLD 0.75f
    static float changeInParams(float f, float sineAmp = 0.35f,
        float sine_threshold = SINE_THRESHOLD, float sineCycles = 3.f)
    {
        f = inRange(0.f, f, 1.f);
        if (f<RAMP_UP) {
            float a = f/RAMP_UP;
            return std::sqrt(a);
        } else if (f < sine_threshold) {
            return 1;
        } else if (f < RAMP_DOWN) {
            float a = M_2_PIf*(f-SINE_THRESHOLD)/(RAMP_DOWN-SINE_THRESHOLD);
            return 1-sineAmp+sineAmp*std::cos(a*sineCycles);
        } else {
            float a = (1-f)/(1-RAMP_DOWN);
            return std::sqrt(a);
        }
    
    }

    float MegashotFSMExecutor::getMegashotFactor() const {
        return (powerUpTime == 0) ? 0 : changeInParams(timer/powerUpTime);
    }
	
	float hudEarthquake = 0;
	
	fsmState_t MegashotFSMExecutor::during(float elapsed)
	{
		float tim = timer.count(elapsed);
		if (tim < powerUpTime) {
            float f = timer/powerUpTime;
            float var = changeInParams(f);
			hudEarthquake = abs(std::sin(tim * 4));
            RenderConstsMirror::ResolveSaturation = 
                regularSaturation + SATURATION_INC*var;
            RenderConstsMirror::ResolveContrast =
                regularContrast + CONTRAST_INC*var;
            RenderConstsMirror::ResolveBrightness =
                regularBrightness + BRIGHTNESS_INC*var;
            CMesh* mesh = meEntity.getSon<CMesh>();
            mesh->setDiffuseAsSelfIllumination(
                regularSelfIll + SELFILL_INC*changeInParams(f));
            RenderConstsMirror::update();
            
			return STATE_during;
		} else {
			return STATE_ending;
		}
	}

	fsmState_t MegashotFSMExecutor::ending(float elapsed)
	{
		if (timer.count(elapsed) < 0) {
			return STATE_ending;
		} else {
			timer.reset();
			((Entity*)meEntity)->sendMsg(MsgPlayerHasMegashot(false));
            
            RenderConstsMirror::ResolveSaturation = regularSaturation;
            RenderConstsMirror::ResolveContrast = regularContrast;
            RenderConstsMirror::ResolveBrightness = regularBrightness;
            RenderConstsMirror::update();
			
			hudEarthquake = 0.0f;
			active = false;
			powerUpTime = 0;
			return STATE_ended;
		}
	}

	fsmState_t MegashotFSMExecutor::ended(float elapsed)
	{
		if (timer.count(elapsed) < TIME_ENDED_MEGASHOT) {
			return STATE_ended;
		} else {
			timer.reset();
			return STATE_check;
		}
	}

	/////////

	fsmState_t InvencibleFSMExecutor::check(float elapsed)
	{
		if (powerUpTime <= 0) {
			return STATE_check;
		}
		else {
			return STATE_activate;
		}
	}

	fsmState_t InvencibleFSMExecutor::activate(float elapsed)
	{
		if (timer.count(elapsed) < TIME_ACTIVATE_INVENCIBLE) {
			return STATE_activate;
		}
		else {
			timer.reset();
			timer.count(0);
			((Entity*)meEntity)->sendMsg(MsgPlayerHasInvencible(true));
			active = true;
			return STATE_during;
		}
	}

	fsmState_t InvencibleFSMExecutor::during(float elapsed)
	{
		if (timer.count(elapsed) < powerUpTime) {
			return STATE_during;
		}
		else {
			return STATE_ending;
		}
	}

	fsmState_t InvencibleFSMExecutor::ending(float elapsed)
	{
		if (timer.count(elapsed) < 0) {
			return STATE_ending;
		}
		else {
			timer.reset();
			((Entity*)meEntity)->sendMsg(MsgPlayerHasInvencible(false));
			active = false;
			powerUpTime = 0;
			return STATE_ended;
		}
	}

	fsmState_t InvencibleFSMExecutor::ended(float elapsed)
	{
		if (timer.count(elapsed) < TIME_ENDED_INVENCIBLE) {
			return STATE_ended;
		}
		else {
			timer.reset();
			return STATE_check;
		}
	}

	////////

	void CPlayerStats::update(float elapsed) {
		const auto& pad(App::get().getPad());
		CPlayerMov* playerMov = Handle(this).getBrother<CPlayerMov>();
		if ((playerMov->getState() & PlayerMovBtExecutor::COD_CAN_QUAKE) != 0) {
			if (    
            (energy >= ENERGY_MEGAPOWER
#ifdef _DEBUG
            || App::get().infiniteEnergy
#endif
            )  
                && pad.getState(CONTROLS_ACTIVATE_MEGAPOWER).isHit() && !hasMegashot()) {	
#ifdef _DEBUG
            if (!App::get().infiniteEnergy)
#endif
				consumeEnergy(ENERGY_MEGAPOWER);
				playerMov->pushEarthQuake();
				megashot.getExecutor().start(TIME_MEGASHOT);
			}
		}
		megashot.update(elapsed);
		invencible.update(elapsed);
		float tim = timerPlayerDead.count(elapsed);
		if (playerDead){
			if(tim >= timeDeathAnim) {
				timerPlayerDead.reset();
				playerDead = false;
				App &app = App::get();
				app.isPlayerDead = true;
				getManager<CAmbientSound>()->forall(&CAmbientSound::stopSound);
				fmodUser::fmodUserClass::stopSounds();
			}else{
				fadeoutTim = tim;
			}
		}
		elapsedDmg += elapsed;
	}

	void CPlayerStats::heal(unsigned amount)
	{
		if(health == MAX_LIFE) return;
		if (health + amount > MAX_LIFE) health = MAX_LIFE;
		else if (health>0) health += amount;
		playerHealed = true;
	}

	void CPlayerStats::revive(const MsgRevive&)
	{
		health = MAX_LIFE;
		energy = MAX_ENERGY;
		points = pointsCheckpoint;
		pointsCollectible = pointsCollectibleCheckpoint;
		playerDead = false;
		Entity* me = Handle(this).getOwner();
		CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
		animPlugger->plug(PLUG_DEATH_STOP);
		getManager<CAmbientSound>()->forall(&CAmbientSound::playSound);

        CCharacterController* charCo = me->get<CCharacterController>();
        charCo->liberate();
	}

	void CPlayerStats::absorbEnergy(unsigned amount)
	{
		if (energy + amount > 100) energy = 100;
		else if (energy >= 0) energy += amount;
	}

	void CPlayerStats::consumeEnergy(unsigned amount)
	{
		if (energy - amount >= 0) energy -= amount;
	}

	void CPlayerStats::damage(unsigned amount, bool dmgFromFalling)
	{
		Entity* me = Handle(this).getOwner();
#ifdef _DEBUG
        if (App::get().godMode) {return;}
        
#endif
		if (!hasInvencibility() && health > 0 && elapsedDmg >= TIME_GET_DMG && ((
			(((CPlayerMov*)me->get<CPlayerMov>())->getPreviousAction()
			& PlayerMovBtExecutor::COD_DAMAGE_PROTECT) == 0))) {
			CAnimationPlugger* animPlugger(me->get<CAnimationPlugger>());
			elapsedDmg = 0;
			health -= amount;
			playerDamaged = true;
			if (health > 0){
				animPlugger->plug(PLUG_DAMAGE);
				if (health > HP_ALERT){
					alertLowHP = true;
				}
				if (health <= HP_ALERT && alertLowHP){
					((Entity *)bichitoEntity)->sendMsg(MsgPlayerLowHP());
					alertLowHP = false;
				}
			} else {
                me->sendMsg(MsgPlayerDead());	
				((Entity *)bichitoEntity)->sendMsg(MsgPlayerDead());				
				EntityListManager::get(CEnemy::TAG).broadcast(MsgPlayerDead());				
				health = 0;
				playerDead = true;
				if (!dmgFromFalling){
					animPlugger->plug(PLUG_DEATH); //TODO -> delegate this to normal BT behavior
					timeDeathAnim = TIME_ANIM_DEAD;
				} else {
					timeDeathAnim = TIME_ANIM_DEAD;
					((CPlayerMov*)me->get<CPlayerMov>())->setDeathByHole();
				}
				timerPlayerDead.reset();
			}
		}
	}

	void CPlayerStats::sumPoints(unsigned amount)
	{
		Entity* me = Handle(this).getOwner();
		if (health > 0) {
			points += amount;
		}
	}

	void CPlayerStats::sumCollectible(unsigned amount)
	{
		Entity* me = Handle(this).getOwner();
		if (health > 0) {
			pointsCollectible += amount;
		}
	}

	void CPlayerStats::checkPoint(){
		pointsCheckpoint = points;
		pointsCollectibleCheckpoint = pointsCollectible;
	}

	void CPlayerStats::drawHUD(float elapsed)
	{		
		float imgWidth		= 0;
		float imgHeight		= 0;
		float imgPosX		= 0;
		float imgPosY		= 0;

		App &app = App::get();

		std::string strPoints;
		int first = points / 100 % 10;
		int second =  points / 10 % 10;
		int third = points % 10;

		app.getImgValues(imgPosX, imgPosY, imgWidth, imgHeight, 1150, 8, 95, 45);
		drawTexture2D(pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
			pixelRect(app.config.xres, app.config.yres), Texture::getManager().getByName("cartel_num"), nullptr, true);
		if (first == 0){
			if (second == 0){
				strPoints = "  " + std::to_string(third);
			}
			else{
				strPoints = " " + std::to_string(second) + std::to_string(third);
			}	
		}
		else{
			strPoints = std::to_string(first) + std::to_string(second) + std::to_string(third);
		}
		app.getImgValues(imgPosX, imgPosY, imgWidth, imgHeight, 1163, 18, 25, 25);

		drawText(pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
            pixelRect(app.config.xres, app.config.yres), strPoints);

		drawEnergyBar(pixelRect(5, 5, 250, 250), pixelRect(app.config.xres, app.config.yres), float(energy));
		drawTexture2D(pixelRect(5, 5, 250, 250), pixelRect(app.config.xres, app.config.yres), 
					Texture::getManager().getByName("static_life"), nullptr, true);
		if (hudEarthquake>0.0f)
			drawTextureFadeOut(pixelRect(5, 5, 250, 250), pixelRect(app.config.xres, app.config.yres),
				Texture::getManager().getByName("static_life_earthquake"), hudEarthquake);

		if(playerDamaged){
			//dbg("DMG health lastFrameLife: %i %i\n", health, lastFrameLife);
			if(health <= 140 && lastFrameLife > 140){
				//Leaf 5 goes to yellow
				playAnimation5Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 130 && lastFrameLife > 130){
				//Leaf 5 goes to red
				playAnimation5Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 120 && lastFrameLife > 120){
				//Leaf 5 falls
				playAnimation5Fall = true;
				playAnimation5Move = false;
				totalTimeFall = 0.0f;
				fallLeafHeight = 0;
			}
			if(health <= 110 && lastFrameLife > 110){
				//Leaf 4 goes to yellow
				playAnimation4Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 100 && lastFrameLife > 100){
				//Leaf 4 goes to red
				playAnimation4Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 90 && lastFrameLife > 90){
				//Leaf 4 falls
				playAnimation4Fall = true;
				playAnimation4Move = false;
				totalTimeFall = 0.0f;
				fallLeafHeight = 0;
			}
			if(health <= 80 && lastFrameLife > 80){
				//Leaf 3 goes to yellow
				playAnimation3Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 70 && lastFrameLife > 70){
				//Leaf 3 goes to red
				playAnimation3Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 60 && lastFrameLife > 60){
				//Leaf 3 falls
				playAnimation3Fall = true;
				playAnimation3Move = false;
				totalTimeFall = 0.0f;
				fallLeafHeight = 0;
			}
			if(health <= 50 && lastFrameLife > 50){
				//Leaf 2 goes to yellow
				playAnimation2Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 40 && lastFrameLife > 40){
				//Leaf 2 goes to red
				playAnimation2Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 30 && lastFrameLife > 30){
				//Leaf 2 falls
				playAnimation2Fall = true;
				playAnimation2Move = false;
				totalTimeFall = 0.0f;
				fallLeafHeight = 0;
			}
			if(health <= 20 && lastFrameLife > 20){
				//Leaf 1 goes to yellow
				playAnimation1Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 10 && lastFrameLife > 10){
				//Leaf 1 goes to red
				playAnimation1Move = true;
				totalTimeMove = 0.0f;
			}
			if(health <= 0 && lastFrameLife > 0){
				//Leaf 1 falls
				playAnimation1Fall = true;
				playAnimation1Move = false;
				totalTimeFall = 0.0f;
				fallLeafHeight = 0;
			}
			playerDamaged = false;
		}
		if(playerHealed){
			if(health > 140){
				if(lastFrameLife <= 140){
					playAnimation5Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 130){
				if(lastFrameLife <= 130){
					playAnimation5Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 120){
				if(lastFrameLife <= 120){
					playAnimation5Fall = false;
					playAnimation5Heal = true;
					totalTimeHeal = 0.0f;
				}
			}
			else if(health > 110){
				if(lastFrameLife <= 110){
					playAnimation4Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 100){
				if(lastFrameLife <= 100){
					playAnimation4Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 90){
				if(lastFrameLife <= 90){
					playAnimation4Fall = false;
					playAnimation4Heal = true;
					totalTimeHeal = 0.0f;
				}
			}
			else if(health > 80){
				if(lastFrameLife <= 80){
					playAnimation3Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 70){
				if(lastFrameLife <= 70){
					playAnimation3Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 60){
				if(lastFrameLife <= 60){
					playAnimation3Fall = false;
					playAnimation3Heal = true;
					totalTimeHeal = 0.0f;
				}
			}
			else if(health > 50){
				if(lastFrameLife <= 50){
					playAnimation2Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 40){
				if(lastFrameLife <= 40){
					playAnimation2Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 30){
				if(lastFrameLife <= 30){
					playAnimation2Fall = false;
					playAnimation2Heal = true;
					totalTimeHeal = 0.0f;
				}
			}
			else if(health > 20){
				if(lastFrameLife <= 20){
					playAnimation1Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 10){
				if(lastFrameLife <= 10){
					playAnimation1Move = true;
					totalTimeMove = 0.0f;
				}
			}
			else if (health > 0){
				playAnimation1Fall = false;
				playAnimation1Heal = true;
				totalTimeHeal = 0.0f;
			}
			playerHealed = false;
		}
		if(playAnimation5Move || playAnimation4Move || playAnimation3Move || playAnimation2Move || playAnimation1Move){		
			if(totalTimeMove >= TIME_MOVING_LEAF){
				playAnimation5Move = false;
				playAnimation4Move = false;
				playAnimation3Move = false;
				playAnimation2Move = false;
				playAnimation1Move = false;
				totalTimeMove = 0.0f;
			}else{
				totalTimeMove += elapsed;		
				angleLeaf = sin(totalTimeMove * SPEED_SIN_MOVING_LEAF) * FORCE_SIN_MOVING_LEAF;
				if(playAnimation5Move){
					if(health > 140)		drawLeafHUD(pixelRect(192, 50, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), 
													deg2rad(angleLeaf),Texture::getManager().getByName("1g"));
					else if(health >= 130)	drawLeafHUD(pixelRect(192, 50, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), deg2rad(angleLeaf),
													Texture::getManager().getByName("1y"));
					else if(health >= 120)	drawLeafHUD(pixelRect(192, 50, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), deg2rad(angleLeaf),
													Texture::getManager().getByName("1r"));
				}
				if(playAnimation4Move){
					if(health > 110)		drawLeafHUD(pixelRect(65, 113, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), 
													deg2rad(angleLeaf),Texture::getManager().getByName("2g"));
					else if(health >= 100)	drawLeafHUD(pixelRect(65, 113, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("2y"));
					else if(health >= 90)	drawLeafHUD(pixelRect(65, 113, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("2r"));
				}
				if(playAnimation3Move){
					if(health > 80)			drawLeafHUD(pixelRect(198, 106, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), 
													deg2rad(angleLeaf),Texture::getManager().getByName("3g"));
					else if(health >= 70)	drawLeafHUD(pixelRect(198, 106, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), 
													deg2rad(angleLeaf),Texture::getManager().getByName("3y"));
					else if(health >= 60)	drawLeafHUD(pixelRect(198, 106, 128, 128), 
													pixelRect(app.config.xres, app.config.yres), 
													deg2rad(angleLeaf),Texture::getManager().getByName("3r"));
				}
				if(playAnimation2Move){
					if(health > 50)			drawLeafHUD(pixelRect(68, 68, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("4g"));
					else if(health >= 40)	drawLeafHUD(pixelRect(68, 68, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("4y"));
					else if(health >= 30)	drawLeafHUD(pixelRect(68, 68, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("4r"));
				}
				if(playAnimation1Move){
					if(health > 20)			drawLeafHUD(pixelRect(130, 161, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("5g"));
					else if(health >= 10)	drawLeafHUD(pixelRect(130, 161, 128, 128), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("5y"));
				}
			}
		}
		if(playAnimation5Fall || playAnimation4Fall || playAnimation3Fall || playAnimation2Fall || playAnimation1Fall){		
			if(totalTimeFall >= TIME_FALLING_LEAF){
				playAnimation5Fall = false;
				playAnimation4Fall = false;
				playAnimation3Fall = false;
				playAnimation2Fall = false;
				playAnimation1Fall = false;
				totalTimeFall = 0.0f;
				fallLeafHeight = 0;
			}else{
				totalTimeFall += elapsed;
				angleLeaf = totalTimeFall * SPEED_SPIN_FALLING_LEAF;
				float tim = totalTimeFall / TIME_FALLING_LEAF;
				fallLeafHeight = int(DISTANCE_FALL_LEAF * tim);
				int LeafHW = (int)(128 * (1 - tim));
                pixelRect screen(app.config.xres, app.config.yres);
				if(playAnimation5Fall) {
                    drawLeafHUD(pixelRect(192, 50 + fallLeafHeight, LeafHW, LeafHW), screen,
                        deg2rad(angleLeaf), Texture::getManager().getByName("1r"));
                }
				if(playAnimation4Fall) {
                    drawLeafHUD(pixelRect(65, 113 + fallLeafHeight, LeafHW, LeafHW), screen,
                        deg2rad(angleLeaf),Texture::getManager().getByName("2r"));
                }
				if(playAnimation3Fall) {
                    drawLeafHUD(pixelRect(198, 106 + fallLeafHeight, LeafHW, LeafHW), screen,
                        deg2rad(angleLeaf),Texture::getManager().getByName("3r"));
                }
				if(playAnimation2Fall) {
                    drawLeafHUD(pixelRect(68, 68 + fallLeafHeight, LeafHW, LeafHW), screen,
                        deg2rad(angleLeaf),Texture::getManager().getByName("4r"));
                }
				if(playAnimation1Fall) {
                    drawLeafHUD(pixelRect(130, 161 + fallLeafHeight, LeafHW, LeafHW), screen,
                        deg2rad(angleLeaf),Texture::getManager().getByName("5r"));
                }
			}
		}
		if(playAnimation5Heal || playAnimation4Heal || playAnimation3Heal || playAnimation2Heal || playAnimation1Heal){		
			if(totalTimeHeal >= TIME_HEALING_LEAF){
				playAnimation5Heal = false;
				playAnimation4Heal = false;
				playAnimation3Heal = false;
				playAnimation2Heal = false;
				playAnimation1Heal = false;
				totalTimeHeal = 0.0f;
			}else{
				totalTimeHeal += elapsed;		
				angleLeaf = sin(totalTimeHeal * SPEED_SIN_MOVING_LEAF) * FORCE_SIN_MOVING_LEAF;
				float tim = totalTimeHeal / TIME_HEALING_LEAF;
				int LeafHW = (int)(128 * tim);
				if(playAnimation5Heal)		drawLeafHUD(pixelRect(192, 50, LeafHW, LeafHW), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("1r"));
				if(playAnimation4Heal)		drawLeafHUD(pixelRect(65, 113, LeafHW, LeafHW), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("2r"));
				if(playAnimation3Heal)		drawLeafHUD(pixelRect(198, 106, LeafHW, LeafHW), 
													pixelRect(app.config.xres, app.config.yres),
													deg2rad(angleLeaf),Texture::getManager().getByName("3r"));
				if(playAnimation2Heal)		drawLeafHUD(pixelRect(68, 68, LeafHW, LeafHW), 
													pixelRect(app.config.xres, app.config.yres), 
													deg2rad(angleLeaf),Texture::getManager().getByName("4r"));
			}
		}
		if(!playerDamaged && !playerHealed){
			if (!playAnimation5Fall && !playAnimation5Move && !playAnimation5Heal){
				//dbg("%f %f %f %f\n", imgPosX1, imgPosY1, imgLeafW, imgLeafH);
				if(health > 140)					drawLeafHUD(pixelRect(192, 50, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0, 
														Texture::getManager().getByName("1g"));
				if(health > 130 && health <= 140)	drawLeafHUD(pixelRect(192, 50, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("1y"));
				if(health > 120 && health <= 130)	drawLeafHUD(pixelRect(192, 50, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0, 
														Texture::getManager().getByName("1r"));
			}
			if (!playAnimation4Fall && !playAnimation4Move && !playAnimation4Heal){
				if(health > 110)					drawLeafHUD(pixelRect(65, 113, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0, 
														Texture::getManager().getByName("2g"));
				if(health > 100 && health <= 110)	drawLeafHUD(pixelRect(65, 113, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("2y"));
				if(health > 90 && health <= 100)	drawLeafHUD(pixelRect(65, 113, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("2r"));
			}
			if (!playAnimation3Fall && !playAnimation3Move && !playAnimation3Heal){
				if(health > 80)						drawLeafHUD(pixelRect(198, 106, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("3g"));
				if(health > 70 && health <= 80)		drawLeafHUD(pixelRect(198, 106, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("3y"));
				if(health > 60 && health <= 70)		drawLeafHUD(pixelRect(198, 106, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("3r"));
			}
			if (!playAnimation2Fall && !playAnimation2Move && !playAnimation2Heal){
				if(health > 50)						drawLeafHUD(pixelRect(68, 68, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("4g"));
				if(health > 40 && health <= 50)		drawLeafHUD(pixelRect(68, 68, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("4y"));
				if(health > 30 && health <= 40)		drawLeafHUD(pixelRect(68, 68, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("4r"));
			}
			if (!playAnimation1Fall && !playAnimation1Move && !playAnimation1Heal){
				if(health > 20)						drawLeafHUD(pixelRect(130, 161, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("5g"));
				if(health > 10 && health <= 20)		drawLeafHUD(pixelRect(130, 161, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("5y"));
				if(health > 0 && health <= 10)		drawLeafHUD(pixelRect(130, 161, 128, 128), 
														pixelRect(app.config.xres, app.config.yres), 0,
														Texture::getManager().getByName("5r"));
			}
		}
		lastFrameLife = health;
		//Fadeout effect
		if (playerDead){
			if (fadeoutTim < timeDeathAnim) {
				drawTextureFadeOut(pixelRect(0, 0, app.config.xres, app.config.yres), pixelRect(app.config.xres, app.config.yres),
					Texture::getManager().getByName("fadeout"), 1 - ((timeDeathAnim - fadeoutTim) / timeDeathAnim));
			}
		}
	}
	
	void CPlayerStats::receive(const MsgFlareHit& msg)
	{ 
		CPlayerMov* playerMov = Handle(this).getBrother<CPlayerMov>();
		if (playerMov->isOnCreep() || playerMov->isOnLiana()){
			damage(msg.damage, true);
		}
		else{
			damage(msg.damage);
		}
	}

	void CPlayerStats::initType()
	{
		MegashotFSM::initType();
		InvencibleFSM::initType();

		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgMeleeHit, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgFlareHit, receive);

		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgPickupHeal, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgPickupEnergy, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgPickupInvincible, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgPickupCoin, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgPickupCollectible, receive);

		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgPlayerAchievedCheckpoint, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgSetBichito, receive);

		SUBSCRIBE_MSG_TO_MEMBER(CPlayerStats, MsgRevive, revive);
	}

}