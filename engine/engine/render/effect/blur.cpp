#include "mcv_platform.h"
#include "blur.h"

#include "render/camera/component.h"
#include "render/render_utils.h"
#include "app.h"

namespace render {
    
ShaderCte<SCTES_Blur> Blur::ctes;
ShaderCte<SCTES_MotionBlur> MotionBlur::ctes;

void Blur::initType()
{
    ctes.create("blur");
}

void Blur::tearDown()
{
    ctes.destroy();
}


void MotionBlur::initType()
{
    ctes.create("blur");
}

void MotionBlur::tearDown()
{
    ctes.destroy();
}

void Blur::init()
{
    assert(resX != 0 && resY != 0);
    dsH = new RenderedTexture;
    bool ok = true;
    ok &= dsH->create(("fx_" + name + "_dsH").c_str(), int(resX * factor), int(resY),
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, RenderedTexture::NO_ZBUFFER);
    out = new RenderedTexture;
    ok &= out->create(("fx_" + name + "_dsV").c_str(), int(resX * factor), int(resY * factor),
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, RenderedTexture::NO_ZBUFFER);
    assert(ok && dsH!=nullptr && out!=nullptr);
}

const Texture* Blur::operator()(const PostProcessPipeline* ppp, const Texture* in)
{
    const Texture* auxTexture = activateTexture.isValid() ? activateTexture(ppp, in) : nullptr;

    ctes.activateInPS(SCTES_Blur::SLOT);
    {
        // Sample horizontally
        auto& cb = ctes.get();
        cb.BlurDelta.x = 1.f / (float)resX;
        cb.BlurDelta.y = 0.f;
        cb.BlurAmp = amplitude;
        cb.BlurTolerance = toleranceX;
        ctes.uploadToGPU();
        dsH->activate();
        if (auxTexture != nullptr) {auxTexture->activate(1);}
        drawTexture2D(pixelRect(0, 0, resX, resY), pixelRect(resX, resY), in, tech);
    }
    {
        // Sample vertically
        auto& cb = ctes.get();
        cb.BlurDelta.x = 0.f;
        cb.BlurDelta.y = 1.f / (float)resY;
        cb.BlurAmp = amplitude;
        cb.BlurTolerance = toleranceY;
        ctes.uploadToGPU();
        out->activate();
        if (auxTexture != nullptr) {auxTexture->activate(1);}
        drawTexture2D(pixelRect(0, 0, resX, resY), pixelRect(resX, resY), dsH, tech);
    }

    return out;
}

const Texture* MotionBlur::operator()(const PostProcessPipeline* ppp, const Texture* in)
{
    const Texture* spaceTex = space.isValid() ? space(ppp, in) : nullptr;
    const Texture* dataTex  = data.isValid()  ? data(ppp, in)  : nullptr;

    ctes.activateInPS(SCTES_MotionBlur::SLOT);
    
    // Sample horizontally
    auto& cb = ctes.get();
    cb.MotionBlurPrevViewProjection = prevViewProjection;
    cb.MotionBlurAmp = amplitude;
    cb.MotionBlurNSamples = nSamples;
    ctes.uploadToGPU();
    out->activate();
    if (spaceTex != nullptr) {spaceTex->activate(1);}
    if (dataTex != nullptr) {dataTex->activate(2);}
    drawTexture2D(pixelRect(0, 0, resX, resY), pixelRect(resX, resY), in, tech);
    prevViewProjection = ((CCamera*) App::get().getCamera().getSon<CCamera>())->getViewProjection();

    return out;
}

#ifdef _DEBUG

template <class T>
static void TW_CALL readBlurDownscale(void *value, void *clientData)
{
    T* blur = static_cast<T*>(clientData);
    if (blur != nullptr) {*static_cast<float *>(value) = blur->getFactor();}
}

template <class T>
static void TW_CALL setBlurDownscale(const void *value, void *clientData)
{
    T* blur = static_cast<T*>(clientData);
    if (blur != nullptr) {
        blur->setFactor(*static_cast<const float *>(value));
        blur->cleanup();
        blur->init();
    }
}

void Blur::writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const
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
        atts.writeStartElement(out, "Blur", indent, " ", "");
        activateTexture.writeToXML(out, "", indent+"\t", "activate");
        atts.writeEndElement(out, "Blur", indent);
    } else {
        atts.writeSingle(out, "Blur", indent, " ", "");
    }
}

bool Blur::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarCB(bar, (name+"_factor").c_str(), TW_TYPE_FLOAT,
        setBlurDownscale<Blur>, readBlurDownscale<Blur>, this,
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

void MotionBlur::writeToXML(std::ostream& out, const std::string& stageName, const std::string& indent) const
{
    utils::MKeyValue atts;

    atts.setFloat("amplitude", amplitude, 3);
    atts.setInt("nSamples", nSamples);
    atts.setString("name", stageName);
    atts.setString("file", tech->getPSFileName());
    atts.setString("ps", tech->getPSName());

    if (space.isValid() || data.isValid()) {
        atts.writeStartElement(out, "MotionBlur", indent, " ", "");
        if (space.isValid()) space.writeToXML(out, "", indent+"\t", "space");
        if (data.isValid()) data.writeToXML(out, "", indent+"\t", "data");
        atts.writeEndElement(out, "MotionBlur", indent);
    } else {
        atts.writeSingle(out, "MotionBlur", indent, " ", "");
    }
}

bool MotionBlur::antTW_addTweak(TwBar* bar, const std::string& groupName)
{
    TwAddVarRW(bar, (name+"_amplitude").c_str(), TW_TYPE_FLOAT, &amplitude,
        std::string("label='Amplitude' min=0 max=100 step=0.001 group='"+groupName+"'").c_str());
    TwAddVarRW(bar, (name+"_nSamples").c_str(), TW_TYPE_UINT32, &nSamples,
        std::string("label='Samples' min=1 max=30 step=1 group='"+groupName+"'").c_str());
    space.antTW_addTweak(bar, groupName, name+"aux", "Activate");
    BasicEffect::antTW_addTweak(bar, groupName);
    return true;
}
#endif
}