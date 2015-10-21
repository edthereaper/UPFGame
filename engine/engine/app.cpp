#include "mcv_platform.h"
#include "app.h"

#ifdef _DEBUG
#define PRINT_FPS_FONT          //print fps on screen
#undef PRINT_FPS                //print fps using dbg()
#undef DEBUG_LIGHTS             //Draw fustrums and icosaedrons
#undef PAUSE_ON_FOCUS_LOSE      //Helpful to disable on debug
#undef ANALIZE_LOAD_TIME       //load level then close
#endif

#if defined(NDEBUG) && !defined(_ANITOOL) && !defined(LOOKAT_TOOL) && !defined(_LIGHTTOOL) && !defined(_OBJECTTOOL) && !defined(_PARTICLES)
#define DISPLAY_VIDEO_AND_MENUS			//To display videos and menus in release
#endif

#include "preloader.h"

#include "handles\importXML.h"

#include "PhysX_USER/pxcomponents.h"
#include "PhysX_USER/PhysicsManager.h"
using namespace physX_user;

#include "Particles/ParticlesManager.h"
using namespace particles;

#include "components/components.h"
#include "components/AABB.h"
#include "components/Transform.h"
#include "components/slot.h"
#include "handles/handle.h"
#include "handles/prefab.h"
using namespace component;

#include "animation\components.h"
#include "animation\animation_max.h"

using namespace animation;

#include "render/camera/component.h"
#include "render/components.h"
#include "render/renderManager.h"
#include "render/render_utils.h"
#include "render/deferredRender.h"      
#include "render/illumination/ptLight.h"
#include "render/illumination/dirLight.h"
#include "render/illumination/cubeshadow.h"
#include "render/illumination/shadow.h"
#include "render/texture/cskybox.h"
using namespace render;

#include "Cinematic\animation_camera.h"
#include "Cinematic\camera_manager.h"
using namespace cinematic;

#include "logic/trigger.h"
#include "logic/logicManager.h"

#include "gameElements/input.h"
#include "gameElements/player/cannonPath.h"
#include "gameElements/module.h"
#include "gameElements/PaintManager.h"
using namespace gameElements;

#include "level\importLevel.h"
#include "level\level.h"
using namespace level;

#if defined(_DEBUG) || defined(_OBJECTTOOL)
#include "antTweakBar_USER/antTW.h"
#include "antTweakBar_USER/lookatTools.h"
using namespace antTw_user;
#endif

#include "gameElements\Ambientsound.h"

using namespace DirectX;
using namespace utils;

#include "fmod_User/fmodUser.h"
#include "fmod_User/fmodStudio.h"
using namespace fmodUser;

#include <Lua_user\lua_component.h>

using namespace lua_user;
using namespace luabridge;

#include "Particles/Emitter.h"
using namespace particles;

App App::instance;

#include "font/font.h"
Font fontScreen;
Font fontDBG;

//TEMP {
Handle skyboxTint_h;
Entity* levelE;

App::App()
 : config(defConfig), deferred("rt_")
{ }

const App::config_t App::defConfig =
    App::config_t(800, 600, true);
CArmPoint* armPoint;

#if defined(DISPLAY_VIDEO_AND_MENUS)
	#include "theoraplayer/TheoraPlayer.h"
	#include "theoraplayer/TheoraDataSource.h"
	#include "theoraplayer/TheoraVideoManager.h"
	TheoraVideoManager *mgr;
	TheoraVideoClip *clip = nullptr;
#endif

//Begin automaton App
using namespace behavior;
namespace behavior {
	AppFSM::container_t AppFSM::states;
	void AppFSM::initType()
	{
		SET_FSM_STATE(loading);
		SET_FSM_STATE(loadvideo);
		SET_FSM_STATE(playvideo);
		SET_FSM_STATE(mainmenu);
		SET_FSM_STATE(chapterselectionmenu);
		SET_FSM_STATE(game);
		SET_FSM_STATE(gameover);
		SET_FSM_STATE(changelvl);
		SET_FSM_STATE(retry);
		SET_FSM_STATE(win);
		SET_FSM_STATE(credits);
		SET_FSM_STATE(quit);
		SET_FSM_STATE(waitvideo);
	}
}

fsmState_t AppFSMExecutor::loading(float elapsed)
{
	App &app = App::get();
	app.loadConfig();
#if defined(DISPLAY_VIDEO_AND_MENUS)
	return STATE_loadvideo;
#endif
	return STATE_game;
}

fsmState_t AppFSMExecutor::loadvideo(float elapsed)
{
	App &app = App::get();
	app.loadVideo("intro.ogv", "video_intro");
	app.videoEndsTo = 0;
	return STATE_playvideo;
}

fsmState_t AppFSMExecutor::playvideo(float elapsed)
{
	App &app = App::get();
	if (app.updateVideo(true))		return STATE_playvideo;
	switch (app.videoEndsTo) {
	case 0:
		return STATE_mainmenu;
		break;
	case 1:
		return STATE_changelvl;
		break;
	case 2:
		return STATE_credits;
		break;
	}
	return STATE_playvideo;
}

fsmState_t AppFSMExecutor::mainmenu(float elapsed)
{
	App &app = App::get();
	switch (app.updateMainMenu()) {
	case -1:
		return STATE_mainmenu;
		break;
	case 0:
		app.chapterSelectionState = 0;
		app.mainMenuState = 0;
		app.gamelvl = 1;
		app.resetTotalStats();
		return STATE_changelvl;
		break;
	case 1:
		app.chapterSelectionState = 0;
		app.mainMenuState = 0;
		return STATE_chapterselectionmenu;
		break;
	case 2:
		app.chapterSelectionState = 0;
		app.mainMenuState = 0;
		return STATE_credits;
		break;
	case 3:
		return STATE_quit;
		break;
	}
	return STATE_mainmenu;
}

fsmState_t AppFSMExecutor::chapterselectionmenu(float elapsed)
{
	App &app = App::get();
	switch (app.updateChapterSelectionMenu()) {
	case -1:
		return STATE_chapterselectionmenu;
		break;
	case 0:
		app.gamelvl = 1;
		app.resetTotalStats();
		return STATE_changelvl;
		break;
	case 1:
		app.gamelvl = 2;
		app.resetTotalStats();
		return STATE_changelvl;
		break;
	case 2:
		app.gamelvl = 3;
		app.resetTotalStats();
		return STATE_changelvl;
		break;
	case 3:
		app.gamelvl = 4;
		app.resetTotalStats();
		return STATE_changelvl;
		break;
	case 4:
		return STATE_mainmenu;
		break;
	}
	return STATE_chapterselectionmenu;
}

fsmState_t AppFSMExecutor::game(float elapsed)
{
	App &app = App::get();
	if (app.isPlayerDead)	return STATE_gameover;
	if (app.winGame)		return STATE_win;
	if (app.returnToMenu){
		app.returnToMenu = false;
#if defined(DISPLAY_VIDEO_AND_MENUS)
		return STATE_mainmenu;
#else
		return STATE_quit;
#endif
	}
	if (!app.doFrame())		return STATE_quit;
	return STATE_game;
}
void AppFSMExecutor::update(float elapsed)
{
    
}

fsmState_t AppFSMExecutor::gameover(float elapsed)
{
	App &app = App::get();
	if (!app.doGameOver()) return STATE_quit;
#if defined(DISPLAY_VIDEO_AND_MENUS)
	if (app.returnToMenu){
		app.returnToMenu = false;
		return STATE_mainmenu;
	}
#endif
	if (app.playAgain){
		app.playAgain = false;
		return STATE_retry;
	}
	return STATE_gameover;
}

fsmState_t AppFSMExecutor::changelvl(float elapsed)
{
	App &app = App::get();
    if (levelE != nullptr) {
        CLevelData* lvl (levelE->get<CLevelData>());
        lvl->stopSong();
    }
	app.loadlvl();
#if defined(DISPLAY_VIDEO_AND_MENUS)
	return STATE_waitvideo;
#else
	return STATE_game;
#endif
}

fsmState_t AppFSMExecutor::retry(float elapsed)
{
	App::get().retry();
	return STATE_game;
}

fsmState_t AppFSMExecutor::waitvideo(float elapsed)
{
	App &app = App::get();
	if (app.waitVideo())	return STATE_waitvideo;
	return STATE_game;
}

fsmState_t AppFSMExecutor::credits(float elapsed)
{
	App &app = App::get();
	app.videoEndsTo = 0;
	app.loadVideo("eyes.ogv", "");
    
    Handle::setCleanup(true);
    auto entityMan = component::getManager<Entity>();
    entityMan->forall<void>([](Entity* e) {e->postMsg(MsgDeleteSelf());});
    MessageManager::dispatchPosts();
    Handle::setCleanup(false);
    app.setWinGame(false);

	return STATE_playvideo;
}

fsmState_t AppFSMExecutor::win(float elapsed)
{
	App &app = App::get();
    if (levelE != nullptr) {
        CLevelData* lvl (levelE->get<CLevelData>());
        lvl->stopSong();
    }
    fmodUser::fmodUserClass::stopSounds();
	//App &app = App::get();
	//app.videoEndsTo = 2;
	//app.loadVideo("eyes.ogv", "");
	//return STATE_playvideo;
    return STATE_credits;
}

fsmState_t AppFSMExecutor::quit(float elapsed)
{
	return STATE_quit;
}

fsmState_t AppFSMExecutor::cinematic(float elapsed)
{
	return STATE_game;
}


void App::initFSM()
{
	AppFSM::initType();
	fsm.init();
	fsm.update(0.0f);
}

void App::update()
{
	fsm.update(0.0f);
    FmodStudio::update();
}

