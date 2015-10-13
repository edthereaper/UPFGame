#include "mcv_platform.h"
#include "volLight.h"
#include "render/render_utils.h"
#include "render/camera/component.h"
#include "render/camera/culling.h"

#include "app.h"
#include "components/transform.h"
using namespace component;

using namespace utils;
using namespace DirectX;


#ifdef _LIGHTTOOL
#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;
#endif

namespace render {

void CVolPtLight::draw()
{
    static const Technique* tech = Technique::getManager().getByName("volLight");
    if(!(culled && enabled && isSpatiallyGood())) {return;}
    
    auto& app = App::get();
    CCamera* camera = app.getCamera().getSon<CCamera>();
    CTransform* transform = Handle(this).getBrother<CTransform>();
    
    //Draw a sphere
    XMMATRIX scale = XMMatrixScaling(radius, radius, radius);
    XMVECTOR pos = transform->getPosition();
    XMMATRIX trans = XMMatrixTranslationFromVector(pos);
    setObjectConstants(scale * trans);
    activateVolPtLight(this, pos);
    tech->activate();
    mesh_icosahedron.activateAndRender();
}

void CVolPtLight::cull(component::Handle camera_h)
{
    Entity* me = Handle(this).getOwner();
    CCullingAABBSpecial* aabb(me->get<CCullingAABBSpecial>());
    if (aabb != nullptr) {
        CCulling* culling = camera_h.getSon<CCulling>();
        assert(culling != nullptr);
        culled = culling->cull(*aabb);
    } else {
        culled = true;
    }
}

void CVolPtLight::drawVolume()
{
	Entity* e(Handle(this).getOwner());
	CTransform* t = e->get<CTransform>();
	auto pos = t->getPosition();
    
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
	    setObjectConstants(XMMatrixAffineTransformation(one_v*radius*decay, zero_v, one_q, pos), color);
	    mesh_icosahedron_wire.render();

	    setObjectConstants(XMMatrixAffineTransformation(one_v*radius, zero_v, y90Rot_q, pos), darker);
	    mesh_icosahedron_wire.render();
    }
}

#ifdef _LIGHTTOOL
void CVolPtLight::setSelectable()
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
}
#endif

}