#include "mcv_platform.h"
#include "culling.h"

#include "component.h"
#include "handles/entity.h"
#include "render/render_utils.h"
#include "render/illumination/ptLight.h"  
#include "render/illumination/dirLight.h"
#include "render/environment/volLight.h"
#include "render/illumination/cubeshadow.h"

using namespace component;
using namespace DirectX;
using namespace utils;

//#define SPECIAL_AABB_CHECKS

namespace render {

void CullingAABB::update(const AABB& aabb, const XMMATRIX& world, bool ignoreWorldChange)
{
    if(dirty || (!ignoreWorldChange && prevWorld != world)){
        dirty = false;
        prevWorld = world;
        operator=(bakeCulling(aabb, world));
        if (scale != 1){scaleAABB(scale);}
        if (skin != 0) {skinAABB(skin);}
    }
}

void culling_t::draw(const AABB& aabb, const Color& color)
{
    setObjectConstants(
        XMMatrixAffineTransformation(2*aabb.getHSize(),
        zero_v, one_q, aabb.getCenter()-aabb.getHSize()), color);
    mesh_cube_wire.activateAndRender();
}

AABB culling_t::bakeCulling(const AABB& aabb, const XMMATRIX& world)
{
    AABB ret;
    XMFLOAT3 baseHSize;
    XMStoreFloat3(&baseHSize, aabb.getHSize());
    
#if defined(_DEBUG) && defined(SPECIAL_AABB_CHECKS)
    if (aabb.isInvalid()) {
        dbg("Invalid AABB.\n");
    }
#endif
    XMFLOAT3 wx, wy, wz;
    XMStoreFloat3(&wx, world.r[0]);
    XMStoreFloat3(&wy, world.r[1]);
    XMStoreFloat3(&wz, world.r[2]);

    XMFLOAT3 newHSize;
    newHSize.x = baseHSize.x * fabsf(wx.x) + baseHSize.y * fabsf(wy.x) + baseHSize.z * fabsf(wz.x);
    newHSize.y = baseHSize.x * fabsf(wx.y) + baseHSize.y * fabsf(wy.y) + baseHSize.z * fabsf(wz.y);
    newHSize.z = baseHSize.x * fabsf(wx.z) + baseHSize.y * fabsf(wy.z) + baseHSize.z * fabsf(wz.z);
    ret.setCenter(XMVector3TransformCoord( aabb.getCenter(), world));
    ret.setHSize(XMLoadFloat3(&newHSize));
    
#if defined(_DEBUG) && defined(SPECIAL_AABB_CHECKS)
    assert(!ret.isInvalid());
#endif

    return ret;
}

void CCullingAABBSpecial::draw() const
{
    Entity* me = Handle(this).getOwner();
    assert(me != nullptr);
    switch(type) {
        case LIGHT_PT: {
            CPtLight* ptLight = me->get<CPtLight>();
            assert(ptLight != nullptr);
            setObjectConstants(
                XMMatrixAffineTransformation(2*hSize, zero_v, one_q, center-hSize),
                ptLight->getColor());
            mesh_cube_wire_split.activateAndRender();

            } break;
        case LIGHT_DIR: {
            CDirLight* dirLight = me->get<CDirLight>();
            assert(dirLight != nullptr);
            draw(dirLight->getColor());
            } break;
        case LIGHT_VOL: {
            CVolPtLight* volLight = me->get<CVolPtLight>();
            assert(volLight != nullptr);
            setObjectConstants(
                XMMatrixAffineTransformation(2*hSize, zero_v, one_q, center-hSize),
                volLight->getColor());
            mesh_cube_wire.activateAndRender();
            } break;
        default: draw(Color::MAGENTA); break;
    }
}

void CCullingAABBSpecial::update(float)
{
    Entity* me = Handle(this).getOwner();
    assert(me != nullptr);
    switch(type) {
        case LIGHT_PT: {
            CPtLight* ptLight = me->get<CPtLight>();
            assert(ptLight != nullptr);
            CTransform* t = me->get<CTransform>();
            assert(t != nullptr);
            center = t->getPosition() + ptLight->getOffset();
            hSize = one_v * ptLight->getRadius();
            break;
            }
        case LIGHT_VOL: {
            CVolPtLight* volLight = me->get<CVolPtLight>();
            assert(volLight != nullptr);
            CTransform* t = me->get<CTransform>();
            assert(t != nullptr);
            center = t->getPosition();
            hSize = one_v * volLight->getRadius();
            break;
            }
        case LIGHT_DIR: {
            CDirLight* dirLight = me->get<CDirLight>();
            assert(dirLight != nullptr);
            CCamera* light = me->get<CCamera>();
            assert(light != nullptr);
            CTransform* t = me->get<CTransform>();
            assert(t != nullptr);
            auto lightPos = t->getPosition() + dirLight->getOffset();
            auto opposite = lightPos + light->getFront()*light->getZFar();
            if (light->isOrthographic()) {
                float zFar = light->getZFar();
                float width = light->getW()/2;
                float height = light->getH()/2;
                XMVECTOR min(XMVectorSet(-width, -height, 0,    1));
                XMVECTOR max(XMVectorSet( width,  height, zFar, 1));
                setCorners(min, max);
                XMMATRIX world = t->getWorld();
                operator=(bakeCulling(*this, world));
            } else {
                //A fustrum is a pyramid, therefore it has 5 vertices
                //The AABB is the min and max of those vertices' components.
                //Top of the pyramid := lightPos

                //Center of the base
                float zFar = light->getZFar();
                auto b = lightPos + t->getFront()*zFar;

                //Corners={base +- up * height + left * width}
                float width = zFar * fabsf(std::tan(light->getFov()*0.5f));
                float height = width * light->getAspectRatio();
                auto u = t->getUp()*height;
                auto l = t->getLeft()*width;

                //Corners
                XMVECTOR top = b+u;
                XMVECTOR bot = b-u;
                XMVECTOR ul = top+l;
                XMVECTOR ur = top-l;
                XMVECTOR dl = bot+l;
                XMVECTOR dr = bot-l;

                XMVECTOR min = XMVectorMin(lightPos, ul);
                XMVECTOR max = XMVectorMax(lightPos, ul);
                min = XMVectorMin(min,ur);
                max = XMVectorMax(max,ur);
                min = XMVectorMin(min,dl);
                max = XMVectorMax(max,dl);
                min = XMVectorMin(min,dr);
                max = XMVectorMax(max,dr);
                setCorners(min, max);
            }
            } break;
        default:
            center = zero_v;
            hSize = one_v*FLT_MAX/2;
            break;
    }
}

void CCullingAABB::update(float)
{

#ifdef _DEBUG
    uint32_t c = dbgColor;
#endif
    Entity* me = Handle(this).getOwner();
    assert(me != nullptr);
    CTransform* transform = me->get<CTransform>();
    assert(transform != nullptr);
    CMesh* mesh = me->get<CMesh>();
    
    
    if((dirty || prevTransform != *transform) &&
        mesh != nullptr && mesh->isVisible()) {
        CAABB* baseAABB = me->get<CAABB>();
        assert(baseAABB != nullptr);
        dirty = false;
        prevTransform.set(*transform);
        #if defined(_DEBUG) && defined(SPECIAL_AABB_CHECKS)
            if (baseAABB->isInvalid()) {
                dbg("%s: Invalid AABB.\n", me->getName().c_str());
            }
        #endif
        operator=(CullingAABB::bakeCulling(*baseAABB, transform->getWorld()));
        if (scale != 1){scaleAABB(scale);}
        if (skin != 0) {skinAABB(skin);}
        #if defined(_DEBUG) && defined(SPECIAL_AABB_CHECKS)
            if (center == zero_v) {
                dbg("Curious coincidence: entity %s's AABB is centered at (0,0,0).\n", me->getName().c_str());
            }
        #endif
    }
}


void CCullingAABBSpecial::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    AABB::loadFromProperties(elem, atts);
    skin = atts.getFloat("skin", skin);
    scale = atts.getFloat("scale", scale);
    if (atts.has("type")) {
        std::string typeStr = atts["type"];
        if (typeStr == "ptLight") {type=LIGHT_PT;}
        else if (typeStr == "dirLight") {type=LIGHT_DIR;}
        else if (typeStr == "volLight") {type=LIGHT_VOL;}
        else {dbg("Unknown CullingAABBSpecial type: \"%s\".\n", typeStr.c_str());}
    } else {
        dbg("CullingAABBSpecial without a type.\n");
    }
}

