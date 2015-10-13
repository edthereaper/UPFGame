#ifndef PRELOAD_H_
#define PRELOAD_H_

#include "utils/XMLParser.h"

#include "render/mesh/mesh.h"
#include "render/texture/material.h"
#include "render/texture/texture.h"
#include "render/texture/tilemap.h"
#include "render/shader/shaders.h"
#include "animation/animationPlugger.h"
#include "animation/skeleton_manager.h"

class Preloader : private utils::XMLParser {
    private:
        Preloader()=default;
        static render::Mesh::Manager& mesh;
        static render::Texture::Manager& texture;
        static render::Material::Manager& material;
        static render::Technique::Manager& technique;
        static animation::AnimationArchetype::Manager& animationArchetype;
        static animation::CoreModel::Manager& coreModel;

        void onStartElement(const std::string &elem, utils::MKeyValue &atts);
    public:
        static bool preloadFile(std::string file);
        static inline bool preload(std::string name) {
            return preloadFile("data/preload/"+name+".xml");
        }

};

#endif