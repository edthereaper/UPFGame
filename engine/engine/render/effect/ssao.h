#ifndef RENDER_EFFECT_SSAO_H_
#define RENDER_EFFECT_SSAO_H_

#include "mcv_platform.h"
#include "effect.h"
#include "tools.h"

#include "render/shader/buffer.h"
#include "data/fx/platform/fx_ctes.h"

namespace render {

class SSAO : public BasicEffect {
    private:
        static ShaderCte<SCTES_SSAO> ctes;
    public:
        static void initType();
        static void tearDown();

    private:
        Fetch normals;
        Fetch space;

        float downsampleFactor = 1;
        
        float jitter = 1.f;

        float radius = 0.3f;
        float intensity = 2.2f;
        float scale = 1.f;
        float depthTolerance = 0.05f;
        float bias = 0.f;

    public:
        SSAO(std::string name, int resX, int resY) : BasicEffect(name, resX, resY) {}
        ~SSAO() {cleanup();}
        inline void cleanup() {BasicEffect::cleanup();}

        void init();
        void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            if (elem == "SSAO") {
                downsampleFactor = atts.getFloat("downsampleFactor", downsampleFactor);
                radius = atts.getFloat("radius", radius);
                intensity = atts.getFloat("intensity", intensity);
                scale = atts.getFloat("scale", scale);
                bias = atts.getFloat("bias", bias);
                depthTolerance = atts.getFloat("depthTolerance", depthTolerance);
                jitter = atts.getFloat("jitter", jitter);
                setTech(atts.getString("file", "ssao.fx"), atts.getString("ps", "SSAO"));
            } else if (elem == "normals") {
                normals.loadFromProperties(elem, atts);
            } else if (elem == "space") {
                space.loadFromProperties(elem, atts);
            }
        }

        inline const RenderedTexture* getOutput() const {return out;}

        const Texture* operator()(const PostProcessPipeline*, const Texture* in);
        
        inline const float getFactor() const {return downsampleFactor;}
        inline void setFactor(float f) {downsampleFactor = f;}
        
#ifdef _DEBUG
        bool antTW_addTweak(TwBar*, const std::string& groupName);
        void writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const;
#endif
};

}

#endif