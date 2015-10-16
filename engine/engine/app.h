#ifndef INC_APP_H_
#define INC_APP_H_

#include "handles/handle.h"
#include "utils/input.h"
#include "render/shader/shaders.h"
#include "render/deferredRender.h"
#include "render/effect/effect.h"
#include "behavior/fsm.h"

#include "render/texture/DDSTextureLoader.h"

enum controls {
    APP_QUIT,
	APP_ENTER,
    APP_TOGGLE_GODMODE,
    APP_TOGGLE_MOUSE_CAPTURE,
    APP_PAUSE_DEBUG,
    APP_PAUSE,
    APP_TOGGLE_DEBUG_RENDERER,
    APP_DEBUG,
    APP_TOGGLE_ANTTW,
    APP_RELOAD_SHADERS,
    APP_RELOAD_FX_PIPELINES,
    APP_TOGGLE_AABBS,
    APP_TOGGLE_GBUFFER,
    APP_SLOWMO,
	APP_CHANGE_CAMERA,
    APP_NUM_ENUMS
};

class AppFSMExecutor;
namespace behavior { typedef Fsm<AppFSMExecutor> AppFSM; }
class App;
class AppFSMExecutor {
public:
	friend App;
	friend behavior::AppFSM;

	enum states {
		STATE_loading,
		STATE_loadvideo,
		STATE_playvideo,
		STATE_mainmenu,
		STATE_chapterselectionmenu,
		STATE_game,
		STATE_gameover,
		STATE_changelvl,
		STATE_retry,
		STATE_win,
		STATE_credits,
		STATE_quit,
		STATE_change_camera,
	};

private:
	//states
	behavior::fsmState_t loading(float elapsed);
	behavior::fsmState_t loadvideo(float elapsed);
	behavior::fsmState_t playvideo(float elapsed);
	behavior::fsmState_t mainmenu(float elapsed);
	behavior::fsmState_t chapterselectionmenu(float elapsed);
	behavior::fsmState_t game(float elapsed);
	behavior::fsmState_t gameover(float elapsed);
	behavior::fsmState_t changelvl(float elapsed);
	behavior::fsmState_t retry(float elapsed);
	behavior::fsmState_t win(float elapsed);
	behavior::fsmState_t credits(float elapsed);
	behavior::fsmState_t quit(float elapsed);
	behavior::fsmState_t cinematic(float elapsed);

public:
	inline behavior::fsmState_t init() { return STATE_loading; }
	void update(float elapsed);
	inline void changeState(behavior::fsmState_t newState) {}
};

class App {
	public:
		friend AppFSMExecutor;
	private:
		behavior::AppFSM fsm;
    public:
	    static inline App& get() {return instance;}
    private:
        static App instance;    
	    App();
        render::DeferredRender deferred;
        bool focus;

    public:
        //Deleting these cosntructors and operators will save us many a headache
	    App(const App& copy) = delete;
        App(App&& move) = delete; 
        App& operator=(const App& copy) = delete;  
        App& operator=(App&& move) = delete;         

        struct config_t {
	        int  xres;
	        int  yres;
            bool windowed = true;

            config_t() = default;
            config_t(
                int xres, int yres,
                bool windowed) :
                xres(xres), yres(yres)
            {}
        };
    private:
        component::Handle camera_h;

		component::Handle playerEntity_h;
		component::Handle bichitoEntity_h;
		component::Handle smokeEntity_h;
		component::Handle fireEntity_h;

		UINT w, h;
		ID3D11Texture2D* tex;
		render::Texture* videoTexture;
		ID3D11ShaderResourceView* m_shaderResourceView;
		unsigned long endframe = 0;
        
		bool exit = false;
		bool playAgain = false;
		bool gameisPaused = false;
        utils::Pad pad;
		utils::Pad xboxPad;
		utils::GamePadXBOXController xboxController;

		component::Handle a;
    private:
        static const config_t defConfig;
		void fixedUpdate(float elapsed);
        bool pauseMenu();
		bool updateCinematic(float elapsed);
		void updateCameras(float elapsed);
        bool updatePaused(float elapsed);
        bool update(float elapsed);
        void render();
		void retry();
		void renderGameOver();
        void addMappings();
		void xboxControllerKeys();
#if defined(_DEBUG) || defined(_OBJECTTOOL)
        void renderGBufferChannels();
        void renderSelectedChannel();
        void printSelectedChannelName();
#endif

    public:
        config_t config;
        HWND hWnd;

		float elapsed;

		component::Handle replaceEntity_h;
		int actualCheckpoint;

#if defined(_DEBUG) || defined(_OBJECTTOOL)

