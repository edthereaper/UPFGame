#include "mcv_platform.h"
#include "Smudge.h"

#include "render/render_utils.h"

namespace render {
    
ShaderCte<SCTES_Smudge> Smudge::ctes;

void Smudge::initType()
{
    ctes.create("Smudge");
}

void Smudge::tearDown()
{
    ctes.destroy();
}

void Smudge::init()
{
    assert(resX != 0 && resY != 0);
    bool ok = true;
    out = new RenderedTexture;
    ok &= out->create(("fx_" + name + "_dsV").c_str(), int(resX * factor), int(resY * factor),
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, RenderedTexture::NO_ZBUFFER);
    assert(ok && out!=nullptr);
}


const Texture* Smudge::operator()(const PostProcessPipeline* ppp, const Texture* in)
{
    const Texture* auxTexture = activateTexture.isValid() ? activateTexture(ppp, in) : nullptr;

    ctes.activateInPS(SCTES_Smudge::SLOT);
    
    // Sample horizontally
    auto& cb = ctes.get();
    cb.SmudgeAmp.x = amplitude / resX;
    cb.SmudgeAmp.y = amplitude / resY;
    cb.SmudgeTolerance.x = toleranceX;
    cb.SmudgeTolerance.y = toleranceY;
    ctes.uploadToGPU();
    out->activate();
    if (auxTexture != nullptr) {auxTexture->activate(1);}
    drawTexture2D(pixelRect(0, 0, resX, resY), pixelRect(resX, resY), in, tech);

    return out;
}

#ifdef _DEBUG

static void TW_CALL readSmudgeDownscale(void *value, void *clientData)
{
    Smudge* smudge = static_cast<Smudge*>(clientData);
    if (smudge != nullptr) {*static_cast<float *>(value) = smudge->getFactor();}
}

static void TW_CALL setSmudgeDownscale(const void *value, void *clientData)
{
    Smudge* smudge = static_cast<Smudge*>(clientData);
    if (smudge != nullptr) {
        smudge->setFactor(*static_cast<const float *>(value));
        smudge->cleanup();
        smudge->init();
    }
}

void Smudge::writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const
{
    utils::MKeyValue atts;
    atts.setFloat("factor", factor, 2);
    atts.setFloat("amplitude", amplitude, 4);
    atts.setFloat("toleranceX", toleranceX);
    atts.setFloat("toleranceY", toleranceY);
    atts.setString("name", stageName);
    atts.setString("file", tech->getPSFileName());
    atts.setString("ps", tech->getPSName());

    if (activateTexture.isValid()) {
        atts.writeStartElement(out, "Smudge", indent, " ", "");
        activateTexture.writeToXML(out, "", indent+"\t", "activate");
        atts.writeEndElement(out, "Smudge", indent);
    } else {
        atts.writeSingle(out, "Smudge", indent, " ", "");
    }
}

bool Smudge::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarCB(bar, (name+"_factor").c_str(), TW_TYPE_FLOAT, setSmudgeDownscale, readSmudgeDownscale, this,
        std::string("label='Factor' min=0.05 max=2 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_amplitude").c_str(), TW_TYPE_FLOAT, &amplitude,
        std::string("label='Amplitude' min=0 max=100 step=0.01 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_toleranceX").c_str(), TW_TYPE_FLOAT, &toleranceX,
        std::string("label='Tolerance in X' min=0 max=10 step=0.001 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_toleranceY").c_str(), TW_TYPE_FLOAT, &toleranceY,
        std::string("label='Tolerance in Y' min=0 max=10 step=0.001 group='"+groupName+"'").c_str());
    activateTexture.antTW_addTweak(bar, groupName, name+"aux", "Activate");
    BasicEffect::antTW_addTweak(bar, groupName);
    return true;
}
#endif
}