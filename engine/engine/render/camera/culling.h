#ifndef RENDER_CULLING_H_
#define RENDER_CULLING_H_

#include "mcv_platform.h"

#include <bitset>

#include "components/AABB.h"
#include "components/color.h"
#include "components/transform.h"

#include "render/mesh/component.h"

namespace render {

struct culling_t
{
    public:
        static component::AABB bakeCulling(const component::AABB&, const XMMATRIX&);
        static void draw(const component::AABB&, const component::Color& color);
    protected:
        bool dirty = true;
        float skin=0, scale=1;
        
    public:
        culling_t()=default;
        culling_t(const culling_t&)=default;
        inline void setScale(float s) {scale = s; dirty=true; }
        inline float getScale() const {return scale;}
        inline void setSkin(float s) {skin = s; dirty=true;}
        inline float getSkin() const {return skin;}
        
        inline void setDirty() {dirty = true;}
};

struct CullingAABB : public component::AABB, public culling_t {
    private:
        XMMATRIX prevWorld;
#ifdef _DEBUG
    public:
        component::Color dbgColor = component::ColorHSL(utils::rand_uniform(1.f));
#endif

    public:
        void update(const AABB& aabb, const XMMATRIX& world, bool ignoreWorldChange);
        
        CullingAABB() = default;
        CullingAABB(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) : AABB(min, max) {}
        inline operator AABB() const {
            AABB aabb;
            aabb.setCenter(center);
            aabb.setHSize(hSize);
            return aabb;
        }

        inline bool operator==(const AABB& b) const {
            return center == b.getCenter() && hSize == b.getHSize();
        }
        
        inline void operator=(const AABB& b) {AABB::operator=(b);}
        inline void operator=(AABB&& b) {AABB::operator=(b);}

        inline void draw(const component::Color& c) const {culling_t::draw(*this, c);}
        inline void draw() const {draw(
#ifdef _DEBUG
            dbgColor
#else
            component::Color::CYAN
#endif
            );
        }
};

class CCullingAABBSpecial : public component::AABB, public culling_t {
    public:
        enum type_e {
            UNDEFINED,
            LIGHT_PT,
            LIGHT_DIR,
            LIGHT_VOL,
        };
    private:
        component::Transform prevTransform;
        type_e type = UNDEFINED;
    
    public:
        inline void init() {}
        void update(float);
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        
        CCullingAABBSpecial() = default;
        CCullingAABBSpecial(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max,
            type_e type = UNDEFINED) : AABB(min, max), type(type) {}
        inline operator AABB() const {
            AABB aabb;
            aabb.setCenter(center);
            aabb.setHSize(hSize);
            return aabb;
        }
        
        inline void operator=(const AABB& b) {AABB::operator=(b);}
        inline void operator=(AABB&& b) {AABB::operator=(b);}

        inline bool operator==(const AABB& b) const {
            return center == b.getCenter() && hSize == b.getHSize();
        }

        inline void draw(const component::Color& c) const {culling_t::draw(*this, c);}
        void draw() const;
};

class CCullingAABB : public component::AABB, public culling_t {
    private:
        component::Transform prevTransform;

#ifdef _DEBUG
    public:
        component::Color dbgColor = component::ColorHSL(utils::rand_uniform(1.f));
#endif
    
    public:
        inline void init() {}
        void update(float);
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        
        CCullingAABB() = default;
        CCullingAABB(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) : AABB(min, max) {}
        inline operator AABB() const {
            AABB aabb;
            aabb.setCenter(center);
            aabb.setHSize(hSize);
            return aabb;
        }
        
        inline void operator=(const AABB& b) {AABB::operator=(b);}
        inline void operator=(AABB&& b) {AABB::operator=(b);}

        inline bool operator==(const AABB& b) const {
            return center == b.getCenter() && hSize == b.getHSize();
        }

        inline void draw(const component::Color& c) const {
            CMesh* mesh = component::Handle(this).getBrother<CMesh>();
            if (mesh!=nullptr && mesh->isVisible()) {
                culling_t::draw(*this, c);
            }
        }
        inline void draw() const {draw(
#ifdef _DEBUG
            dbgColor
#else
            component::Color::YELLOW
#endif
            );
        }
};

class CCulling;
class CCullingCube;

class Culling {
    public:
        static const size_t MAX_CULLERS = 512;
        typedef std::bitset<MAX_CULLERS> mask_t;

        enum cullDirection_e {
            NORMAL=-1,
            XPOS=0, XNEG=1,
            YPOS=2, YNEG=3,
            ZNEG=4, ZPOS=5,
        };

        /*
            Creates a delegate that smartly behaves as a CCulling or
            a CCullingCube depending on the type parameter.

            Internally, it uses pointers instead of handles, so it
            is invalidated by creations and destructions of new elements
            of either type
        */
        class CullerDelegate {
            private:
                const void* culler;
                cullDirection_e type;

            public:
                CullerDelegate(component::Handle e_h, cullDirection_e type);

                bool hasChanged() const;
                bool contains(XMVECTOR point)const;
                bool cull(const component::AABB&) const;
                mask_t getMask() const;

                bool operator==(const CullerDelegate& b) const{
                    return culler == b.culler && type == b.type;
                }
        };
    protected:
        typedef XMVECTOR planes_t[6];
        Culling()=default;
    private:
        typedef std::vector<CullerDelegate> cullers_t;
        static cullers_t cullers;
        static unsigned currentCuller;
        static bool cullerListChanged;

        static mask_t addDelegate(CullerDelegate&& del);

    protected:
        static inline mask_t addCCulling(component::Handle e_h) {
            return addDelegate(CullerDelegate(e_h, NORMAL));
        }
        static inline mask_t addCCullingCube(component::Handle e_h, cullDirection_e dir){
            return addDelegate(CullerDelegate(e_h, dir));
        }

    public:
        static inline void rewindCullers() {
            currentCuller = 0;
            cullerListChanged = false;
        }
        static inline bool hasCullerListChanged() {return cullerListChanged;}
        static inline utils::range_t<cullers_t::const_iterator> iterateCullers() {
            return utils::range_t<cullers_t::const_iterator>(cullers.begin(), cullers.end());
        }

};

class CCulling : public Culling {
    private:
        XMMATRIX previousViewProjection;
        planes_t planes;
        mask_t mask;
        bool changed = true;

    public:
        inline void init() {}
        inline void loadFromProperties(const std::string& what, const utils::MKeyValue&) {}

        void update(float);
        bool contains(XMVECTOR point)const;
        bool cull(const component::AABB&) const;
        const mask_t& getMask() const {return mask;}
        
        inline bool hasChanged() const {return changed;}
};

class CCullingCube : public Culling {
    private:
        XMMATRIX previousViewProjection[6];
        planes_t planes[6];
        mask_t mask[6];
        bool changed = true;

    public:
        inline void init() {}
        inline void loadFromProperties(const std::string& what, const utils::MKeyValue&) {}

        void update(float);
        bool contains(XMVECTOR point, Culling::cullDirection_e dir) const;
        bool cull(const component::AABB&, Culling::cullDirection_e dir) const;
        inline const mask_t& getMask(Culling::cullDirection_e dir) const {return mask[dir];}
        inline bool hasChanged() const {return changed;}
};

}

#endif