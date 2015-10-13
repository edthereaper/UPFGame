#ifndef RENDER_EFFECT_BLUR_H_
#define RENDER_EFFECT_BLUR_H_

#include "mcv_platform.h"
#include "effect.h"
#include "tools.h"

#include "render/shader/buffer.h"
#include "data/fx/platform/fx_ctes.h"

namespace render {

class MotionBlur;

class Blur : public BasicEffect {
    private:
        static ShaderCte<SCTES_Blur> ctes;
    public:
        static void initType();
        static void tearDown();

    private:
        Fetch activateTexture;
        RenderedTexture* dsH = nullptr;

        float factor = 0.5f;
        float amplitude = 4.f/3.f;
        float toleranceX = 0.05f;
        float toleranceY = 0.05f;

    public:
        Blur(std::string name, int resX, int resY) : BasicEffect(name, resX, resY) {}
        ~Blur() {cleanup();}
        inline void cleanup() {
            if(dsH != nullptr) {dsH->destroy(); delete dsH; dsH =nullptr;}
            BasicEffect::cleanup();
        }

        void init();
        void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            if (elem == "Blur") { 
                factor = atts.getFloat("factor", factor);
                amplitude = atts.getFloat("amplitude", amplitude);
                toleranceX = atts.getFloat("toleranceX", toleranceX);
                toleranceY = atts.getFloat("toleranceY", toleranceY);
                setTech(atts.getString("file", "blur.fx"), atts.getString("ps", "PSBlur"));
            } else if (elem == "activate") {
                activateTexture.loadFromProperties(elem, atts);
            }
        }

        inline const RenderedTexture* getOutput() const {return out;}

        const Texture* operator()(const PostProcessPipeline*, const Texture* in);

        inline const float getFactor() const {return factor;}
        inline void setFactor(float f) {factor = f;}
        inline const float getAmplitude() const {return amplitude;}
        inline void setAmplitude(float f) {amplitude = f;}
        inline const float getToleranceX() const {return toleranceX;}
        inline void setToleranceX(float f) {toleranceX = f;}
        inline const float getToleranceY() const {return toleranceY;}
        inline void setToleranceY(float f) {toleranceY = f;}

#ifdef _DEBUG
        bool antTW_addTweak(TwBar*, const std::string& groupName);
        void writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const;
#endif
};


class MotionBlur : public BasicEffect {
    private:
        static ShaderCte<SCTES_MotionBlur> ctes;
    public:
        static void initType();
        static void tearDown();

    private:
        Fetch space;
        Fetch data;

        XMMATRIX prevViewProjection;
        float factor = 0.5f;
        float amplitude = 4.f/3.f;
        unsigned nSamples = 5;

    public:
        MotionBlur(std::string name, int resX, int resY) : BasicEffect(name, resX, resY) {}
        ~MotionBlur() {cleanup();}
        inline void cleanup() {
            BasicEffect::cleanup();
        }

        void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            if (elem == "MotionBlur") { 
                amplitude = atts.getFloat("amplitude", amplitude);
                nSamples = atts.getInt("nSamples", nSamples);
                setTech(atts.getString("file", "motionblur.fx"), atts.getString("ps", "PSMotionBlur"));
            } else if (elem == "space") {
                space.loadFromProperties(elem, atts);
            } else if (elem == "data") {
                data.loadFromProperties(elem, atts);
            }
        }

        inline const RenderedTexture* getOutput() const {return out;}

        const Texture* operator()(const PostProcessPipeline*, const Texture* in);

        inline const float getFactor() const {return factor;}
        inline void setFactor(float f) {factor = f;}
        inline const float getAmplitude() const {return amplitude;}
        inline void setAmplitude(float f) {amplitude = f;}

#ifdef _DEBUG
        bool antTW_addTweak(TwBar*, const std::string& groupName);
        void writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const;
#endif
};

}

#endif