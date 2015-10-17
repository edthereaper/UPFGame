#include "mcv_platform.h"
#include "antTW.h"

#include "render/render_utils.h"

#include "gameElements/player/playerStats.h"
using namespace gameElements;

#include "app.h"

#ifdef _DEBUG

using namespace render;

namespace antTw_user {

unsigned AntTWManager::xRes;
unsigned AntTWManager::yRes;

void TW_CALL AntTWManager::copyStr(std::string& destinationClientString, const std::string& sourceLibraryString)
{
    // Copy the content of souceString handled by the AntTweakBar library
    // to destinationClientString handled by your application
    destinationClientString = sourceLibraryString;
}

void AntTWManager::tearDown()
{
    TwCopyStdStringToClientFunc(NULL);
#if defined(_LIGHTTOOL)
    AntTWManager::cleanupLightTool();
#endif

    TwDeleteAllBars();
	TwTerminate();
}

void AntTWManager::init(unsigned xres, unsigned yres)
{
    xRes = xres;
    yRes = yres;
	TwInit(TW_DIRECT3D11, render::Render::getDevice());
    TwCopyStdStringToClientFunc(AntTWManager::copyStr);
    TwDefine(" GLOBAL contained=true iconpos=br ");
    TwDefine(" TW_HELP visible=false ");
	TwWindowSize(xRes, yRes);
}

std::vector<char*> pppNameHolder;

TwType gbufferFXSelectEnum()
{
    for (auto& s : pppNameHolder) {delete s;}
    pppNameHolder.clear();

    std::vector<TwEnumVal> v;
    TwEnumVal enumVal;
    enumVal.Label = "Select";
    enumVal.Value = 0;
    v.push_back(enumVal);
    int i = 1;
    PostProcessPipeline::getManager().forall<void>(
        [&v, &i](const PostProcessPipeline& p){
            TwEnumVal enumVal;
            std::string pppName = p.getFullName();
            char* name = new char [pppName.size()+1];
            std::strcpy(name, pppName.c_str());
            enumVal.Label = name;
            enumVal.Value = i++;
            v.push_back(enumVal);
        });
    return TwDefineEnum("FXPipeline", v.data(), unsigned(v.size()));
}

TwType gbufferChannelSelectEnum()
{
    
    TwEnumVal enumVals[] = {
       {App::FINAL, "Out"},
       {App::FX_FINAL, "FX Out"},
       {App::ALBEDO, "Albedo"},
       {App::LIGHTS, "Lights"},
       {App::SPECULAR, "Specular"},
       {App::SELFILL, "Self-Illumination"},
       {App::FXSELFILL, "FX Self-Illumination"},
       {App::AMBIENT, "Ambient"},
       {App::DEPTH, "Depth"},
       {App::DATA, "Data"},
       {App::PAINT, "Paint"},
       {App::PAINT_AMOUNT, "PAINT_AMOUNT"},
       {App::NORMALS, "Normals"},
       {App::POSITION, "Positions"},
       {App::SHADOW, "ShadowBuffer"},
       {App::FXSHADOW, "FX ShadowBuffer"},
       {App::CUBESHADOW, "Cube ShadowBuffer"},
       {App::FXCUBESHADOW, "FX Cube ShadowBuffer"},
    };
    return TwDefineEnum("GBufferChannel", enumVals, ARRAYSIZE(enumVals));
}

inline void TW_CALL openPipelineEdit(const void* value, void* clientData)
{
    PostProcessPipeline::getManager()[*static_cast<const int*>(value)-1]
        ->second.item->antTW_createTweak();
}
inline void TW_CALL returnZero(void* value, void*) {*static_cast<int*>(value)=0;}


static bool updateRenderVars = true;
static void TW_CALL setUpdateRender(const void* value, void*)
{
    updateRenderVars = *static_cast<const bool*>(value);
}
static void TW_CALL updateRender(void* value, void*)
{
    *static_cast<bool*>(value)=updateRenderVars;
    if (updateRenderVars) {RenderConstsMirror::dirty = true;}
}

static void TW_CALL killPlayer(void*)
{
    auto& app = App::get();
    CPlayerStats* stats = app.getPlayer().getSon<CPlayerStats>();
    if (stats != nullptr) {
        auto prevGodMode = app.godMode;
        app.godMode = false;
        stats->damage(666+666+666+666+666+666, true);
        app.godMode = prevGodMode;
    }
}

void AntTWManager::createRenderTweak()
{
    auto bar = TwNewBar("RENDER");
    TwAddVarCB(bar, "Update", TW_TYPE_BOOLCPP, setUpdateRender, updateRender, nullptr, "");  

    TwAddVarRW(bar, "GlobalLightDirection", TW_TYPE_DIR3F, &RenderConstsMirror::GlobalLightDirection,
        "label=`Global light direction` group='RenderParams' axisx=-x axisz=-z");
    TwAddVarRW(bar, "GlobalLightStrength", TW_TYPE_FLOAT, &RenderConstsMirror::GlobalLightStrength,
        "label=`Global light strength` group='RenderParams' min=0 max=1 step=0.001");
    TwAddVarRW(bar, "SkyBoxBlend", TW_TYPE_FLOAT, &RenderConstsMirror::SkyBoxBlend,
        "label=`Skybox blend` group='RenderParams' min=0 max=1 step=0.001");
    TwAddVarRW(bar, "SkyBoxBright", TW_TYPE_FLOAT, &RenderConstsMirror::SkyBoxBright,
        "label=`Skybox brightness` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "AmbientMax", TW_TYPE_FLOAT, &RenderConstsMirror::AmbientMax,
        "label=`Ambient max` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "AmbientMin", TW_TYPE_FLOAT, &RenderConstsMirror::AmbientMin,
        "label=`Ambient min` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "ResolveAlbedo", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveAlbedo,
        "label=`A` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "ResolveAlbedoLight", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveAlbedoLight,
        "label=`A*L` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "ResolveLight", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveLight,
        "label=`L` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "ResolveSpecular", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveSpecular,
        "label=`S` group='RenderParams' min=-3 max=3 step=0.001");
    TwAddVarRW(bar, "PowerSpecular", TW_TYPE_FLOAT, &RenderConstsMirror::PowerSpecular,
        "label=`SP` group='RenderParams' min=0 max=100 step=0.1");
    TwAddVarRW(bar, "ResolveSelfIll", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveSelfIll,
        "label=`Si` group='RenderParams' step=0.001");
    TwAddVarRW(bar, "Saturation", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveSaturation,
        "group='SBC' step=0.001");
    TwAddVarRW(bar, "Brightness", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveBrightness,
        "group='SBC' step=0.001");
    TwAddVarRW(bar, "Contrast", TW_TYPE_FLOAT, &RenderConstsMirror::ResolveContrast,
        "group='SBC' step=0.001");

    TwDefine("RENDER iconified=true color='255 255 0' label='Render' text=dark");
    TwDefine("RENDER/SBC opened=false label='Resolve parameters'");
}

static void TW_CALL readlevel(void *value, void *clientData)
{
	*static_cast<int *>(value) = App::get().getLvl();
}

static void TW_CALL updatelevel(const void *value, void *clientData)
{
	int level = *static_cast<const int *>(value);

	if (0 < level && level <= 5){
		App::get().setLvl(level);
	}

#ifdef _LIGHTTOOL
   AntTWManager:: deleteLightBars();
#endif
}

void AntTWManager::createDebugTweak()
{
    static const TwType enumChannel = gbufferChannelSelectEnum();
    static const TwType enumFX = gbufferFXSelectEnum();

    auto& app = App::get();
    auto bar = TwNewBar("DEBUG");

    static const TwEnumVal level_e[] = {
			{ 1, "Level 1" },
			{ 2, "Level 2" },
			{ 3, "Level 3" },
			{ 4, "Level 4" },
			{ 5, "Sandbox" },
			{ 0, "<choose level>" }, //Do not use! use this to try things
	};
	static const TwType levelType = 
        TwDefineEnum("levels", level_e, ARRAYSIZE(level_e));
	TwAddVarCB(bar, "Level:", levelType, updatelevel, readlevel, NULL, "");

    TwAddVarRW(bar, "Debug layer", TW_TYPE_BOOLCPP, &app.getDeferredRender()->debugLayer,
        "label=`Active` group='DebugLayer'");
    TwAddVarRW(bar, "Paint", TW_TYPE_BOOLCPP, &app.drawPaintVolume,
        "label=`Terrain transform` group='DebugLayer'");
    TwAddVarRW(bar, "aabb", TW_TYPE_BOOLCPP, &app.renderAABBs,
        "label=`Any AABB` group='DebugLayer'");
    TwAddVarRW(bar, "aabbM", TW_TYPE_BOOLCPP, &app.renderMeshAABBs,
        "label=`Non-instanced AABBs` group='DebugLayer'");
    TwAddVarRW(bar, "aabbI", TW_TYPE_BOOLCPP, &app.renderInstanceMeshAABBs,
        "label=`Instances AABBs` group='DebugLayer'");
    TwAddVarRW(bar, "aabbL", TW_TYPE_BOOLCPP, &app.renderLightAABBs,
        "label=`Lights AABBs` group='DebugLayer'");
    TwAddVarRW(bar, "fonts", TW_TYPE_BOOLCPP, &app.printFonts,
        "label=`Text` group='DebugLayer'");
    TwAddSeparator(bar, nullptr, "");
    
    TwAddVarRW(bar, "skybox", TW_TYPE_BOOLCPP, &app.drawSkyBox,
        "label=`Draw skybox` group='Render'");
    TwAddVarRW(bar, "gbufferchannels", TW_TYPE_BOOLCPP, &app.enableRenderGBufferChannels,
        "label=`GBuffer Channels` group='Render' ");
    TwAddVarRW(bar, "gbufferchannel", enumChannel, &app.selectedChannel,
        "label=`Single channel` group='Render' keyincr=CTRL+PGDOWN keydecr=CTRL+PGUP ");
    TwAddVarRW(bar, "channelDirShadow", TW_TYPE_UINT16, &app.shadowToRender,
        "label=`Select dir shadowbuffer` group='Render' keyincr=CTRL+8 keydecr=CTRL+5");
    TwAddVarRW(bar, "channelPtShadow", TW_TYPE_UINT16, &app.cubeShadowToRender,
        "label=`Select pt shadowbuffer` group='Render' keyincr=CTRL+7 keydecr=CTRL+4");
    TwAddVarRW(bar, "animOff", TW_TYPE_BOOLCPP, &app.animOff,
        "label=`Freeze animations` group='Render'");
    TwAddVarCB(bar, "pipelineEdit", enumFX, openPipelineEdit, returnZero,
        app.getDeferredRender(), "label=`Tweak FX` group='Render' ");
    TwAddSeparator(bar, nullptr, "");

    TwAddVarRW(bar, "culling", TW_TYPE_BOOLCPP, &app.instanceCulling,
        "label=`Instance culling` group='Performance'");
    TwAddVarRW(bar, "paint", TW_TYPE_BOOLCPP, &app.drawPaint,
        "label=`Paint scenery` group='Performance'");
    TwAddVarRW(bar, "shadows", TW_TYPE_BOOLCPP, &app.enableShadows,
        "label=`Shadows` group='Performance'");
    TwAddSeparator(bar, nullptr, "");
        
    TwAddVarRW(bar, "HighlightTransformables", TW_TYPE_BOOLCPP, &app.highlightTransformables,
        "label=`Highlight transformables` group='Others'");
    TwAddVarRW(bar, "transformClick", TW_TYPE_BOOLCPP, &app.clickToTransform,
        "label=`Transform click (tools only)` group='Others'");
    TwAddVarRW(bar, "godMode", TW_TYPE_BOOLCPP, &app.godMode,
        "label=`God Mode` group='Others' key=CTRL+g");
    TwAddVarRW(bar, "infinteEnergy", TW_TYPE_BOOLCPP, &app.infiniteEnergy,
        "label=`Infinite energy` group='Others'");
    TwAddButton(bar, "Kill player", killPlayer, nullptr, " group='Others' key=CTRL+k ");

    TwDefine("DEBUG/DebugLayer opened=false");
    TwDefine("DEBUG iconified=true color='255 255 255' label='Debug' text=dark");
}

}

#endif