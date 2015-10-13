#include "mcv_platform.h"
#include "effect.h"

#include "render/render_utils.h"

#ifdef _DEBUG
#include "antTweakBar_USER/antTW.h"
using namespace antTw_user;
#endif

namespace render {

PostProcessPipeline::Manager PostProcessPipeline::manager;

const Texture* Effect::getStage(const PostProcessPipeline* ppp, const std::string& stageName)
{
    return ppp->getStage(stageName);
}
const Texture* Effect::getResource(const PostProcessPipeline* ppp, const std::string& resourceName)
{
    return ppp->getResource(resourceName);
}

void PostProcessPipeline::destroy()
{
    for (const auto& fx : effects) {
        delete fx.effect;
    } 
}

void PostProcessPipeline::retire()
{
    manager.destroy(this, false);
}

const Texture* PostProcessPipeline::getResource(const std::string& name) const
{
    return utils::atOrDefault(resources, name, nullptr);
}

const Texture* PostProcessPipeline::getStage(const std::string& stageName) const
{
    for (const auto& fx : effects) {
        if(fx.name == stageName) {
            return fx.effect->getOutput();
        }
    }
    return nullptr;
}

const Texture* PostProcessPipeline::operator()(const Texture* in)
{
    out = in;
#ifdef _DEBUG
    if (skip) {return in;}

    auto regName = name;
    if (subname != "") {
        regName += '[' + subname + ']';
    }
    TraceScoped scope(("FX: "+regName).c_str());
#endif
    setResource("INPUT", in);
    for (const auto& fx : effects) {
#ifdef _DEBUG
        if (!fx.skip) {
#endif
            currentEffect = fx.effect;
            out = (*currentEffect)(this, out);
#ifdef _DEBUG
        }
        if (fx.end) {break;}
#endif
    }
    return out;
}

std::string PostProcessPipeline::registerName() const
{
    auto regName = name;
    if (subname != "") {
        regName += '[' + subname + ']';
    }
    return regName;
}

bool PostProcessPipeline::load(const std::string& fname, const std::string& newSubname)
{
    name = fname;
    subname = newSubname;
    currentEffect = nullptr;
    manager.registerNew(registerName(), this, false);
    bool ok = xmlParseFile("data/fx/pipelines/"+fname+".xml");
    assert(ok);
    return ok;
}

bool PostProcessPipeline::reload(const std::string& registerName)
{
#ifdef _DEBUG
    bool reloadBar = false;
    if (antTW_bar != nullptr) {
        reloadBar = true;
        TwDeleteBar(antTW_bar);
        antTW_bar = nullptr;
    }
#endif

    for(auto& fx : effects) {
        delete fx.effect;
    }
    effects.clear();
    bool ret = load(name, subname);

#ifdef _DEBUG
    if (reloadBar) {antTW_createTweak();}
#endif
    return ret;
}

void PostProcessPipeline::onStartElement(const std::string &elem, utils::MKeyValue &atts)
{
    if (elem == "pipeline") {
        //Nothing to do
    } else if (currentEffect != nullptr) {
        currentEffect->loadFromProperties(elem, atts);
    } else if (EffectLibrary::exists(elem)){
        unsigned stage = unsigned(effects.size());
        std::string stageName = "s" + std::to_string(stage);
        if (atts.has("name")) {stageName =  atts.getString("name", stageName);}
        std::string fxName = "FX_" + registerName() + "_" + stageName;
        currentEffect = EffectLibrary::build(elem, fxName, xRes, yRes);
        assert(currentEffect != nullptr);
        currentEffect->loadFromProperties(elem, atts);
        xmlFxDepth = xmlDepth;
        effects.push_back(entry_t(elem, currentEffect, stageName));
    } else {
        utils::dbg("Unregistered fx %s.\n", elem.c_str());
    }
    ++xmlDepth;
}
void PostProcessPipeline::onEndElement(const std::string &elem)
{
    --xmlDepth;
    if (xmlFxDepth == xmlDepth) {
        if (currentEffect != nullptr) {
            currentEffect->init();
            currentEffect = nullptr;
        }
        xmlFxDepth=0;
    }
}

void BasicEffect::setTech(std::string fxName, std::string psName)
{
    tech = new Technique("Textured.fx", "vdcl_position_uv", "VSTextured", fxName, psName);
    assert(tech != nullptr);
    Technique::getManager().registerNew(Technique::getManager().getFreshName("textured::"+psName), tech);
}

const Texture* BasicEffect::operator()(const PostProcessPipeline*, const Texture* in)
{
    out->activate();
    drawTexture2D(pixelRect(resX, resY), pixelRect(resX, resY), in, tech);
    return out;
}

void BasicEffect::init()
{
    out = new RenderedTexture;
    out->create(("fx_" + name + "_out").c_str(), resX, resY,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, zBufferType);
}

std::map<std::string, EffectLibrary::factory_fn> EffectLibrary::factory;


#ifdef _DEBUG

static void TW_CALL setBasicEffectTech(const void *value, void *clientData)
{
    auto codedName = *static_cast<const std::string*>(value);
    auto separator = codedName.find("::");
    std::string file = codedName.substr(0, separator);
    std::string ps = codedName.substr(separator+2, std::string::npos);
    BasicEffect* fx = static_cast<BasicEffect*>(clientData);
    fx->setTech(file, ps);
}

static void TW_CALL readBasicEffectTech(void *value, void *clientData)
{
    BasicEffect* fx = static_cast<BasicEffect*>(clientData);
    const Technique* tech = fx->getTech();
    std::string codedName = tech->getPSFileName() + "::" + tech->getPSName();
    TwCopyStdStringToLibrary(*static_cast<std::string*>(value), codedName);
}

bool BasicEffect::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarCB(bar, (name+"_tech").c_str(), TW_TYPE_STDSTRING, setBasicEffectTech, readBasicEffectTech, this,
        std::string("label='Tech' group='"+groupName+"'").c_str());
    return true;
}


