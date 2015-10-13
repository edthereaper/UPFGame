#ifndef RENDER_ILLUMINATION_DIRLIGHT
#define RENDER_ILLUMINATION_DIRLIGHT

#include "mcv_platform.h"
#include "components/color.h"

#include "level/spatialIndex.h"

namespace render {

class CDirLight : public level::SpatiallyIndexed {
    private:
		component::Color color = component::ColorHSL(1.f/6.f);
        float intensity = 1.f; 
        float decayFactor = 0.7f; 
        float spotDecayFactor = 0.7f; 
        float shadowFocus = 0.f;
        float shadowJittering = 1.f;
        float radius = 0;
        float shadowIntensity = 1.0f;
        bool spotlight = true;
        bool lockSpatialIndex = false;
        bool culled = true;
        bool enabled = true;
        float specularAmountModifier = 0;
        float specularPowerFactor = 1;

#ifdef _LIGHTTOOL
		bool selected = false;
		bool exportLight = true;
        bool lookAtLocked = false;
#endif

    public:
        CDirLight() {}
    
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            radius = atts.getFloat("radius", radius);
            decayFactor = atts.getFloat("decay", decayFactor);
            intensity = atts.getFloat("intensity", intensity);
            color = atts.getHex("color", color);
            setOffset(atts.getPoint("offset", getOffset()));
            spotDecayFactor = atts.getFloat("spotDecay", spotDecayFactor);
            spotlight = atts.getBool("spotlight", spotlight);
            shadowIntensity = atts.getFloat("shadowIntensity", shadowIntensity);
            shadowFocus = atts.getFloat("shadowFocus", shadowFocus);
            shadowJittering = atts.getFloat("shadowJittering", shadowJittering);
            lockSpatialIndex = atts.getBool("lockSpatialIndex", lockSpatialIndex);
            enabled = atts.getBool("enabled", enabled);
            specularAmountModifier = atts.getFloat("specularAmountModifier", specularAmountModifier);
            specularPowerFactor = atts.getFloat("specularPowerFactor", specularPowerFactor);
            intensity *= color.af();
            color.setAf(1.f);
#ifdef _LIGHTTOOL
            exportLight = atts.getBool("export", exportLight);
#endif
        }
    
        void draw();

        inline void init() {}
        void update(float);
        
        inline float getIntensity() const {return intensity;}
        inline void setIntensity(float f) {intensity = f;}

        inline float getRadius() const {return radius;}
        inline float getDecay() const {return decayFactor;}
        inline float getSpotDecay() const {return spotDecayFactor;}
        inline float getShadowIntensity() const {return shadowIntensity;}
        inline float getShadowFocus() const {return shadowFocus;}
        inline float getShadowJittering() const {return shadowJittering;}
        inline bool isSpotlight() const {return spotlight;}
        inline component::Color getColor() const {return color;}
        
        inline float getSpecularAmountModifier() const {return specularAmountModifier;}
        inline float getSpecularPowerFactor() const {return specularPowerFactor;}
        inline void setSpecularAmountModifier(float f) {specularAmountModifier = f;}
        inline void setSpecularPowerFactor(float f) {specularPowerFactor = f;}

        inline void setRadius(float r) {radius = r;}
        inline void setDecay(float d) {decayFactor = d;}
        inline void setSpotDecay(float d) {spotDecayFactor = d;}
        inline void setSpotlight(bool b = true) {spotlight = b;}
        inline void setColor(component::Color t) {color = t;}
        inline void setShadowJittering(float j) {shadowJittering = j;}
        void setOffset(XMVECTOR nOff);
        XMVECTOR getOffset() const;
        inline void setShadowIntensity(float f) {shadowIntensity = utils::inRange(0.f,f,1.f);}
        inline void setShadowFocus(float f) {shadowFocus = f;}
        
        inline bool isEnabled() const {return enabled;}
        inline void setEnabled(bool b=true) {enabled=b;} 

        void cull(component::Handle camera_h);

        void drawVolume();

        inline void findSpatialIndex(){if (!lockSpatialIndex) {SpatiallyIndexed::findSpatialIndex(this);}}

#ifdef _LIGHTTOOL
		void setSelectable();
		inline bool getSelected() { return selected; }
		inline void setSelected(bool b) { selected = b; }
        inline bool getExport() const {return exportLight;}
        inline void setExport(bool b) {exportLight = b;}
        inline bool getLookAtLocked() const {return lookAtLocked;}
        inline void setLookAtLocked(bool b) {lookAtLocked = b;}
#endif
};

}
#endif
