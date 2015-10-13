#include "mcv_platform.h"
#include "importXML.h"

#include "handle.h"
#include "handleManager.h"
#include "entity.h"
#include "prefab.h"

using namespace utils;

namespace component {

void Importer::onStartElement(const std::string &elem, MKeyValue &atts)
{
    // If we are inside a component, send the xml tag to the component
    if (currentComponent.isValid()) {
        currentComponent.loadFromProperties(elem, atts);
        return;
    }

    if (currentEntity.isValid() && elem == "Entity") {
        assert(!"Entities can't be loaded as components.");
        return;
    }

    if (currentEntity.isValid() && elem == "abort") {
        dbg("Entity aborted - (%s).\n", atts.has("reason")?atts["reason"].c_str():"unknown");
        currentEntity.destroy();
    }

    if(currentEntity.isValid() && elem == "prefab") {
        std::string name = atts["name"].c_str();
        bool is_ok = PrefabManager::get().prefabricateComponents(atts["name"].c_str(), currentEntity);
        if (is_ok) {
            return;
        } else {
            dbg("Failed to prefabricate components from %s\n.", name.c_str());
            if (atts.has("default")) {
                name = atts["default"];
                dbg("\t- Trying default %s.\n", name.c_str());
                is_ok = PrefabManager::get().prefabricateComponents(name.c_str(), currentEntity);
                if (!is_ok) {
                    dbg("\t- Failed to load prefab default %s.\n", name.c_str());
                    return;
                } else {
                    return;
                }
            }
            return;
        }
        
    }

    // If not, check if it's a new component type
	HandleManager* hm = HandleManager::mRegister.get(elem.c_str());
	if (!hm) {
#ifdef _DEBUG
        if (elem[0] >= 'A' && elem[0] <= 'Z') {
            //All components start with uppercase. All noncompontent tags would start with lowercase
	  	    dbg("Unknown component %s. (C%s?)\n", elem.c_str(), elem.c_str());
        }
#endif
	   	return;
	}

    bool reusingComponent = false;

    Handle h;
    if (elem == "Entity" && atts.has("prefab")) {
        h = PrefabManager::get().prefabricate(atts["prefab"].c_str());
    } else {
        if (currentEntity.isValid()) {
            Entity* e = currentEntity;
            Handle existingComponent = e->getByType(hm->getType());
            if (existingComponent.isValid()) {
                reusingComponent = true;
                h = existingComponent;
            }
        }

        // If we are not reusing an existing component of the entity, create a new one
        if (!reusingComponent) {
            h = hm->createObj();
        }
    }
    if (elem != "Entity") {
        assert(currentEntity.isValid());
        Entity* e = currentEntity;
        if (!reusingComponent) {
            e->add(h);
        }
        currentComponent = h;
    } else {
        currentEntity = h;
        initEntity = atts.getBool("init");
    }

    if(!rootHandle.isValid()) {rootHandle = h;}

    h.loadFromProperties( elem, atts );
}

void Importer::onEndElement(const std::string &elem) {
	if (elem == "Entity") {
        if (!prefabs) {
            auto key(EntityListManager::getKey(currentEntity));
            if (key != EntityListManager::INVALID_KEY) {
                EntityListManager::get(key).add(currentEntity);
            }
        }
        if (initEntity) {
            ((Entity*) currentEntity)->init();
            initEntity = false;
        }
        currentEntity = Handle();
    } else if (elem == "abort") {
        currentEntity = Handle();
        initEntity = false;
        currentComponent = Handle();
    } else {
        if (currentComponent.getTypeName() == elem) {
            currentComponent = Handle();
        }
    }
}

}