void BasicEffect::writeToXML(std::ostream& out, const std::string& stageName,
    const std::string& indent, const std::string& tag, utils::MKeyValue& atts) const
{
    atts.setString("name", stageName);
    atts.setString("file", tech->getPSFileName());
    atts.setString("ps", tech->getPSName());
    if (debugZBuffer) {
        atts.setString("backZBuffer", "debug");
    } else if (zBufferType == RenderedTexture::USE_BACK_ZBUFFER) {
        atts.setBool("backZBuffer", true);
    }
    atts.writeSingle(out, tag.c_str(), indent, " ", "");
}

static void TW_CALL savePPP(void* clientData)
{
    PostProcessPipeline* ppp(static_cast<PostProcessPipeline*>(clientData));
    std::string pppName = ppp->getName();
    utils::MKeyValue atts;
    atts.setString("name", pppName);
    std::ofstream out("data/fx/pipelines/"+pppName+".xml");
    atts.writeStartElement(out, "pipeline");
    for(const auto& fx : ppp->getEffects()) {
        fx.effect->writeToXML(out, fx.name, "\t");
    }
    atts.writeEndElement(out, "pipeline");
}

TwBar*& PostProcessPipeline::antTW_createTweak()
{
    if(antTW_bar != nullptr) {TwDeleteBar(antTW_bar); antTW_bar = nullptr;}
    std::string barName = (getFullName()+"_PPP");
    antTW_bar = TwNewBar(barName.c_str());

    TwAddButton(antTW_bar, "_close", closeTwBar, &antTW_bar, "label='Close' ");
    TwAddButton(antTW_bar, "_label", nullptr, this, ("label='data/fx/pipelines/"+name+"'.xml").c_str());
    TwAddButton(antTW_bar, "_save", savePPP, this, "label='Save' ");
    TwAddVarRW(antTW_bar, "_skip", TW_TYPE_BOOLCPP, &skip, "label='MUTE'");
    TwAddSeparator(antTW_bar,nullptr, nullptr);
    for(auto& fx : effects) {
        TwAddVarRW(antTW_bar, (fx.name+"_PPPskip").c_str(), TW_TYPE_BOOLCPP, &fx.skip,
            std::string("label='MUTE' group='"+fx.name+"'").c_str());
        TwAddVarRW(antTW_bar, (fx.name+"_PPPbreak").c_str(), TW_TYPE_BOOLCPP, &fx.end,
            std::string("label='END' group='"+fx.name+"'").c_str());
        if (fx.effect->antTW_addTweak(antTW_bar, fx.name)) {
            std::string str("'"+barName + "'/'"+fx.name+"' opened=false "+
                "label='"+ fx.name+": "+ fx.className +"' ");
            TwDefine(str.c_str());
        } else {
            std::string str("label='"+ fx.name+": "+ fx.className +"' ");
            TwAddButton(antTW_bar, NULL, NULL, NULL, str.c_str());
        }
    }
    TwDefine(std::string("'"+barName + "' "
        "label='FXPipeline - " + name + "' " +
        "color='225 225 200' text=dark"
        ).c_str());
    return antTW_bar;
}

#endif

}