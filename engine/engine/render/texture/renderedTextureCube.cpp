#include "mcv_platform.h"
#include "RenderedTextureCube.h"
#include "renderedTexture.h"

namespace render {

bool RenderedTextureCube::create(const char* newName, int newXRes, int newYRes,
    DXGI_FORMAT newColorFormat, DXGI_FORMAT newDepthFormat,
    zBufferType_e zbuffer_type, bool owned)
{
    // Save the params
    xRes = newXRes;
    yRes = newYRes;
    zBufferType = zbuffer_type;
    name = Texture::getManager().getFreshName(newName);

    assert(xRes > 0 && yRes > 0);
    colorFormat = newColorFormat;
    depthFormat = newDepthFormat;

    if (colorFormat != DXGI_FORMAT_UNKNOWN) {
        if (!createColorBuffer(owned)) {
            return false;
        }
    }

    // Create ZBuffer
    if (zbuffer_type == USE_SIX_ZBUFFER || zbuffer_type == USE_ONE_ZBUFFER /*TODO: do this!*/) {
        if (depthFormat != DXGI_FORMAT_UNKNOWN) {
            if (!createDepthBuffer(owned)) {
                return false;
            }
        }
    } else if (zbuffer_type == RenderedTexture::NO_ZBUFFER) {
        for(auto& dsv : depthStencilView) {dsv = nullptr;}
    } else {
        assert(zbuffer_type == RenderedTexture::USE_BACK_ZBUFFER);
        for(auto& dsv : depthStencilView) {dsv = Render::getDepthStencilView();}
    }
    
    return true;
}

bool RenderedTextureCube::createColorBuffer(bool owned)
{
    // Create a color surface
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = xRes;
    desc.Height = yRes;
    desc.MipLevels = 1;
    desc.ArraySize = 6;
    desc.Format = colorFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    HRESULT hr = Render::getDevice()->CreateTexture2D(&desc, NULL, &resource);
    if (FAILED(hr)) {return false;}

    // Create the render target views
    for (int i=0; i<6; ++i) {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        ZeroMemory(&rtvDesc, sizeof(rtvDesc));
        rtvDesc.Format = colorFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        rtvDesc.Texture2DArray.ArraySize = 1;
        hr = Render::getDevice()->CreateRenderTargetView(resource, &rtvDesc, &renderTargetView[i]);
        #ifdef _DEBUG
            std::stringstream ss;
            ss <<name<<"_RTV["<<i<<"]";
            setDbgName(renderTargetView[i], ss.str().c_str());
        #endif
        if (FAILED(hr)) {return false;}
    }

    // Create a resource view so we can use it in the shaders as input
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory(&SRVDesc, sizeof(SRVDesc));
    SRVDesc.Format = colorFormat;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SRVDesc.TextureCube.MipLevels = desc.MipLevels;
    SRVDesc.TextureCube.MostDetailedMip = 0;
    hr = Render::getDevice()->CreateShaderResourceView(resource, &SRVDesc, &resourceView);
    if (FAILED(hr)) {return false;}

    return Texture::getManager().registerNew(name.c_str(), this, owned);
}

bool RenderedTextureCube::createDepthBuffer(bool owned)
{
    ID3D11Device* device = Render::getDevice();
    //assert(ztexture.resource_view == NULL);
    assert(depthFormat != DXGI_FORMAT_UNKNOWN);
    assert(depthFormat == DXGI_FORMAT_R32_TYPELESS
        || depthFormat == DXGI_FORMAT_R24G8_TYPELESS
        || depthFormat == DXGI_FORMAT_R16_TYPELESS
        || depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT
        || depthFormat == DXGI_FORMAT_R8_TYPELESS);

    D3D11_TEXTURE2D_DESC          dbDesc;
    ZeroMemory(&dbDesc, sizeof(dbDesc));

    // Set up the description of the depth buffer.
    dbDesc.Width = xRes;
    dbDesc.Height = yRes;
    dbDesc.MipLevels = 1;
    dbDesc.ArraySize = 6;
    dbDesc.Format = depthFormat;
    dbDesc.SampleDesc.Count = 1;
    dbDesc.SampleDesc.Quality = 0;
    dbDesc.Usage = D3D11_USAGE_DEFAULT;
    dbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dbDesc.CPUAccessFlags = 0;
    dbDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    //  if (bind_shader_resource)
    dbDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
    DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;       // Stencil format
    DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;       // Depth format

    switch (depthFormat) {
    case DXGI_FORMAT_R32_TYPELESS:
        SRVfmt = DXGI_FORMAT_R32_FLOAT;
        DSVfmt = DXGI_FORMAT_D32_FLOAT;
        break;
    case DXGI_FORMAT_R24G8_TYPELESS:
        SRVfmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        DSVfmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    case DXGI_FORMAT_R16_TYPELESS:
        SRVfmt = DXGI_FORMAT_R16_UNORM;
        DSVfmt = DXGI_FORMAT_D16_UNORM;
        break;
    case DXGI_FORMAT_R8_TYPELESS:
        SRVfmt = DXGI_FORMAT_R8_UNORM;
        DSVfmt = DXGI_FORMAT_R8_UNORM;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        SRVfmt = dbDesc.Format;
        DSVfmt = dbDesc.Format;
        break;
    }

    // Create the texture for the de  pth buffer using the filled out description.
    ID3D11Texture2D* ztexture2d;
    HRESULT hr = device->CreateTexture2D(&dbDesc, NULL, &ztexture2d);
    if (FAILED(hr)) {return false;}


    // Create the depth stencil views
    for (int i=0; i<6; ++i) {
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        ZeroMemory(&dsvDesc, sizeof(dsvDesc));
        dsvDesc.Format = DSVfmt;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.ArraySize = 1;
        hr = Render::getDevice()->CreateDepthStencilView(ztexture2d, &dsvDesc, &depthStencilView[i]);
        if (FAILED(hr)) {return false;}
        #ifdef _DEBUG
            std::stringstream ss;
            ss <<name<<"_DSV["<<i<<"]";
            setDbgName(depthStencilView[i], ss.str().c_str());
        #endif
    }

    // Setup the description of the shader resource view.
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    SRVDesc.Format = SRVfmt;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SRVDesc.TextureCube.MostDetailedMip = 0;
    SRVDesc.TextureCube.MipLevels = dbDesc.MipLevels;

    // Create the shader resource view.
    ID3D11ShaderResourceView* depth_resource_view = nullptr;
    hr = device->CreateShaderResourceView(ztexture2d, &SRVDesc, &depth_resource_view);
    if (FAILED(hr)) {return false;}

    std::string texName = name + "_Z";
    zTexture = new TextureCube();
    zTexture->resourceView = depth_resource_view;
    setDbgName(zTexture->resourceView, texName.c_str());

    SAFE_RELEASE(ztexture2d);
    return Texture::getManager().registerNew(texName.c_str(), zTexture, owned);
}

void RenderedTextureCube::activateViewport()
{
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)xRes;
    vp.Height = (FLOAT)yRes;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    Render::getContext()->RSSetViewports(1, &vp);
}

void RenderedTextureCube::activateFace(size_t i)
{
    assert(i<6);
    Render::getContext()->OMSetRenderTargets(1, &renderTargetView[i], depthStencilView[i]);
    activateViewport();
}

void RenderedTextureCube::clearRenderTargetView(size_t i, const float color[4])
{
    assert(i<6);
    auto rtv = renderTargetView[i];
    assert(rtv);
    Render::getContext()->ClearRenderTargetView(rtv, color);
}
void RenderedTextureCube::clearDepthBuffer(size_t i)
{
    assert(i<6);
    auto dsv = depthStencilView[i];
    assert(dsv);
    Render::getContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void RenderedTextureCube::clearRenderTargetViews(const float color[4])
{
    for(auto& rtv : renderTargetView) {
        assert(rtv);
        Render::getContext()->ClearRenderTargetView(rtv, color);
    }
}

void RenderedTextureCube::clearDepthBuffers()
{
    for(auto& dsv : depthStencilView) {
        assert(dsv);
        Render::getContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
}

}