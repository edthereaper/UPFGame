#ifndef RENDER_SHADOW_H_
#define RENDER_SHADOW_H_

#include "../camera/component.h"
#include "../texture/renderedTexture.h"
#include "render/effect/effect.h"

#include "components/color.h"

#include "level/spatialIndex.h"

namespace render {

class CShadow : public level::SpatiallyIndexed {
    private:
        int              resolution = 512;
        RenderedTexture  shadowMap;
        RenderedTexture  shadowBuffer;
        PostProcessPipeline* fxPipeline = nullptr;
        bool             enabled = true;
        bool lockSpatialIndex = false;
        void createShadowMap();
        void createShadowMap(std::string name);
        bool valid = false;

    public:
        CShadow()=default;
        ~CShadow() {
            if (valid) {
                shadowMap.destroy();
                shadowBuffer.destroy();
                if(fxPipeline != nullptr) {
                    fxPipeline->retire();
                    delete fxPipeline;
                }
            }
        }

        inline void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            resolution = atts.getInt("resolution", resolution);
            lockSpatialIndex = atts.getBool("lockSpatialIndex", lockSpatialIndex);
        }

        void init();
        inline void resetShadow() {shadowMap.destroy(); createShadowMap();}
        inline void update(float){}
        void generate();
        void generateShadowBuffer(const Texture* space);
        void processShadowBuffer(const Texture* space);

		inline int getResolution() const { return resolution; }
        inline void setResolution(int res) {
            resolution = res;
            resetShadow();
        }

        inline const Texture* getShadowMap() const {return shadowMap.getZTexture();}
        inline const Texture* getShadowBuffer() const {return &shadowBuffer;}
        inline const Texture* getFxShadowBuffer() const {return fxPipeline->getOutput();}

        void setEnabled(bool b=true) {enabled = b;}
        
        inline void findSpatialIndex(){if (!lockSpatialIndex) {SpatiallyIndexed::findSpatialIndex(this);}}
};

}

#endif