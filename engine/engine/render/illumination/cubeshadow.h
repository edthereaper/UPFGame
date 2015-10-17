#ifndef RENDER_CUBESHADOW_H_
#define RENDER_CUBESHADOW_H_
                      
#include "../render_utils.h"
#include "../camera/component.h"
#include "../texture/renderedTexture.h"
#include "../texture/renderedTextureCube.h"
#include "render/effect/effect.h"

#include "components/color.h"

#include "level/spatialIndex.h"

namespace render {

class CCubeShadow : public level::SpatiallyIndexed {
    public:
        struct camCache_t {
            XMMATRIX view;
            XMMATRIX viewProjection;
            XMVECTOR front;
            XMVECTOR right;
            XMVECTOR up;
        };
    private:
        int                 resolution = 512;
        RenderedTextureCube shadowCubeMap;
        RenderedTexture     shadowBuffer;
        PostProcessPipeline* fxPipeline;
        bool                enabled = true;
        bool lockSpatialIndex = false;
        camCache_t cachedCam[6];
        XMVECTOR prevPos;
        uint8_t enableMask  = ~0; //mask: +x-x+y-y-z+z (lowest to highest bit)
        uint8_t cullingMask = ~0; //mask: +x-x+y-y-z+z (lowest to highest bit)
        bool valid = false;
        bool passedSpatial = true;

    private:
        void createShadowMap(std::string name);
        void createShadowMap();
        
    public:
        CCubeShadow()=default;
        ~CCubeShadow() {
            if (valid) {
                shadowCubeMap.destroy();
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
            enableMask = uint8_t(atts.getHex("enable", enableMask));
        }

        void init();
        inline void resetShadow() {shadowCubeMap.destroy(); createShadowMap();}
        void update(float);
        void generate();
        void generateShadowBuffer(const Texture* space);
        void processShadowBuffer(const Texture* space);

		inline int getResolution() const { return resolution; }
        inline void setResolution(int res) {
            resolution = res;
            resetShadow();
        }

        const RenderedTextureCube* getShadowMap() const {return &shadowCubeMap;}
        inline const Texture* getShadowBuffer() const {return &shadowBuffer;}
        inline const Texture* getFxShadowBuffer() const {return fxPipeline->getOutput();}

        void setEnabled(bool b=true) {enabled = b;}

        inline void findSpatialIndex(){if (!lockSpatialIndex) {SpatiallyIndexed::findSpatialIndex(this);}}

        inline camCache_t getCachedCamData(unsigned i) {
            assert (i<6);
            return cachedCam[i];
        }

        inline void setCullingMask(uint8_t mask) {cullingMask = mask;}
        inline uint8_t getCullingMask() const {return cullingMask;}
        inline void setEnableMask(uint8_t mask) {enableMask = mask;}
        inline uint8_t getEnableMask() const {return enableMask;}

        inline bool hasPassedSpatial() const {return passedSpatial;}
};

}

#endif