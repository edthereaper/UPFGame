#ifndef RENDER_ILLUMINATION_VOLPTLIGHT_H_
#define RENDER_ILLUMINATION_VOLPTLIGHT_H_

#include "mcv_platform.h"

#include "components/color.h"
#include "level/spatialIndex.h"

namespace render {

class CVolPtLight : public level::SpatiallyIndexed {
    private:
        component::Color color = component::ColorHSL(1.f/6.f);
	    float density =1.f;
	    float weight = 1.f;
	    float rayDecay = 0.998f;
	    float decay = 0.9f;
	    float occludedAddend = 0.f;
	    float illuminatedAddend = 1.f;
        float radius = 10.f;
	    float normalShadeMin = 0.5f;
        unsigned maxSamples = 200;

        bool culled = true;
        bool enabled = true;
        
#ifdef _LIGHTTOOL
        bool selected = false;
        bool exportLight = true;
#endif

    public:
        inline void init() {}
        inline void update(float) {}
        inline void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            density = atts.getFloat("density", density);
            weight = atts.getFloat("weight", weight);
            decay = atts.getFloat("decay", decay);
            rayDecay = atts.getFloat("rayDecay", rayDecay);
            radius = atts.getFloat("radius", radius);
            occludedAddend = atts.getFloat("occludedAddend", occludedAddend);
            illuminatedAddend = atts.getFloat("illuminatedAddend", illuminatedAddend);
            normalShadeMin = atts.getFloat("normalShadeMin", normalShadeMin);
            maxSamples = atts.getInt("maxSamples", maxSamples);
            color = atts.getHex("color", color);
            
            enabled = atts.getBool("enabled", enabled);
            #ifdef _LIGHTTOOL
                exportLight = atts.getBool("export", exportLight);
            #endif
        }

        inline component::Color getColor() const {return color;}
        inline float getDensity() const {return density;}
        inline float getWeight() const {return weight;}
        inline float getRayDecay() const {return rayDecay;}
        inline float getDecay() const {return decay;}
        inline float getOccludedAddend() const {return occludedAddend;}
        inline float getIlluminatedAddend() const {return illuminatedAddend;}
        inline float getRadius() const {return radius;}
        inline float getNormalShadeMin() const {return normalShadeMin;}
        inline unsigned getMaxSamples() const {return maxSamples;}
        
        inline void setColor(const component::Color& c) {color = c;}
        inline void setDensity(float f) {density = f;}
        inline void setWeight(float f) {weight = f;}
        inline void setRayDecay(float f) {rayDecay = f;}
        inline void setDecay(float f) {decay = f;}
        inline void setOccludedAddend(float f) {occludedAddend = f;}
        inline void setIlluminatedAddend(float f) {illuminatedAddend = f;}
        inline void setRadius(float f) {radius = f;}
        inline void setNormalShadeMin(float f) {normalShadeMin = f;}
        inline void setMaxSamples(unsigned n) {maxSamples = n;}
        
        void cull(component::Handle camera_h);
        inline bool isEnabled() const {return enabled;}
        inline void setEnabled(bool b=true) {enabled = b;}

        void draw();
        void drawVolume();
        
#ifdef _LIGHTTOOL
        void setSelectable();
        inline bool isSelected() const {return selected;}
        inline void setSelected(bool b=true) {selected = b;}
        inline bool getExport() const {return exportLight;}
        inline void setExport(bool b) {exportLight = b;}
#endif
        
        inline void findSpatialIndex(){SpatiallyIndexed::findSpatialIndex(this);}
};

}

#endif