void App::loadConfig()
{
	char windowedStr[32] {};
	char shadowsStr[32] {};
#ifdef _DEBUG
	config.xres = GetPrivateProfileInt("display_debug", "x", defConfig.xres, ".\\config.ini");
	config.yres = GetPrivateProfileInt("display_debug", "y", defConfig.yres, ".\\config.ini");
	GetPrivateProfileString("display_debug", "windowed", "true", windowedStr, ARRAYSIZE(windowedStr)-1, ".\\config.ini");
    gamelvl = GetPrivateProfileInt("debug", "level", 1, ".\\config.ini");
    GetPrivateProfileString("display_debug", "windowed", "true",
        windowedStr, ARRAYSIZE(windowedStr)-1, ".\\config.ini");
    GetPrivateProfileString("debug", "shadows", "true",
        shadowsStr, ARRAYSIZE(shadowsStr)-1, ".\\config.ini");
	char channelStr[32] {};
    GetPrivateProfileString("debug", "initChannel", "FX_FINAL",
        channelStr, ARRAYSIZE(channelStr)-1, ".\\config.ini");

#define IF_CASE_CONFIG_CHANNEL(name) if (!strcmp(channelStr, #name)) {selectedChannel = name;}
#define ELIF_CASE_CONFIG_CHANNEL(name) else if (!strcmp(channelStr, #name)) {selectedChannel = name;}
    IF_CASE_CONFIG_CHANNEL(ALBEDO)
    ELIF_CASE_CONFIG_CHANNEL(ALBEDO_PLUS_PARTICLES)
    ELIF_CASE_CONFIG_CHANNEL(FX_FINAL)
    ELIF_CASE_CONFIG_CHANNEL(FINAL)
    ELIF_CASE_CONFIG_CHANNEL(POSITION)
    ELIF_CASE_CONFIG_CHANNEL(DEPTH)
    ELIF_CASE_CONFIG_CHANNEL(SELFILL)
    ELIF_CASE_CONFIG_CHANNEL(FXSELFILL)
    ELIF_CASE_CONFIG_CHANNEL(LIGHTS)
    ELIF_CASE_CONFIG_CHANNEL(PAINT)
    ELIF_CASE_CONFIG_CHANNEL(PAINT_AMOUNT)
    ELIF_CASE_CONFIG_CHANNEL(NORMALS)
    ELIF_CASE_CONFIG_CHANNEL(DATA)
    ELIF_CASE_CONFIG_CHANNEL(AMBIENT)
    ELIF_CASE_CONFIG_CHANNEL(SHADOW)
    ELIF_CASE_CONFIG_CHANNEL(FXSHADOW)
    ELIF_CASE_CONFIG_CHANNEL(CUBESHADOW)
    ELIF_CASE_CONFIG_CHANNEL(FXCUBESHADOW)
    ELIF_CASE_CONFIG_CHANNEL(UVPAINT)
    ELIF_CASE_CONFIG_CHANNEL(SPECULAR)

#else	
	config.xres = GetPrivateProfileInt("display", "x", defConfig.xres, ".\\config.ini");
	config.yres = GetPrivateProfileInt("display", "y", defConfig.yres, ".\\config.ini");
	GetPrivateProfileString("display", "windowed", "true",
        windowedStr, ARRAYSIZE(windowedStr)-1, ".\\config.ini");
    GetPrivateProfileString("display", "shadows", "true",
        shadowsStr, ARRAYSIZE(shadowsStr)-1, ".\\config.ini");
#endif
	xboxPadSensiblity = GetPrivateProfileInt("sensibility", "xbox", 5, ".\\config.ini");
	if (xboxPadSensiblity < 1)	xboxPadSensiblity = 1;
	if (xboxPadSensiblity > 10) xboxPadSensiblity = 10;
	//Check the desktop resolution, if its lower than the resolution set in config. Set the resolution same as desktop.
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	if (config.xres > desktop.right)	config.xres = desktop.right - desktop.left;
	if (config.yres > desktop.bottom)	config.yres = desktop.bottom - desktop.top;
	config.windowed = !strcmp(windowedStr, "true");
    enableShadows = !strcmp(shadowsStr, "true");
}

CamCannonController cam1P;

#if defined(_OBJECTTOOL) || defined(_LIGHTTOOL) || defined(_PARTICLES)

XMVECTOR pos, rotQ;
Entity* entitytramp = nullptr;
Handle tramp_h;
CTrampoline*		trampoline;


Handle createTransformableEntity2(
	const XMVECTOR& pos,
	const XMVECTOR& rot,
	const std::string meshName
	)
{
	Handle entity_h, h;
	Entity* entity;
	entity_h = getManager<Entity>()->createObj();
	entity = entity_h;
	h = getManager<CTransform>()->createObj();
	CTransform* transform = h;
	transform->setPosition(pos);
	transform->setRotation(rot);
	entity->add(h);
	entity->add(getManager<CTint>()->createObj());
	CMesh::load(meshName, Handle(entity));
	CMesh* mesh = entity->get<CMesh>();
	mesh->init();
	return entity_h;
}
#endif

#if defined(_ANITOOL) 
float elapsed2;
#endif

float mAccumulator = 0.0f;

#if defined (PRINT_FPS_FONT)
float delta_secs, fps;
#endif

float App::countTime()
{
    static LARGE_INTEGER before= {0,0};
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    LARGE_INTEGER delta_ticks;
    delta_ticks.QuadPart = now.QuadPart - before.QuadPart;
  
#if !defined (PRINT_FPS_FONT)
    float delta_secs, fps;
#endif
    delta_secs = delta_ticks.QuadPart * ( 1.0f / freq.LowPart );
    fps = 1.0f / delta_secs;

    before = now;

    return delta_secs;
}

void App::getImgValues(float& posX, float& posY, float& imgW, float& imgH, float originX, float originY, float originW, float originH){
	posX = ((float)(config.xres) / 1280) * originX;
	posY = ((float)(config.yres) / 720) * originY;
	imgW = ((float)(config.xres) / 1280) * originW;
	imgH = ((float)(config.yres) / 720) * originH;
}

void App::xboxControllerKeys()
{
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_A].isHit())						App::get().getPad().onKey(XINPUT_GAMEPAD_A, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_A].isRelease())					App::get().getPad().onKey(XINPUT_GAMEPAD_A, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_B].isHit())						App::get().getPad().onKey(XINPUT_GAMEPAD_B, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_B].isRelease())					App::get().getPad().onKey(XINPUT_GAMEPAD_B, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_X].isHit())						App::get().getPad().onKey(XINPUT_GAMEPAD_X, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_X].isRelease())					App::get().getPad().onKey(XINPUT_GAMEPAD_X, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_Y].isHit())						App::get().getPad().onKey(XINPUT_GAMEPAD_Y, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_Y].isRelease())					App::get().getPad().onKey(XINPUT_GAMEPAD_Y, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_START].isHit())					App::get().getXboxPad().onKey(XINPUT_GAMEPAD_START, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_START].isRelease())				App::get().getXboxPad().onKey(XINPUT_GAMEPAD_START, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_DOWN].isHit())				App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_DOWN, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_DOWN].isRelease())			App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_DOWN, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_UP].isHit())				App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_UP, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_UP].isRelease())			App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_UP, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_LEFT].isHit())				App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_LEFT, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_LEFT].isRelease())			App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_LEFT, false);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_RIGHT].isHit())			App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_RIGHT, true);
	if (xboxController.State._buttons[GamePadXBOXController::GamePad_Button_DPAD_RIGHT].isRelease())		App::get().getXboxPad().onKey(XINPUT_GAMEPAD_DPAD_RIGHT, false);

	if (xboxController.State._left_thumbstickX > 0.3f){
		App::get().getPad().onKey('D', true);
	}
	else{
		if (xboxController.State._left_thumbstickX < -0.3f){
			App::get().getPad().onKey('A', true);
		}
		else{
			App::get().getPad().onKey('A', false);
			App::get().getPad().onKey('D', false);
		}
	}
	if (xboxController.State._left_thumbstickY > 0.3f){
		App::get().getPad().onKey('W', true);
	}
	else{
		if (xboxController.State._left_thumbstickY < -0.3f){
			App::get().getPad().onKey('S', true);
		}
		else{
			App::get().getPad().onKey('W', false);
			App::get().getPad().onKey('S', false);
		}
	}
	//At the moment we ignore triggers
	/*if (xboxController.State._left_trigger > 0.3f){
	App::get().getPad().onKey(VK_RBUTTON, true);
	}
	else{
	App::get().getPad().onKey(VK_RBUTTON, false);
	}
	if (xboxController.State._right_trigger > 0.3f){
	App::get().getPad().onKey(VK_LBUTTON, true);
	}
	else{
	App::get().getPad().onKey(VK_LBUTTON, false);
	}*/
	Mouse::setSysXboxController(int(xboxController.State._right_thumbstickX  * 400 * elapsed * xboxPadSensiblity),
								int(-xboxController.State._right_thumbstickY * 400 * elapsed * (xboxPadSensiblity/2)));
}

void App::addMappings()
{
#ifdef _ANITOOL
	pad.addMapping(VK_F11, APP_SLOWMO);
#endif
#ifdef _DEBUG
	pad.addMapping(VK_ESCAPE, APP_QUIT);
    pad.addMapping(VK_F1,  APP_TOGGLE_GODMODE);
    pad.addMapping(VK_F2,  APP_RELOAD_FX_PIPELINES);
    pad.addMapping(VK_F3,  APP_TOGGLE_MOUSE_CAPTURE);
	pad.addMapping(VK_F4,  APP_DEBUG);
	pad.addMapping(VK_F5,  APP_TOGGLE_GBUFFER);
    pad.addMapping(VK_F6,  APP_TOGGLE_ANTTW);
    pad.addMapping(VK_F7,  APP_RELOAD_SHADERS);
    pad.addMapping(VK_F8,  APP_TOGGLE_AABBS);
    pad.addMapping(VK_F9,  APP_TOGGLE_DEBUG_RENDERER);
    pad.addMapping(VK_F9,  APP_PAUSE_DEBUG);
    pad.addMapping('P',  APP_PAUSE);
    pad.addMapping(VK_F11, APP_SLOWMO);
#else
    pad.addMapping(VK_ESCAPE,  APP_PAUSE);
#endif
	pad.addMapping(VK_RETURN, APP_ENTER);
	pad.addMapping(VK_F12, APP_QUIT);
	
	gameElements::initGameInput(pad);
	gameElements::initXboxGameInput(xboxPad);
}

bool App::create()
{
	seedRand();

	

	//mistEffect(true);

	if (!Render::createDevice()) { return false; }

    EffectLibrary::init();
    
#ifdef _DEBUG
	antTw_user::AntTWManager::init(config.xres, config.yres);
#endif

	component::init();
	bool ok;
	ok = renderUtilsCreate();
	assert(ok);
	ok = deferred.create(config.xres, config.yres);
	assert(ok);
	RenderManager::init();
    
	PhysicsManager &physicsManager = PhysicsManager::get();
#if defined(_PARTICLES)
	ParticleFileParser::get().loadLibrary();
	EmitterParser::get().loadLibrary();
#endif
	ParticleSystemParser::get().loadLibrary();
	
	fmodUser::FmodStudio::init();
	fmodUser::fmodUserClass::initFmod();
	
	physicsManager.init();

	addMappings();

    Preloader::preload("onCreation");

#ifndef DISPLAY_VIDEO_AND_MENUS
	loadlvl();
    #ifdef ANALIZE_LOAD_TIME
        dbg("Flag ANALIZE_LOAD_TIME was on. Closing App...\n");
        return false;
    #endif
#else
	//Necessary to display a video
	camera_h = PrefabManager::get().prefabricate("camera");
	Entity* camera_e(camera_h);
	CCamera* camera = camera_e->get<CCamera>();
	camera->setViewport(0.f, 0.f, (float)config.xres, (float)config.yres);
	activateCamera(*camera);
	activateObjectConstants();
#endif

#if !defined(_OBJECTTOOL)
    ok = fontScreen.create();
	fontScreen.size = 11;
	fontScreen.color = 0xFF000000;
	assert(ok);
#endif

#if defined(PRINT_FPS_FONT) || defined(_LIGHTTOOL)
	ok = fontDBG.create();
	fontDBG.size = 10;
	fontDBG.color = 0xFFFF0000;
	assert(ok);
#endif

#ifdef _DEBUG
    antTw_user::AntTWManager::createDebugTweak();
    antTw_user::AntTWManager::createRenderTweak();
#endif

    return true;
}

