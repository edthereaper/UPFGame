#include "mcv_platform.h"
#include "app.h"

//#define DEBUG_RENDER_LAYER // WARNING! Very very verbose

namespace render {

ID3D11Device*           Render::device=nullptr;
ID3D11DeviceContext*    Render::ctx=nullptr;
IDXGISwapChain*         Render::swap_chain=nullptr;
ID3D11RenderTargetView* Render::render_target_view=nullptr;
ID3D11Texture2D*        Render::depth_stencil=nullptr;
ID3D11DepthStencilView* Render::depth_stencil_view=nullptr;

unsigned Render::width  = 0;
unsigned Render::height = 0;

bool Render::activateBackBuffer()
{
    ctx->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
    
    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    ctx->RSSetViewports(1, &vp);

    return true;
}

bool Render::createDepthStencils()
{
    HRESULT hr;

    //Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    utils::dbg("new depth stencil: %dx%d\n", width, height);
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = device->CreateTexture2D(&descDepth, NULL, &depth_stencil);
    if (FAILED(hr)) {return false;}

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = device->CreateDepthStencilView(depth_stencil, &descDSV, &depth_stencil_view);
    if (FAILED(hr))
        return false;

    return true;
}

bool Render::createRenderTargetView()
{
    HRESULT hr;
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {return false;}

    hr = device->CreateRenderTargetView(pBackBuffer, NULL, &render_target_view);
    pBackBuffer->Release();
    if (FAILED(hr)) {return false;}

    return true;
}

bool Render::createDevice()
{
    HRESULT hr = S_OK;
    App& app = App::get();

    RECT rc;
    GetClientRect(app.hWnd, &rc);
	width = app.getConfigX();
	height = app.getConfigY();

	//Check the desktop resolution, if its lower than the resolution set in config. Set the resolution same as desktop.
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	if ((int)width > desktop.right)		width = desktop.right;
	if ((int)height > desktop.bottom)	height = desktop.bottom;

    UINT createDeviceFlags = 0;
    #if defined(_DEBUG) && defined(DEBUG_RENDER_LAYER)
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_0,
      //D3D_FEATURE_LEVEL_10_1,
      //D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = app.hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
      D3D_DRIVER_TYPE g_driverType = driverTypes[driverTypeIndex];
      hr = D3D11CreateDeviceAndSwapChain(
          NULL, g_driverType, NULL, createDeviceFlags,
          featureLevels, numFeatureLevels,
          D3D11_SDK_VERSION,
          &sd, &swap_chain,
          &device, &featureLevel,
          &ctx);
      if (SUCCEEDED(hr))
        break;
    }

    if (FAILED(hr))
      return false;

    createRenderTargetView();
    createDepthStencils();
    activateBackBuffer();
    
    if (!app.config.windowed) {
        hr = swap_chain->SetFullscreenState(TRUE, NULL);
        if (FAILED(hr))
          return false;
    }

    return true;
}

void Render::destroyDevice()
{
    ID3D11Debug * debug = nullptr;
#ifdef DEBUG_RENDER_LAYER
    HRESULT hr = device->QueryInterface(IID_PPV_ARGS(&debug));
#endif

    swap_chain->SetFullscreenState(FALSE, NULL);

    if (ctx) ctx->ClearState();
    SAFE_RELEASE(depth_stencil_view);
    SAFE_RELEASE(depth_stencil);
    SAFE_RELEASE(render_target_view);
    SAFE_RELEASE(swap_chain);
    SAFE_RELEASE(ctx);
    
#ifdef DEBUG_RENDER_LAYER
    if (debug != nullptr) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        debug->Release();
    }
#endif

    SAFE_RELEASE(device);
}

bool Render::compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    #if defined( DEBUG ) || defined( _DEBUG )
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;
    #endif

    // To avoid having to transpose the matrix in the ctes buffers when
    // uploading matrix to the gpu, and to keep multiplying vector * matrix
    // in the shaders instead of matrix * vector (as in gles20)
    dwShaderFlags |= D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;

    wchar_t wFilename[MAX_PATH];
    mbstowcs(wFilename, szFileName, MAX_PATH);

    while (true) {
        ID3DBlob* pErrorBlob = nullptr;
        hr = D3DCompileFromFile(
            wFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            szEntryPoint, szShaderModel,
            dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

        if (FAILED(hr)) {
            const char* error = (pErrorBlob==nullptr) ? "Unknown error." : (const char*)pErrorBlob->GetBufferPointer();
            utils::dbg("Compiling %s.%s: %s", szFileName, szEntryPoint, error);
            if (pErrorBlob != nullptr) {
                int rc = MessageBox(NULL, error,
                    "Shader compiler Error", MB_YESNO | MB_ICONEXCLAMATION);
                if (rc == IDNO) {utils::fatal("Shader compiler Error\n");}
            }
            if (pErrorBlob!=nullptr) {pErrorBlob->Release();}
            continue;
        } else {
            if (pErrorBlob != nullptr) {
                utils::dbg("Compiling %s.%s: %s", szFileName, szEntryPoint,
                    pErrorBlob->GetBufferPointer());
            }
            if (pErrorBlob!=nullptr) {pErrorBlob->Release();}
            break;
        }
    }
    return true;
}

}