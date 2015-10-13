#ifndef COMPONENT_TINT_H_
#define COMPONENT_TINT_H_
#include "mcv_platform.h"
namespace component {

//Color in RGB space
struct Color {
    //Grayscale
    static const Color WHITE;
    static const Color LIGHT_GRAY;
    static const Color GRAY;
    static const Color DARK_GRAY;
    static const Color BLACK;

    //Primary additive
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;

    //Secondary additive (primary substractive)
    static const Color CYAN;
    static const Color YELLOW;
    static const Color MAGENTA;

    //Mixes
    static const Color ORANGE;
    static const Color PINK;
    static const Color LIME;
    static const Color SPRING_GREEN;
    static const Color PURPLE;
    static const Color CERULEAN;

    //Other
    static const Color AQUA;
    static const Color LEMON_LIME;
    static const Color BEIGE;
    static const Color BROWN;
    static const Color CELADON;
    static const Color VIOLET;
    static const Color STEEL_BLUE;
    static const Color BURGUNDY;
    static const Color PALE_PINK;
    static const Color MOUNTAIN_GREEN;
    static const Color TEAL;
    static const Color MINT;

    uint32_t rgba = 0;

    inline Color& setR(uint8_t v) {rgba&=~(0xFF<<24); rgba|=v<<24; return *this;}
    inline Color& setG(uint8_t v) {rgba&=~(0xFF<<16); rgba|=v<<16; return *this;}
    inline Color& setB(uint8_t v) {rgba&=~(0xFF<< 8); rgba|=v<< 8; return *this;}
    inline Color& setA(uint8_t v) {rgba&=~(0xFF<< 0); rgba|=v<< 0; return *this;}
    inline Color& setRf(float v) {return setR(static_cast<uint8_t>(utils::inRange(0.f,v,1.f)*255.f));}
    inline Color& setGf(float v) {return setG(static_cast<uint8_t>(utils::inRange(0.f,v,1.f)*255.f));}
    inline Color& setBf(float v) {return setB(static_cast<uint8_t>(utils::inRange(0.f,v,1.f)*255.f));}
    inline Color& setAf(float v) {return setA(static_cast<uint8_t>(utils::inRange(0.f,v,1.f)*255.f));}

    inline uint8_t r() const {return (rgba&(0xFF<<24)) >> 24;}
    inline uint8_t g() const {return (rgba&(0xFF<<16)) >> 16;}
    inline uint8_t b() const {return (rgba&(0xFF<< 8)) >> 8;}
    inline uint8_t a() const {return (rgba&(0xFF<< 0)) >> 0;}

    inline float rf() const {return float(r())/255.f;}
    inline float gf() const {return float(g())/255.f;}
    inline float bf() const {return float(b())/255.f;}
    inline float af() const {return float(a())/255.f;}

	void toString(){
		utils::dbg("\tred=%f,\tgreen=%f,\tblue=%f,\alpha=%f\n");
	}

    //casts
    inline operator uint32_t() const {return rgba;}
    inline operator XMVECTOR() const {return DirectX::XMVectorSet(rf(), gf(), bf(), af());}
    inline operator XMFLOAT4() const {return XMFLOAT4(rf(), gf(), bf(), af());}

    //assignment
    inline Color& set(const uint32_t& c) {rgba=c; return *this;}
    inline Color& operator=(const uint32_t& c) { return set(c);}
    inline Color& set(const XMVECTOR& v) {
        return set(DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v),
            DirectX::XMVectorGetZ(v), DirectX::XMVectorGetW(v));
    }
    inline Color& set(const float& r, const float& g, const float& b, const float& a=1) {
        uint8_t nr = uint8_t(utils::inRange(0.f,r,1.f)*255);
        uint8_t ng = uint8_t(utils::inRange(0.f,g,1.f)*255);
        uint8_t nb = uint8_t(utils::inRange(0.f,b,1.f)*255);
        uint8_t na = uint8_t(utils::inRange(0.f,a,1.f)*255);
        rgba = (nr<<24)|(ng<<16)|(nb<<8)|na;
        return *this;
    }
    inline Color& set(const Color& c) {
        rgba = c.rgba;
        return *this;
    }
    inline Color& set(const float v[4]) {
        return set(v[0], v[1], v[2], v[3]);
    }
    inline Color& set(XMFLOAT4 v) {
        return set(v.x, v.y, v.z, v.w);
    }
    inline Color& operator=(const XMVECTOR& v) {return set(v);}
    
    Color blend(const Color& t, float f=0.5f);

    inline Color operator+(const Color& t) const {
        float a1 = af();
        float a2 = t.af();
        XMVECTOR c1 = *this;
        XMVECTOR c2 = t;
        DirectX::operator*=(c1, a1);
        DirectX::operator*=(c2, a2);
        return DirectX::operator+(c1,c2);
    }