bool isLoadingThreadActve = false;
bool loadingLevelComplete = false;

void loadingThread()
{
	App &app = App::get();
	if (app.getLvl() == 1){
		app.isLoadingThreadActve = true;
		while (app.updateVideo(app.loadingLevelComplete, true));
		app.isLoadingThreadActve = false;
	}
	else{
		while (app.loadingthreadVar){
			Render::getContext()->ClearRenderTargetView(Render::getRenderTargetView(), utils::BLACK);
			Render::getContext()->ClearDepthStencilView(Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			drawTexture2D(pixelRect(app.config.xres, app.config.yres), pixelRect(app.config.xres, app.config.yres), Texture::getManager().getByName("loading"));
			drawTexture2DAnim(pixelRect(5, 5, 100, 100), pixelRect(app.config.xres, app.config.yres),
				Texture::getManager().getByName("loadingSpin"), true, app.timerThreadAnim.count(app.countTime()));
			Render::getSwapChain()->Present(0, 0);
		}
	}
}

void App::loadlvl()
{
    dbg("Loading level %d...\n", gamelvl);


    Handle::setCleanup(true);
    PaintManager::clear();
	ParticleUpdaterManager::get().deleteAll();
	CSmokeTower::resetFX();

	EntityListManager::clearLists();
    
    auto entityMan = component::getManager<Entity>();
    entityMan->clear();
    assert(entityMan->getSize() == 0);

    Handle::setCleanup(false);
    PaintManager::load();

	char level[20];
	sprintf(level,"%s%i", "level", gamelvl);

	CameraManager::get().load(level);
	camera_h = PrefabManager::get().prefabricate("camera");
	Entity* camera_e(camera_h);

	CCamera* camera = camera_e->get<CCamera>();
	camera->setViewport(0.f, 0.f, (float)config.xres, (float)config.yres);

	CTransform *camT = nullptr;
#ifdef _CINEMATIC
	cam1P.setup(XMVectorSet(-167.023f, 30.0037f, 53.3516f, 0.f), XMVectorSet(1, 0, 0, 0));
	camT = ((Entity*)camera_movie_h)->get<CTransform>();
	camT->setPosition(XMVectorSet(-167.023f, 30.0037f, 53.3516f, 0.f));
#elif defined(_OBJECTTOOL) || defined(_LIGHTTOOL) || defined(_PARTICLES)
    auto eye = XMVectorSet(-2, 10, 0, 0);
	camT = ((Entity*)camera_h)->get<CTransform>();
	camT->setPosition(eye);
	cam1P.setup(eye);
	lookAt3D(camT, zero_v);
#endif

    //Loading pic 
	activateCamera(*camera);
	activateObjectConstants();

#if defined(DISPLAY_VIDEO_AND_MENUS)
	switch (gamelvl) {
		case 1:
			loadVideo("level1.ogv", "video_level1");
		break;
		case 2:
			loadingthreadVar = true;
			timerThreadAnim.reset();
		break;
		case 3:
			loadingthreadVar = true;
			timerThreadAnim.reset();
		break;
		case 4:
			loadingthreadVar = true;
			timerThreadAnim.reset();
		break;
	}
	thread_1 = std::thread(loadingThread);
#endif


#if !defined(_OBJECTTOOL) && !defined(_LIGHTTOOL) && !defined(_PARTICLES)
	playerEntity_h = PrefabManager::get().prefabricate("player/player");
#else 
	playerEntity_h = PrefabManager::get().prefabricate("tools/player");
#ifdef _LIGHTTOOL
	playerModelEntity_h = PrefabManager::get().prefabricate("tools/playerModel");
#endif

#endif

    Handle skybox_h = getManager<CSkyBox>()->createObj();
    camera_e->add(skybox_h);
    CSkyBox* skybox = skybox_h;
    skybox->setTextureB("skybox_dirty");
    skybox->setTextureA("skybox_clean");
    skybox->init();
    skyboxTint_h = getManager<CTint>()->createObj();

#ifdef _PARTICLES
	antTw_user::AntTWManager::createBarManagerTweak();
#elif defined(MUSIC_TOOL)
    levelE = LevelImport::load("sandbox", playerEntity_h);
#endif

#ifdef _CINEMATIC_
	antTw_user::AntTWManager::createCameraTweak();
#endif

    camera_e->add(skyboxTint_h);
	bichitoEntity_h = PrefabManager::get().prefabricate("bichito/bichito");
    
#if defined(LOOKAT_TOOL)
    levelE = LevelImport::load("sandbox", playerEntity_h);
	CLevelData* lvlT = levelE->get<CLevelData>();
    CTint* skytint = skyboxTint_h;
    skytint->set(Color::BLACK).setAf(0.75);
#else
	CLevelData* lvlT = nullptr;
	bool isOK;

	switch (gamelvl) {
		case 1:
			levelE = LevelImport::load("level1", playerEntity_h);
			lvlT = levelE->get<CLevelData>();
			isOK = Importer::parse("data/lights/light1.xml");
			isOK = Importer::parse("data/levelsounds/sounds1.xml");
		    break;
		case 2:
			levelE = LevelImport::load("level2", playerEntity_h);
			lvlT = levelE->get<CLevelData>();
			isOK = Importer::parse("data/lights/light2.xml");
			isOK = Importer::parse("data/levelsounds/sounds2xml");
		    break;
		case 3:
			levelE = LevelImport::load("level3", playerEntity_h);
			lvlT = levelE->get<CLevelData>();
			isOK = Importer::parse("data/lights/light3.xml");
			isOK = Importer::parse("data/levelsounds/sounds3.xml");
		    break;
		case 4:
			levelE = LevelImport::load("level4", playerEntity_h);
			lvlT = levelE->get<CLevelData>();
			isOK = Importer::parse("data/lights/light4.xml");
			isOK = Importer::parse("data/levelsounds/sounds1.xml");
		    break;
		case 5:
        default:
			levelE = LevelImport::load("sandbox", playerEntity_h);
			lvlT = levelE->get<CLevelData>();
		break;
	}

#endif

#ifdef _DEBUG
    lvlT->playSong();
#endif

    assert(lvlT != nullptr);
    if (lvlT->isBossLevel()) {
        PaintManager::abort();
    }
    if (lvlT->isHighZFarLevel()) {
        CCamera* cam = getCamera().getSon<CCamera>();
        cam->setZFar(lvlT->getZFar());
    }
    RenderConstsMirror::SkyBoxBlend = lvlT->getSkyboxBlend();
    RenderConstsMirror::SkyBoxBright = lvlT->getSkyboxBright();
    RenderConstsMirror::update();


#ifdef LOOKAT_TOOL
	CTint* tint = skyboxTint_h;
	tint->set(Color::BLACK).setAf(0.75);
	auto meleeV = lvlT->getTaggedEntity("melee");
	auto flareV = lvlT->getTaggedEntity("flare");

	auto createLookAt = [=](LookAtTW*& tw, Handle e_h, const std::string& file, const Color& color) {
		if (e_h.isValid()) {
			tw = new LookAtTW(e_h, getCamera(), file, color);
			if (!tw->isValid()) {
				delete tw;
				tw = nullptr;
			}
		}
	};
	createLookAt(lookAtTw[0], playerEntity_h, "data/prefabs/player/boneLookAt.xml", Color::PINK);
	createLookAt(lookAtTw[1], bichitoEntity_h, "data/saveBichitoLookat.xml", Color::LIME);
	if (!meleeV.empty()) {createLookAt(lookAtTw[2], meleeV[0], "data/saveMeleeLookat.xml", Color::BROWN);}
	if (!flareV.empty()) {createLookAt(lookAtTw[3], flareV[0], "data/saveFlareLookat.xml", Color::BLUE); }

	auto createArmPoint = [=](ArmPointTW*& tw, Handle e_h, const std::string& file, const Color& color) {
		if (e_h.isValid()) {
			tw = new ArmPointTW(e_h, getCamera(), file, color);
			if (!tw->isValid()) {
				delete tw;
				tw = nullptr;
			}
		}
	};
	createArmPoint(armPointTw[0], playerEntity_h, "data/prefabs/player/armPoint.xml", Color::ORANGE);
	createArmPoint(armPointTw[1], bichitoEntity_h, "data/saveBichitoArmPoint.xml", Color::LEMON_LIME);
	if (!meleeV.empty()) { createArmPoint(armPointTw[2], meleeV[0], "data/saveMeleeArmPoint.xml", Color::BEIGE); }
	if (!flareV.empty()) { createArmPoint(armPointTw[3], flareV[0], "data/saveFlareArmPoint.xml", Color::CYAN); }

#endif

#if !defined(_OBJECTTOOL) && !defined(_LIGHTTOOL) && !defined(_PARTICLES)

	Entity* playerEntity(playerEntity_h);
    playerEntity->sendMsg(MsgSetCam(getCamera()));
    playerEntity->init();
#ifndef _DEBUG
	if (gamelvl == 1)		playerEntity->sendMsg(MsgPlayerSpawn());
#endif
    spawn();
	Entity* bichitoEntity(bichitoEntity_h);
	bichitoEntity->sendMsg(MsgSetPlayer(playerEntity));
	playerEntity->sendMsg(MsgSetBichito(bichitoEntity));
	bichitoEntity->init();

	EntityListManager::get(CTrampoline::TAG).broadcast(MsgSetPlayer(playerEntity));
	EntityListManager::get(CCannon::TAG).broadcast(MsgSetPlayer(playerEntity));
	EntityListManager::get(CCreep::TAG).broadcast(MsgSetPlayer(playerEntity));
	EntityListManager::get(CEnemy::TAG).broadcast(MsgSetBichito(bichitoEntity));
    EntityListManager::get(CEnemy::TAG).broadcast(MsgSetPlayer(playerEntity));
    EntityListManager::get(CBoss::TAG).broadcast(MsgSetPlayer(playerEntity));
	EntityListManager::get(CBoss::TAG).broadcast(MsgSetBichito(bichitoEntity));
    EntityListManager::get(CPickup::TAG).broadcast(MsgSetPlayer(playerEntity));
    EntityListManager::get(logic::Trigger_AABB_TAG).broadcast(MsgSetPlayer(playerEntity));
	EntityListManager::get(CSmokePanel::TAG).broadcast(MsgSetPlayer(playerEntity));
	EntityListManager::get(CSmokePanel::TAG_ALWAYSHOT).broadcast(MsgSetPlayer(playerEntity));
#endif

#if defined(_OBJECTTOOL) || defined(_LIGHTTOOL)

	Entity* playerEntity;

	playerEntity = playerEntity_h;
	playerEntity->sendMsg(MsgSetCam(getCamera()));
	playerEntity->init();
    spawn();
    CTransform* t = playerEntity_h.getSon<CTransform>();
    auto offset = XMVectorSet(-1, 3, -1, 0);
	cam1P.setup(t->getPosition()+offset, -offset);
	camT->setPosition(t->getPosition()+offset);

#ifdef _LIGHTTOOL
	getManager<CDirLight>()->forall(&CDirLight::setSelectable);
	getManager<CPtLight>()->forall(&CPtLight::setSelectable);
	getManager<CVolPtLight>()->forall(&CVolPtLight::setSelectable);
	getManager<CMist>()->forall(&CMist::setSelectable);
	getManager<particles::CEmitter>()->forall(&particles::CEmitter::setSelectable);
	antTw_user::AntTWManager::createLightEditorTweak(getCamera());
#endif
#ifdef _OBJECTTOOL
	pos = XMVectorSet(3, 3, 0, 1);
	rotQ = XMVectorSet(0, 0, 0, 1);
	entitytramp = createTransformableEntity2(pos, rotQ, "trampolin");
	tramp_h = getManager<CTrampoline>()->createObj();
	trampoline = tramp_h;
	trampoline->setRotation(rotQ);
	entitytramp->add(tramp_h);
	antTw_user::AntTWManager::createObjectViewerTweak(tramp_h);
#endif


#endif
    
	getManager<CPtLight>()->forall(&CPtLight::findSpatialIndex);
	getManager<CDirLight>()->forall(&CDirLight::findSpatialIndex);
	getManager<CShadow>()->forall(&CShadow::findSpatialIndex);
	getManager<CCubeShadow>()->forall(&CCubeShadow::findSpatialIndex);
	getManager<CAmbientSound>()->forall(&CAmbientSound::playSound);

	dbg("Load complete.\n");
	loadingLevelComplete = true;
#if defined(_LIGHTTOOL) || defined(_PARTICLES)
    Entity* cam = camera_h;
    cam->add(getManager<CAABB>()->createObj());
    CAABB* camAABB = cam->get<CAABB>();
    camAABB->setCorners(-0.1f*one_v,0.1f*one_v);
	getManager<logic::CSpatialIndex>()->forall<void, Handle>(
        &logic::CSpatialIndex::setPlayer, camera_h, false);
#endif
}

bool App::waitVideo(){
	switch (gamelvl) {
	case 1:
		if (isLoadingThreadActve) return true;
		loadingthreadVar = false;
		thread_1.join();
		loadingLevelComplete = false;
		break;
	case 2:
		loadingthreadVar = false;
		thread_1.join();
		break;
	case 3:
		loadingthreadVar = false;
		thread_1.join();
		break;
	case 4:
		loadingthreadVar = false;
		thread_1.join();
		break;
	}
#if !defined(_PARTICLES) && !defined(_LIGHTTOOL)
	CLevelData* levelData = levelE->get<CLevelData>();
	levelData->playSong();
#endif
	return false;
}

void resetLiana(Entity* l, CLevelData* level)
{
    CLiana* old_liana = l->get<CLiana>(); 
    int nLinks = old_liana->getNLinks();
    float limitX = old_liana->getXLimit();
    float limitZ = old_liana->getZLimit();

    Entity* replacement = getManager<Entity>()->createObj();
    PrefabManager::get().prefabricateComponents("components/liana", replacement);

    CLiana* liana = replacement->get<CLiana>();
    liana->setNLinks(nLinks);
    liana->setLimits(limitX, limitZ);
    
    CRestore* r = replacement->get<CRestore>();
    CRestore* r_prev = l->get<CRestore>();
    r->set(*r_prev);

    CTransform* tran = replacement->get<CTransform>();
    tran->set(*r_prev);
    CTransformable* transformable = replacement->get<CTransformable>();
    transformable->setCenterAim(tran->getPosition() - yAxis_v * 0.5f * (nLinks / 2.f));
    
    
    level->replaceTaggedEntity(l, replacement);

    replacement->init();
    EntityListManager::get(CLiana::TAG).add(replacement);
    l->postMsg(MsgDeleteSelf());
}

void resetEnemy(Entity* enemy, CLevelData* level)
{
    CRestore* restore = enemy->get<CRestore>();
    assert(restore != nullptr);
    
    Entity* replacement = getManager<Entity>()->createObj();
    
    Handle h = getManager<CTransform>()->createObj();
    replacement->add(h);
    CTransform* t(h);
    t->set(*restore);
    
    h = getManager<CRestore>()->createObj();
    replacement->add(h);
    CRestore* newRestore = h;
    assert(restore != nullptr);
    newRestore->set(*restore);
    newRestore->setSpatialIndex(restore->getSpatialIndex());
    
    EntityListManager::get(CEnemy::TAG).remove(enemy);
    EntityListManager::get(CEnemy::TAG).add(replacement);
    
    level->replaceTaggedEntity(enemy, replacement);
    
    if (enemy->has<CFlare>()) {
        PrefabManager::get().prefabricateComponents("components/flare", replacement);
    } else if (enemy->has<CMelee>()) {
        PrefabManager::get().prefabricateComponents("components/melee", replacement);
    } else {
        fatal("no enemy!\n");
    }
    
    //Move up the enemy to avoid collisions with the ground
    CTransform* transform = replacement->get<CTransform>();
    transform->setPosition(transform->getPosition() + XMVectorSet(0, 0.45f, 0, 0));
    
    CSkeleton* skel = replacement->get<CSkeleton>();
    skel->setSpatialIndex(restore->getSpatialIndex());
    
    replacement->init();
    enemy->postMsg(MsgDeleteSelf());
}

void App::retry()
{
#ifndef _DEBUG
	Entity* player = playerEntity_h;
	player->sendMsg(MsgPlayerSpawn());
#endif
	static const auto sendRevive = [](Entity* e) {e->sendMsg(MsgRevive());};
	CLevelData* levelData = levelE->get<CLevelData>();

	getManager<CBullet>()->forall(&CBullet::removeFromScene);
	getManager<CPickup>()->forall(&CPickup::removeFromScene);
	getManager<CFlareShot>()->forall(&CFlareShot::removeFromScene);
    component::getManager<Entity>()->forall<void>(sendRevive);

    EntityList(EntityListManager::get(CEnemy::TAG)).forall(
        [levelData] (Handle h) {resetEnemy(h, levelData);}
    );
    EntityList(EntityListManager::get(CLiana::TAG)).forall(
        [levelData] (Handle h) {resetLiana(h, levelData);}
    );

	if (levelData != nullptr) {
		CCheckPoint* checkPoint = levelData->getSpawnCheckPoint();
		if (checkPoint != nullptr) {
			levelData->setCurrentCheckPoint(checkPoint);
		}
	}

	EntityListManager::get(CEnemy::TAG).broadcast(MsgSetPlayer(playerEntity_h));
	EntityListManager::get(CEnemy::TAG).broadcast(MsgSetBichito(bichitoEntity_h));
	spawn();
    
    if (!levelData->isBossLevel()) {
        PaintManager::reset();
    }
    MessageManager::dispatchPosts();

    EntityListManager::cleanupLists();
	playSong();
}

bool App::doGameOver()
{
	if (xboxController.is_connected()){
		xboxControllerKeys();
		xboxController.update();
		xboxPad.update();
		if ((pad.getState(CONTROLS_UP).isPressed() ||
			pad.getState(CONTROLS_LEFT).isPressed()) && pauseState == 1){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			pauseState = 0;
		}
		if ((pad.getState(CONTROLS_DOWN).isPressed() ||
			pad.getState(CONTROLS_RIGHT).isPressed()) && pauseState == 0){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			pauseState = 1;
		}
		if (pad.getState(CONTROLS_JUMP).isPressed()){
			if (pauseState == 1){
				returnToMenu = true;
			}
			if (pauseState == 0){
				playAgain = true;
			}
			fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
			pauseState = 0;
			isPlayerDead = false;
			return true;
		}
	}
	pad.update();
	if ((pad.getState(CONTROLS_MENU_UP).isPressed() ||
		pad.getState(CONTROLS_MENU_LEFT).isPressed()) && pauseState == 1){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		pauseState = 0;
	}
	if ((pad.getState(CONTROLS_MENU_DOWN).isPressed() ||
		pad.getState(CONTROLS_MENU_RIGHT).isPressed()) && pauseState == 0){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		pauseState = 1;
	}
	if (pad.getState(APP_ENTER).isPressed()){
		if (pauseState == 1){
			returnToMenu = true;
		}
		if (pauseState == 0){
			playAgain = true;
		}
		fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
		pauseState = 0;
		isPlayerDead = false;
		return true;
	}
	renderGameOver();
	return true;
}

bool App::doFrame()
{
	static bool debugPause = false;
    if (exit || pad.getState(APP_QUIT).isHit()) {return false;}

	if (xboxController.is_connected()){
		if (xboxPad.getState(APP_PAUSE).isHit()) { 
			paused = true; 
			pauseSong();
		}
		xboxControllerKeys();
		xboxController.update();
		xboxPad.update();
	}
	Mouse::update();
	pad.update();

	if (pad.getState(APP_PAUSE).isHit()) { 
		paused = true; 
		pauseSong();
	}
    if (pad.getState(APP_TOGGLE_MOUSE_CAPTURE).isHit()) {
        Mouse::toggleCapture();
    }

#if !defined(_OBJECTTOOL) && !defined(_LIGHTTOOL)
	if (pad.getState(APP_PAUSE_DEBUG).isHit()) { 
		debugPause = !debugPause;
#ifdef _DEBUG
        if (debugPause) {
            CTransform* camT = getCamera().getSon<CTransform>();
            cam1P.setup(camT->getPosition(), camT->getFront());
        }
#endif
    }
#else
	if (pad.getState(APP_TOGGLE_DEBUG_RENDERER).isHit()) { debugRenderer = !debugRenderer; }
	if (antTW){
		if (pad.getState(CONTROLS_DASH).isPressed()){
			if (!canLookWithCam){
				Mouse::capture();
				canLookWithCam = true;
			}
		}
		else{
			if (canLookWithCam){
				Mouse::release();
				canLookWithCam = false;
			}
		}
	}
#endif

	#ifdef _PARTICLES

	if (!antTW){
		if (pad.getState(CONTROLS_DASH).isPressed()){
			if (!canLookWithCam){
				Mouse::capture();
				canLookWithCam = true;
			}
		}
		else{
			if (canLookWithCam){
				Mouse::release();
				canLookWithCam = false;
			}
		}
	}
#endif
    
    #ifdef _DEBUG
    if (pad.getState(APP_TOGGLE_ANTTW).isHit()) {
        antTW = !antTW;
        if (antTW) {Mouse::release();}
        else {Mouse::capture();}
    }
    #endif
    
    #ifdef _DEBUG

        if (pad.getState(APP_TOGGLE_GODMODE).isHit()) {
            godMode = !godMode;
        }
        if (pad.getState(APP_RELOAD_SHADERS).isHit()) {
            Technique::getManager().forall(&Technique::reload);
        }
        if (pad.getState(APP_RELOAD_FX_PIPELINES).isHit()) {
            PostProcessPipeline::getManager().forall(&PostProcessPipeline::reload);
        }
        if (pad.getState(APP_TOGGLE_AABBS).isHit()) {
            renderAABBs = !renderAABBs;
        }
        if (pad.getState(APP_TOGGLE_GBUFFER).isHit()) {
            enableRenderGBufferChannels = !enableRenderGBufferChannels;
        }
        if (pad.getState(APP_DEBUG).isHit()) {
            #ifdef LOOKAT_TOOL
                animOff = !animOff;
            #endif
            dbg("Debug requested.\n");
        }
    #endif

    elapsed = countTime();
    float pxElapsed = elapsed;
    static bool slowmo = false;
	#if defined(_DEBUG) || defined(_ANITOOL)
        if (pad.getState(APP_SLOWMO).isHit()) {
            slowmo = !slowmo;
        }
		if (slowmo) elapsed /= 10.f;
    #endif
	#if defined(_ANITOOL)
		elapsed2 = elapsed;
	#endif

		if (!CameraManager::get().isPlayerCam())
		{
			updateCinematic(elapsed);
			render();
		}
        if (paused) {
            if (pauseMenu()) {
        		renderPaused();
                return true;
            } else {return false;}
        } else if (debugPause
#ifdef PAUSE_ON_FOCUS_LOSE
        || !focus
#endif        
            ) {
            if (updatePaused(elapsed)) {
        		render();
                return true;
            } else {return false;}

        } else if(update(elapsed)) {
        
        	PhysicsManager &physicsManager = PhysicsManager::get();
        
        	if (pxElapsed < 0.5) {
        		// Fixed update
        		mAccumulator += pxElapsed;
        		if (mAccumulator >= physicsManager.timeStep) {
        			fixedUpdate(mAccumulator);
        			mAccumulator = 0.0f;
        		}
        	}
        
            render();
        	return true;
        } else {
            return false;
        }
    }


void App::spawn()
{
	Entity* player = playerEntity_h;
    assert(levelE != nullptr);
    CLevelData* levelData = levelE->get<CLevelData>();
    CCharacterController* charCo = player->get<CCharacterController>();
    XMVECTOR pos = yAxis_v;
    CPlayerMov* mov = player->get<CPlayerMov>();
    if (mov != nullptr) {mov->setupSpawn();}
    if (levelData != nullptr) {
        CCheckPoint* checkPoint = levelData->getCurrentCheckPoint();
        if(checkPoint != nullptr) {
            pos += checkPoint->getPosition();
            CTransform* checkT = (Handle(checkPoint).getBrother<CTransform>());
            CTransform* meT = player->get<CTransform>();
            meT->setRotation(checkT->getRotation());
            if (mov != nullptr) {mov->resetCamSpawn();}
        }
    }

    CTransform* meT = player->get<CTransform>();
    meT->setPosition(pos);

#ifdef _LIGHTTOOL
	CTransform* modelT = playerModelEntity_h.getSon<CTransform>();
    modelT->set(*meT);
#else
    charCo->teleport(pos, false);
	Entity* bichito = bichitoEntity_h;
	bichito->sendMsg(MsgTeleportToPlayer());
#endif
	EntityListManager::get(CSmokeTower::TAG).broadcast(MsgSmokeTowerResetPhase());
    fixedUpdate(0);
}

void App::fixedUpdate(float elapsed)
{
    CPaintGroup::fixedUpdate(elapsed);
    PhysicsManager::get().fixedUpdate(elapsed);
}

void App::updateCameras(float elapsed)
{

	getManager<CCamera>()->update(elapsed);
	CCulling::rewindCullers();
	getManager<CCulling>()->update(elapsed);
	getManager<CCullingCube>()->update(elapsed);
	getManager<CCullingAABB>()->update(elapsed);

	//Render-related components
	auto dirLights = getManager<CDirLight>();
	auto ptLights = getManager<CPtLight>();
	auto volLights = getManager<CVolPtLight>();

	dirLights->forall<void, Handle>(&CDirLight::cull, getCamera(), true);
	ptLights->forall<void, Handle>(&CPtLight::cull, getCamera(), true);
	volLights->forall<void, Handle>(&CVolPtLight::cull, getCamera(), true);

	dirLights->update(elapsed);
	ptLights->update(elapsed);
}

bool App::updateCinematic(float elapsed)
{
	CTransform* camT = getCamera().getSon<CTransform>();
	cam1P.update(*camT, elapsed, true);

    updateCameras(elapsed);

	getManager<CSmokeTower>()->forall<void>(&CSmokeTower::updatePaused, elapsed);

	// Skeletons
	auto skelManager(getManager<CSkeleton>());
	skelManager->update(0);
	getManager<CBoneLookAt>()->update(0);
	getManager<CArmPoint>()->update(0);
	skelManager->forall(&CSkeleton::testAndAddBonesToBuffer);
	
	if (CameraManager::get().isPlayerCam())
		getManager<CAnimationSounds>()->update(0);

	updateGlobalConstants(elapsed);
	return true;
}

bool App::updatePaused(float elapsed)
{

#if defined(_DEBUG)
    CTransform* camT = getCamera().getSon<CTransform>();
    cam1P.update(*camT, elapsed, true);
#else
	if (CameraManager::get().isPlayerCam())
		getManager<CPlayerMov>()->forall<void>(&CPlayerMov::updatePaused, elapsed);
#endif
    
    updateCameras(elapsed);

    getManager<CSmokeTower>()->forall<void>(&CSmokeTower::updatePaused, elapsed);

    // Skeletons
    auto skelManager(getManager<CSkeleton>());
    skelManager->update(0);
    getManager<CBoneLookAt>()->update(0);
    getManager<CArmPoint>()->update(0);
    skelManager->forall(&CSkeleton::testAndAddBonesToBuffer);
	getManager<CAnimationSounds>()->update(0);
    
    updateGlobalConstants(elapsed);
	return true;
}

bool App::pauseMenu()
{
	pad.update();
	if (xboxController.is_connected()){
		xboxControllerKeys();
		xboxController.update();
		xboxPad.update();
		if ((pad.getState(CONTROLS_UP).isPressed() ||
			pad.getState(CONTROLS_LEFT).isPressed()) && pauseState == 1){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			pauseState = 0;
		}
		if ((pad.getState(CONTROLS_DOWN).isPressed() ||
			pad.getState(CONTROLS_RIGHT).isPressed()) && pauseState == 0){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			pauseState = 1;
		}
		if (pad.getState(CONTROLS_JUMP).isPressed()){
			if (pauseState == 1){
				returnToMenu = true;
				stopSong();
			}
			else{
				resumeSong();
			}
			fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
			pauseState = 0;
			paused = false;
			return true;
		}
	}
	if ((pad.getState(CONTROLS_MENU_UP).isPressed() ||
		pad.getState(CONTROLS_MENU_LEFT).isPressed()) && pauseState == 1){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		pauseState = 0;
	}
	if ((pad.getState(CONTROLS_MENU_DOWN).isPressed() ||
		pad.getState(CONTROLS_MENU_RIGHT).isPressed()) && pauseState == 0){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		pauseState = 1;
	}
	if (pad.getState(APP_ENTER).isPressed()){
		if (pauseState == 1){
			returnToMenu = true;
			stopSong();
		}
		else{
			resumeSong();
		}
		fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
		pauseState = 0;
		paused = false;
		return true;
	}
	return true;
}

void App::renderPaused()
{
	Render::getContext()->ClearRenderTargetView(Render::getRenderTargetView(), utils::BLACK);
	Render::getContext()->ClearDepthStencilView(Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	if(timerGameOver.count(countTime()) > 0.5f){
		timerGameOver.reset();
		gameOverImg = !gameOverImg;
	}
	if (!gameOverImg){
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("pause"));
	}
	else{
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("pauseon"));
	}
	switch (pauseState) {
	case 0:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("GameOverContinueON"), nullptr, true);
		break;
	case 1:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("GameOverExitON"), nullptr, true);
		break;
	}
	Render::getSwapChain()->Present(0, 0);
}

