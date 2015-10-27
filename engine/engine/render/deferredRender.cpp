#include "mcv_platform.h"       
#include "deferredRender.h"    
#include "renderManager.h" 
#include "mesh/CInstancedMesh.h"
#include "illumination/ptLight.h"
#include "illumination/dirLight.h"   
#include "illumination/shadow.h"
#include "illumination/cubeShadow.h" 
#include "environment/volLight.h"   
#include "environment/mist.h" 
#include "Particles/ParticleSystem.h" 
#include "app.h"

#include "gameElements/smoketower.h"
#include "gameElements/PaintManager.h"
using namespace gameElements;

#ifdef _DEBUG
#include "Particles/Emitter.h"
using namespace particles;
#endif

#ifdef LOOKAT_TOOL
#include "antTweakBar_USER/lookatTools.h"
#endif

using namespace particles;

namespace render {
    
bool DeferredRender::create(int xres, int yres)
{
    xRes = xres;
    yRes = yres;

    //These are owned by Texture::manager
    rt_lights = new RenderedTexture;
    rt_albedo = new RenderedTexture;
    rt_normals = new RenderedTexture;
    rt_space = new RenderedTexture;
    rt_selfIllumination = new RenderedTexture;
    rt_paintGlow = new RenderedTexture;
    rt_data1 = new RenderedTexture;
    rt_out = new RenderedTexture;
    rt_data2 = new RenderedTexture;
    rt_normals_transform = new RenderedTexture;

    bool is_ok = true;
    is_ok &= rt_lights->create((prefix+"lights").c_str(),
        xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, RenderedTexture::USE_BACK_ZBUFFER);
    is_ok &= rt_albedo->create((prefix+"albedo").c_str(),
        xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_normals->create((prefix+"normals").c_str(),
        xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_normals_transform->create((prefix+"tangents").c_str(),
        xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_space->create((prefix+"space").c_str(),
        xres, yres, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_selfIllumination->create((prefix+"selfIllumination").c_str(),
        xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_paintGlow->create((prefix+"paintGlow").c_str(),
        xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_data1->create((prefix+"data").c_str(),
        xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, RenderedTexture::USE_BACK_ZBUFFER);
    is_ok &= rt_data2->create((prefix+"paint").c_str(),
        xres, yres, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_UNKNOWN);
    is_ok &= rt_out->create((prefix+"out").c_str(),
        xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, RenderedTexture::USE_BACK_ZBUFFER);

    generateAmbientPPP.setRes(xres, yres);
    postProcessSelfIll.setRes(xres, yres);
    postProcessOut.setRes(xres, yres);

    generateAmbientPPP.load("ambient");
    postProcessSelfIll.load("selfIll");
    postProcessOut.load("main");

    return is_ok;
}

void DeferredRender::clearGBuffer()
{
    static float clearData[4] = {0,10000,0,0};

#ifdef _PARTICLES
    rt_albedo->clearRenderTargetView(utils::DARK_BLUE);
#else
    rt_albedo->clearRenderTargetView(utils::BLACK_A);
#endif
    rt_normals->clearRenderTargetView(utils::BLACK_A);
    rt_normals_transform->clearRenderTargetView(utils::BLACK_A);
    rt_lights->clearRenderTargetView(utils::BLACK_A);
    rt_space->clearRenderTargetView(utils::BLACK_A);
    rt_selfIllumination->clearRenderTargetView(utils::BLACK_A);
    rt_data1->clearRenderTargetView(clearData);
    rt_data2->clearRenderTargetView(utils::BLACK_A);
    rt_paintGlow->clearRenderTargetView(utils::BLACK_A);
}

void DeferredRender::initGBuffer(uint32_t mask) const
{
    ID3D11RenderTargetView* rtvs[] = {
        (mask & ALBEDO) ? rt_albedo->getRenderTargetView() : nullptr,
        (mask & NORMALS) ? rt_normals->getRenderTargetView() : nullptr,
        (mask & LIGHT) ? rt_lights->getRenderTargetView() : nullptr,
        (mask & SPACE) ? rt_space->getRenderTargetView() : nullptr,
        (mask & SELFILLUMINATION) ? rt_selfIllumination->getRenderTargetView() : nullptr,
        (mask & DATA1) ? rt_data1->getRenderTargetView() : nullptr,
        (mask & DATA2) ? rt_data2->getRenderTargetView() : nullptr,
        (mask & NORMALS_PAINTED) ? rt_normals_transform->getRenderTargetView() : nullptr,
    };
    Render::getContext()->OMSetRenderTargets(
        ARRAYSIZE(rtvs), rtvs, Render::getDepthStencilView());    

    rt_albedo->activateViewport();
}

void DeferredRender::generateShadowBuffers()
{
    TraceScoped scope("shadow_buffers");
    
    activateRSConfig(RSCFG_REVERSE_CULLING);
    activateZConfig(ZCFG_TEST_GT);
    activateBlendConfig(BLEND_CFG_ADDITIVE);
    activateDirLightCB();
    component::getManager<CShadow>()->forall<void>(&CShadow::generateShadowBuffer, rt_space);
    activatePtLightCB();
    component::getManager<CCubeShadow>()->forall<void>(&CCubeShadow::generateShadowBuffer, rt_space);
    Texture::deactivate(5,2);
    
    activateRSConfig(RSCFG_DEFAULT);
    activateZConfig(ZCFG_DEFAULT);
    activateBlendConfig(BLEND_CFG_DEFAULT);
    component::getManager<CShadow>()->forall<void>(&CShadow::processShadowBuffer, rt_space);
    component::getManager<CCubeShadow>()->forall<void>(&CCubeShadow::processShadowBuffer, rt_space);
    Texture::deactivate(2);
}

void DeferredRender::generateLightBuffer()
{
    TraceScoped scope("light_buffer");

    activateRSConfig(RSCFG_REVERSE_CULLING);
    activateZConfig(ZCFG_TEST_GE);
    activateBlendConfig(BLEND_CFG_ADDITIVE);

    rt_lights->activate();
    rt_normals->Texture::activate(1);
    rt_space->Texture::activate(2);
    
    {
        TraceScoped scope("point_lights");
        activatePtLightCB();
        component::getManager<CPtLight>()->forall(&CPtLight::draw);
    }
    {
        TraceScoped scope("dir_lights");
        activateDirLightCB();
        component::getManager<CDirLight>()->forall(&CDirLight::draw);
    }
    Texture::deactivate( 0, 6 );
}
void DeferredRender::generateAmbient()
{
    activateRSConfig(RSCFG_DEFAULT);
    activateZConfig(zConfig_e::ZCFG_TEST_ALL);
    activateBlendConfig(BlendConfig::BLEND_CFG_DEFAULT);

    generateAmbientPPP.setResource("normals", rt_normals);
    generateAmbientPPP.setResource("space", rt_space);
    generateAmbientPPP();
}

void DeferredRender::postProcessGBuffer()
{
    activateRSConfig(RSCFG_DEFAULT);
    activateZConfig(zConfig_e::ZCFG_TEST_ALL);
    activateBlendConfig(BlendConfig::BLEND_CFG_DEFAULT);
    
    postProcessSelfIll.setResource("normals", rt_normals);
    postProcessSelfIll.setResource("space", rt_space);
    postProcessSelfIll(rt_selfIllumination);
}

void DeferredRender::resolve()
{
    TraceScoped scope("resolve");
    static const Technique* tech = Technique::getManager().getByName("deferred_resolve");
    pixelRect screen(rt_out->getXRes(), rt_out->getYRes());
    
    activateZConfig(zConfig_e::ZCFG_TEST_ALL);
    
    rt_out->activate();
    rt_normals->Texture::activate(1);
    rt_space->Texture::activate(2);
    rt_lights->Texture::activate(3);
    getFxSelfIllumination()->activate(4);
    getAmbient()->activate(5);
    rt_data1->Texture::activate(6);
    rt_data2->Texture::activate(7);
    rt_paintGlow->Texture::activate(8);
    rt_normals_transform->Texture::activate(9);
    drawTexture2D(screen, screen, rt_albedo, tech);
    Texture::deactivate(0, 6);
}

void DeferredRender::drawVolumetricLights()
{
    TraceScoped scope("vol_lights");

    activateRSConfig(RSCFG_REVERSE_CULLING);
    activateBlendConfig(BLEND_CFG_ADDITIVE);
    activateZConfig(ZCFG_TEST_GT);
    
    rt_out->activate();
    rt_normals->Texture::activate(0);
    rt_space->Texture::activate(1);
    activateVolPtLightCB();
    component::getManager<CVolPtLight>()->forall(&CVolPtLight::draw);
}

void DeferredRender::drawMists()
{
    TraceScoped scope("mists");

    initGBuffer(ALBEDO|NORMALS|LIGHT|SELFILLUMINATION|DATA1);
    rt_space->Texture::activate(2);

    activateRSConfig(RSCFG_DISABLE_CULLING);
    activateBlendConfig(BLEND_MIST);
    activateZConfig(ZCFG_TEST_LT);

    activateMistCB();
    component::getManager<CMist>()->forall(&CMist::draw);
}

void DeferredRender::postProcessOutput()
{
    activateRSConfig(RSCFG_DEFAULT);
    activateZConfig(zConfig_e::ZCFG_TEST_ALL);
    activateBlendConfig(BlendConfig::BLEND_CFG_DEFAULT);
    postProcessOut.setResource("space", rt_space);
    postProcessOut.setResource("normals", rt_normals);
    postProcessOut.setResource("data", rt_data1);
    postProcessOut(rt_out);
}

void DeferredRender::renderParticles()
{
    TraceScoped _("particles");
    rt_normals->Texture::activate(1);
    rt_space->Texture::activate(2);
    activateRSConfig(RSCFG_DEFAULT);
    activateZConfig(ZCFG_TEST_LT);
    getManager<CParticleSystem>()->forall(&CParticleSystem::render);
}

void DeferredRender::drawPaint()
{
#if defined(_DEBUG)
    if (!App::get().drawPaint) {return;}
#endif
    TraceScoped _("paint");

    ID3D11RenderTargetView* rtvs[] = {
        rt_paintGlow->getRenderTargetView(),
        rt_data2->getRenderTargetView(),
        rt_normals->getRenderTargetView(),
    };
    Render::getContext()->OMSetRenderTargets(
        ARRAYSIZE(rtvs), rtvs, Render::getDepthStencilView());    
    rt_paintGlow->activateViewport();
    
    rt_normals_transform->Texture::activate(1);
    rt_space->Texture::activate(2);
    rt_data1->Texture::activate(3);
    
    static const Technique* tech = Technique::getManager().getByName("paint");
    activatePaintCB();
    tech->activate();
    activateRSConfig(RSCFG_REVERSE_CULLING);
    activateZConfig(ZCFG_TEST_GT);
    activateBlendConfig(BLEND_CFG_PAINT);

    PaintManager::drawAll();
}

void DeferredRender::screenEffects()
{
    if(CSmokeTower::isFXActive()) {
        pixelRect screen(rt_out->getXRes(), rt_out->getYRes());
        auto tex = Material::getManager().getByName("lava_torre")->getDiffuse();
        static const auto tech = Technique::getManager().getByName("multiply");
        drawTexture2D(screen, screen, tex, tech, true, 0xFFFFFFA0);
    }
}

void DeferredRender::operator()(component::Handle camera_h)
{
//In debug, if we're rendering only some channels,
//exit the pipeline early and improve performance

#ifdef _DEBUG
    auto& app = App::get();
    const auto& b = !app.enableRenderGBufferChannels;
    const auto& c = app.selectedChannel;
#endif
    TraceScoped _("deferred render");
    renderGBuffer(camera_h);
#ifdef _DEBUG
    if (b && (c == App::POSITION || c == App::DEPTH)) {return;}
#endif
    drawMists();
#ifdef _DEBUG
    if (b && (c == App::ALBEDO || c == App::SELFILL || c == App::DATA)) {return;}
    if (b && (c == App::ALBEDO_PLUS_PARTICLES)) {
        initGBuffer(ALBEDO);
        renderParticles();
        if (debugLayer) {
            renderDebug();
        }
        return;
    }
#endif
    drawPaint();
#ifdef _DEBUG
    if (b && (c == App::PAINT || c == App::PAINT_AMOUNT ||
        c == App::NORMALS ||c == App::UVPAINT)) {return;}
#endif
    {
        TraceScoped _("illumination");
        generateShadowBuffers();
#ifdef _DEBUG
        if (b && (c == App::SHADOW || c == App::FXSHADOW ||
            c == App::CUBESHADOW || c == App::FXCUBESHADOW)) {
            return;
        }
#endif
        generateLightBuffer();  
#ifdef _DEBUG
        if (b && (c == App::LIGHTS || c == App::SPECULAR)) {return;}
#endif 
    }
    generateAmbient();
#ifdef _DEBUG
    if (b && (c == App::AMBIENT)) {return;}
#endif
    postProcessGBuffer();  
#ifdef _DEBUG
    if (b && (c == App::FXSELFILL)) {return;}
#endif 
    resolve();
    screenEffects();
    drawVolumetricLights();
    renderParticles();
#ifdef _DEBUG
    if (b && (c == App::FINAL)) {return;}
#endif
    postProcessOutput();
#if defined(_DEBUG)
    if (debugLayer) {
        renderDebug();
    }
    
#endif
}

void DeferredRender::renderGBuffer(component::Handle camera_h)
{
    TraceScoped _("gbuffer");
	activateZConfig(zConfig_e::ZCFG_DEFAULT);
    RenderManager::renderAll(camera_h, *this);
}

void DeferredRender::destroy()
{
    rt_lights->destroy();
    rt_albedo->destroy();
    rt_normals->destroy();
    rt_space->destroy();
    rt_selfIllumination->destroy();
    rt_data1->destroy();
    rt_out->destroy();
    postProcessSelfIll.retire();
    postProcessOut.retire();
    generateAmbientPPP.retire();
}

#if defined(_DEBUG ) || defined(_OBJECTTOOL)
void DeferredRender::renderDebug() const
{
    static const Technique* tech = Technique::getManager().getByName("basicTint");
    TraceScoped _("Debug & tools");
    activateZConfig(ZCFG_TEST_LT);
    
    auto& app = App::get();

    tech->activate();
    #if defined(_LIGHTTOOL)
	    if (!app.debugRenderer)
    #endif
    #if (defined(_DEBUG) && defined(DEBUG_LIGHTS)) || defined(_LIGHTTOOL)
        {
            TraceScoped trace("LIGHT-DEBUG");
            getManager<CDirLight>()->forall(&CDirLight::drawVolume);
            getManager<CPtLight>()->forall(&CPtLight::drawVolume);
            getManager<CVolPtLight>()->forall(&CVolPtLight::drawVolume);
			getManager<CMist>()->forall(&CMist::drawMarker);
			getManager<CEmitter>()->forall(&CEmitter::drawMarker);
        }
    #endif
    
    #if defined(_DEBUG) && !defined(_PARTICLES)
        if (app.renderAABBs) {
            TraceScoped trace("AABB-DEBUG");
            if(app.renderMeshAABBs) getManager<CCullingAABB>()->forall(&CCullingAABB::draw);
            if(app.renderInstanceMeshAABBs) getManager<CInstancedMesh>()->forall(&CInstancedMesh::drawAABBs);
            if(app.renderLightAABBs) getManager<CCullingAABBSpecial>()->forall(&CCullingAABBSpecial::draw);
        }
    #endif
    #if defined(LOOKAT_TOOL)
        {
            TraceScoped _("LOOKAT-DEBUG");
            for (const auto& tw : antTw_user::lookAtTw) {if (tw!=nullptr) {tw->draw();}}
            for (const auto& tw : antTw_user::armPointTw) {if (tw!=nullptr) {tw->draw();}}
        }
    #endif
    if (app.drawPaintVolume) {
        TraceScoped _("PAINT-DEBUG");
        auto paints = getManager<CPaintGroup>();
        paints->forall(&CPaintGroup::drawAABB);
        static const Technique* paintTech = Technique::getManager().getByName("paintBasic");
        activatePaintCB();
        activateBlendConfig(BLEND_CFG_COMBINATIVE);
        paintTech->activate();
        paints->forall(&CPaintGroup::drawVolume);
    }
}
#endif

}
