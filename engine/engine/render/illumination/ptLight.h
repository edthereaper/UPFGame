#ifndef RENDER_ILLUMINATION_PTLIGHT
#define RENDER_ILLUMINATION_PTLIGHT

#include "mcv_platform.h"
#include "components/color.h"

#include "level/spatialIndex.h"

namespace render {

class CPtLight : public level::SpatiallyIndexed{
    private:
        component::Color color = component::ColorHSL(1.f/6.f);
        float radius = 1.f;
        float intensity = 1.f;
        float decayFactor = 0.7f; 
        float shadowIntensity = 1.0f;
        float shadowFocus = 0.0f;
        float shadowJittering = 1.f;
        float specularAmountModifier = 0;
        float specularPowerFactor = 1;
        XMVECTOR offset = utils::zero_v;
        bool offsetRelative = false;
        bool lockSpatialIndex = false;
        bool culled = true;
        bool enabled = true;
#ifdef _LIGHTTOOL
		bool selected = false;
		bool exportLight = true;
#endif

    public:    
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
    
        void draw();

        void init();
        void update(float);

        inline float getIntensity() const {return intensity;}
        inline void setIntensity(float f) {intensity = f;}
        
        inline float getRadius() const {return radius;}
        inline float getDecay() const {return decayFactor;}
        inline component::Color getColor() const {return color;}
        inline float getShadowIntensity() const {return shadowIntensity;}
        
        inline void setRadius(float r) {radius = r;}
        inline void setDecay(float d) {decayFactor = d;}
        inline void setColor(component::Color t) {color = t;}
        void setOffset(XMVECTOR nOff);
        inline XMVECTOR getOffset() const {return offset;}
        inline void setShadowIntensity(float f) {shadowIntensity = utils::inRange(0.f,f,1.f);}
		inline float getShadowIntensity() { return shadowIntensity; }
        inline float getShadowFocus() const {return shadowFocus;}
        inline void setShadowFocus(float f) {shadowFocus = f;}
        inline float getShadowJittering() const {return shadowJittering;}
        inline void setShadowJittering(float j) {shadowJittering = j;}
        
        inline float getSpecularAmountModifier() const {return specularAmountModifier;}
        inline float getSpecularPowerFactor() const {return specularPowerFactor;}
        inline void setSpecularAmountModifier(float f) {specularAmountModifier = f;}
        inline void setSpecularPowerFactor(float f) {specularPowerFactor = f;}

        void cull(component::Handle camera_h);

        inline bool isEnabled() const {return enabled;}
        inline void setEnabled(bool b=true) {enabled=b;} 

        void drawVolume();

        inline void findSpatialIndex(){if (!lockSpatialIndex) {SpatiallyIndexed::findSpatialIndex(this);}}

#ifdef _LIGHTTOOL
		void setSelectable();
		inline bool getSelected() { return selected; }
		inline void setSelected(bool b) { selected = b; }
        inline bool getExport() const {return exportLight;}
        inline void setExport(bool b) {exportLight = b;}
#endif
};

}
#endif
