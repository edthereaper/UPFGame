#include "mcv_platform.h"
#include "whitebox.h"

#include "render/mesh/component.h"
#include "render/render_utils.h"
using namespace render;

#include "handles/entity.h"
#include "handles/handle.h"
using namespace component;

using namespace DirectX;

namespace gameElements {

void CWhiteBox::createBox(XMVECTOR size, Color tint)
{
    mesh = new Mesh;
    bool isOK = createPUNTBox(*mesh, size/2, -size/2);
    if (isOK) {
        Entity* me = Handle(this).getOwner();

        Handle h(me->get<CMesh>());
        if (!h.isValid()) {
            getManager<CMesh>()->createObj();
            me->add(h);
        }
        CMesh* cmesh = h;
        cmesh->setMesh(mesh);
        CMesh::key_t key;
        key.group0 = key.groupf =0;
        key.material=Material::getManager().getByName("white");
        key.tint = tint;
        cmesh->addKey(key);
    }
}

CWhiteBox::~CWhiteBox()
{
    Handle h = Handle(this);
    if (h.isValid()) {
        h = h.getOwner();
        if (h.isValid()) {
            Entity* me = Handle(this).getOwner();
            h = me->get<CMesh>();
            if (h.isValid()) {
                CMesh* cmesh = h;
                cmesh->~CMesh();
            }
        }
    }
    SAFE_DELETE(mesh);
}

}