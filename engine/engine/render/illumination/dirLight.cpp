#include "mcv_platform.h"
#include "dirLight.h"

#include "handles/handle.h"
#include "components/transform.h"
#include "../render_utils.h"    
#include "../camera/component.h"
#include "../camera/culling.h"
#include "../shader/shaders.h"    
#include "shadow.h"

#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;

using namespace utils;
using namespace DirectX;
using namespace component;

namespace render {
    
#ifdef _LIGHTTOOL
void CDirLight::setSelectable()
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

void CDirLight::cull(component::Handle camera_h)
{
    CCullingAABBSpecial* aabb(Handle(this).getBrother<CCullingAABBSpecial>());
    if (aabb != nullptr) {
        CCulling* culling = camera_h.getSon<CCulling>();
        assert(culling != nullptr);
        culled = culling->cull(*aabb);
    } else {
        culled = true;
    }
}

void CDirLight::drawVolume()
{
	Entity* e(Handle(this).getOwner());
	//Draw the pivot of the light
	CTransform* t = e->get<CTransform>();
	auto pos = t->getPosition() + getOffset();
    
#ifdef _LIGHTTOOL
    static const XMVECTOR cube_rot = XMQuaternionMultiply(y45Rot_q, z45Rot_q);
	if (selected){
        mesh_icosahedron_wire.activate();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, y90Rot_q, pos), Color::RED);
	    mesh_icosahedron_wire.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), color);
	    mesh_icosahedron_wire.render();
        mesh_cube_wire_unit.activate();
	    setObjectConstants(XMMatrixAffineTransformation(one_v* .05f, zero_v, cube_rot, t->getLookAt()), Color::RED);
	    mesh_cube_wire_unit.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.04f, zero_v, one_q, t->getLookAt()), color);
	    mesh_cube_wire_unit.render();
    } else {
	    Color darker = Color(color).factor(.5f);
        mesh_icosahedron_wire.activate();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, y90Rot_q, pos), darker);
	    mesh_icosahedron_wire.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::DARK_GRAY);
	    mesh_cube_wire_unit.render();
        mesh_cube_wire_unit.activate();
	    setObjectConstants(XMMatrixAffineTransformation(one_v* .05f, zero_v, cube_rot, t->getLookAt()), darker);
	    mesh_cube_wire_unit.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.04f, zero_v, one_q, t->getLookAt()), Color::DARK_GRAY);
	    mesh_cube_wire_unit.render();
    }
    
#else
    setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), color);
	mesh_icosahedron_wire.render();
    setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::GRAY);
	mesh_icosahedron_wire.render();
#endif

	//Draw camera volume
    if (enabled) {
	    CCamera* lcam = e->get<CCamera>();
	    assert(lcam);
	    drawViewVolume(*lcam, color);
    }
}

void CDirLight::draw()
{
    if (!(isSpatiallyGood() && enabled && culled)) {return;}
    static const Technique*const techRegular = Technique::getManager().getByName("deferred_dir_light");
    static const Technique*const techShadows = Technique::getManager().getByName("deferred_dir_light_shadow");

    // Use the inv view projection as world matrix, so our unit cube mesh
    // becomes the fustrum in world space
    Entity* me(Handle(this).getOwner());
    CCamera* light = me->get<CCamera>();
    CShadow* shadow = me->get<CShadow>();
    assert(light != nullptr);

    XMMATRIX invViewProj = XMMatrixInverse(nullptr, light->getViewProjection());
    setObjectConstants(invViewProj);
    activateLight(*light);
    activateDirLight(this, light, shadow);

    if (shadowIntensity > 0.f && shadow!=nullptr && shadow->hasPassedSpatial()) {
        // Activate the previously generated shadow map
        shadow->getFxShadowBuffer()->activate(6);
        techShadows->activate();
    } else {
        techRegular->activate();
    }
    mesh_view_volume.activateAndRender();
}

void CDirLight::setOffset(XMVECTOR nOff) {
    ((CCamera*)(Handle(this).getBrother<CCamera>()))->setOffset(nOff);
}
XMVECTOR CDirLight::getOffset() const {
    return ((CCamera*)(Handle(this).getBrother<CCamera>()))->getOffset();
}

void CDirLight::update(float)
{
    Entity* me = Handle(this).getOwner();
    CShadow* shadow = me->get<CShadow>();
    if (shadow != nullptr) {
        shadow->setEnabled(enabled && culled && shadowIntensity > 0.f);
    }
}

}