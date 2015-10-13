#include "mcv_platform.h"
#include "textureCube.h"
#include "DDSTextureLoader.h"

using namespace DirectX;
using namespace utils;

namespace render {
    
const XMVECTOR TextureCube::faceDir[6] =
{
    xAxis_v, -xAxis_v,
    yAxis_v, -yAxis_v,
    zAxis_v, -zAxis_v,
};    
const XMVECTOR TextureCube::faceDirFix[6] =
{
    xAxis_v, -xAxis_v,
    yAxis_v, -yAxis_v,
    -zAxis_v, zAxis_v,
};    
const XMVECTOR TextureCube::faceUp[6] =
{
    yAxis_v,  yAxis_v,
    zAxis_v,  -zAxis_v,
    yAxis_v,  yAxis_v,
};

bool TextureCube::load(const char* name)
{
    char full_name[MAX_PATH];
    sprintf(full_name, "%s/%s.dds", "data/textures", name);

    // Convert the byte string to wchar string
    wchar_t wname[MAX_PATH];
    mbstowcs(wname, full_name, MAX_PATH);

    // Load the TextureCube
    HRESULT hr = DirectX::CreateDDSTextureFromFileEx(
        Render::getDevice(),
        wname, 0,
        D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE,
        false,
        (ID3D11Resource**)&resource, &resourceView,
        nullptr
        );
    if (FAILED(hr)) {return false;}

    return true;
}

}