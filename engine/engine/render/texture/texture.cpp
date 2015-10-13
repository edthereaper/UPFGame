#include "mcv_platform.h"
#include "texture.h"
#include "DDSTextureLoader.h"

namespace render {

Texture::Manager Texture::manager;
const std::string Texture::PATH = "data/textures";

void Texture::activate(int slot) const
{
    assert(resourceView != nullptr);
    Render::getContext()->PSSetShaderResources(slot, 1, &resourceView);
}

void Texture::deactivate(int slot, int nslots)
{
    static ID3D11ShaderResourceView* null_views[16] = {
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr,};
    Render::getContext()->PSSetShaderResources(slot, nslots, null_views);
}

bool Texture::load(const char* name)
{
    char full_name[MAX_PATH];
    sprintf(full_name, "%s/%s.dds", PATH.c_str(), name);

    // Convert the byte string to wchar string
    wchar_t wname[MAX_PATH];
    mbstowcs(wname, full_name, MAX_PATH);

    // Load the Texture
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        Render::getDevice(),
        wname,
        (ID3D11Resource**)&resource, &resourceView,
        0, nullptr
        );
    if (FAILED(hr)) {return false;}

    return true;
}

void Texture::destroy()
{
  SAFE_RELEASE(resource);
  SAFE_RELEASE(resourceView);
}

}