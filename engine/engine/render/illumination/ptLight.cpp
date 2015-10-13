#include "mcv_platform.h"
#include "ptLight.h"

#include "cubeshadow.h"
#include "handles/handle.h"
#include "components/transform.h"
#include "../render_utils.h"      
#include "../camera/culling.h"

#ifdef _LIGHTTOOL
#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;
#endif

using namespace utils;
using namespace DirectX;
using namespace component;

namespace render {

#ifdef _LIGHTTOOL
void CPtLight::setSelectable()
{
	Handle h;
	Entity* e(Handle(this).getOwner());
	h = getManager<CRigidBody>()->createObj();
	e->add(h);
	CRigidBody* r(h);
	r->setSphere(0.5f);
	r->setFilters(filter_t::TOOLS_SELECTABLE, filter_t::ALL_IDS);
	r->createRigidBody();
	r->setKinematic(true);
	r->init();
    r->getShape()->setLocalPose(toPxTransform(XMMatrixTranslationFromVector(getOffset())));
}
#endif

void CPtLight::cull(component::Handle camera_h)
{
    Entity* me = Handle(this).getOwner();
    CCullingAABBSpecial* aabb(me->get<CCullingAABBSpecial>());
    if (aabb != nullptr) {
        CCulling* culling = camera_h.getSon<CCulling>();
        assert(culling != nullptr);
        culled = culling->cull(*aabb);
        if (culled && me->has<CCubeShadow>()) {
            CTransform* t = me->get<CTransform>();
            auto pos = t->getPosition();

            // Test each face (if it was enabled to begin with, to avoid redundant disabling)
            uint8_t mask = ~0;
            auto min = aabb->getMin();
            auto max = aabb->getMax();

            CCubeShadow* shadow = me->get<CCubeShadow>();
            uint8_t enableMask = shadow->getEnableMask();

            //+X
            if ((enableMask & (1<<0)) != 0 ) {
                AABB newAABB(XMVectorSetX(min, XMVectorGetX(pos)), max);
                if (!culling->cull(newAABB)) {mask &= ~(1<<0);}
            }
            //-X
            if ((enableMask & (1<<1)) != 0) {
                AABB newAABB(min, XMVectorSetX(max, XMVectorGetX(pos)));
                if (!culling->cull(newAABB)) {mask &= ~(1<<1);}
            }
            //+Y
            if ((enableMask & (1<<2)) != 0) {
                AABB newAABB(XMVectorSetY(min, XMVectorGetY(pos)), max);
                if (!culling->cull(newAABB)) {mask &= ~(1<<2);}
            }
            //-Y
            if ((enableMask & (1<<3)) != 0) {
                AABB newAABB(min, XMVectorSetY(max, XMVectorGetY(pos)));
                if (!culling->cull(newAABB)) {mask &= ~(1<<3);}
            }
            //+Z
            if ((enableMask & (1<<4)) != 0) {
                AABB newAABB(min, XMVectorSetZ(max, XMVectorGetZ(pos)));
                if (!culling->cull(newAABB)) {mask &= ~(1<<4);}
            }
            //-Z
            if ((enableMask & (1<<5)) != 0) {
                AABB newAABB(XMVectorSetZ(min, XMVectorGetZ(pos)), max);
                if (!culling->cull(newAABB)) {mask &= ~(1<<5);}
            }

            shadow->setCullingMask(mask);
        }
    } else {
        culled = true;
    }
}

void CPtLight::drawVolume()
{
	Entity* e(Handle(this).getOwner());
	CTransform* t = e->get<CTransform>();
	auto pos = t->getPosition() + getOffset();
    
    mesh_icosahedron_wire.activate();
	Color darker = Color(color).factor(.5f);

#ifdef _LIGHTTOOL
    if (selected){
        setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), Color::RED);
	    mesh_icosahedron_wire.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::GRAY);
	    mesh_icosahedron_wire.render();
    } else {
        setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), darker);
	    mesh_icosahedron_wire.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::GRAY);
	    mesh_icosahedron_wire.render();
    }
    
#else
    setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), color);
	mesh_icosahedron_wire.render();
    setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::GRAY);
	mesh_icosahedron_wire.render();
#endif
    
    if (enabled) {
	    setObjectConstants(XMMatrixAffineTransformation(one_v*radius*decayFactor, zero_v, one_q, pos), color);
	    mesh_icosahedron_wire.render();

	    setObjectConstants(XMMatrixAffineTransformation(one_v*radius, zero_v, y90Rot_q, pos), darker);
	    mesh_icosahedron_wire.render();
    }
}

void CPtLight::draw()
{
    if (!(isSpatiallyGood() && enabled && culled)) {return;}
    static const Technique*const techRegular = Technique::getManager().getByName("deferred_point_light");
    static const Technique*const techShadows = Technique::getManager().getByName("deferred_point_light_shadow");

    // Get the transform of the entity
    Entity* me = Handle(this).getOwner();
    CTransform* t = me->get<CTransform>();
    CCubeShadow* shadow = me->get<CCubeShadow>();
    XMMATRIX scale = XMMatrixScaling(radius, radius, radius);
    XMVECTOR pos = t->getPosition() + offset;
    XMMATRIX trans = XMMatrixTranslationFromVector(pos);
    setObjectConstants(scale * trans);
    activatePtLight(this, pos, shadow);

    if (shadowIntensity > 0.f && shadow!=nullptr) {
        // Activate the camera
        CCamera* camera = me->get<CCamera>();
        activateLight(*camera);
        // Activate the previously generated shadow map
        CCubeShadow* shadow = me->get<CCubeShadow>();
        shadow->getFxShadowBuffer()->activate(6);
        techShadows->activate();
    } else {
        techRegular->activate();
    }
    
    mesh_icosahedron.activateAndRender();
}

void CPtLight::setOffset(XMVECTOR nOff)
{
    offset = nOff;
    Handle cam_h = Handle(this).getBrother<CCamera>();
    if (cam_h.isValid()) {
        CCamera* cam(cam_h);
        cam->setOffset(nOff);
    }
}

void CPtLight::update(float)
{
    Entity* me = Handle(this).getOwner();
    CCubeShadow* shadow = me->get<CCubeShadow>();
    if (shadow != nullptr) {
        shadow->setEnabled(enabled && culled && shadowIntensity > 0.f);
    }
}

}
