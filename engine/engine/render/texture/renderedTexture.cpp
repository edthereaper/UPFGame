#include "mcv_platform.h"
#include "renderedTexture.h"

namespace render {

bool RenderedTexture::create(const char* newName, int newXRes, int newYRes,
    DXGI_FORMAT newColorFormat, DXGI_FORMAT newDepthFormat,
    zBufferType_e zbuffer_type, bool owned)
{
    // Save the params
    xRes = newXRes;
    yRes = newYRes;
    name = Texture::getManager().getFreshName(newName);
    zBufferType = zbuffer_type;

    assert(xRes > 0 && yRes > 0);
    colorFormat = newColorFormat;
    depthFormat = newDepthFormat;

    if (colorFormat != DXGI_FORMAT_UNKNOWN) {
        if (!createColorBuffer(owned)) {
            return false;
        }
    }

    // Create ZBuffer
    if (zbuffer_type == USE_OWN_ZBUFFER) {
        if (depthFormat != DXGI_FORMAT_UNKNOWN) {
            if (!createDepthBuffer(owned)) {
                return false;
            }
        }
    } else if (zbuffer_type == NO_ZBUFFER) {
        depthStencilView = nullptr;
    } else {
        assert(zbuffer_type == USE_BACK_ZBUFFER);
        depthStencilView = Render::getDepthStencilView();
    }
    
    return true;
}

bool RenderedTexture::createColorBuffer(bool owned)
{
    // Create a color surface
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = xRes;
    desc.Height = yRes;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = colorFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HRESULT hr = Render::getDevice()->CreateTexture2D(&desc, NULL, &resource);
    if (FAILED(hr)) {return false;}

    // Create the render target view
    hr = Render::getDevice()->CreateRenderTargetView(resource, NULL, &renderTargetView);
    if (FAILED(hr)) {return false;}
    #ifdef _DEBUG
    {
        std::stringstream ss;
        ss << name << "_RTV";
        setDbgName(renderTargetView, ss.str().c_str());
    }
    #endif

    // Create a resource view so we can use it in the shaders as input
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    memset(&SRVDesc, 0, sizeof(SRVDesc));
    SRVDesc.Format = colorFormat;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = desc.MipLevels;
    hr = Render::getDevice()->CreateShaderResourceView(resource, &SRVDesc, &resourceView);
    if (FAILED(hr)) {return false;}

    return getManager().registerNew(name.c_str(), this, owned);
}

bool RenderedTexture::createDepthBuffer(bool owned)
{
    ID3D11Device* device = Render::getDevice();
    //assert(ztexture.resource_view == NULL);
    assert(depthFormat != DXGI_FORMAT_UNKNOWN);
    assert(depthFormat == DXGI_FORMAT_R32_TYPELESS
        || depthFormat == DXGI_FORMAT_R24G8_TYPELESS
        || depthFormat == DXGI_FORMAT_R16_TYPELESS
        || depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT
        || depthFormat == DXGI_FORMAT_R8_TYPELESS);

    D3D11_TEXTURE2D_DESC          depthBufferDesc;

    // Init depth and stencil buffer
    // Initialize the description of the depth buffer.
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = xRes;
    depthBufferDesc.Height = yRes;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = depthFormat;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    //  if (bind_shader_resource)
    depthBufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

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
        SRVfmt = depthBufferDesc.Format;
        DSVfmt = depthBufferDesc.Format;
        break;
    }

    // Create the texture for the de  pth buffer using the filled out description.
    ID3D11Texture2D* ztexture2d;
    HRESULT hr = device->CreateTexture2D(&depthBufferDesc, NULL, &ztexture2d);
    if (FAILED(hr)) {return false;}

    // Initialize the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DSVfmt;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    hr = device->CreateDepthStencilView(ztexture2d, &depthStencilViewDesc, &depthStencilView);
    if (FAILED(hr)) {return false;}
    #ifdef _DEBUG
    {
        std::stringstream ss;
        ss << name << "_DSV";
        setDbgName(depthStencilView, ss.str().c_str());
    }
    #endif

    // Setup the description of the shader resource view.
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = SRVfmt;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = depthBufferDesc.MipLevels;

    // Create the shader resource view.
    ID3D11ShaderResourceView* depth_resource_view = nullptr;
    hr = device->CreateShaderResourceView(ztexture2d, &shaderResourceViewDesc, &depth_resource_view);
    if (FAILED(hr)) {return false;}

    std::string texName = name + "_Z";
    zTexture = new Texture();
    zTexture->resourceView = depth_resource_view;
    setDbgName(zTexture->resourceView, texName.c_str());
    
    SAFE_RELEASE(ztexture2d);
    return Texture::getManager().registerNew(texName.c_str(), zTexture, owned);
}

void RenderedTexture::activateViewport() const
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

void RenderedTexture::activate() const
{
    Render::getContext()->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    activateViewport();
}


void RenderedTexture::clearRenderTargetView(const float color[4])
{
    Render::getContext()->ClearRenderTargetView(renderTargetView, color);
}

void RenderedTexture::clearDepthBuffer()
{
    assert(depthStencilView);
    Render::getContext()->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

}