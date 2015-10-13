#ifndef ANT_TW_USER_ANTTW_H_
#define ANT_TW_USER_ANTTW_H_

#if defined(_DEBUG) || defined(_OBJECTTOOL) || defined(_CINEMATIC_)

#include <AntTweakBar.h>

#include "handles/entity.h"
#include "components/color.h"

#ifdef LOOKAT_TOOL
#include "animation/cArmPoint.h"
#include "animation/cbonelookat.h"
#include "animation/cskeleton.h"
#endif

#ifdef _LIGHTTOOL 
#include "render/illumination/dirLight.h"
#include "render/illumination/ptLight.h"
#include "render/environment/volLight.h"
#include "render/environment/mist.h"
#include "Particles/ParticlesManager.h"
#endif

namespace antTw_user {

#define ANTTW_ENUMVAL(x) {x, #x}
#define ANTTW_ENUMVAL_PREFIX(prefix, x) {prefix##x, #x}

class AntTWManager {
	private:
		static unsigned xRes, yRes;
        static void TW_CALL copyStr(std::string& dest, const std::string& src);
	public:
		static void init(unsigned xres, unsigned yres);
        static void tearDown();
		static void createObjectViewerTweak(component::Handle tramp);
		static void createDebugTweak();
        static void createRenderTweak();


#ifdef _LIGHTTOOL
		static void createLightList();
		static void updateLightList();
		static void createLightEditorTweak(component::Entity* mainCam);
		static void selectPointLightTweak(render::CPtLight* light);
		static void selectDirectionalLightTweak(render::CDirLight* light);
        static void selectVolLightTweak(render::CVolPtLight*);
        static void selectMistTweak(render::CMist*);
		static void selectEmitterTweak(particles::CEmitter* emitter);
        static void cleanupLightTool();
        static void deleteLightBars();
#endif

#ifdef _PARTICLES
		//particles
		
		static void createBarManagerTweak();
		static bool validNameTexture(std::string name);
		static void createParticleEditorTweak();
		static void createEmitterEditorTweak(int idx);
		static void createTextureManagerTweak();
		static void GetFilesInDirectory(std::vector<std::string> &out, const std::string &directory);
		static void loadPreviousParticleSystem();
#endif

#ifdef _CINEMATIC_
		static void createCameraTweak();
#endif

};

inline static void TW_CALL closeTwBar(void* clientData){
    TwBar** bar = static_cast<TwBar**>(clientData);
    TwDeleteBar(*bar);
    *bar = nullptr;
}

}

#endif

#endif