bool App::update(float elapsed)
{
#if !defined(_OBJECTTOOL) && !defined(_LIGHTTOOL) && !defined(_PARTICLES) && !defined(MUSIC_TOOL) 
    getManager<CCharacterController>()->update(elapsed);

	if (CameraManager::get().isPlayerCam())
		getManager<CPlayerMov>()->update(elapsed);

    getManager<CPlayerAttack>()->update(elapsed);
#ifndef LOOKAT_TOOL
    getManager<CPlayerStats>()->update(elapsed);
#endif

    //TEMP: "checkpoint"
    Entity* player = playerEntity_h;
    CTransform* playerT = player->get<CTransform>();
    XMVECTOR pos(playerT->getPosition());

    if (XMVectorGetY(pos) < -20 && !didspawn
	#if defined _DEBUG && !defined LOOKAT_TOOL
		  || pad.getState(APP_DEBUG).isHit()  
	#endif
      ) {
		didspawn = true;
		((CPlayerStats*)player->get<CPlayerStats>())->damage(30, true);
		if (((CPlayerStats*)player->get<CPlayerStats>())->getHealth() > 0){
			spawn();
		}
	}
	if (XMVectorGetY(pos) >= 0){
		didspawn = false;
	}

    getManager<CBullet>()->update(elapsed);
	getManager<CCannonPath>()->update(elapsed);	

    // Enemies
	if (CameraManager::get().isPlayerCam()){

		getManager<CBoss>()->update(elapsed);
		getManager<CWeakSpot>()->update(elapsed);
		getManager<CSmokePanel>()->update(elapsed);
		getManager<CEnemy>()->update(elapsed);
	}
    getManager<CFlareShot>()->update(elapsed);
	getManager<CBichito>()->update(elapsed);
    getManager<CMobile>()->update(elapsed);
    getManager<CFlyingMobile>()->update(elapsed);
	getManager<CTextHelper>()->update(elapsed);
    // Triggers
    getManager<logic::CSpatialIndex>()->update(elapsed);

    getManager<logic::CScriptTrigger>()->update(elapsed);
    getManager<level::CCheckPoint>()->update(elapsed);

#else
    getManager<logic::CSpatialIndex>()->update(elapsed);
    TransformableFSMExecutor::updateHighlights(elapsed, highlightTransformables);
    CTransform* camT(getCamera().getSon<CTransform>());
	cam1P.update(*camT, elapsed);
	
#endif
    getManager<CCamera>()->update(elapsed);
	getManager<CCameraAnim>()->update(elapsed);
    getManager<CCubeShadow>()->update(elapsed);
    getManager<CShadow>()->update(elapsed);
    CCulling::rewindCullers(); 
    getManager<CCulling>()->update(elapsed);
    getManager<CCullingCube>()->update(elapsed);
    getManager<CCullingAABB>()->update(elapsed);
    getManager<CCullingAABBSpecial>()->update(elapsed);
	getManager<CAmbientSound>()->update(elapsed);
	fmodUser::fmodUserClass::updateFmod();

    //Render-related components
    auto dirLights = getManager<CDirLight>();
    auto ptLights = getManager<CPtLight>();
	auto volLights = getManager<CVolPtLight>();

    dirLights->forall<void, Handle>(&CDirLight::cull, getCamera(), true);
	ptLights->forall<void, Handle>(&CPtLight::cull, getCamera(), true);
	volLights->forall<void, Handle>(&CVolPtLight::cull, getCamera(), true);
    
	dirLights->update(elapsed);
	ptLights->update(elapsed);

#if defined(_PARTICLES)
	getManager<CParticleSystem>()->update(elapsed);
#else
	getManager<particles::CEmitter>()->update(elapsed);
	ParticleUpdaterManager::get().update(elapsed);
#endif

    getManager<CMist>()->update(elapsed);
	
#if !defined(_OBJECTTOOL) && !defined(_PARTICLES)
    // Skeletons
    auto skelManager(getManager<CSkeleton>());
#ifdef _DEBUG

	#if !defined(_LIGHTTOOL)
		skelManager->update(animOff? FLT_EPSILON : elapsed);
	#endif

#else 
    skelManager->update(elapsed);
	
#endif
	getManager<CBoneLookAt>()->update(elapsed);
    getManager<CArmPoint>()->update(elapsed);
    skelManager->forall(&CSkeleton::testAndAddBonesToBuffer);
	getManager<CAnimationSounds>()->update(elapsed);
	getManager<CMaxAnim>()->update(elapsed);

	// Level elements
	getManager<CPickup>()->update(elapsed);
	getManager<CTransformable>()->update(elapsed);
	getManager<CDestructible>()->update(elapsed);
	getManager<CLiana>()->update(elapsed);
#if !defined(_LIGHTTOOL)
	getManager<CSmokeTower>()->update(elapsed);
	getManager<CKnife>()->update(elapsed);
#endif
	
	getManager<CLua>()->update(elapsed);
#endif

    component::MessageManager::dispatchPosts();
    updateGlobalConstants(elapsed);
    Material::updateAnimatedMaterials(elapsed);
    return true;
}

