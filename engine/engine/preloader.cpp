#include "mcv_platform.h"
#include "preloader.h"

using namespace render;
using namespace animation;

Mesh::Manager& Preloader::mesh = Mesh::getManager();
Texture::Manager& Preloader::texture = Texture::getManager();
Material::Manager& Preloader::material = Material::getManager();
Technique::Manager& Preloader::technique = Technique::getManager();
AnimationArchetype::Manager& Preloader::animationArchetype = AnimationArchetype::getManager();
CoreModel::Manager& Preloader::coreModel = CoreModel::getManager();

void Preloader::onStartElement(const std::string &elem, utils::MKeyValue &atts)
{
    if (elem == "mesh") {
        if (atts.has("name")){mesh.getByName(atts["name"].c_str());}
    } else if (elem == "texture") {
        if (atts.has("name")){texture.getByName(atts["name"].c_str());}
    } else if (elem == "material") {
        if (atts.has("name")){material.getByName(atts["name"].c_str());}
    } else if (elem == "technique") {
        if (atts.has("name")){technique.getByName(atts["name"].c_str());}
    } else if (elem == "animationArchetype") {
        if (atts.has("name")){animationArchetype.getByName(atts["name"].c_str());}
    } else if (elem == "coreModel") {
        if (atts.has("name")){coreModel.getByName(atts["name"].c_str());}
    }
}

bool Preloader::preloadFile(std::string file)
{
    Preloader preloader;
    bool ok = preloader.xmlParseFile(file);
    if (!ok) {
        dbg("Preloader error (%s) : %s", file.c_str(), preloader.getXMLError());
    }
    return ok;
}
