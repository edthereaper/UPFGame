#ifdef GAME_ELEMENTS_DESTRUCTIBLE_H
#endif GAME_ELEMENTS_DESTRUCTIBLE_H

#include "mcv_platform.h"
#include "render/mesh/mesh.h"
#include "components/transform.h"
#include "gameMsgs.h"

namespace gameElements {

class CDestructible {
    private:
        render::Mesh* mesh = nullptr;
        XMVECTOR boxSize = utils::one_v;

    public:
        ~CDestructible();
        inline void init(){}
        inline void update(float elapsed){}
        inline void loadFromProperties(std::string, utils::MKeyValue) {}
        
        void createBox(XMVECTOR size);
        void breakGlass();
};


class CDestructibleRestorer : public component::Transform {
    public:
        friend CDestructible;
    private:
        XMVECTOR boxSize = utils::one_v;
    public:
        inline void init(){}
        inline void update(float elapsed) {}
        inline void loadFromProperties(std::string, utils::MKeyValue) {}
        void revive();
};

}