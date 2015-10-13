#include "mcv_platform.h"

using namespace utils;
using namespace DirectX;

#include "mist.h"

#include "app.h"

#include "render/render_utils.h"
#include "render/mesh/component.h"
#include "render/camera/component.h"

#include "handles/entity.h"
#include "components/transform.h"
using namespace component;
using namespace utils;
using namespace DirectX;

#ifdef _LIGHTTOOL
#include "PhysX_USER/pxcomponents.h"
using namespace physX_user;
#endif


namespace render {

void CMist::loadFromProperties(const std::string elem, utils::MKeyValue atts)
{
    if (elem == "Mist") {
        w = atts.getFloat("w", w);
        h = atts.getFloat("h", h);
        l = atts.getFloat("l", l);
        dX = atts.getFloat("dX", dX);
        dZ = atts.getFloat("dZ", dZ);
        factor = atts.getFloat("factor", factor);
        minimun = atts.getFloat("minimun", minimun);
        layerDecay = atts.getFloat("layerDecay", layerDecay);
        intensity = atts.getFloat("intensity", intensity);
        sqSqrt = atts.getFloat("sqSqrt", sqSqrt);
        depthTolerance = atts.getFloat("depthTolerance", depthTolerance);
        darkenAlpha = atts.getFloat("darkenAlpha", darkenAlpha);
        unitWorldSize = atts.getFloat("unitWorldSize", unitWorldSize);
        chaotic = atts.getBool("chaotic", chaotic);
#ifdef _LIGHTTOOL
        exportable = atts.getBool("export", exportable);
#endif
    } else if (elem == "top") {
        colorTop.loadFromProperties(elem, atts);
        colorTop.premultiplyAlpha();
    } else if (elem == "bottom") {
        colorBottom.loadFromProperties(elem, atts);
        colorBottom.premultiplyAlpha();
    }
}

void CMist::drawMarker() const
{
	Entity* e(Handle(this).getOwner());
	CTransform* t = e->get<CTransform>();
	auto pos = t->getPosition();
    
    mesh_icosahedron_wire.activate();

#ifdef _LIGHTTOOL
    if (selected){
        setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), Color::RED);
	    mesh_icosahedron_wire.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), colorBottom);
	    mesh_icosahedron_wire.render();
    } else {
        setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), colorTop);
	    mesh_icosahedron_wire.render();
        setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), colorBottom);
	    mesh_icosahedron_wire.render();
    }
#else
    setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), colorTop);
	mesh_icosahedron_wire.render();
    setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), colorBottom);
	mesh_icosahedron_wire.render();

#endif
}

void CMist::draw() const
{
    if (muted) {return;}
    static const Technique* tech = Technique::getManager().getByName("mist");
    tech->activate();

    Entity* e = Handle(this).getOwner();
    CTransform* ct = e->get<CTransform>();

    Transform t(*ct);
    auto cam = App::get().getCamera();
    CTransform* camT = cam.getSon<CTransform>();

    auto pos = t.getPosition();
    if (XMVectorGetY(camT->getPosition()) > XMVectorGetY(t.getPosition())-h) {
        t.refPosition() -= yAxis_v*h;
        for(int i=4; i>=0; --i) {
            t.setPosition(pos - XMVectorSet(0, i * h/4.f, 0, 0));
            setObjectConstants(DirectX::XMMatrixScaling(w,1,l)*t.getWorld());
            activateMist(this, unsigned(i));
            mesh_textured_quad_xz_centered.activateAndRender();
        }
    } else {
        for(int i=0; i<5; ++i) {
            t.setPosition(pos - XMVectorSet(0, i * h/4.f, 0, 0));
            setObjectConstants(DirectX::XMMatrixScaling(w,1,l)*t.getWorld());
            activateMist(this, unsigned(i));
            mesh_textured_quad_xz_centered.activateAndRender();
        }
    }
}

#ifdef _LIGHTTOOL
void CMist::setSelectable()
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