#if defined(_ANITOOL)
void renderAnimationState(CAnimationPlugger* vine){
	fontScreen.size = 15;
	fontScreen.color = 0xFF00FFFF;
	Handle h(vine);
	Entity*  element = h.getOwner();
	vine->setAnimationElapsed(elapsed2);
	if (element->has<CPlayerMov>()){
		CPlayerMov* cP = element->get<CPlayerMov>();
		if (vine->getActualPlug().type == AnimationArchetype::anim_e::MAIN_CYCLE){
			std::string str = "Cycle: " + vine->getActualPlug().name;
			fontScreen.print3D(cP->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
		}
		if (vine->getActualPlug().type == AnimationArchetype::anim_e::BLEND_CYCLE){
			if (vine->getActualPlug().unplug){
				std::string str = "Cycle: " + vine->getBackgroundCycle().name;
				fontScreen.print3D(cP->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
			}
			else{
				std::string str = "Blend: " + vine->getActualPlug().name;
				fontScreen.print3D(cP->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
			}
		}
		if (vine->getActualPlug().type == AnimationArchetype::anim_e::ACTION){
			if (vine->getAnimationElapsed() < vine->getActualPlugDurationWithDelay()){
				std::string str = "Action: " + vine->getActualPlug().name;
				fontScreen.print3D(cP->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
				std::string str2 = std::to_string(vine->getAnimationElapsed()) + " / " + std::to_string(vine->getActualPlugDurationWithDelay()) + " / " + std::to_string(vine->getActualPlugDuration());
				fontScreen.print3D(cP->getPosition() + XMVectorSet(0, 1.65f, 0, 0), str2.c_str());
			}
			else{
				std::string str = "Cycle: " + vine->getBackgroundCycle().name;
				fontScreen.print3D(cP->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
			}
		}
	}
	if (element->has<CMelee>()){
		CMelee* cM = element->get<CMelee>();
		if (vine->getActualPlug().type == AnimationArchetype::anim_e::MAIN_CYCLE){
			std::string str = "Cycle: " + vine->getActualPlug().name;
			fontScreen.print3D(cM->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
		}
		if (vine->getActualPlug().type == AnimationArchetype::anim_e::ACTION){
			if (vine->getAnimationElapsed() < vine->getActualPlugDurationWithDelay()){
				std::string str = "Action: " + vine->getActualPlug().name;
				fontScreen.print3D(cM->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
				std::string str2 = std::to_string(vine->getAnimationElapsed()) + " / " + std::to_string(vine->getActualPlugDurationWithDelay()) + " / " + std::to_string(vine->getActualPlugDuration());
				fontScreen.print3D(cM->getPosition() + XMVectorSet(0, 1.65f, 0, 0), str2.c_str());
			}
			else{
				std::string str = "Cycle: " + vine->getBackgroundCycle().name;
				fontScreen.print3D(cM->getPosition() + XMVectorSet(0, 1.75f, 0, 0), str.c_str());
			}
		}
	}
}
#endif

#if defined(_LIGHTTOOL)
void renderAxis(const XMVECTOR& pos, const Color& tint=0, float dist=1.0f)
{
    fontScreen.print3D(pos + xAxis_v*dist, "X");
    fontScreen.print3D(pos + yAxis_v*dist, "Y");
    fontScreen.print3D(pos + zAxis_v*dist, "Z");
    setObjectConstants(XMMatrixAffineTransformation(one_v*dist*0.9f, zero_v, one_q, pos), tint);
    mesh_axis.activateAndRender();
}

void renderLightToolItems(XMVECTOR camPos)
{
    Technique::getManager().getByName("basic")->activate();
    fontScreen.size = 10;
    fontScreen.color = 0xFF00FF00;
    auto grayed = Color::GRAY;
    grayed.setAf(0.5f);
    auto renderPtLight = [camPos, grayed](CPtLight* l) {
        Entity* e(Handle(l).getOwner());
        CTransform* t = e->get<CTransform>();
        auto pos = t->getPosition() + l->getOffset();
        if (l->getSelected()) {
            renderAxis(pos, grayed);
        }
        if (testDistanceSqEu(pos, camPos, 30.0f)){
			fontScreen.print3D(pos, e->getName().c_str());
		}
    };
    auto renderDirLight = [camPos, grayed](CDirLight* l) {
        Entity* e(Handle(l).getOwner());
        CTransform* t = e->get<CTransform>();
        auto pos = t->getPosition() + l->getOffset();
        if (l->getSelected()) {
            renderAxis(pos, grayed);
            renderAxis(t->getLookAt(), grayed, 0.25f);
        }
        if (testDistanceSqEu(pos, camPos, 30.0f)){
			fontScreen.print3D(pos, e->getName().c_str());
		}
    };
    auto renderVolPtLight = [camPos, grayed](CVolPtLight* l) {
        Entity* e(Handle(l).getOwner());
        CTransform* t = e->get<CTransform>();
        auto pos = t->getPosition();
        if (l->isSelected()) {
            renderAxis(pos, grayed);
            renderAxis(t->getLookAt(), grayed, 0.25f);
        }
        if (testDistanceSqEu(pos, camPos, 30.0f)){
			fontScreen.print3D(pos, e->getName().c_str());
		}
    };
    auto renderMist = [camPos, grayed](CMist* l) {
        Entity* e(Handle(l).getOwner());
        CTransform* t = e->get<CTransform>();
        auto pos = t->getPosition();
        if (l->isSelected()) {
            renderAxis(pos, grayed);
            renderAxis(t->getLookAt(), grayed, 0.25f);
        }
        if (testDistanceSqEu(pos, camPos, 30.0f)){
			fontScreen.print3D(pos, e->getName().c_str());
		}
    };

    getManager<CDirLight>()->forall<void>(renderDirLight);
    getManager<CPtLight>()->forall<void>(renderPtLight);
    getManager<CVolPtLight>()->forall<void>(renderVolPtLight);
    getManager<CMist>()->forall<void>(renderMist);
}
#endif

void App::renderGameOver()
{
	Render::getContext()->ClearRenderTargetView(Render::getRenderTargetView(), utils::BLACK);
	Render::getContext()->ClearDepthStencilView(Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	if (timerGameOver.count(countTime()) > 0.5f){
		timerGameOver.reset();
		gameOverImg = !gameOverImg;
	}
	if (!gameOverImg){
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("GameOver"));
	}
	else{
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("GameOverON"));
	}
	switch (pauseState) {
	case 0:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("GameOverContinueON"), nullptr, true);
		break;
	case 1:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("GameOverExitON"), nullptr, true);
		break;
	}
	Render::getSwapChain()->Present(0, 0);
}

#if defined(_DEBUG) || defined(_OBJECTTOOL)
void App::renderSelectedChannel()
{
    activateRSConfig(RSCFG_DEFAULT);
    activateZConfig(ZCFG_DEFAULT);
    activateBlendConfig(BLEND_CFG_DEFAULT);
    static const Technique*const alphaTech = Technique::getManager().getByName("FX_alpha");
    static const Technique*const invAlphaTech = Technique::getManager().getByName("FX_invalpha");
    const Technique* tech = nullptr;

    const Texture* channel = nullptr;

    auto shadowMan = getManager<CShadow>();
    auto cubeShadowMan = getManager<CCubeShadow>();

    switch (selectedChannel) {
        case FINAL: channel = deferred.getOut(); break;
        case FX_FINAL: channel = deferred.getFxOut(); break;
        case ALBEDO_PLUS_PARTICLES:
        case ALBEDO: channel = deferred.getAlbedo(); break;
        case LIGHTS: channel = deferred.getLights(); break;
        case SELFILL: channel = deferred.getSelfIllumination(); break;
        case FXSELFILL: channel = deferred.getFxSelfIllumination(); break;
        case NORMALS: channel = deferred.getNormals(); break;
        case POSITION: channel = deferred.getSpace(); break;
        case AMBIENT: channel = deferred.getAmbient(); break;
        case DATA: channel = deferred.getData1(); break;
        case UVPAINT: channel = deferred.getData2(); break;
        case PAINT: channel = deferred.getPaint(); break;
        case CUBESHADOW:
            if(cubeShadowMan->getSize() > cubeShadowToRender) {
                channel = (*cubeShadowMan->begin() + cubeShadowToRender)->getShadowBuffer();
            } break;
        case FXCUBESHADOW: 
            if(cubeShadowMan->getSize() > cubeShadowToRender) {
                channel = (*cubeShadowMan->begin() + cubeShadowToRender)->getFxShadowBuffer();
            } break;
        case SHADOW:
            if(shadowMan->getSize() >shadowToRender) {
                channel = (*shadowMan->begin() + shadowToRender)->getShadowBuffer();
            } break;
        case FXSHADOW: 
            if(shadowMan->getSize() > cubeShadowToRender) {
                channel = (*shadowMan->begin() + shadowToRender)->getFxShadowBuffer();
            } break;

        case SPECULAR: channel = deferred.getLights(); tech = alphaTech; break;
        case DEPTH: channel = deferred.getSpace(); tech = alphaTech; break;
        case PAINT_AMOUNT: channel = deferred.getData2(); tech = alphaTech; break;

        default: channel = deferred.getFxOut(); break;
    }

    pixelRect screenRect(config.xres, config.xres);
    if (channel != nullptr) {
        drawTexture2D(screenRect, screenRect, channel, tech);
    }
}

void App::printSelectedChannelName()
{
#define _CASE_NAME(name, txt) case name: txt = #name; break;
    std::string txt = "<unnamed>";
    switch (selectedChannel) {
        _CASE_NAME(FINAL, txt)
        _CASE_NAME(FX_FINAL, txt)
        _CASE_NAME(ALBEDO, txt)
        _CASE_NAME(ALBEDO_PLUS_PARTICLES, txt)
        _CASE_NAME(LIGHTS, txt)
        _CASE_NAME(SPECULAR, txt)
        _CASE_NAME(SELFILL, txt)
        _CASE_NAME(FXSELFILL, txt)
        _CASE_NAME(DEPTH, txt)
        _CASE_NAME(NORMALS, txt)
        _CASE_NAME(POSITION, txt)
        _CASE_NAME(DATA, txt)
        _CASE_NAME(PAINT, txt)
        _CASE_NAME(PAINT_AMOUNT, txt)
        _CASE_NAME(AMBIENT, txt)
        _CASE_NAME(UVPAINT, txt)
        case SHADOW: {
            auto shadowMan = getManager<CShadow>();
            if(shadowMan->getSize() > shadowToRender) {
                Entity* e = Handle(*shadowMan->begin() + shadowToRender).getOwner();
                txt = "SHADOW: " + e->getName();
            } else {
                txt = "SHADOW (invalid selected)";
            }
        } break;
        case FXSHADOW: {
            auto shadowMan = getManager<CShadow>();
            if(shadowMan->getSize() > shadowToRender) {
                Entity* e = Handle(*shadowMan->begin() + shadowToRender).getOwner();
                txt = "FXSHADOW: " + e->getName();
            } else {
                txt = "FXSHADOW (invalid selected)";
            }
        } break;
        case CUBESHADOW: {
            auto shadowMan = getManager<CCubeShadow>();
            if(shadowMan->getSize() > cubeShadowToRender) {
                Entity* e = Handle(*shadowMan->begin() + cubeShadowToRender).getOwner();
                txt = "CUBESHADOW: " + e->getName();
            } else {
                txt = "CUBESHADOW (invalid selected)";
            }
        } break;
        case FXCUBESHADOW: {
            auto shadowMan = getManager<CCubeShadow>();
            if(shadowMan->getSize() > cubeShadowToRender) {
                Entity* e = Handle(*shadowMan->begin() + cubeShadowToRender).getOwner();
                txt = "FXCUBESHADOW: " + e->getName();
            } else {
                txt = "FXCUBESHADOW (invalid selected)";
            }
        } break;
        default: break;
    }
    fontDBG.color = (Color::PALE_PINK).abgr();
    fontDBG.print(2,2, txt.c_str());
}

void App::renderGBufferChannels()
{
    activateBlendConfig(BLEND_CFG_DEFAULT);
    static const Technique*const alphaTech = Technique::getManager().getByName("FX_alpha");

    unsigned sy = config.yres / 4;
	unsigned sx = sy*config.xres / config.yres;

    pixelRect dstRect(config.xres, config.yres);

	drawTexture2D(pixelRect(0 * sx, 0 * sy, sx, sy), dstRect, deferred.getAlbedo());
	drawTexture2D(pixelRect(1 * sx, 0 * sy, sx, sy), dstRect, deferred.getOut());
	drawTexture2D(pixelRect(2 * sx, 0 * sy, sx, sy), dstRect, deferred.getFxOut());
	drawTexture2D(pixelRect(3 * sx, 0 * sy, sx, sy), dstRect, deferred.getAmbient());
	drawTexture2D(pixelRect(0 * sx, 1 * sy, sx, sy), dstRect, deferred.getLights());
	drawTexture2D(pixelRect(1 * sx, 1 * sy, sx, sy), dstRect, deferred.getLights(), alphaTech);
	drawTexture2D(pixelRect(2 * sx, 1 * sy, sx, sy), dstRect, deferred.getSelfIllumination());
	drawTexture2D(pixelRect(3 * sx, 1 * sy, sx, sy), dstRect, deferred.getFxSelfIllumination());
	drawTexture2D(pixelRect(0 * sx, 2 * sy, sx, sy), dstRect, deferred.getData1());
	drawTexture2D(pixelRect(1 * sx, 2 * sy, sx, sy), dstRect, deferred.getData1(), alphaTech);
	drawTexture2D(pixelRect(2 * sx, 2 * sy, sx, sy), dstRect, deferred.getData2());
	drawTexture2D(pixelRect(3 * sx, 2 * sy, sx, sy), dstRect, deferred.getData2(), alphaTech);
	drawTexture2D(pixelRect(0 * sx, 3 * sy, sx, sy), dstRect, deferred.getNormals());
	drawTexture2D(pixelRect(1 * sx, 3 * sy, sx, sy), dstRect, deferred.getPaintedNormals());
	drawTexture2D(pixelRect(2 * sx, 3 * sy, sx, sy), dstRect, deferred.getAmbient());
	drawTexture2D(pixelRect(3 * sx, 3 * sy, sx, sy), dstRect, deferred.getPaint());
}
#endif

void App::render()
{
	Render::getContext()->ClearRenderTargetView(
		Render::getRenderTargetView(), utils::BLACK);
    
	Render::getContext()->ClearDepthStencilView(
		Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (RenderConstsMirror::dirty) {RenderConstsMirror::update();}
	activateTextureSamplers();

    auto cam_h = getCamera();
	CCamera* camera = cam_h.getSon<CCamera>();
    camera->setViewport(0, 0, (float)config.xres, (float)config.yres);
	activateCamera(*camera);

#if !defined(_PARTICLES)
	uploadBonesToGPU();
	activateBoneBuffer();
#endif

#ifdef _DEBUG
    if (enableRenderGBufferChannels || selectedChannel >= SHADOW)
#endif
	{
	    //shadow
		TraceScoped scope("shadows");
		activateZConfig(zConfig_e::ZCFG_DEFAULT);
        activateRSConfig(RSCFG_SHADOWS);
		getManager<CShadow>()->forall(&CShadow::generate);
		getManager<CCubeShadow>()->forall(&CCubeShadow::generate);
	}
    deferred.clearGBuffer();
	deferred.initGBuffer();
	activateCamera(*camera); 
	//Skybox
#if defined(_DEBUG)
	if (drawSkyBox)
#endif
	{
		TraceScoped scope("skybox");
        static const Technique* tech = Technique::getManager().getByName("skybox");
		activateZConfig(zConfig_e::ZCFG_LE);
		activateBlendConfig(BlendConfig::BLEND_CFG_DEFAULT);
        activateRSConfig(RSCFG_DEFAULT);
		tech->activate();
		getManager<CSkyBox>()->forall(&CSkyBox::render);
	}    

	deferred(cam_h);

   
    Render::activateBackBuffer();
#if (defined(_DEBUG) || defined(_OBJECTTOOL)) && !defined(MUSIC_TOOL)
    if (enableRenderGBufferChannels) {
        renderGBufferChannels();
    } else {
        renderSelectedChannel();
    }
#else
        pixelRect screenRect(config.xres, config.yres);
        drawTexture2D(screenRect, screenRect, deferred.getFxOut(), false);
#endif



#if !defined(_OBJECTTOOL) && !defined(_LIGHTTOOL) && !defined(_PARTICLES) && !defined(LOOKAT_TOOL)
#ifdef _DEBUG
    if (!enableRenderGBufferChannels)
#endif
    {
        TraceScoped scope("HUD");
        Entity* player(playerEntity_h);
        CPlayerStats* playerStats = player->get<CPlayerStats>();
        CPlayerMov* playerMov = player->get<CPlayerMov>();
		float life = (float)playerStats->getHealth();
		float mana = (float)playerStats->getEnergy();

        activateZConfig(zConfig_e::ZCFG_TEST_ALL);

		fontScreen.size = 30;
		fontScreen.color = 0xFF0000FF;

		unsigned sy = config.yres / 10;
		unsigned sx = (sy*config.xres / config.yres) * 2;
		
		if (playerMov->isOnCannon()) {
				drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
                Texture::getManager().getByName("cannon_aim"),
                nullptr, true);
				float imgPosX, imgPosY, imgWidth, imgHeight;
				getImgValues(imgPosX, imgPosY, imgWidth, imgHeight, 614, 64, 80, 80);
			if (playerMov->getCannonState() == 1)
				drawText(pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
				pixelRect(config.xres, config.yres), "3");
			if(playerMov->getCannonState() == 2)
				drawText(pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
				pixelRect(config.xres, config.yres), "2");
			if(playerMov->getCannonState() == 3)
				drawText(pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
				pixelRect(config.xres, config.yres), "1");
        }
		playerStats->drawHUD(elapsed);
		getManager<CBichito>()->forall(&CBichito::renderHelpers);
		CTextHelper* playerTextHelper = player->get<CTextHelper>();
		playerTextHelper->renderHelper(elapsed);
    }
#endif

#ifdef _OBJECTTOOL
	//Text
	fontScreen.size = 13;
	fontScreen.color = 0xFFFFFF00;
	fontScreen.printf(3, 3, "WASD QE to move\nF6 and click to transform\nF6 to object viewer\nF9 debugRenderer (can't transform)");

#endif

#ifdef _DEBUG
    if (printFonts)
#endif
    {
        TraceScoped _("DEBUG");

        #ifdef _LIGHTTOOL
	        if (!debugRenderer && !enableRenderGBufferChannels) {
	            activateCamera(*camera);
	            renderLightToolItems(camera->getPosition());
            }
                #if defined(PRINT_FPS_FONT)
                    #define _FACTOR_FONT_HEIGHT 2
                #else
                    #define _FACTOR_FONT_HEIGHT 1
                #endif
	                fontDBG.color = (Color::LEMON_LIME).abgr();
	                fontDBG.printf(2, config.yres - fontDBG.size*_FACTOR_FONT_HEIGHT - 2,
                        "F9=toggle fustrum display  --- "
                        "F8=toggle AABB display (try to keep these small!) ---"
                        "SHIFT/CAPSLOCK cam speed \n");
        #endif
        #ifdef _ANITOOL
        	    component::getManager<CAnimationPlugger>()->forall<void>(renderAnimationState);
        #endif
        #if defined(PRINT_FPS_FONT)
        	    fontDBG.color = (Color::BLUE).abgr();
        	    fontDBG.printf(2, config.yres - fontDBG.size - 2, "elapsed=%f\tfps=%f\n", delta_secs, fps);
        #ifdef _DEBUG
                if (!enableRenderGBufferChannels && printChannelName) {printSelectedChannelName();}
        #endif
        #endif

        #ifdef _PARTICLES
                fontScreen.size = 13;
                fontScreen.color = 0xFFFFFFFF;
                fontScreen.printf(2, config.yres - fontDBG.size*2 - 2, "WASDQE to move - F6 editor");
        #endif
    }

    //AntTweakBar
#ifdef _DEBUG
	if (antTW && TwGetBarCount()) {
        TraceScoped _("AntTW");
		TwDraw();
	}
#endif

    Render::getSwapChain()->Present(0, 0);
    rewindBoneBuffer();

	
}

void App::destroy()
{
    dbg("Destroying app.\n");
    try {
#ifdef LOOKAT_TOOL
        for (auto& tw : lookAtTw) {SAFE_DELETE(tw);}
        for (auto& tw : armPointTw) {SAFE_DELETE(tw);}
#endif

#if defined(_DEBUG)
        AntTWManager::tearDown();
#endif
        EffectLibrary::tearDown();

        fontScreen.destroy();//old
        component::cleanup();

        deferred.destroy();

		fmodUser::fmodUserClass::shutdownFmod();
		fmodUser::FmodStudio::shutdown();
	    PhysicsManager::get().ShutdownPhysX();
        Mesh::getManager().destroyAll();
        Texture::getManager().destroyAll();
        Technique::getManager().destroyAll();

        renderUtilsDestroy();
        Render::destroyDevice();

    } catch (...) {
        dbg("Something happened during teardown.\n");
    }
}

void App::loadVideo(const char* name, const char* audio)
{
#if defined(DISPLAY_VIDEO_AND_MENUS)
	mgr = new TheoraVideoManager();
	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s", "data/videos", name);
	clip = mgr->createVideoClip(full_name, TheoraOutputMode::TH_RGBX, 16);
	w = clip->getWidth();
	h = clip->getHeight();
	D3D11_TEXTURE2D_DESC desc;
	// Create tex2D
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = w;
	desc.Height = h;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.MipLevels = 1;
	HRESULT hr = Render::getDevice()->CreateTexture2D(&desc, NULL, &tex);
	if (FAILED(hr)) 	dbg("FAILED CreateTexture2D\n");
	videoTexture = new Texture();
	endframe = int(clip->getDuration() * clip->getFPS());
	if (audio != ""){
		char full_name_audio[MAX_PATH];
		sprintf(full_name_audio, "%s/%s", "", audio);
		fmodUser::fmodUserClass::playSound(full_name_audio, 1.0f, 0.0f);
	}
	clip->play();
	countTime();
#endif
		}

bool App::updateVideo(bool canSkipVideo, bool inThread){
	pad.update();
	if (xboxController.is_connected()){
		xboxControllerKeys();
		xboxController.update();
		xboxPad.update();
		if (pad.getState(CONTROLS_JUMP).isHit() && canSkipVideo){
			fmodUser::fmodUserClass::stopSounds();
			return false;
		}
	}
	if (inThread){
		if (isKeyPressed(VK_RETURN) && canSkipVideo){
			fmodUser::fmodUserClass::stopSounds();
			return false;
		}
	}
	else{
		if (pad.getState(APP_ENTER).isHit() && canSkipVideo){
			fmodUser::fmodUserClass::stopSounds();
			return false;
		}
	}
	
	return renderVideo();
}

bool App::renderVideo()
{
#if defined(DISPLAY_VIDEO_AND_MENUS)
	TheoraVideoFrame *frame = clip->getNextFrame();
	Render::getContext()->ClearRenderTargetView(Render::getRenderTargetView(), utils::BLACK);
	Render::getContext()->ClearDepthStencilView(Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	if (frame){
		D3D11_MAPPED_SUBRESOURCE resource;
		HRESULT hr = Render::getContext()->Map(tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		if (FAILED(hr)) {
			return false;
		}
		// Copy full memory block
		unsigned char* buffer = frame->getBuffer();
		BYTE* mappedData = reinterpret_cast<BYTE*>(resource.pData);
		for (UINT i = 0; i < h; ++i)
		{
			memcpy(mappedData, buffer, w * 4);
			mappedData += resource.RowPitch;
			buffer += w * 4;
		}
		// Unlock resource
		Render::getContext()->Unmap(tex, 0);
		clip->popFrame();
		if (frame->getFrameNumber() >= (endframe - 1)){
			clip->stop();
			mgr->destroyVideoClip(clip);
			fmodUser::fmodUserClass::stopSounds();
			clip = nullptr;
			mgr = nullptr;
			return false;
		}
	}
	Render::getDevice()->CreateShaderResourceView(tex, 0, &m_shaderResourceView);
	videoTexture->setResource(tex);
	videoTexture->setResourceView(m_shaderResourceView);
	drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres), videoTexture);
	Render::getSwapChain()->Present(0, 0);
	float elaps = countTime();
	//dbg("elaps: %f\n", elaps);
	mgr->update(elaps);
#endif
	return true;
}

int App::updateMainMenu(){
	pad.update();
	if (xboxController.is_connected()){	
		xboxControllerKeys();
		xboxController.update();
		xboxPad.update();
		if (pad.getState(CONTROLS_UP).isHit() && mainMenuState > 0){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			mainMenuState--;
		}
		if (pad.getState(CONTROLS_DOWN).isHit() && mainMenuState < 3){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			mainMenuState++;
		}
		if (pad.getState(CONTROLS_JUMP).isHit()){
			fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
			return mainMenuState;
		}
	}
	if (pad.getState(APP_QUIT).isHit()){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		mainMenuState = 3;
	}
	if (pad.getState(CONTROLS_MENU_UP).isHit() && mainMenuState > 0){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		mainMenuState--;
	}
	if (pad.getState(CONTROLS_MENU_DOWN).isHit() && mainMenuState < 3){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		mainMenuState++;
	}
	if (pad.getState(APP_ENTER).isHit()){
		fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
		return mainMenuState;
	}
	renderMainMenu();
	return -1;
}

void App::renderMainMenu()
{
	Render::getContext()->ClearRenderTargetView(Render::getRenderTargetView(), utils::BLACK);
	Render::getContext()->ClearDepthStencilView(Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
		Texture::getManager().getByName("menu_off"));
	switch (mainMenuState) {
	case 0:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("menu_newgame"), nullptr, true);
		break;
	case 1:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("menu_chapter"), nullptr, true);
		break;
	case 2:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("menu_credits"), nullptr, true);
		break;
	case 3:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("menu_exit"), nullptr, true);
		break;
	}
	Render::getSwapChain()->Present(0, 0);
}

