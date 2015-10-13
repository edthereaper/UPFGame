#include "mcv_platform.h"
#include "prefab.h"
#include "importXML.h"

using namespace utils;

namespace component {

PrefabManager PrefabManager::instance = PrefabManager();

bool PrefabManager::prefabricateComponents(const char* name, Handle currentEntity, bool reload)
{
    bool had = reload && has(name);
    auto def = getByName(name);
    if (had) {def->load(name);}
    if(def == nullptr) {
        return false;
    } else {
        Importer importer;
        importer.setParsingPrefabs();
        importer.setCurrentEntity(currentEntity);
        std::istringstream istr(def->xml);
        bool is_ok = importer.xmlParseStream(istr, name);
        assert(is_ok);
        return true;
    }
}

Handle PrefabManager::prefabricate(const char* name, bool reload)
{
    bool had = reload && has(name);
    auto def = getByName(name);
    if (had) {def->load(name);}
    assert(def != nullptr);
    
    Importer importer;
    importer.setParsingPrefabs();
    std::istringstream istr(def->xml);
    bool is_ok = importer.xmlParseStream(istr, name);
    assert(is_ok);

    return importer.getRootHandle();
}

bool PrefabDefinition::load(const char* name)
{
    char full_name[MAX_PATH];
    sprintf(full_name, "%s/%s.xml", "data/prefabs", name);

    FileDataProvider fdp(full_name);
    if (!fdp.isValid()) {return false;}

    size_t file_size = fdp.getFileSize();

    char* buf = new char[file_size + 1];
    
    buf[file_size] = 0x0;
    fdp.read(buf, file_size);
    xml = std::string(buf);
    delete[] buf;

    return true;
}

}