void CCullingAABB::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    AABB::loadFromProperties(elem, atts);
    skin = atts.getFloat("skin", skin);
    scale = atts.getFloat("scale", scale);
}

Culling::cullers_t Culling::cullers;
unsigned Culling::currentCuller = 0;
bool Culling::cullerListChanged = true;

Culling::mask_t Culling::addDelegate(CullerDelegate&& delegate)
{
    if (cullers.size() > currentCuller && cullers[currentCuller] == delegate) {
        ++currentCuller;
        return 1<<(currentCuller-1);
    } else {
        if (!cullerListChanged) {cullers.erase(cullers.begin()+currentCuller, cullers.end());}
        cullers.push_back(delegate);
        size_t size = cullers.size();
        assert(size < MAX_CULLERS);
        ++currentCuller;
        cullerListChanged = true;
        return 1<<(currentCuller-1);
    }
}

void CCulling::update(float)
{
    Handle me_h = Handle(this).getOwner();
    Entity* me(me_h);
    CCamera* camera = me->get<CCamera>();
    assert(camera != nullptr);
    auto vp = camera->getViewProjection();
    CMesh* mesh = me->get<CMesh>();
    if (vp != previousViewProjection) {
        previousViewProjection = vp;
        XMMATRIX vpt = XMMatrixTranspose(vp);
        XMVECTOR x = vpt.r[0];
        XMVECTOR y = vpt.r[1];
        XMVECTOR z = vpt.r[2];
        XMVECTOR w = vpt.r[3];
        planes[0] = w + x;
        planes[1] = w - x;
        planes[2] = w + y;
        planes[3] = w - y;
        planes[4] = w + z;
        planes[5] = w - z;
        changed = true;
    } else {
        changed = false;
    }
    mask = addCCulling(me_h);
}

