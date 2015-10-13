#ifndef RENDER_MIST_H_
#define RENDER_MIST_H_

#include "components/color.h"
#include "components/transform.h"

namespace render {

class CMist {
    private:
        float offsetX=0;
        float offsetZ=0;

        component::Color colorTop = component::Color::WHITE;
        component::Color colorBottom = component::Color::LIGHT_GRAY;
        float w=8.f;
        float h=0.5f;
        float l=8.f;
        float dX=1.f;
        float dZ=1.f;
        float unitWorldSize = 25;
	    bool chaotic = true;
	    float darkenAlpha = 0.4f;
	    float factor = 1.f;
	    float minimun = 0.0f;
	    float layerDecay = 0.05f;
	    float intensity = 1.f;
	    float sqSqrt = 0;
	    float depthTolerance = 200;
        float spin = 0;

        bool cheatsCameraZFar = true;

        bool muted = false;

#ifdef _LIGHTTOOL
        bool exportable = true;
        bool selected = false;
#endif

    public:
        inline void init() {}
        inline void update(float elapsed) {
            offsetX += elapsed * dX;
            offsetZ += elapsed * dZ;
            if (spin != 0) {
                component::CTransform* t =
                    component::Handle(this).getBrother<component::CTransform>();
                t->applyRotation(
                    DirectX::XMQuaternionRotationAxis(t->getUp(),elapsed*spin));
            }
        }
        void loadFromProperties(const std::string elem, utils::MKeyValue);

        inline float getOffsetX() const {return offsetX;}
        inline float getOffsetZ() const {return offsetZ;}

        inline float getWidth() const {return w;}
        inline float getHeight() const {return h;}
        inline float getLength() const {return l;}
        inline float getDeltaX() const {return dX;}
        inline float getDeltaZ() const {return dZ;}
        inline component::Color getColorTop() const {return colorTop;}
        inline component::Color getColorBottom() const {return colorBottom;}
        inline float getUnitWorldSize() const {return unitWorldSize;}
        inline bool getChaotic() const {return chaotic;}
        inline bool getMuted() const {return muted;}
        inline float getDarkenAlpha() const {return darkenAlpha;}
        inline float getFactor() const {return factor;}
        inline float getMinimun() const {return minimun;}
        inline float getLayerDecay() const {return layerDecay;}
        inline float getIntensity() const {return intensity;}
        inline float getDepthTolerance() const {return depthTolerance;}
        inline float getSqSqrt() const {return sqSqrt;}
        
        inline void setSqSqrt(float f) {sqSqrt = f;}
        inline void setDepthTolerance(float f) {depthTolerance = f;}
        inline void setMuted(bool b) {muted = b;}
        inline void setWidth(float f) {w = f;}
        inline void setHeight(float f) {h = f;}
        inline void setLength(float f) {l = f;}
        inline void setDeltaX(float f) {dX = f;}
        inline void setDeltaZ(float f) {dZ = f;}
        inline void setColorTop(const component::Color& c) {colorTop = c;}
        inline void setColorBottom(const component::Color& c) {colorBottom = c;}
        inline void setUnitWorldSize(float f) {unitWorldSize = f;}
        inline void setChaotic(bool b) {chaotic = b;}
        inline void setDarkenAlpha(float f) {darkenAlpha = f;}
        inline void setFactor(float f) {factor = f;}
        inline void setMinimun(float f) {minimun = f;}
        inline void setLayerDecay(float f) {layerDecay = f;}
        inline void setIntensity(float f) {intensity = f;}

        inline float getSpin() const {return spin;}
        inline void setSpin(float f) {spin = f;}

        void draw() const;
        void drawMarker() const;
        
#ifdef _LIGHTTOOL
        void setSelectable();
        inline bool isSelected() const {return selected;}
        inline void setSelected(bool b=true) {selected = b;}
        inline bool getExport() const {return exportable;}
        inline void setExport(bool b) {exportable = b;}
#endif

};

}

#endif