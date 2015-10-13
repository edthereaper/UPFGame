#ifdef COMPONENTS_WHITEBOX_H
#endif COMPONENTS_WHITEBOX_H

#include "mcv_platform.h"
#include "render/mesh/mesh.h"
#include "components/color.h"

namespace gameElements {

class CWhiteBox {
    private:
        render::Mesh* mesh = nullptr;

    public:
        ~CWhiteBox();
        
        void createBox(XMVECTOR size, component::Color tint);

        inline void init(){}
        inline void update(float) {}
        inline void loadFromProperties(std::string, utils::MKeyValue) {}
};

}