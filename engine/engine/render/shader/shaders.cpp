#include "mcv_platform.h"
#include "shaders.h"

using namespace utils;

namespace render {

VertexShader::Manager VertexShader::manager;
PixelShader::Manager PixelShader::manager;

bool Technique::reloading = false;
Technique* Technique::current = nullptr;

bool VertexShader::compile(const char* szFileName, const char* szEntryPoint, const VertexDecl &decl) {
    ID3DBlob* pVSBlob = NULL;
    bool is_ok = Render::compileShaderFromFile(szFileName, szEntryPoint, "vs_4_0", &pVSBlob);
    if (!is_ok) {return false;}

    // Create the vertex shader
    HRESULT hr;
    hr = Render::getDevice()->CreateVertexShader(
        pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vs);

    if (FAILED(hr)) {
      pVSBlob->Release();
      return false;
    }

    // Create the input layout
    hr = Render::getDevice()->CreateInputLayout(
        decl.elems, decl.nelems,
        pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
        &vtx_layout);

    pVSBlob->Release();
    if (FAILED(hr)) {return false;}
    #ifdef _DEBUG
        std::stringstream ss;
        ss << szFileName <<"::"<< szEntryPoint;
        setDbgName( vs, ss.str().c_str());
    #endif
    return true;
}

void VertexShader::destroy()
{
    SAFE_RELEASE(vs);
    SAFE_RELEASE(vtx_layout);
}

void VertexShader::activate() const
{
    assert(vs);
    Render::getContext()->VSSetShader( vs, NULL, 0);

    // Set the input layout
    Render::getContext()->IASetInputLayout( vtx_layout );
}

void VertexShader::deactivate()
{
    Render::getContext()->VSSetShader( nullptr, NULL, 0);
    Render::getContext()->IASetInputLayout( nullptr );
}

bool PixelShader::compile(const char* szFileName, const char* szEntryPoint)
{
    ID3DBlob* pPSBlob = NULL;
    bool is_ok = Render::compileShaderFromFile(szFileName, szEntryPoint, "ps_4_0", &pPSBlob);
    if (!is_ok) {return false;}

    // Create the vertex shader
    HRESULT hr;
    hr = Render::getDevice()->CreatePixelShader(
      pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),
      NULL, &ps);

    if (FAILED(hr)) {
      pPSBlob->Release();
      return false;
    }
    #ifdef _DEBUG
        std::stringstream ss;
        ss << szFileName <<"::"<< szEntryPoint;
        setDbgName( ps, ss.str().c_str());
    #endif

    pPSBlob->Release();
    return true;
}

void PixelShader::destroy()
{
    SAFE_RELEASE(ps);
}


void PixelShader::activate() const
{
    Render::getContext()->PSSetShader(ps, NULL, 0);
}
void PixelShader::deactivate()
{
    Render::getContext()->PSSetShader(nullptr, NULL, 0);
}

Technique::Manager Technique::manager;

bool Technique::load(const char* name)
{
    char full_name[MAX_PATH];
    sprintf(full_name, "%s/%s.xml", "data/techniques", name);
    return xmlParseFile(full_name);
}

void Technique::activate() const
{
    if (current == this) {return;}

    assert(vs != nullptr);
    vs->activate();
    if (ps != nullptr) {ps->activate();}
    else {PixelShader::deactivate();}

    current = const_cast<Technique*const>(this);
}

void Technique::destroy()
{
}

bool Technique::loadVS(std::string _fxName, std::string _vsName, const VertexDecl* decl)
{
    vsfxName = _fxName;
    vsName = _vsName;
    assert(decl != nullptr);
    if (!vsName.empty()) {
        std::stringstream ss;
        ss << vsfxName <<"::"<<vsName;
        vs = VertexShader::getManager().getByName(ss.str().c_str());
        assert(vs != nullptr);
        if (vs->vs == nullptr || reloading) {
            if (reloading && vs->vs != nullptr) {vs->destroy();}
            return vs->compile(("data/fx/"+vsfxName).c_str(), vsName.c_str(), *decl);
        }
        return true;
    } else {
        return true;
    }
}

bool Technique::loadPS(std::string _fxName, std::string _psName)
{
    psfxName = _fxName;
    psName = _psName;
    if (!psName.empty()) {
        std::stringstream ss;
        ss << psfxName <<"::"<<psName;
        ps = PixelShader::getManager().getByName(ss.str().c_str());
        assert(ps != nullptr);
        if (ps->ps == nullptr || reloading) {
            if (reloading && ps->ps != nullptr) {ps->destroy();}
                return ps->compile(("data/fx/"+psfxName).c_str(), psName.c_str());
        }
        return true;
    } else {
        return true;
    }
}

const VertexDecl* Technique::getVertexDeclaration(std::string vdName)
{
    const VertexDecl* decl = nullptr;
    if (vdName == "vdcl_position") {
      decl = getVertexDecl<VertexPos>();
    } else if (vdName == "vdcl_position_color") {
      decl = getVertexDecl<VertexPosColor>();
    } else if (vdName == "vdcl_position_uv") {
      decl = getVertexDecl<VertexPosUV>();
    } else if (vdName == "vdcl_position_uv_normal") {
      decl = getVertexDecl<VertexPosUVNormal>();
    } else if (vdName == "vdcl_position_uv_normal_tangent") {
      decl = getVertexDecl<VertexPosUVNormalTangent>();
    } else if (vdName == "vdcl_skin") {
      decl = getVertexDecl<VertexSkin>();
    } else if (vdName == "vdcl_punt_instance") {
      decl = getVertexDecl<VertexPUNTInstance>();
	} else if (vdName == "vdcl_particle_data") {
	  decl = getVertexDecl<VertexParticleUData>();
    } else if (vdName == "vdcl_paint") {
	  decl = getVertexDecl<VertexPaintData>();
    } else {
        assert(!fatal("Unsupported vertex declaration %s", vdName.c_str()));
    }
    return decl;
}

Technique::Technique(std::string vsfxName, std::string _vdName,
    std::string vsName, std::string psfxName, std::string psName)
{
    vdName = _vdName;
    auto decl = getVertexDeclaration(vdName);
    bool is_ok=true;
    is_ok &= loadVS(vsfxName, vsName, decl);
    is_ok &= loadPS(psfxName, psName);
    assert(is_ok);
}

void Technique::onStartElement(const std::string &elem, MKeyValue &atts)
{
    if (elem == "technique") {
        std::string vsName = atts["vs"];
        std::string psName = atts["ps"];
        std::string fxName = atts.getString("fx", "textured.fx");

        vdName = atts["vs_decl"];

        const VertexDecl* decl = getVertexDeclaration(vdName);

        bool is_ok = true;
        
        is_ok &= loadVS(fxName, vsName, decl);
        is_ok &= loadPS(fxName, psName);

        uses_bones = atts.getBool("uses_bones", false);

        assert(is_ok);
    } else {
        dbg("Unknown element %s when parsing file.", elem.c_str());
    }
}

}