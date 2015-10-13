#ifndef RENDER_EFFECT_TOOLS_H_
#define RENDER_EFFECT_TOOLS_H_

#include "effect.h"
#include "data/fx/platform/fx_ctes.h"

namespace render {

//A BasicEffect that only runs in debug
class DebugFX : public BasicEffect {
    private:
        bool off = false;
        bool force = false;
    public:
        DebugFX(std::string name, int resX, int resY) :
            BasicEffect(name, resX, resY) {}
        inline void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
#ifdef _DEBUG
            BasicEffect::loadFromProperties(elem, atts);
            off = !atts.getBool("on", !off);
#endif
        }

        inline const Texture* operator()(const PostProcessPipeline* ppp, const Texture* in) {
#ifdef _DEBUG
            return (off || force) ? in : BasicEffect::operator()(ppp, in);
#else
            return in;
#endif
        }

#ifdef _DEBUG
        bool antTW_addTweak(TwBar*, const std::string& groupName);
        void writeToXML(std::ostream& out, const std::string& stageName,
            const std::string& indent) const;
#endif

};

class Fetch : public Effect {
    private:
        const Texture* fetched = nullptr;
        struct params_t {
            enum {NONE, STAGE_OUTPUT, RESOURCE, TEXTURE} resourceType = NONE;
            std::string name = "<none>";
        } params;
    public:
        Fetch()=default;
        Fetch(std::string name, int resX, int resY){}

        inline const Texture* operator()(const PostProcessPipeline* ppp, const Texture*) {
            switch (params.resourceType) {
                case params_t::STAGE_OUTPUT: fetched = getStage(ppp, params.name); break;
                case params_t::RESOURCE: fetched = getResource(ppp, params.name); break;
                case params_t::TEXTURE: fetched = Texture::getManager().getByName(params.name); break;
                default: fetched = nullptr;
            }
            return fetched;
        }
        inline const Texture* getOutput() const {
            return fetched;
        }

        inline void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            if (atts.has("stage")) {
                params.resourceType = params_t::STAGE_OUTPUT;
                params.name = atts.getString("stage", params.name);
            } else if (atts.has("resource")) {
                params.resourceType = params_t::RESOURCE;
                params.name = atts.getString("resource", params.name);
            } else if (atts.has("texture")) {
                params.resourceType = params_t::TEXTURE;
                params.name = atts.getString("texture", params.name);
            } else {
                params.resourceType = params_t::NONE;
                params.name = atts.getString("fetch", params.name);
            }
        }

        inline bool isValid() const {return params.resourceType != params_t::NONE;}

        
#ifdef _DEBUG
        bool antTW_addTweak(TwBar*, const std::string& groupName);
        bool antTW_addTweak(TwBar*, const std::string& groupName,
            const std::string& varName, const std::string& varLabel="Fetch");
        bool writeToXML(std::ostream& out, const std::string& stageName,
            const std::string& indent, const std::string& elem, bool ignoreNone=true) const;
        inline void writeToXML(std::ostream& out, const std::string& stageName,
            const std::string& indent) const {
            writeToXML(out, stageName, indent, "Fetch", false);
        }
#endif

};

class Mix : public BasicEffect {
    private:
        static ShaderCte<SCTES_Mix> ctes;
    public:
        static void initType();
        static void tearDown();

    private:
        const Texture* realOut = nullptr;

        Fetch sourceFetch;
        Fetch weightFetch;
        
        XMVECTOR mask = utils::xAxis_v;
        float factor = 1;
        float baseFactor = 0.f;
        float rangeMax = 1.00001f;
        float rangeMin = -0.000001f;

    public:
        Mix(std::string name, int resX, int resY) : BasicEffect(name, resX, resY) {}

        inline void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            if (elem == "Mix") {
                factor = atts.getFloat("factor", 1);
                mask = atts.getFloat4("mask", mask);
                baseFactor = atts.getFloat("baseFactor", baseFactor);
                rangeMax = atts.getFloat("rangeMax", rangeMax);
                rangeMin = atts.getFloat("rangeMin", rangeMin);
                setTech(atts.getString("file", "mix.fx"), atts.getString("ps", "Mix"));
            } else if (elem == "source") {
                sourceFetch.loadFromProperties(elem, atts);
            } else if (elem == "weight") {
                weightFetch.loadFromProperties(elem, atts);
            }
        }

        inline void init() {
            BasicEffect::init();
        }

        const Texture* operator()(const PostProcessPipeline* ppp, const Texture*);

        const Texture* getOutput() const {return realOut;}

#ifdef _DEBUG
        bool antTW_addTweak(TwBar*, const std::string& groupName);
        void writeToXML(std::ostream& out, const std::string& stageName,
            const std::string& indent) const;
#endif
};

}

#endif