int App::updateChapterSelectionMenu(){
	pad.update();
	if (xboxController.is_connected()){
		xboxControllerKeys();
		xboxController.update();
		xboxPad.update();
		if (pad.getState(CONTROLS_UP).isHit() && chapterSelectionState > 0){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			chapterSelectionState--;
		}
		if (pad.getState(CONTROLS_DOWN).isHit() && chapterSelectionState < 4){
			fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
			chapterSelectionState++;
		}
		if (pad.getState(CONTROLS_JUMP).isHit()){
			fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
			return chapterSelectionState;
		}
	}
	if (pad.getState(APP_QUIT).isHit()){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		chapterSelectionState = 3;
	}
	if (pad.getState(CONTROLS_MENU_UP).isHit() && chapterSelectionState > 0){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		chapterSelectionState--;
	}
	if (pad.getState(CONTROLS_MENU_DOWN).isHit() && chapterSelectionState < 4){
		fmodUser::fmodUserClass::playSound("Menu_move", 1.0f, 0.0f);
		chapterSelectionState++;
	}
	if (pad.getState(APP_ENTER).isHit()){
		fmodUser::fmodUserClass::playSound("Menu_enter", 1.5f, 0.0f);
		return chapterSelectionState;
	}
	renderChapterSelectionMenu();
	return -1;
}

