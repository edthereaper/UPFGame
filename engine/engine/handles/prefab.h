#ifndef COMPONENT_PREFAB_H_
#define COMPONENT_PREFAB_H_

#include "mcv_platform.h"
#include "utils/itemsByName.h"
#include "handle.h"

namespace component {

class PrefabManager;

/* Holds the xml file with the prefab data */
class PrefabDefinition {
    private:
        std::string name;
        std::string xml;
    public:
      bool load(const char* name);
      inline void setName(const char *new_name) { name = new_name; }
      inline const std::string& getName() const { return name; }
    public:
      friend PrefabManager;
};

class PrefabManager : public utils::ItemsByName<PrefabDefinition> {
    private:
        PrefabManager(){}
        static PrefabManager instance;
    public:
        static PrefabManager& get() {return instance;}
        Handle prefabricate(const char* name, bool reload = false);
        bool prefabricateComponents(const char* name, Handle currentEntity, bool reload = false);
};

}

#endif