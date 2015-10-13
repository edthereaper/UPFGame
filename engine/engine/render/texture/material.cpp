#include "mcv_platform.h"
#include "material.h"

#include "utils/data_provider.h"
using namespace utils;

namespace render {

Material::Manager Material::manager;

std::vector<Material*> Material::animatedMaterials;

void Material::texture_t::load(Material* self, const std::string& name)
{
    if (DirLister::isDir(Texture::PATH+"/"+name)) {
        if (type != RANDOM) {
            type = ANIMATED;
        }
        texture = nullptr;
        valid = true;

        auto v = DirLister::listDir(Texture::PATH+"/"+name);
        std::sort(v.begin(), v.end());
        animatedTexture.frames.reserve(v.size());
        for(const auto& frameName : v) {
            std::string filenameFile = frameName.substr(0, frameName.rfind('.'));
            std::string filename = name+"/"+filenameFile;
            auto tex = Texture::getManager().getByName(filename);
            assert(tex != nullptr);
            animatedTexture.frames.push_back(tex);
        }
        if (std::find(animatedMaterials.begin(), animatedMaterials.end(), self) == animatedMaterials.end()) {
            animatedMaterials.push_back(self);
        }
    } else {
        type = REGULAR;
        texture = Texture::getManager().getByName(name);
        valid = texture != nullptr;

        animatedTexture.frames.clear();
    }
}

//An std::string that is only declared in Release (otherwise declared elsewhere)
#ifdef _DEBUG
#define STR_RELEASE
#else
#define STR_RELEASE std::string
#endif

void Material::onStartElement(const std::string &elem, MKeyValue &atts)
{
    if (elem == "std_material") {
        random = atts.getBool("random");

        if (atts.has("diffuse")) {
            STR_RELEASE diffuseTexName = atts.getString("diffuse", "white");
            if (random) {diffuse.type = texture_t::RANDOM ;}
            diffuse.load(this, diffuseTexName);
        }
        if (atts.has("normal")) {
            STR_RELEASE normalsTexName = atts.getString("normal", "identity");
            if (random) {normals.type = texture_t::RANDOM ;}
            normals.load(this, normalsTexName);
        }
        if (atts.has("selfIllumination")) {
            STR_RELEASE selfIllTexName = atts.getString("selfIllumination", "black");
            if (random) {selfIll.type = texture_t::RANDOM ;}
            selfIll.load(this, selfIllTexName);
        }
        std::string tech_name = atts.getString("tech", "deferred_gbuffer");
        tech = Technique::getManager().getByName(tech_name);

        shadows = atts.getBool("shadows", shadows);
        glass = atts.getBool("glass", glass);
        alphaAsSpecular = atts.getBool("alphaAsSpecular", alphaAsSpecular);
        baseSpecular = atts.getFloat("baseSpecular", baseSpecular);
        diffuse_selfIllumination = atts.getFloat("diffuse_selfIllumination", diffuse_selfIllumination);
        siTint = atts.getHex("siTint", siTint);
        tint = atts.getHex("tint", tint);
        animatedMaterialData.frameRate = atts.getFloat("animFPS", animatedMaterialData.frameRate);
        paint_clamp = atts.getFloat("paint_clamp", paint_clamp);
        
#ifdef _DEBUG
        debug = atts.getBool("debug", debug);
#endif
        assert(tech != nullptr);
    }
}

void Material::activateTextures(unsigned materialRand) const
{
    if (diffuse.valid) {
        diffuse.activate(animatedMaterialData, materialRand, 0);
    } else {
        Texture::getManager().getByName("white")->activate(0);
    }

    if (normals.valid) {
        normals.activate(animatedMaterialData, materialRand, 1);
    } else {
        Texture::getManager().getByName("Identity")->activate(1);
    }
    
    if (selfIll.valid) {
        selfIll.activate(animatedMaterialData, materialRand, 4);
    } else {
        Texture::getManager().getByName("Black")->activate(4);
    }
}



bool Material::load(const char* name)
{
    char full_name[MAX_PATH];
    sprintf(full_name, "%s/%s.xml", "data/materials", name);
    return xmlParseFile(full_name);
}

const Texture* Material::animatedTexture_t::get(const animatedMaterialData_t& data) const
{
    auto size = frames.size();
    assert(size > 0);
    unsigned frame = unsigned(std::floor(data.currentElapsed * data.frameRate)) % size;
    assert (frame < size);
    return frames[frame];
}

const Texture* Material::animatedTexture_t::getFrame(unsigned f) const
{
    auto size = frames.size();
    assert(size > 0);
    unsigned frame = f % size;
    assert (frame < size);
    return frames[frame];
}



}