        void drawDebug() const;

#define INIT_GODMODE true
#define INIT_GBUFFER false
#define INIT_PRINTCHANNELNAME true

#ifdef LOOKAT_TOOL
    #define INIT_ALLOWLONGIDLE false
#else
    #define INIT_ALLOWLONGIDLE true
#endif

#ifdef _LIGHTTOOL
    #define INIT_RENDERMESHAABB false
    #define INIT_RENDERINSTANCEMESHAABB INIT_RENDERMESHAABB
    #define INIT_RENDERLIGHTAABB true
#else
    #define INIT_RENDERMESHAABB true
    #define INIT_RENDERINSTANCEMESHAABB INIT_RENDERMESHAABB
    #define INIT_RENDERLIGHTAABB false
#endif
#if defined(_PARTICLES)
    #define INIT_SELECTEDCHANNEL FX_FINAL
    #define INIT_DRAWSKYBOX false
#else
    #define INIT_SELECTEDCHANNEL FX_FINAL
    #define INIT_DRAWSKYBOX true
#endif
        bool antTW = false;
		bool canLookWithCam = false;
        bool enableRenderGBufferChannels = INIT_GBUFFER;
        bool renderAABBs = false;
        bool renderMeshAABBs = INIT_RENDERMESHAABB;
        bool renderInstanceMeshAABBs = INIT_RENDERINSTANCEMESHAABB;
        bool renderLightAABBs = INIT_RENDERLIGHTAABB;
        bool drawSkyBox = INIT_DRAWSKYBOX;
        bool instanceCulling = true;
        bool useCullF = false;
        bool godMode = INIT_GODMODE;
        bool animOff = false;
        bool printFonts = true;
        bool printChannelName = INIT_PRINTCHANNELNAME;
        bool debugRenderer = false;
        bool allowLongIdle = INIT_ALLOWLONGIDLE;
        bool clickToTransform = true;
        bool highlightTransformables = true;
        bool infiniteEnergy = false;
        bool drawPaintVolume = false;
        bool drawPaint = true;
		bool mistMode = false;

		component::Handle playerModelEntity_h;

        enum channel_e {
            FINAL,
            FX_FINAL, 
            ALBEDO,
            LIGHTS,
            SPECULAR,
            SELFILL,
            FXSELFILL,
            DEPTH,
            NORMALS,
            AMBIENT,
            POSITION,
            SHADOW,
            FXSHADOW,
            CUBESHADOW,
            FXCUBESHADOW,
            PAINT,
            PAINT_AMOUNT,
            DATA,
            channel_e_MAX
        } selectedChannel = INIT_SELECTEDCHANNEL;
        unsigned shadowToRender = 0;
        unsigned cubeShadowToRender = 0;
#endif
		enum camera_switch{

			CAMERA_PLAYER,
			CAMERA1,
			CAMERA2,
			CAMERA3
		}selected_camera = CAMERA_PLAYER;

		bool didspawn = false;
		void initFSM();
		void update();
		inline int getGameState(){ return fsm.getState(); }
		bool changelvl = false;
		int gamelvl = 0;
		bool isPlayerDead = false;
		bool winGame = false;
		int globalPoints = 0;
		int globalHealth = 150;
		int globalEnergy = 100;
		inline void setGlobalPoints(int p){ globalPoints = p; }
		inline int getGlobalPoints(){ return globalPoints; }
		inline void setGlobalHealth(int p){ globalHealth = p; }
		inline int getGlobalHealth(){ return globalHealth; }
		inline void setGlobalEnergy(int p){ globalEnergy = p; }
		inline int getGlobalEnergy(){ return globalEnergy; }
		inline void resetTotalStats(){ globalPoints = 0; globalHealth = 150; globalEnergy = 100; }
		
		void changeLevel(){ fsm.changeState(AppFSMExecutor::states::STATE_changelvl); }
		void loadlvl();
		void spawn();

		bool loadingthreadVar = true;
		utils::Counter<float> timerThreadAnim, timerGameOver;

		int xboxPadSensiblity;
		bool gameOverImg = false;
		bool paused = false;
		int pauseState = 0;
		bool returnToMenu = false;
		int videoEndsTo = 0;		// 0 -> Menu, 1 -> Game
		int mainMenuState = 0;
		int updateMainMenu();
		void renderMainMenu();
		int chapterSelectionState = 0;
		int updateChapterSelectionMenu();
		void renderChapterSelectionMenu();
		void renderPaused();
		void loadVideo(const char* name, const char* audio);
		bool updateVideo();
		bool renderVideo();
		
		void getImgValues(float& posX, float& posY, float& imgW, float& imgH, float originX, float originY, float originW, float originH);
	    void loadConfig();
	    bool create();
	    bool doFrame(); /* returns false on exit */
		bool doGameOver();
	    void destroy();
        float countTime();
		
		void playSong();
		void stopSong();
		void pauseSong();
		void resumeSong();

		inline const utils::Pad& getPad() const { return pad; }
		inline utils::Pad& getPad() { return pad; }

		inline bool isXboxControllerConnected() { return xboxController.is_connected(); }
		inline const utils::Pad& getXboxPad() const { return xboxPad; }
		inline utils::Pad& getXboxPad() { return xboxPad; }
		

		inline component::Handle getBichito() const { return bichitoEntity_h; }
		inline component::Handle getPlayer() const { return playerEntity_h; }
        inline component::Handle getCamera() const {return camera_h;}

		inline int getConfigX() const { return config.xres; }
		inline int getConfigY() const { return config.yres; }
        
		inline void setCamera(component::Handle camera) {
            assert(camera.isValid());
            camera_h = camera;
        }


        inline void setFocus(bool b) {focus=b;}
        inline bool hasFocus() const {return focus;}

#ifdef _DEBUG
        //for anttw purposes
        inline render::DeferredRender* getDeferredRender() {return &deferred;}
#endif

		void mistEffect(bool enable);

        inline void setExit(bool b=true) {exit=b;}
};

#endif
