#ifndef INC_DEFERRED_RENDER_H_
#define INC_DEFERRED_RENDER_H_

#include "texture/renderedTexture.h"
#include "effect/effect.h"    

namespace render {

class Camera;

class DeferredRender {
    public:
        enum gbufferOMRT_e {
            ALBEDO              =1<<0,
            NORMALS             =1<<1,
            LIGHT               =1<<2,
            SPACE               =1<<3,
            SELFILLUMINATION    =1<<4,
            DATA1               =1<<5,
            DATA2               =1<<6,
            NORMALS_PAINTED     =1<<7,
        };
    private:

        std::string prefix = "";
        RenderedTexture*  rt_lights;
        RenderedTexture*  rt_albedo;
        RenderedTexture*  rt_normals;
        RenderedTexture*  rt_normals_transform;
        RenderedTexture*  rt_space;
        RenderedTexture*  rt_selfIllumination;
        RenderedTexture*  rt_paintGlow;
        RenderedTexture*  rt_out;

        // R: motion blur amount
        // G: self illumination clamp
        // B: mist mask
        // A: ObjectPaintableAmount
        RenderedTexture*  rt_data1;

        // R: U
        // G: V
        // B: paint amount
        // A: paint glow clamp (inverted)
        RenderedTexture*  rt_data2;
        
        PostProcessPipeline generateAmbientPPP;
        PostProcessPipeline postProcessSelfIll;
        PostProcessPipeline postProcessOut;

        bool spaceToggle = false;
        unsigned xRes=0, yRes=0;
    private:
        void generateShadowBuffers();
        void generateLightBuffer();

    public:
        DeferredRender()=default;
        DeferredRender(std::string prefix) : prefix(prefix) {}
        bool create(int xres, int yres);
        void clearGBuffer();
        void initGBuffer(uint32_t mask = ~0) const;
        void renderGBuffer(component::Handle camera_h);
        void postProcessGBuffer();
        void resolve();
        void postProcessOutput();
        void renderParticles();
        void renderFlowers();
        void drawVolumetricLights();
        void drawMists();
        void drawPaint();
        void screenEffects();
        void generateAmbient();
#if defined(_DEBUG ) || defined(_OBJECTTOOL)
        bool debugLayer = true;
        void renderDebug() const;
#endif
        
        inline const Texture* getData1() const {return rt_data1;}
        inline const Texture* getData2() const {return rt_data2;}
        inline const Texture* getPaint() const {return rt_paintGlow;}
        inline const Texture* getLights() const {return rt_lights;}
        inline const Texture* getAlbedo() const {return rt_albedo;}
        inline const Texture* getNormals() const {return rt_normals;}
        inline const Texture* getPaintedNormals() const {return rt_normals_transform;}
        inline const Texture* getSpace() const {return rt_space;}
        inline const Texture* getSelfIllumination() const {return rt_selfIllumination;}
        inline const Texture* getOut() const {return rt_out;}
        inline const Texture* getAmbient() const {return generateAmbientPPP.getOutput();}

        inline const Texture* getFxSelfIllumination() const {return postProcessSelfIll.getOutput();}
        inline const Texture* getFxOut() const {return postProcessOut.getOutput();}

        void destroy();

        void operator()(component::Handle camera_h);
};

}
#endif
