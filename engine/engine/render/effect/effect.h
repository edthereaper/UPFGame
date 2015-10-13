#ifndef RENDER_EFFECT_EFFECT_H_
#define RENDER_EFFECT_EFFECT_H_

#include "mcv_platform.h"

#include <fstream>
#ifdef _DEBUG
#include <AntTweakBar.h>
#endif

#include "render/texture/RenderedTexture.h"
#include "render/texture/Texture.h"
#include "render/shader/shaders.h"
#include "utils/XMLParser.h"

namespace render {

class PostProcessPipeline;

class Effect {
    protected:
        const Texture* getStage(const PostProcessPipeline*, const std::string& stageName);
        const Texture* getResource(const PostProcessPipeline*, const std::string& resourceName);
    public:
        virtual ~Effect() {}

        virtual const Texture* operator()(const PostProcessPipeline*, const Texture* in)=0;
        virtual const Texture* getOutput() const=0;

        virtual void loadFromProperties(const std::string elem, utils::MKeyValue)=0;
        virtual void init(){};

#ifdef _DEBUG
        virtual bool antTW_addTweak(TwBar*, const std::string& groupName) {return false;}
        virtual void writeToXML(std::ostream& out, const std::string& stageName,
            const std::string& indent) const {}
#endif
};

class BasicEffect : public Effect {
    protected:
        Technique* tech = nullptr;
        RenderedTexture* out;
        const std::string name;
        const int resX;
        const int resY;

        RenderedTexture::zBufferType_e zBufferType = RenderedTexture::NO_ZBUFFER;
#if defined(_DEBUG) || defined(_OBJECTTOOL)
        bool debugZBuffer = false;
#endif

    public:
        BasicEffect(std::string name, int resX, int resY) : name(name), resX(resX), resY(resY) {}
        ~BasicEffect() {cleanup();}
        void init();
        
        inline void cleanup() {
            if(out != nullptr) {out->destroy(); delete out; out=nullptr;}
        }

        void setTech(std::string fxName, std::string psName);
        inline Technique* getTech() const {return tech;}
        
        const Texture* operator()(const PostProcessPipeline*, const Texture* in);
        inline const Texture* getOutput() const {return out;}
        inline void loadFromProperties(const std::string elem, utils::MKeyValue atts) {
            setTech(atts.getString("file"), atts.getString("ps"));
            if (atts.has("backZBuffer")) {
                if (atts.getBool("backZBuffer")) {
                    zBufferType = RenderedTexture::USE_BACK_ZBUFFER;
                } 
#if defined(_DEBUG) || defined(_OBJECTTOOL)
                else if (atts.getString("backZBuffer") == "debug") {
                    zBufferType = RenderedTexture::USE_BACK_ZBUFFER;
                    bool debugZBuffer = true;
                }
#endif
            }
        }

#ifdef _DEBUG
        bool antTW_addTweak(TwBar* bar, const std::string& groupName);
        void writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent,
            const std::string& tag, utils::MKeyValue& atts = utils::MKeyValue()) const;
        void writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const {
            return writeToXML(out, stageName, indent, "BasicEffect");
        }
#endif
};

class PostProcessPipeline : utils::XMLParser {
    private:
        struct entry_t {
            Effect* effect = nullptr;
            std::string className = "<unknown>";
            std::string name = "<unnamed>";
#ifdef _DEBUG
            bool skip = false;
            bool end = false;
#endif

            entry_t()=default;
            entry_t(std::string className, Effect* effect, std::string name) :
                className(className), effect(effect), name(name) {}
        };

    public:
        typedef std::vector<entry_t> vector_t;
        typedef utils::ItemsByName<PostProcessPipeline> Manager;
        friend Effect;

        static inline const Manager& getManager() {return manager;}
    private:
        static Manager manager;

    private:
        std::map<std::string, const Texture*> resources;
        Effect* currentEffect = nullptr;
        unsigned xmlDepth=0, xmlFxDepth=0;

        vector_t effects;
        const Texture* out = nullptr;

        std::string name, subname;
        int xRes;
        int yRes;

    private:
        const Texture* getStage(const std::string& stageName) const;
        const Texture* getResource(const std::string& resourceName) const;
        std::string registerName() const;

    public:
        PostProcessPipeline()=default;
        PostProcessPipeline(const PostProcessPipeline& copy)=delete;
        PostProcessPipeline(PostProcessPipeline&& move)=delete;

        inline std::string getName() const {return name;}
        inline std::string getFullName() const {return registerName();}
        void destroy();
        void retire();

        inline void setRes(int x, int y) {xRes = x; yRes = y;}
        inline int getXRes() const {return xRes;}
        inline int getYRes() const {return yRes;}

        const Texture* operator()(const Texture* in=nullptr);

        inline const Texture* getOutput() const {return out;}

        void onStartElement(const std::string &elem, utils::MKeyValue &atts);
        void onEndElement(const std::string &elem);

        bool load(const std::string& name, const std::string& subname="");
        bool reload(const std::string& name);

        inline void clearResources() {resources.clear();}
        inline void setResource(std::string name, const Texture* resource) {
            resources[name] = resource;
        }

        inline const vector_t getEffects() const {return effects;}
        
#ifdef _DEBUG
        TwBar*& antTW_createTweak();
    private:
        bool skip = false;
        TwBar* antTW_bar;
#endif
};

/*Effect factory class.

Registered effect types must conform to the following duck type:

- (constructor) (std::string name, int xRes, int yRes)

*/
class EffectLibrary
{
    public:
        typedef std::function<Effect*(std::string, int, int)> factory_fn;
    private:
        static std::map<std::string, factory_fn> factory;

    public:
        
        template<class Effect_T>
        inline static factory_fn registerNew(std::string className) {
            return factory[className] = std::bind(
                utils::allocate<Effect_T, std::string, int, int>,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        //Allocates a new effect of the solicited class and returns it (it's your responsibility to delete it)
        inline static Effect* build(std::string className, std::string fxName, int xRes, int yRes) {
            return factory[className](fxName, xRes, yRes);
        }
        
        inline static bool exists(std::string className) {
            return factory.find(className) != factory.end();
        }

        static void init();
        static void tearDown();

        #define FX_REGISTER(Effect_T) EffectLibrary::registerNew<Effect_T>(#Effect_T)
};

}

#endif