bool CCulling::contains(XMVECTOR point) const
{
    for (const auto& plane : planes) {
        if (DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(plane, point)) < 0) {
            return false;
        }
    }
    return true;
}  

bool CCulling::cull(const AABB& aabb) const
{
    // We must be in the positive part of all the planes
    for (const auto& plane : planes) {
        XMVECTOR absNormal = XMVectorMax(plane, -plane);
    
        // Classifies aabb as per the plane: intersection, in back side, or in front side
        float r = XMVectorGetX(XMPlaneDotNormal(aabb.getHSize(), absNormal));

        // Distance from box center to the plane
        float c = XMVectorGetX(XMPlaneDotCoord(plane, aabb.getCenter()));

        if (fabsf(c) < r) {continue;}
        if (c < r) {return false;}
    }
    return true;
}

void CCullingCube::update(float)
{
    Handle me_h = Handle(this).getOwner();
    Entity* me(me_h);
    CCubeShadow* shadow = me->get<CCubeShadow>();
    assert(shadow != nullptr);
    CMesh* mesh = me->get<CMesh>();
    changed = false;
    for(unsigned i=0; i<6; ++i) {
        auto vp = shadow->getCachedCamData(i).viewProjection;
        if (vp != previousViewProjection[i]) {
            previousViewProjection[i] = vp;
            XMMATRIX vpt = XMMatrixTranspose(vp);
            XMVECTOR x = vpt.r[0];
            XMVECTOR y = vpt.r[1];
            XMVECTOR z = vpt.r[2];
            XMVECTOR w = vpt.r[3];
            planes[i][0] = w + x;
            planes[i][1] = w - x;
            planes[i][2] = w + y;
            planes[i][3] = w - y;
            planes[i][4] = w + z;
            planes[i][5] = w - z;
            changed = true;
        }
        mask[i] = addCCullingCube(me_h, Culling::cullDirection_e(i));
    }
}

bool CCullingCube::contains(XMVECTOR point, Culling::cullDirection_e dir) const
{
    assert (dir >=0 && dir <6);
    for (const auto& plane : planes) {
        if (DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(plane[dir], point)) < 0) {
            return false;
        }
    }
    return true;
}  

bool CCullingCube::cull(const AABB& aabb, Culling::cullDirection_e dir) const
{
    assert (dir >=0 && dir <6);
    // We must be in the positive part of all the planes
    for (const auto& plane : planes[dir]) {
        XMVECTOR absNormal = XMVectorMax(plane, -plane);
    
        // Classifies aabb as per the plane: intersection, in back side, or in front side
        float r = XMVectorGetX(XMPlaneDotNormal(aabb.getHSize(), absNormal));

        // Distance from box center to the plane
        float c = XMVectorGetX(XMPlaneDotCoord(plane, aabb.getCenter()));

        if (c < r && fabsf(c) >= r) {return false;}
    }
    return true;
}

Culling::CullerDelegate::CullerDelegate(component::Handle e_h, cullDirection_e type) : type(type)
{
    if (type == Culling::NORMAL) {
        culler = (CCulling*) e_h.getSon<CCulling>();
    } else {
        assert(type >= 0 && type <6);
        culler = (CCullingCube*) e_h.getSon<CCullingCube>();
    }
    assert(culler!=nullptr);
}

Culling::mask_t Culling::CullerDelegate::getMask() const
{
    return (type == Culling::NORMAL) ?
        ((CCulling*)culler)->getMask() :
        ((CCullingCube*)culler)->getMask(type);
}

bool Culling::CullerDelegate::cull(const component::AABB& aabb) const
{
    return (type == Culling::NORMAL) ?
        ((CCulling*)culler)->cull(aabb) :
        ((CCullingCube*)culler)->cull(aabb, type);
}

bool Culling::CullerDelegate::contains(XMVECTOR p) const
{
    return (type == Culling::NORMAL) ?
        ((CCulling*)culler)->contains(p) :
        ((CCullingCube*)culler)->contains(p, type);
}

bool Culling::CullerDelegate::hasChanged() const
{
    return (type == Culling::NORMAL) ?
        ((CCulling*)culler)->hasChanged() :
        ((CCullingCube*)culler)->hasChanged();
}

}