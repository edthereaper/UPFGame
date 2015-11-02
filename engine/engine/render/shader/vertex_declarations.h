#ifndef INC_VERTEX_DECLARATIONS_H_
#define INC_VERTEX_DECLARATIONS_H_

#include "components/color.h"

namespace render {

class VertexDecl {
    public:
        enum eVertexTypes {
            POSITIONS = 1001,
            POSITION_UV,
            POSITION_COLOR,
            POSITION_UV_NORMAL,
            POSITION_UV_NORMAL_TANGENT,
            SKIN,

            POSITION_PUNT_INSTANCED,
        };
    public:
        D3D11_INPUT_ELEMENT_DESC* elems = nullptr;
        UINT                      nelems = 0;
        unsigned getBytesPerVertex(unsigned stream = -1) const;
        VertexDecl(D3D11_INPUT_ELEMENT_DESC* elems, UINT nelems) :
            elems(elems), nelems(nelems) {}
};

template<class TVertex>
VertexDecl* getVertexDecl();

// Vertex types
struct VertexPos {
    XMFLOAT3 Pos;
    VertexPos()=default;
    VertexPos(XMFLOAT3 Pos) : Pos(Pos){}
};
struct VertexPosColor {
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
    VertexPosColor()=default;
    VertexPosColor(XMFLOAT3 Pos, XMFLOAT4 Color=XMFLOAT4(1,1,1,1)) : Pos(Pos), Color(Color) {}
};
struct VertexPosUV {
    XMFLOAT3 Pos;
    XMFLOAT2 UV;
    VertexPosUV()=default;
    VertexPosUV(XMFLOAT3 Pos, XMFLOAT2 UV = XMFLOAT2(0,0)) : Pos(Pos), UV(UV) {}
};
struct VertexPosUVNormal {
    XMFLOAT3 Pos;
    XMFLOAT2 UV;
    XMFLOAT3 Normal;
    VertexPosUVNormal()=default;
    VertexPosUVNormal(XMFLOAT3 Pos, XMFLOAT2 UV = XMFLOAT2(0,0), XMFLOAT3 Normal = XMFLOAT3(0,1,0)) :
        Pos(Pos), UV(UV), Normal(Normal) {}
};
struct VertexPosUVNormalTangent {
    XMFLOAT3 Pos;
    XMFLOAT2 UV;
    XMFLOAT3 Normal;
    XMFLOAT4 Tangent;

    VertexPosUVNormalTangent()=default;
    VertexPosUVNormalTangent(XMFLOAT3 Pos, XMFLOAT2 UV = XMFLOAT2(0,0),
        XMFLOAT3 Normal = XMFLOAT3(0,1,0), XMFLOAT4 Tangent = XMFLOAT4(0,0,0,0)) :
        Pos(Pos), UV(UV), Normal(Normal), Tangent(Tangent) {}
};
struct VertexSkin {
    XMFLOAT3 Pos;
    XMFLOAT2 UV;
    XMFLOAT3 Normal;
    XMFLOAT4 Tangent;
    uint8_t BoneIds[4];
    uint8_t Weights[4];
};

//Instanced Vertex types
struct VertexPUNTInstance {
    typedef VertexPosUVNormalTangent vertex;
    struct instance_t {
        public:
            typedef DirectX::XMMATRIX world_t;

            world_t world;
        private:
            uint32_t tint = component::Color(0);
            uint32_t selfIllumination = component::Color(0);
        public:
            uint16_t userDataA = ~0;
            uint16_t userDataB = ~0;
            float padding;

            inline component::Color getTint() const {
                return component::Color(tint).abgr();
            }
            inline component::Color getSelfIllumination() const {
                return component::Color(selfIllumination).abgr();
            }
            inline void setTint(component::Color c) {
                tint = component::Color(c).abgr();
            }
            inline void setSelfIllumination(component::Color c) {
                selfIllumination = component::Color(c).abgr();
            }

            instance_t()=default;
            instance_t(world_t world,
                component::Color tint = component::Color(0),
                component::Color sill = component::Color(0)) :
                world(world),
                tint(component::Color(tint).abgr()),
                selfIllumination(component::Color(sill).abgr())
                {}

            inline bool operator==(const instance_t& b) const {
                return world == b.world &&
                    tint == b.tint &&
                    selfIllumination == b.selfIllumination &&
                    userDataB == b.userDataB;
            }
            inline bool operator==(const instance_t& b) {
                return world == b.world &&
                    tint == b.tint &&
                    selfIllumination == b.selfIllumination &&
                    userDataB == b.userDataB;
            }
    };
};

struct VertexParticleUData {
    typedef VertexPosUV vertex;
    public:
	    XMFLOAT3 pos;
        float qz=1, qw=1;
    private:
        uint32_t colorA = 0;
        uint32_t colorB = 0;
    public:
	    float colorWeight = 0;
	    float frame = 0;
	    float scale = 1;
        uint16_t dead = 0;
        uint16_t index = 0;

	    inline void setAngle(float angle_){
		    qz = std::sin(angle_ / 2);
		    qw = std::cos(angle_ / 2);
	    }
	    inline float getAngle(){return std::acos(qw) * 2;}

        inline void setColorA(const component::Color& c){colorA = component::Color(c).abgr();}
        inline void setColorB(const component::Color& c){colorB = component::Color(c).abgr();}
	    inline component::Color getColorA() const { return component::Color(colorA).abgr();}
	    inline component::Color getColorB() const { return component::Color(colorB).abgr();}
};

struct VertexPaintData {
    typedef VertexPosColor vertex;
    struct instance_t {
        public:
            XMFLOAT3 pos;
            float radius = 0.f;
            
            inline XMVECTOR getPos() const {return DirectX::XMLoadFloat3(&pos);}
            inline void setPos(const XMVECTOR& v) {DirectX::XMStoreFloat3(&pos, v);}
            inline void set(const XMVECTOR& v, float r) {
                setPos(v);
                radius = r;
            }
    };
};

struct VertexFlowerData {
    typedef VertexPosUV vertex;
    struct instance_t {
        public:
            XMFLOAT3 pos;
            uint16_t user=0;
            int16_t frame=0;
            XMFLOAT2 sca = XMFLOAT2(1,1);
            float life = 0;
            
            inline XMVECTOR getPos() const {return DirectX::XMLoadFloat3(&pos);}
            inline void setPos(const XMVECTOR& v) {DirectX::XMStoreFloat3(&pos, v);}

            instance_t()=default;
            instance_t(const XMVECTOR& v, uint16_t user, int16_t frame,
                const XMFLOAT2& sca = XMFLOAT2(1,1)) :
                user(user), frame(frame), sca(sca) {
                setPos(v);
            }
    };
};

}

#endif