    inline Color abgr() const {
        return
            ((rgba&0xFF000000) >> 24)|
            ((rgba&0x00FF0000) >> 8 )|
            ((rgba&0x0000FF00) << 8 )|
            ((rgba&0x000000FF) << 24);
    }

    Color()=default;
    inline Color(int tint) : rgba(tint) {}
    inline Color(uint32_t tint) : rgba(tint) {}
    inline Color(const XMVECTOR& v) {set(v);}
    inline Color(const float v[4]) {set(v);}
    inline Color(const float r, const float g, const float b, const float a) {
        set(r,g,b,a);
    }
    inline Color(const XMFLOAT4& v) {set(v);}

    inline Color factor(float f) {return set(rf()*f, gf()*f, bf()*f, af());}
    inline Color factor(float f) const {return Color(rf()*f, gf()*f, bf()*f, af());}
    inline Color add(float f) {return set(rf()+f, gf()+f, bf()+f, af());}

    /* Read from XML */
    void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

    inline Color premultiplyAlpha() {return factor(af()).setA(0xFF);}
    inline Color premultiplyAlpha() const {return factor(af()).setA(0xFF);}
};
using ColorRGB = Color;

//HSL space (normalized)
struct ColorHSL
{
    public:
        //Greyscale
        static const ColorHSL WHITE;
        static const ColorHSL LIGHT_GRAY;
        static const ColorHSL GRAY;
        static const ColorHSL DARK_GRAY;
        static const ColorHSL BLACK;

        //Hexacone corners
        static const ColorHSL RED;
        static const ColorHSL YELLOW;
        static const ColorHSL GREEN;
        static const ColorHSL CYAN;
        static const ColorHSL BLUE;
        static const ColorHSL MAGENTA;

    protected:
        float _h=0.f, _s=1.f, _l=0.5f, _a=1.f;
    public:
        ColorHSL(float nh=0.f, float ns=1.f, float nl=0.5f, float na=1.f) {
            h(nh); s(ns); l(nl); a(na);
        }
        ColorHSL(ColorRGB);
        operator ColorRGB() const;

        inline float h() const {return _h;}
        inline float l() const {return _l;}
        inline float s() const {return _s;}
        inline float a() const {return _a;}
        inline ColorHSL& h(float v) {_h = utils::inRange(0.f,v,1.f); return *this;}
        inline ColorHSL& l(float v) {_l = utils::inRange(0.f,v,1.f); return *this;}
        inline ColorHSL& s(float v) {_s = utils::inRange(0.f,v,1.f); return *this;}
        inline ColorHSL& a(float v) {_a = utils::inRange(0.f,v,1.f); return *this;}
};

/* A color
 * 
 * XML properties:
 *  color: the hex code 0xRRGGBBAA
 */
struct CTint : public ColorRGB {
    
    CTint()=default;
    CTint(int tint) : Color(tint) {}
    CTint(uint32_t tint) : Color(tint) {}
    CTint(unsigned long tint) : Color(static_cast<uint32_t>(tint)) {}
    CTint(XMVECTOR v) : Color(v) {}
    CTint(const float v[4]) : Color(v) {}

    inline void update(float elapsed) {}
    inline void init() {}
};

struct CSelfIllumination : public ColorRGB {

    CSelfIllumination()=default;
    CSelfIllumination(int tint) : Color(tint) {}
    CSelfIllumination(uint32_t tint) : Color(tint) {}
    CSelfIllumination(XMVECTOR v) : Color(v) {}
    CSelfIllumination(const float v[4]) : Color(v) {}

    inline void update(float elapsed) {}
    inline void init() {}
};

//RGBA as XMVECTOR
class ColorV {
    private:
        XMVECTOR v = utils::zero_v;

    public:
        ColorV()=default;

        ColorV(XMVECTOR v) : v(v) {}
        ColorV(float r, float g, float b, float  a) : v(DirectX::XMVectorSet(r,g,b,a)) {}
        ColorV(const Color& c) : v(c) {}

        inline operator XMVECTOR () const {return v;}
        inline operator Color() const {return v;}

        inline ColorV operator+(const ColorV& b) const {
            return DirectX::operator+(v, b.v);
        }
        inline ColorV operator-(const ColorV& b) const {
            return DirectX::operator-(v, b.v);
        }
        inline ColorV operator*(const float& b) const {
            return DirectX::operator*(v, b);
        }
        inline ColorV operator/(const float& b) const {
            return DirectX::operator/(v, b);
        }
        inline ColorV operator=(const XMVECTOR& b) {
            v = b;
            return *this;
        }

        inline float rf() const { return DirectX::XMVectorGetX(v); }
        inline float gf() const { return DirectX::XMVectorGetY(v); }
        inline float bf() const { return DirectX::XMVectorGetZ(v); }
        inline float af() const { return DirectX::XMVectorGetW(v); }

        inline ColorV argb() const {
            return DirectX::XMVectorSet(af(), bf(), gf(), rf());
        }

};

}
#endif