void App::renderChapterSelectionMenu()
{
	Render::getContext()->ClearRenderTargetView(Render::getRenderTargetView(), utils::BLACK);
	Render::getContext()->ClearDepthStencilView(Render::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
		Texture::getManager().getByName("chapter_selection_back"));
	drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
		Texture::getManager().getByName("chapter_selection_main"), nullptr, true);
	switch (chapterSelectionState) {
	case 0:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("chapter_selection_option1"), nullptr, true);
		break;
	case 1:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("chapter_selection_option2"), nullptr, true);
		break;
	case 2:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("chapter_selection_option3"), nullptr, true);
		break;
	case 3:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("chapter_selection_option4"), nullptr, true);
		break;
	case 4:
		drawTexture2D(pixelRect(config.xres, config.yres), pixelRect(config.xres, config.yres),
			Texture::getManager().getByName("chapter_selection_option5"), nullptr, true);
		break;
	}
	Render::getSwapChain()->Present(0, 0);
}

void App::playSong(){
	if (levelE != nullptr) {
		CLevelData* lvl(levelE->get<CLevelData>());
		lvl->playSong();
	}
}

void App::stopSong(){
	if (levelE != nullptr) {
		CLevelData* lvl(levelE->get<CLevelData>());
		lvl->stopSong();
	}
}

void App::pauseSong(){
	if (levelE != nullptr) {
		CLevelData* lvl(levelE->get<CLevelData>());
		lvl->pauseSong();
	}
}

void App::resumeSong(){
	if (levelE != nullptr) {
		CLevelData* lvl(levelE->get<CLevelData>());
		lvl->resumeSong();
	}
}