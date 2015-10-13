#ifndef RENDER_CSKYBOX_H_
#define RENDER_CSKYBOX_H_

#include "textureCube.h"
#include "handles/handle.h"
#include "components/transform.h"
#include "components/color.h"
#include "../mesh/mesh.h"
#include "../render_utils.h"

namespace render {

/*
 COMPONENT SKYBOX
 Must have brothers:
    CTransform
 Optionally:
    CTint
 */
class CSkyBox {
    public:
        static const unsigned RADIUS = 220;
        static const unsigned LATITUDES = 10;
        static const unsigned LONGITUDES = 10;
    private:
        Mesh* sphere        = nullptr;
        TextureCube* blendA = nullptr;
        TextureCube* blendB = nullptr;
    public:
        CSkyBox() = default;
        ~CSkyBox() {
            SAFE_DELETE(sphere);
            if (blendA != nullptr) {
                blendA->destroy();
                delete blendA;
                blendA = nullptr;
            }
            if (blendB != nullptr) {
                blendB->destroy();
                delete blendB;
                blendB = nullptr;
            }
        }

        inline void setTextureA(TextureCube* cube) {blendA = cube;}
        inline void setTextureB(TextureCube* cube) {blendB = cube;}
        
        inline void setTextureA(std::string name) {
            blendA = new TextureCube();
            blendA->load(name.c_str());
        }
        inline void setTextureB(std::string name) {
            blendB = new TextureCube();
            blendB->load(name.c_str());
        }
        
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
        inline void update(float elapsed) {}
        inline void init() {
            sphere = new Mesh();
            createSphere(*sphere, float(RADIUS), LATITUDES, LONGITUDES);
        }

        //Assumes skybox technique and ZCFG_LESS_EQUAL zConfigs are already activated
        inline void render() {
            if (sphere == nullptr || blendA == nullptr || blendB == nullptr) {return;} 
            component::Handle h(this);
            component::CTransform* transf(h.getBrother<component::CTransform>());
            CTransform skyT;
            skyT.setPosition(transf->getPosition());
            CTint tint = h.hasBrother<component::CTint>() ?
                *((CTint*)h.getBrother<component::CTint>()) : 0;
            CSelfIllumination selfIllumination = h.hasBrother<component::CSelfIllumination>() ?
                *((CSelfIllumination*)h.getBrother<component::CSelfIllumination>()) : 0;
            setObjectConstants(skyT.getWorld(), tint, selfIllumination);
            blendA->activate(32);
            blendB->activate(33);
            sphere->activateAndRender();
        }
};

}

#endif