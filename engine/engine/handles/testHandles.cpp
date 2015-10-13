#include "mcv_platform.h"
#ifdef _TEST

#include "testHandles.h"
#include "components/components.h"

#include "handle.h"
#include "entity.h"
#include "importXML.h"

using namespace component;
namespace test {

struct testKeyGen : public EntityListManager::KeyGen {
    static const EntityListManager::key_t TAGA = 0xAAAAAAAA;
    static const EntityListManager::key_t TAGB = 0xBBBBBBBB;
    static const EntityListManager::key_t TAGC = 0xCCCCCCCC;
    bool c = false;
    inline EntityListManager::key_t operator()(Handle h) {
        component::Entity* e(h);
        if (c) {
            return TAGC;
        } else {
            if (e->has<_TCA>()) {return TAGA;}
            else if (e->has<_TCB>()) {return TAGB;}
            else {return EntityListManager::INVALID_KEY;}
        }
    }
};


void _TCA::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (atts.has("a")) a = atts.getInt("a", 0);
    if (atts.has("b")) b = atts.getInt("b", 0);
}

void _TCB::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (atts.has("a")) a = atts.getInt("a", 0);
    if (atts.has("b")) b = atts.getInt("b", 0);
}

void testHandles()
{

    utils::dbg("TEST: testHandles --- BEGIN\n");
    
    auto entityManager = getManager<component::Entity>();

    Handle eh;
    component::Entity* e = eh.create<component::Entity>();

    //Create some components and check their integrity
    _TCA* tca1 = getManager<_TCA>()->createObj();
    tca1->a = 3;
    e->add(tca1);
    tca1->b = 5;
    _TCA* tca2 = e->get<_TCA>();
    assert(tca1 == tca2 && *tca1 == *tca2);

    //Destroy an entity and see what happened to its components
    Handle tca1h = e->get<_TCA>();
    entityManager->destroyObj(Handle(e));
    assert(!tca1h.isValid());

    //test conversion to raw
    void* raw = tca1h.getRawAsVoidPtr();
    Handle hraw = Handle::fromRaw(raw);
    void* raw2 = hraw.getRawAsVoidPtr();
    Handle hraw2= Handle::fromRaw(raw2);
    assert(tca1h == hraw);
    assert(raw == raw2);
    assert(tca1h == hraw2);

    //Create an entity list with some entities with components
    component::Entity* e1 = entityManager->createObj();
    component::Entity* e2 = entityManager->createObj();
    component::Entity* e3 = entityManager->createObj();
    component::Entity* e4 = entityManager->createObj();

    EntityList list;
    list.add(e1);
    list.add(e2);
    list.add(e3);

    tca1 = getManager<_TCA>()->createObj();
    tca1->a = 1; tca1->b = 11;
    tca2 = getManager<_TCA>()->createObj();
    tca2->a = 2; tca2->b = 22;
    _TCB* tcb1 = getManager<_TCB>()->createObj();
    tcb1->a = 1; tcb1->b = 33;
    e1->add(tca1);
    e1->add(tcb1);
    e2->add(tca2);
    e3->add(Handle(tca1).clone());
    e4->add(Handle(tca2).clone());
    tca1 = e3->get<_TCA>();
    tca1->a = 3;
    tca1 = e4->get<_TCA>();
    tca1->a = 4;
    _TMsgA msga;
    _TMsgB msgb;

    //Subscribe _TCA to _TMsg
    SUBSCRIBE_MSG_TO_MEMBER(_TCA,_TMsgA,receiveMsg);
    SUBSCRIBE_MSG_TO_MEMBER(_TCB,_TMsgB,receiveMsg);
    SUBSCRIBE_MSG_TO_MEMBER(_TCA,_TMsgC,receiveMsg);
    SUBSCRIBE_MSG_TO_MEMBER(_TCB,_TMsgC,receiveMsg);

    utils::dbg("msga.id=%d, msgb.id=%d\n", _TMsgA::getId(), _TMsgB::getId());

    utils::dbg("\nSending some messages...\n");
    //Send a _TMsgA to a given entity
    msga.c = 100;
    msgb.c = 500;
    e1->sendMsg(msga);
    e1->sendMsg(msgb);

    //Post a _TMsgA to a given entity
    msga.c = 200;
    msga.c = 600;
    e1->postMsg(msga);
    e1->postMsg(msgb);
    

    //Broadcast a message through it
    msga.c = 10;
    utils::dbg("list.broadcast(msga):\n");
    list.broadcast(msga);

    //Post-broadcast
    msgb.c = 20;
    utils::dbg("list.broadcastPost(msgb):\n");
    list.broadcastPost(msgb);
    
    utils::dbg("MessageManager::dispatchPosts(): (called first time)\n");
    MessageManager::dispatchPosts();
    utils::dbg("MessageManager::dispatchPosts(): (called again)\n");
    MessageManager::dispatchPosts();
    
    utils::dbg("\nLet's kill entities :DDDD\n");
    Handle h1(e1), h2(e2), h3(e3);
    
    utils::dbg("list.broadcast(msga): 1,2,3\n");
    msga.c = 1000;
    list.broadcast(msga);
    e = h1;
    e->destroy();
    assert(!h1.isValid());
    msga.c = 1100;
    utils::dbg("list.broadcast(msga): 2,3\n");
    list.broadcast(msga);

    e = h2;
    e->sendMsg(MsgDeleteSelf()); //immediato... a priori así no
    assert(!h2.isValid());
    msga.c = 1200;
    utils::dbg("list.broadcast(msga): 3\n");
    list.broadcast(msga);

    e = h3;
    e->postMsg(MsgDeleteSelf()); // así si
    assert(h3.isValid());
    msga.c = 1300;
    utils::dbg("list.broadcast(msga): 3\n");
    list.broadcast(msga);

    utils::dbg("MessageManager::dispatchPosts():\n");
    MessageManager::dispatchPosts();
    assert(!h3.isValid());
    utils::dbg("list.broadcast(msga): -\n");
    msga.c = 1400;
    list.broadcast(msga);

    utils::dbg("\nParse an XML\n");
    EntityListManager::setKeyGen(new testKeyGen);
    Importer::parse("testData/testHandles.xml");
    msga.c=9999;
    msgb.c=8888;

    utils::dbg("expected 1, 2:\n");
    EntityListManager::get(testKeyGen::TAGA).broadcast(msga);
    utils::dbg("expected 3:\n");
    EntityListManager::get(testKeyGen::TAGB).broadcast(msgb);
    
    utils::dbg("PREFABS:\n");
    dynamic_cast<testKeyGen*>(EntityListManager::getKeyGen())->c =true; //should not fail
    Importer::parse("testData/testPrefabs.xml");
    EntityListManager::get(testKeyGen::TAGC).broadcast(_TMsgC());

    utils::dbg("\nTEST: testHandles --- END\n");
}

}

#endif