#ifndef RENDER_MATERIAL_H_
#define RENDER_MATERIAL_H_

#include <algorithm>

#include "texture.h"
#include "../shader/shaders.h"

#include "components/color.h"

#include "utils/itemsByName.h"
#include "utils/XMLParser.h"

namespace render {

class Technique;

class Material : private utils::XMLParser {
    public:
        typedef utils::ItemsByName< Material > Manager;
        static inline Manager& getManager() {return manager;}
        static inline void updateAnimatedMaterials(float elapsed) {
            for (const auto& mat : animatedMaterials) {
                if (mat != nullptr) {
                    mat->animatedMaterialData.update(elapsed);
                }
            }
        }

    private:
        static Manager manager;
        static std::vector<Material*> animatedMaterials;

    private:
#ifdef _DEBUG
        std::string diffuseTexName;
        std::string normalsTexName;
        std::string selfIllTexName;
        bool debug;
    public:
        inline bool hasDebug() const {return debug;}
    private:
#endif

        struct animatedMaterialData_t {
            float frameRate = 0;
            float currentElapsed = 0;

            inline void update(float elapsed) {
                currentElapsed += elapsed;
            }
        } animatedMaterialData;
        
        struct animatedTexture_t {
            std::vector<const Texture*> frames;
            const Texture* get(const animatedMaterialData_t& data) const;
            const Texture* getFrame(unsigned n) const;
        };

        struct texture_t {
            enum type_e {REGULAR, ANIMATED, RANDOM} type = REGULAR;
            bool valid = false;
            //This would be an union if VS2013 actually followed C++11
            const Texture* texture = nullptr;
            animatedTexture_t animatedTexture;

            texture_t() {texture=nullptr;}

            inline const Texture* get(
                const animatedMaterialData_t& animData, unsigned materialRand) const {
                switch(type) {
                    case ANIMATED: return animatedTexture.get(animData); break;
                    case RANDOM: return animatedTexture.getFrame(materialRand); break;
                    default: return texture;
                }
            }

            inline void activate(const animatedMaterialData_t& animData,
                unsigned materialRand, unsigned slot) const {
                const Texture* tex = get(animData, materialRand);
                assert(tex != nullptr);
                tex->activate(slot);
            }
            void load(Material* self, const std::string& name);
        };
            
        texture_t diffuse;
        texture_t normals;
        texture_t selfIll;
        bool random = false;

        const Technique* tech = nullptr;
        bool shadows = true;
        bool glass   = false;
        bool alphaAsSpecular = false;
        float baseSpecular = 0.10f;
        float diffuse_selfIllumination = 0;
        float paint_clamp = 1;
        component::Color tint=0;
        component::Color siTint=0;
        void onStartElement(const std::string &elem, utils::MKeyValue &atts);

    public:
        ~Material() {utils::eraseAll(animatedMaterials, this);}

        bool load(const char* name);

        void activateTextures(unsigned materialRand=0) const;
        const Technique* getTechnique() const {return tech;}

        inline bool castsShadows() const {return shadows;}
        inline bool isGlass() const {return glass;}
        inline bool usesAlphaAsSpecular() const {return alphaAsSpecular;}
        inline component::Color getTint() const {return tint;}
        inline component::Color getSelfIlluminationTint() const {return siTint;}
        inline float getDiffuseAsIlluminationFactor() const {return diffuse_selfIllumination;}
        inline float getBaseSpecular() const {return baseSpecular;}
        inline float getPaintClamp() const {return paint_clamp;}
        inline bool isRandom() const {return random;}

        inline const Texture* getDiffuse(unsigned materialRand=0) const {
            return diffuse.get(animatedMaterialData, materialRand);
        }
        inline const Texture* getNormals(unsigned materialRand=0) const {
            return normals.get(animatedMaterialData, materialRand);
        }
        inline const Texture* getSelfIllumination(unsigned materialRand=0) const {
            return selfIll.get(animatedMaterialData, materialRand);
        }
};

}
#endif
