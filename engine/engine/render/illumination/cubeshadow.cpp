#include "mcv_platform.h"
#include "cubeshadow.h"
#include "app.h"

#include <sstream>
#include <iomanip>

#include "../renderManager.h"
#include "../render_utils.h"

#include "ptLight.h"

#include "handles/entity.h"
using namespace component;

using namespace utils;
using namespace DirectX;

namespace render {

//(currently useless as all cameras associated to cubeshadows are used exclusively by it)
//#define CCUBESHADOW_RESTORE_CAMERA

void CCubeShadow::createShadowMap(std::string name)
{
    bool is_ok = shadowCubeMap.create(
        (name+".ShadowMap").c_str(), resolution, resolution,
        DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_TYPELESS, RenderedTextureCube::USE_ONE_ZBUFFER, false);
    assert(is_ok);
}

void CCubeShadow::createShadowMap()
{
    component::Entity* e = component::Handle(this).getOwner();
    assert(e != nullptr);
    createShadowMap(e->getName());
}

void CCubeShadow::init()
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

void CCubeShadow::processShadowBuffer(const Texture* space)
{
    if (!(enabled && isSpatiallyGood())) {return;}
    fxPipeline->setResource("space", space);
    (*fxPipeline)(&shadowBuffer);
}

void CCubeShadow::generateShadowBuffer(const Texture* space)
{
    static const Technique*const tech = Technique::getManager().getByName("deferred_point_shadowBuffer");
    if (!(enabled && isSpatiallyGood())) {return;}

    #ifdef _DEBUG
        std::stringstream ss;
        ss << "OMNISB:" << ((Entity*)Handle(this).getOwner())->getName();
        TraceScoped s(ss.str().c_str());
    #else
        TraceScoped s("OMNISB");
    #endif

    shadowBuffer.clearRenderTargetView(utils::BLACK);
    shadowBuffer.activate();

    space->activate(2);
    getShadowMap()->activate(6);

    Entity* me = Handle(this).getOwner();
    CTransform* t = me->get<CTransform>();
    CPtLight* light = me->get<CPtLight>();
    CCamera* camera = me->get<CCamera>();
    assert(t != nullptr && light != nullptr && camera != nullptr);

    float radius = light->getRadius();
    XMMATRIX scale = XMMatrixScaling(radius, radius, radius);
    XMVECTOR pos = t->getPosition() + light->getOffset();
    XMMATRIX trans = XMMatrixTranslationFromVector(pos);
    setObjectConstants(scale * trans);

    activateLight(*camera);
    activatePtLight(light, pos, this);

    tech->activate();
    mesh_icosahedron.activateAndRender();
}

void CCubeShadow::update(float)
{
    CCamera* light = Handle(this).getBrother<CCamera>();
    assert(light || !"CCubeShadow requires a CCamera");
    auto pos(light->getPosition());
    
#ifdef CCUBESHADOW_RESTORE_CAMERA
    //Save previous camera position
    auto front(light->getFront());
#endif
    if(pos != prevPos) {
        for (unsigned i=0; i<6; ++i) {
            light->lookAt(pos, pos+TextureCube::faceDirFix[i], TextureCube::faceUp[i]);
            cachedCam[i].view = light->getView();
            cachedCam[i].viewProjection = light->getViewProjection();
            cachedCam[i].front = light->getFront();
            cachedCam[i].right = light->getRight();
            cachedCam[i].up = light->getUp();
        }
    }
#ifdef CCUBESHADOW_RESTORE_CAMERA
    //Put back camera
    light->lookAt(pos, pos+front, yAxis_v);
#endif
}

void CCubeShadow::generate()
{
    if (!(enabled && isSpatiallyGood())) {return;}
    static Technique*const normalTech =
        Technique::getManager().getByName("gen_cubeshadows");
    static Technique*const skinnedTech =
        Technique::getManager().getByName("gen_cubeshadows_skinned");
    static Technique*const instancedTech =
        Technique::getManager().getByName("gen_cubeshadows_instanced");

#ifdef _DEBUG
    std::stringstream ss;
    ss << "OMNISM:" << ((Entity*)Handle(this).getOwner())->getName();
    TraceScoped s(ss.str().c_str());
#else
    TraceScoped s("OMNISM");
#endif

    shadowCubeMap.clearRenderTargetViews(utils::BLACK);
    shadowCubeMap.clearDepthBuffers();

    Handle e_h = Handle(this).getOwner();
    Entity* e(e_h);
#ifdef CCUBESHADOW_RESTORE_CAMERA
    //Save previous camera position
    CCamera* light = e->get<CCamera>();
    auto front(light->getFront());
    auto pos(light->getPosition());
#endif
    uint8_t mask = cullingMask & enableMask;
    for (unsigned i=0; i<6; ++i) {
        if ((mask&(1<<i)) != 0) {
            #ifdef _DEBUG
                std::stringstream ss;
                ss << e->getName() << ".face:"<< i;
                TraceScoped scope(ss.str().c_str());
            #endif
            shadowCubeMap.activateFace(i);

            RenderManager::renderShadows(e_h, Culling::cullDirection_e(i),
                normalTech, skinnedTech, instancedTech);
        }
    }
#ifdef CCUBESHADOW_RESTORE_CAMERA
    //Put back camera
    light->lookAt(pos, pos+front, yAxis_v);
#endif
}

}