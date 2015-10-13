#include "mcv_platform.h"
#include "shadow.h"
#include "app.h"

#include <sstream>
#include <iomanip>

#include "../renderManager.h"
#include "../render_utils.h"

#include "dirLight.h"

#include "handles/entity.h"
using namespace component;

using namespace utils;
using namespace DirectX;

namespace render {

void CShadow::createShadowMap(std::string name)
{
    bool is_ok = shadowMap.create(
        (name+".ShadowMap").c_str(), resolution, resolution,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32_TYPELESS,  RenderedTexture::USE_OWN_ZBUFFER, false);
    assert(is_ok);
}

void CShadow::createShadowMap()
{
    component::Entity* e = component::Handle(this).getOwner();
    assert(e != nullptr);
    createShadowMap(e->getName());
}

void CShadow::init()
{
    valid = true;
    component::Entity* e = component::Handle(this).getOwner();
    assert(e != nullptr);
    auto name = e->getName();

    auto& app = App::get();
    auto resX = app.getConfigX();
    auto resY = app.getConfigY();

    createShadowMap(name);
    bool is_ok = shadowBuffer.create(
        (name+".ShadowBuffer").c_str(), resX, resY,
        DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_UNKNOWN, RenderedTexture::USE_OWN_ZBUFFER, false);

    fxPipeline = new PostProcessPipeline;
    fxPipeline->setRes(resX, resY);
    is_ok &= fxPipeline->load("shadowBuffer", name);

    assert(is_ok);
}

void CShadow::processShadowBuffer(const Texture* space)
{
    if (!(enabled && isSpatiallyGood())) {return;}
    fxPipeline->setResource("space", space);
    (*fxPipeline)(&shadowBuffer);
}

void CShadow::generateShadowBuffer(const Texture* space)
{
    static const Technique*const tech = Technique::getManager().getByName("deferred_dir_shadowBuffer");

    if (!(enabled && isSpatiallyGood())) {return;}
    
    #ifdef _DEBUG
        std::stringstream ss;
        ss << "SPOTSB:" << ((Entity*)Handle(this).getOwner())->getName();
        TraceScoped s(ss.str().c_str());
    #else
        TraceScoped s("SPOTSB");
    #endif

    shadowBuffer.clearRenderTargetView(utils::BLACK);
    shadowBuffer.activate();

    space->activate(2);
    getShadowMap()->activate(6);
    
    Entity* me(Handle(this).getOwner());
    CCamera* light = me->get<CCamera>();
    CDirLight* dlight = me->get<CDirLight>();
    assert(light != nullptr && dlight != nullptr);

    XMMATRIX invViewProj = XMMatrixInverse(nullptr, light->getViewProjection());
    setObjectConstants(invViewProj);

    activateLight(*light);
    activateDirLight(dlight, light, this);
    
    tech->activate();
    mesh_view_volume.activateAndRender();
}

void CShadow::generate()
{
    if (!(enabled && isSpatiallyGood())) {return;}
#ifdef _DEBUG
    std::stringstream ss;
    ss << "SPOTSM:" << ((Entity*)Handle(this).getOwner())->getName();
    TraceScoped s(ss.str().c_str());
#else
    TraceScoped s("SPOTSM");
#endif

    // Start rendering in the rt of the depth buffer
    shadowMap.clearDepthBuffer();
    shadowMap.activate();

    RenderManager::renderShadows(Handle(this).getOwner());
}

}