#include "mcv_platform.h"
#include "color.h"

using namespace DirectX;

namespace component {

// I am fully aware that we don't need so many colors

const Color Color::WHITE                = 0xFFFFFFFF;
const Color Color::LIGHT_GRAY           = 0xBFBFBFFF;
const Color Color::GRAY                 = 0x7F7F7FFF;
const Color Color::DARK_GRAY            = 0x3F3F3FFF;
const Color Color::BLACK                = 0x000000FF;
                                        
const Color Color::RED                  = 0xFF0000FF;
const Color Color::GREEN                = 0x00FF00FF;
const Color Color::BLUE                 = 0x0000FFFF;
                                        
const Color Color::CYAN                 = 0x00FFFFFF;        
const Color Color::MAGENTA              = 0xFF00FFFF;
const Color Color::YELLOW               = 0xFFFF00FF;
                                        
                                        
const Color Color::ORANGE               = 0xFF7F00FF;
const Color Color::PINK                 = 0xFF007FFF;
const Color Color::LIME                 = 0x7FFF00FF;
const Color Color::SPRING_GREEN         = 0x00FFF7FF;
const Color Color::PURPLE               = 0x7F00FFFF;
const Color Color::CERULEAN             = 0x007FFFFF;

const Color Color::AQUA                 = 0x00BFFFFF;
const Color Color::LEMON_LIME           = 0xBFFF00FF;
const Color Color::BEIGE                = 0xF5F5DCFF;
const Color Color::BROWN                = 0xBF7F3FFF;
const Color Color::CELADON              = 0x7FBF3FFF;
const Color Color::VIOLET               = 0x7F3FBFFF;
const Color Color::STEEL_BLUE           = 0x3F7FBFFF;
const Color Color::BURGUNDY             = 0x50404DFF;
const Color Color::PALE_PINK            = 0xFFC0EBFF;
const Color Color::MOUNTAIN_GREEN       = 0x336600FF;
const Color Color::TEAL                 = 0x18A7B5FF;
const Color Color::MINT                 = 0x98FF98FF;

const ColorHSL ColorHSL::WHITE          = ColorHSL(0,1,1);
const ColorHSL ColorHSL::LIGHT_GRAY     = ColorHSL(0,0,0.75f);
const ColorHSL ColorHSL::GRAY           = ColorHSL(0,0,0.5f);
const ColorHSL ColorHSL::DARK_GRAY      = ColorHSL(0,0,0.25f);
const ColorHSL ColorHSL::BLACK          = ColorHSL(0,0,0);
const ColorHSL ColorHSL::MAGENTA        = ColorHSL(0.f/6.f);
const ColorHSL ColorHSL::RED            = ColorHSL(1.f/6.f);
const ColorHSL ColorHSL::YELLOW         = ColorHSL(2.f/6.f);
const ColorHSL ColorHSL::GREEN          = ColorHSL(3.f/6.f);
const ColorHSL ColorHSL::CYAN           = ColorHSL(4.f/6.f);
const ColorHSL ColorHSL::BLUE           = ColorHSL(5.f/6.f);

Color Color::blend(const Color& t, float f)
{
    return DirectX::XMVectorLerp(factor(af()), t.factor(t.af()), f);
}

ColorHSL::ColorHSL(ColorRGB color)
{
    XMFLOAT4 rgba;
    XMStoreFloat4(&rgba, color);
    const auto& r(rgba.x);
    const auto& g(rgba.y);
    const auto& b(rgba.z);
    _a = rgba.w;
    float cMin = std::min({r, g, b});
    float cMax = std::max({r, g, b});
    _l = (cMax + cMin) * 0.5f;
    if (cMin == cMax) {
        _s = 0.f;
        _h = 0.f; //convention (technically is undefined)
    } else {
        auto range(cMax - cMin);
        _s = _l < 0.5f ? range/(cMax + cMin) : range/(2 - cMax - cMin);
        _h = (cMax == r) ? (g-b)/range : (cMax == g) ? 2 + (b-r)/range : 4 + (r-g)/range;
        _h = fmod(_h, 6.f)/6.f;
    }
}

ColorHSL::operator ColorRGB() const
{
    if (_s==0) {
        return XMFLOAT4(_l,_l,_l,_a);
    } else {
        float c = (1.f-fabsf(2.f*_l-1.f))*_s;
        float q = _h*6.f;
        float x = c * (1 - fabsf( fmodf(q, 2.f) - 1.f));
        float m = _l - c*0.5f;
        float r=0.f,g=0.f,b=0.f;

        switch(int(std::floor(q))) {
            case 0: r=c; g=x; break;
            case 1: g=c; r=x; break;
            case 2: g=c; b=x; break;
            case 3: b=c; g=x; break;
            case 4: r=c; b=x; break;
            case 5:
            case 6: b=c; r=x; break;
        }
        r+=m; g+=m; b+=m;

        float ret[4] = {r,g,b,_a};
        return ret;
    }
}

void Color::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    if (atts.has("color")) {
        std::string color = atts.getString("color");
        if (color == "white")               {set(Color::WHITE);}
        else if (color == "light gray")     {set(Color::LIGHT_GRAY);}
        else if (color == "gray")           {set(Color::GRAY);}
        else if (color == "dark gray")      {set(Color::DARK_GRAY);}
        else if (color == "black")          {set(Color::BLACK);}
        else if (color == "red")            {set(Color::RED);}
        else if (color == "green")          {set(Color::GREEN);}
        else if (color == "blue")           {set(Color::BLUE);}
        else if (color == "cyan")           {set(Color::CYAN);}
        else if (color == "yellow")         {set(Color::YELLOW);}
        else if (color == "magenta")        {set(Color::MAGENTA);}
        else if (color == "burgundy")       {set(Color::BURGUNDY);}
        else if (color == "aqua")           {set(Color::AQUA);}
        else if (color == "celadon")        {set(Color::CELADON);}
        else if (color == "orange")         {set(Color::ORANGE);}
        else if (color == "brown")          {set(Color::BROWN);}
        else if (color == "cerulean")       {set(Color::CERULEAN);}
        else if (color == "lemon lime")     {set(Color::LEMON_LIME);}
        else if (color == "lime")           {set(Color::LIME);}
        else if (color == "pale pink")      {set(Color::PALE_PINK);}
        else if (color == "pink")           {set(Color::PINK);}
        else if (color == "purple")         {set(Color::PURPLE);}
        else if (color == "steel blue")     {set(Color::STEEL_BLUE);}
        else if (color == "spring green")   {set(Color::SPRING_GREEN);}
        else if (color == "violet")         {set(Color::VIOLET);}
        else if (color == "beige")          {set(Color::BEIGE);}
        else if (color == "teal")           {set(Color::TEAL);}
        else if (color == "mint")           {set(Color::MINT);}
        else if (color == "mountain green") {set(Color::MOUNTAIN_GREEN);}
        
        //alternative spellings
        else if (color == "light grey")     {set(Color::LIGHT_GRAY);}
        else if (color == "grey")           {set(Color::GRAY);}
        else if (color == "dark grey")      {set(Color::DARK_GRAY);}

        else {
            rgba= atts.getHex("color", rgba);
        }
    }

    if (atts.has("r")) {setRf(atts.getFloat("r", rf()));}
    if (atts.has("g")) {setGf(atts.getFloat("g", gf()));}
    if (atts.has("b")) {setBf(atts.getFloat("b", bf()));}
    if (atts.has("a")) {setAf(atts.getFloat("a", af()));}
}

}