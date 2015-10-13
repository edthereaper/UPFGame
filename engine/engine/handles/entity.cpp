#include "mcv_platform.h"
#include "entity.h"

#include <iomanip>

namespace component {
void Entity::initType()
{
    MessageManager::subscribe(
        MsgDeleteSelf::getId(),
        Entity::SELF,
        new Member<Entity, MsgDeleteSelf>(&Entity::destroy));
}

Entity::Entity( const Entity& e )
{
    Handle h(this);

	// Clone each of the components
	for( int i=0; i<Handle::MAX_TYPES; ++i ) {
		if( e.components[i].isValid() ) {
			components[i] = e.components[i].clone();
            components[i].setOwner(h);
        }
	}
}

void Entity::init()
{
	for( int i=0; i<Handle::MAX_TYPES; ++i ) {
		if( components[i].isValid()) {
			components[i].init();
        }
	}
}

void Entity::sendMsg(MessageManager::msgId_t id, const void* data)
{
    for (auto suscription : MessageManager::getSubscriptions(id)) {
        if (suscription.second.execute != nullptr) {
            if (suscription.second.compType == SELF) {
                /* Distinguish a suscription of "self" from a suscription of entity     *
                 * component, even though an entity component is unlikely to ever exist */
                (*suscription.second.execute)(Handle(this), data);
            } else {
                Handle component(getByType(suscription.second.compType));
                if (component.isValid()) {
                    (*suscription.second.execute)(component, data);
                }
            }
        }
    }
}

void Entity::add(Handle h)
{
    assert(h.isValid()
        || utils::fatal("Handle must be valid\n"));
    Handle::typeId_t type = h.getType();
    assert(!components[type].isValid()
        || utils::fatal("Entity has already a handle of type %d (%s)\n", type, h.getTypeName()));
    assert(!h.getOwner().isValid()
        || utils::fatal("Handle %08x is already owned by %08x\n", h.getRaw(), h.getOwner().getRaw()));
    components[type] = h;
    h.setOwner(Handle(this));
    assert(h.getOwner() == Handle(this));
}

EntityListManager::KeyGen* EntityListManager::keyGen = nullptr;
EntityListManager::container_t EntityListManager::lists = EntityListManager::container_t();

Handle CName::get(const std::string& str)
{
    for (CName* name : *getManager<CName>()) {
        if (name->getName() == str) {return name;}
    }
    return Handle();
}

std::string CName::generate(const std::string& str)
{
    unsigned count=0;
    getManager<CName>()->forall<void>([str, &count] (CName* name) {
        if (name->getName().compare(0, str.length(), str) == 0) {count++;}
        });
    std::stringstream ss;
    std::string name = "";
    do {
        ss.str("");
        ss << str << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << count;
        name = ss.str();
        count++;
    } while (get(name).isValid());
    return name;
}

void CName::loadFromProperties(std::string, utils::MKeyValue atts)
{
    if (atts.has("name")) {
        setName(atts.getString("name"));
    } else if (atts.has("generate")) {
        setName(generate(atts.getString("generate")));
    } else {
        setName(((Entity*) Handle(this).getOwner())->getName());
    }
}

}