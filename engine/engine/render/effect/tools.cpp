#include "mcv_platform.h"
#include "tools.h"

#include "render/render_utils.h"

using namespace DirectX;

namespace render {

ShaderCte<SCTES_Mix> Mix::ctes;


void Mix::initType()
{
    ctes.create("mix");
}

void Mix::tearDown()
{
    ctes.destroy();
}

const Texture* Mix::operator()(const PostProcessPipeline* ppp, const Texture* in)
{
    const Texture* weight = weightFetch.isValid() ? weightFetch(ppp, in) : nullptr;
    const Texture* mix = sourceFetch(ppp, in);

    ctes.activateInPS(SCTES_Mix::SLOT);
    
    auto& cb = ctes.get();
    cb.MixVector = mask;
    cb.MixFactor = factor;
    cb.MixBaseFactor = baseFactor;
    cb.MixRangeMax = rangeMax;
    cb.MixRangeMin = rangeMin;
    ctes.uploadToGPU();
    out->activate();
    if (mix != nullptr) {
        mix->activate(1);
    } else {
        Texture::deactivate(1);
    }
    if (weight != nullptr) {weight->activate(2);}
    drawTexture2D(pixelRect(0, 0, resX, resY), pixelRect(resX, resY), in, tech);
    realOut = out;
    return realOut;
}

#ifdef _DEBUG


bool Fetch::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    //{ STATIC
    static const TwEnumVal fetchType_e [] = {
        {params_t::NONE, "None"}, 
        {params_t::RESOURCE, "Resource"},
        {params_t::STAGE_OUTPUT, "Stage"},
        {params_t::TEXTURE, "Texture"},
    };
    static const TwType fetchType = TwDefineEnum("FetchFX-TYPE", fetchType_e, ARRAYSIZE(fetchType_e));
    //} STATIC

    TwAddVarRW(bar, (groupName+"_Fetch_type").c_str(), fetchType, &params.resourceType,
        std::string("label='Fetch' group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (groupName+"_Fetch_resource").c_str(), TW_TYPE_STDSTRING, &params.name,
        std::string("label='Name' group='"+groupName+"'").c_str());
    return true;
}

bool Fetch::antTW_addTweak(TwBar* bar, const std::string& groupName,
    const std::string& varName, const std::string& varLabel)
{
    //{ STATIC
    static const TwEnumVal fetchType_e [] = {
        {params_t::NONE, "None"}, 
        {params_t::RESOURCE, "Resource"},
        {params_t::STAGE_OUTPUT, "Stage"},
        {params_t::TEXTURE, "Texture"},
    };
    static const TwType fetchType = TwDefineEnum("FetchFX-TYPE2", fetchType_e, ARRAYSIZE(fetchType_e));

    static const TwStructMember fetchTWStruct_def [] = {
        {"Fetch", fetchType, offsetof(params_t, resourceType), "label=Fetch"},
        {"Name", TW_TYPE_STDSTRING, offsetof(params_t, name), "label=Name"},
    };
    static const TwType fetchTWStruct = TwDefineStruct("FetchFX-STRUCT",
        fetchTWStruct_def, ARRAYSIZE(fetchTWStruct_def), sizeof(fetchTWStruct_def), nullptr, nullptr);
    //} STATIC

    TwAddVarRW(bar, varName.c_str(), fetchTWStruct, &params,
        std::string("label='"+varLabel+"' opened=false group='"+groupName+"'").c_str());
    return true;
}

bool Fetch::writeToXML(std::ostream& out, const std::string& stageName,
    const std::string& indent, const std::string& elem, bool ignoreNone) const
{
    utils::MKeyValue atts;
    if (stageName!="") atts.setString("name", stageName);
    switch(params.resourceType) {
        case params_t::RESOURCE: atts.setString("resource", params.name); break;
        case params_t::STAGE_OUTPUT: atts.setString("stage", params.name); break;
        case params_t::TEXTURE: atts.setString("texture", params.name); break;
        default: if (ignoreNone) {return false;}
    }
    atts.writeSingle(out, elem.c_str(), indent);
    return true;
}

void Mix::writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const
{
    utils::MKeyValue atts;
    atts.setFloat4("mask", mask, 2);
    atts.setFloat("factor", factor, 2);
    atts.setFloat("baseFactor", baseFactor, 3);
    atts.setFloat("rangeMax", rangeMax, 3);
    atts.setFloat("rangeMin", rangeMin, 3);
    atts.setString("name", stageName);
    atts.setString("file", tech->getPSFileName());
    atts.setString("ps", tech->getPSName());
    atts.writeStartElement(out, "Mix", indent, " ", "");
    sourceFetch.writeToXML(out, "", indent+"\t", "source");
    weightFetch.writeToXML(out, "", indent+"\t", "weight");
    atts.writeEndElement(out, "Mix", indent);
}

bool Mix::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarRW(bar, (name+"_factor").c_str(), TW_TYPE_FLOAT, &factor,
        std::string("label='Factor' min=-5 max=5 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_baseFactor").c_str(), TW_TYPE_FLOAT, &baseFactor,
        std::string("label='Base factor' min=0 max=1 step=0.001 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_rangeMax").c_str(), TW_TYPE_FLOAT, &rangeMax,
        std::string("label='Range max' min=-0.000001 max=5 step=0.001 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_rangeMin").c_str(), TW_TYPE_FLOAT, &rangeMin,
        std::string("label='Range min' min=-0.000001 max=5 step=0.001 group='"+groupName+"'").c_str());
    sourceFetch.antTW_addTweak(bar, groupName, name+"src", "Source");
    weightFetch.antTW_addTweak(bar, groupName, name+"wgt", "Weight");
    TwAddVarRW(bar, (name+"_mask").c_str(), TW_TYPE_COLOR4F, &mask,
        std::string("label='Mask' group='"+groupName+"'").c_str());
    BasicEffect::antTW_addTweak(bar, groupName);
    return true;
}

bool DebugFX::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarRW(bar, (name+"_mute").c_str(), TW_TYPE_BOOLCPP, &off,
        std::string("label='Off' group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_mute").c_str(), TW_TYPE_BOOLCPP, &force,
        std::string("label='Force' group='"+groupName+"'").c_str());
    return BasicEffect::antTW_addTweak(bar, groupName);
}

void DebugFX::writeToXML(std::ostream& out, const std::string& stageName,
    const std::string& indent) const
{
    utils::MKeyValue atts;
    atts.setBool("on", !off);
    BasicEffect::writeToXML(out, stageName, indent, "DebugFX", atts);
}

#endif
}