#include "mcv_platform.h"
#include "destructible.h"

#include "render/mesh/component.h"
#include "render/render_utils.h"
using namespace render;

#include "handles/entity.h"
#include "handles/handle.h"
#include "handles/prefab.h"
#include "components/color.h"
using namespace component;


#include "physX_USER/pxcomponents.h"
using namespace physX_user;

using namespace DirectX;

#include "fmod_User/fmodUser.h"
using namespace fmodUser;

#include "particles/ParticlesManager.h"
using namespace particles;

namespace gameElements {

void CDestructible::breakGlass()
{
    Entity* me(Handle(this).getOwner());
    me->get<CStaticBody>().destroy();
    //create particles
	fmodUser::fmodUserClass::playSound("Glass_break", 0.5f, 0.0f);
    CTransform* t = me->get<CTransform>();
    me->postMsg(MsgDeleteSelf());

    Entity* restorer = getManager<Entity>()->createObj();
    CDestructibleRestorer* dr = getManager<CDestructibleRestorer>()->createObj();
    dr->set(*t);
    dr->boxSize = boxSize;

    restorer->add(dr);
}

void CDestructible::createBox(XMVECTOR size)
{
    mesh = new Mesh;
    bool isOK = createPUNTBox(*mesh, size/2, -size/2);
    Entity* me = Handle(this).getOwner();

    Handle h(getManager<CMesh>()->createObj());
    me->add(h);
    CMesh* cmesh = h;
    cmesh->setMesh(mesh);
    CMesh::key_t key;
    key.group0 = key.groupf = 0;
    key.material = Material::getManager().getByName("glass");
    cmesh->addKey(key);

    boxSize = size;
}

CDestructible::~CDestructible()
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

void CDestructible::setup()
{
	Entity* me = Handle(this).getOwner();
	if (me->has<CEmitter>()){
		CEmitter *emitter = me->get<CEmitter>();
		auto k = emitter->getKey("emitter_0");
		ParticleUpdaterManager::get().sendActive(k);
	}
}

void CDestructibleRestorer::revive(const MsgRevive&)
{
    Entity* e = PrefabManager::get().prefabricate("destructible");
    CTransform* t = e->get<CTransform>();
    t->set(*this);
    
    CStaticBody* s = e->get<CStaticBody>();
    CTrigger* trigger = e->get<CTrigger>();
    s->setBox(boxSize);
    trigger->setBox(boxSize);
    
    CDestructible* destructible = e->get<CDestructible>();
    destructible->createBox(boxSize);
    destructible->setup();
    e->init();

    Entity* me = Handle(this).getOwner();
    me->postMsg(MsgDeleteSelf());
}

}