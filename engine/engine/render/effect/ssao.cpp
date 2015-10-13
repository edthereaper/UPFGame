#include "mcv_platform.h"
#include "ssao.h"

#include "render/render_utils.h"

namespace render {
    
ShaderCte<SCTES_SSAO> SSAO::ctes;

void SSAO::initType()
{
    ctes.create("ssao");
}

void SSAO::tearDown()
{
    ctes.destroy();
}

void SSAO::init()
{
    assert(resX != 0 && resY != 0);
    out = new RenderedTexture;
    out->create(("fx_" + name + "_out").c_str(), int(resX * downsampleFactor), int(resY * downsampleFactor),
        DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_UNKNOWN, RenderedTexture::NO_ZBUFFER);
}


const Texture* SSAO::operator()(const PostProcessPipeline* ppp, const Texture* in)
{
    const Texture* norTx = normals.isValid() ? normals(ppp, in) : nullptr;
    const Texture* posTx = space.isValid() ? space(ppp, in) : nullptr;

    ctes.activateInPS(SCTES_SSAO::SLOT);
    auto& cb = ctes.get();
    cb.SSAORadius = radius;
    cb.SSAOIntensity = intensity;
    cb.SSAOScale = scale;
    cb.SSAOBias = bias;
    cb.SSAODepthTolerance = depthTolerance;
    cb.SSAOJitter = jitter;
    ctes.uploadToGPU();
    out->activate();
    if (norTx != nullptr) {norTx->activate(1);}
    if (posTx != nullptr) {posTx->activate(2);}
    drawTexture2D(pixelRect(0, 0, resX, resY), pixelRect(resX, resY), in, tech);

    return out;
}

#ifdef _DEBUG

static void TW_CALL readSSAODownscale(void *value, void *clientData)
{
    SSAO* ssao = static_cast<SSAO*>(clientData);
    if (ssao != nullptr) {*static_cast<float *>(value) = ssao->getFactor();}
}

static void TW_CALL setSSAODownscale(const void *value, void *clientData)
{
    SSAO* ssao = static_cast<SSAO*>(clientData);
    if (ssao != nullptr) {
        ssao->setFactor(*static_cast<const float *>(value));
        ssao->cleanup();
        ssao->init();
    }
}

bool SSAO::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarCB(bar, (name+"_factor").c_str(), TW_TYPE_FLOAT, setSSAODownscale, readSSAODownscale, this,
        std::string("label='Downsample' min=0.05 max=2 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_radius").c_str(), TW_TYPE_FLOAT, &radius,
        std::string("label='Radius' min=0.001 max=10 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_intensity").c_str(), TW_TYPE_FLOAT, &intensity,
        std::string("label='Intensity' min=0 max=50 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_scale").c_str(), TW_TYPE_FLOAT, &scale,
        std::string("label='Scale' min=0 max=50 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_bias").c_str(), TW_TYPE_FLOAT, &bias,
        std::string("label='Bias' min=0 max=1 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_tolerance").c_str(), TW_TYPE_FLOAT, &depthTolerance,
        std::string("label='Depth tolerance' min=0 max=20 step=0.001 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_jitter").c_str(), TW_TYPE_FLOAT, &jitter,
        std::string("label='Jittering' min=0 max=1 step=0.01 group='"+groupName+"'").c_str());
    normals.antTW_addTweak(bar, groupName, name+"norm", "Normals");
    space.antTW_addTweak(bar, groupName, name+"space", "Space");
    BasicEffect::antTW_addTweak(bar, groupName);
    return true;
}


void SSAO::writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const
{
    utils::MKeyValue atts;
    atts.setFloat("downsampleFactor", downsampleFactor, 2);
    atts.setFloat("radius", radius, 2);
    atts.setFloat("intensity", intensity, 2);
    atts.setFloat("scale", scale, 2);
    atts.setFloat("bias", bias, 2);
    atts.setFloat("depthTolerance", depthTolerance, 4);
    atts.setFloat("jitter", jitter, 2);
    atts.setString("name", stageName);
    atts.setString("file", tech->getPSFileName());
    atts.setString("ps", tech->getPSName());
    atts.writeStartElement(out, "SSAO", indent, " ", "");
    normals.writeToXML(out, "", indent+"\t", "normals");
    space.writeToXML(out, "", indent+"\t", "space");
    atts.writeEndElement(out, "SSAO", indent);
}
#endif

}