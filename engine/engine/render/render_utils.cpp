#include "mcv_platform.h"
#include "render_utils.h"

#include "render_utils.h"

#include "components/color.h"

#include "mesh/component.h"

#include "illumination/shadow.h"
#include "illumination/cubeshadow.h"
#include "illumination/ptLight.h"
#include "illumination/dirLight.h"
#include "environment/volLight.h"
#include "environment/mist.h"
#include "gameElements/PaintManager.h"
using namespace gameElements;

using namespace DirectX;
using namespace utils;
using namespace component;

#include "../data/fx/platform/shader_ctes.h"

namespace render {

ShaderCte<SCTES_Object>     ctes_object;
ShaderCte<SCTES_Camera>     ctes_camera;
ShaderCte<SCTES_Light>      ctes_light;
ShaderCte<SCTES_PtLight>    ctes_ptlight;
ShaderCte<SCTES_DirLight>   ctes_dirlight;
ShaderCte<SCTES_VolLight>   ctes_volLight;
ShaderCte<SCTES_Mist>       ctes_mist;
ShaderCte<SCTES_Paint>      ctes_paint;
ShaderCte<SCTES_Render>     ctes_render;
ShaderCte<SCTES_Texture>    ctes_texture;
ShaderCte<SCTES_Global>     ctes_global;
Buffer<float4> boneBuffer SHADER_REGISTER(t10);

Mesh    mesh_cube_wire;
Mesh    mesh_cube_wire_unit;
Mesh    mesh_cube_wire_split;
Mesh    mesh_view_volume;
Mesh    mesh_view_volume_wire;
Mesh    mesh_textured_quad_xy;
Mesh    mesh_textured_quad_xy_centered;
Mesh    mesh_textured_quad_xz_centered;
Mesh    mesh_stacked_quad_xz_centered;
Mesh    mesh_icosahedron;
Mesh    mesh_line;
Mesh    mesh_grid;
Mesh    mesh_axis;
Mesh    mesh_star;
Mesh    mesh_icosahedron_wire;

Texture*     whiteTexture;
TextureCube* whiteTextureCube;

enum eSamplerType {
    SAMPLER_WRAP_LINEAR = 0, 
    SAMPLER_CLAMP_LINEAR, 
    SAMPLER_BORDER_LINEAR, 
    SAMPLER_PCF_SHADOWS, 
    SAMPLERS_COUNT
};

ID3D11SamplerState* all_samplers[SAMPLERS_COUNT];
ID3D11DepthStencilState* z_cfgs[ZCFG_COUNT]; 

bool createSamplers()
{
    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = Render::getDevice()->CreateSamplerState(
        &sampDesc, &all_samplers[SAMPLER_WRAP_LINEAR]);
    if (FAILED(hr)) {return false;}

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Render::getDevice()->CreateSamplerState(
        &sampDesc, &all_samplers[SAMPLER_CLAMP_LINEAR]);
    if (FAILED(hr)) {return false;}

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.BorderColor[0] = 0;
    sampDesc.BorderColor[1] = 0;
    sampDesc.BorderColor[2] = 0;
    sampDesc.BorderColor[3] = 0;
    hr = Render::getDevice()->CreateSamplerState(
        &sampDesc, &all_samplers[SAMPLER_BORDER_LINEAR]);
    if (FAILED(hr)) {return false;}

    // PCF sampling
    D3D11_SAMPLER_DESC sampler_desc = {
        D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// D3D11_FILTER Filter;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
        0,//FLOAT MipLODBias;
        0,//UINT MaxAnisotropy;
        D3D11_COMPARISON_LESS, //D3D11_COMPARISON_FUNC ComparisonFunc;
        0, 0, 0, 0,//FLOAT BorderColor[ 4 ];
        0,//FLOAT MinLOD;
        0//FLOAT MaxLOD;   
    };
    hr = Render::getDevice()->CreateSamplerState(
        &sampler_desc, &all_samplers[SAMPLER_PCF_SHADOWS]);
    if (FAILED(hr)) {return false;}

    return true;
}

void destroySamplers()
{
    for (int i = 0; i < SAMPLERS_COUNT; ++i) {
        SAFE_RELEASE(all_samplers[i]);
    }
}

void activateTextureSamplers()
{
    Render::getContext()->PSSetSamplers(0, SAMPLERS_COUNT, all_samplers);
}

// -----------------------------------------------

void activateZConfig(enum zConfig_e cfg)
{
    assert(z_cfgs[cfg] != nullptr);
    Render::getContext()->OMSetDepthStencilState(z_cfgs[cfg], 0);
}

bool createDepthStencilStates()
{
    D3D11_DEPTH_STENCIL_DESC desc;
    HRESULT hr;
    
    //Test and write
    memset(&desc, 0x00, sizeof(desc));
    desc.DepthEnable = TRUE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

    desc.DepthFunc = D3D11_COMPARISON_LESS;
    desc.StencilEnable = FALSE;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_LT]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_LT], "ZCFG_LT");
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_LE]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_LT], "ZCFG_LE");
	desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_GE]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_LT], "ZCFG_GE");
	desc.DepthFunc = D3D11_COMPARISON_GREATER;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_GT]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_GT], "ZCFG_GT");
	desc.DepthFunc = D3D11_COMPARISON_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_EQ]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_EQ], "ZCFG_EQ");
	desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_NE]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_NE], "ZCFG_NE");
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_ALL]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_ALL], "ZCFG_ALL");

    // test but don't write
    memset(&desc, 0x00, sizeof(desc));
    desc.DepthEnable = TRUE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    desc.DepthFunc = D3D11_COMPARISON_LESS;           
    desc.StencilEnable = FALSE;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_LT]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_LT], "ZCFG_TEST_LT");
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_LE]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_LE], "ZCFG_TEST_LE");
    desc.DepthFunc = D3D11_COMPARISON_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_EQ]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_EQ], "ZCFG_TEST_EQ");
    desc.DepthFunc = D3D11_COMPARISON_GREATER;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_GT]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_GT], "ZCFG_TEST_GT");
    desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_GE]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_GE], "ZCFG_TEST_GE");
    desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_NE]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_NE], "ZCFG_TEST_NE");
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    hr = Render::getDevice()->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_ALL]);
    if (FAILED(hr)) {return false;}
    setDbgName(z_cfgs[ZCFG_TEST_ALL], "ZCFG_TEST_ALL");

    z_cfgs[ZCFG_DEFAULT] = z_cfgs[ZCFG_LT];
    return true;
}


void destroyDepthStencilStates()
{
    for (int i = 0; i < ZCFG_COUNT; ++i) {SAFE_RELEASE(z_cfgs[i]);}
}

ID3D11RasterizerState *rasterize_states[RSCFG_COUNT];

bool createRasterizationStates()
{
    rasterize_states[RSCFG_DEFAULT] = nullptr;
    
    // Depth bias options when rendering the shadows
    D3D11_RASTERIZER_DESC desc = {
        D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
        D3D11_CULL_BACK,  // D3D11_CULL_MODE CullMode;
        FALSE,            // BOOL FrontCounterClockwise;
        13,               // INT DepthBias;
        0.0f,             // FLOAT DepthBiasClamp;
        2.0,              // FLOAT SlopeScaledDepthBias;
        TRUE,             // BOOL DepthClipEnable;
        FALSE,            // BOOL ScissorEnable;
        FALSE,            // BOOL MultisampleEnable;
        FALSE,            // BOOL AntialiasedLineEnable;
    };
    HRESULT hr = Render::getDevice()->CreateRasterizerState(&desc, &rasterize_states[RSCFG_SHADOWS]);
    if (FAILED(hr))
      return false;
    setDbgName(rasterize_states[RSCFG_SHADOWS], "RS_DEPTH");
    
    // Culling is reversed. Used when rendering the light volumes
    D3D11_RASTERIZER_DESC rev_desc = {
        D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
        D3D11_CULL_FRONT, // D3D11_CULL_MODE CullMode;
        FALSE,            // BOOL FrontCounterClockwise;
        0,                // INT DepthBias;
        0.0f,             // FLOAT DepthBiasClamp;
        0.0,              // FLOAT SlopeScaledDepthBias;
        FALSE,            // BOOL DepthClipEnable;
        FALSE,            // BOOL ScissorEnable;
        FALSE,            // BOOL MultisampleEnable;
        FALSE,            // BOOL AntialiasedLineEnable;
    };
    hr = Render::getDevice()->CreateRasterizerState(&rev_desc, &rasterize_states[RSCFG_REVERSE_CULLING]);
    if (FAILED(hr)) {return false;}
    setDbgName(rasterize_states[RSCFG_REVERSE_CULLING], "RS_REVERSE_CULLING");
    
    // Culling is reversed. Used when rendering the light volumes
    D3D11_RASTERIZER_DESC disable_desc = {
        D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
        D3D11_CULL_NONE, // D3D11_CULL_MODE CullMode;
        FALSE,            // BOOL FrontCounterClockwise;
        0,                // INT DepthBias;
        0.0f,             // FLOAT DepthBiasClamp;
        0.0,              // FLOAT SlopeScaledDepthBias;
        FALSE,            // BOOL DepthClipEnable;
        FALSE,            // BOOL ScissorEnable;
        FALSE,            // BOOL MultisampleEnable;
        FALSE,            // BOOL AntialiasedLineEnable;
    };
    hr = Render::getDevice()->CreateRasterizerState(&disable_desc, &rasterize_states[RSCFG_DISABLE_CULLING]);
    if (FAILED(hr)) {return false;}
    setDbgName(rasterize_states[RSCFG_DISABLE_CULLING], "RSCFG_DISABLE_CULLING");
    
    return true;
}

void destroyRasterizationStates() {
    for (int i = 0; i < RSCFG_COUNT; ++i ) {SAFE_RELEASE(rasterize_states[i]);}
}

void activateRSConfig(enum RSConfig cfg) {
    Render::getContext()->RSSetState(rasterize_states[cfg]);
}

ID3D11BlendState *blend_states[BLEND_CFG_COUNT];
float blendFactor[BLEND_CFG_COUNT][4] = {};

bool createBlendStates()
{
    static auto const D3D11_COLOR_WRITE_ENABLE_RGB = 
        D3D11_COLOR_WRITE_ENABLE_RED |
        D3D11_COLOR_WRITE_ENABLE_GREEN |
        D3D11_COLOR_WRITE_ENABLE_BLUE;
    static auto const D3D11_COLOR_WRITE_ENABLE_BA = 
        D3D11_COLOR_WRITE_ENABLE_BLUE |
        D3D11_COLOR_WRITE_ENABLE_ALPHA;

    blend_states[BLEND_CFG_DEFAULT] = nullptr;
    
    D3D11_BLEND_DESC desc;
    HRESULT hr;

    // Weird translucid blending
    memset(&desc, 0x00, sizeof(desc));
    desc.IndependentBlendEnable = TRUE;
    //ALBEDO Glass-like translucid mixing
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
    desc.RenderTarget[0].RenderTargetWriteMask =  D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[1] = desc.RenderTarget[0]; //NORMALS
    desc.RenderTarget[3] = desc.RenderTarget[0]; //SPACE
    desc.RenderTarget[5] = desc.RenderTarget[0]; //DATA
    //LIGHTING additive
    desc.RenderTarget[2].BlendEnable = TRUE;
    desc.RenderTarget[2].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[2].SrcBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[2].DestBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[2].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[2].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[2].DestBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[2] = desc.RenderTarget[4]; //SELFILL

    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_GLASS]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_GLASS], "BLEND_GLASS");

    // Combinative blending
    memset(&desc, 0x00, sizeof(desc));
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_CFG_COMBINATIVE]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_CFG_COMBINATIVE], "BLEND_COMBINATIVE");

    // Additive blending
    memset(&desc, 0x00, sizeof(desc));
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_CFG_ADDITIVE]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_CFG_ADDITIVE], "BLEND_ADDITIVE");
    
    // Additive blending controlled by src alpha
    memset(&desc, 0x00, sizeof(desc));
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_CFG_ADDITIVE_BY_SRC_ALPHA]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_CFG_ADDITIVE_BY_SRC_ALPHA], "BLEND_ADDITIVE_BY_SRC_ALPHA");
    
    memset(&desc, 0x00, sizeof(desc));
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_PARTICLES]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_PARTICLES], "BLEND_PARTICLES");

    //BLENDING CONFIG FOR MIST
    memset(&desc, 0x00, sizeof(desc));
    desc.IndependentBlendEnable = TRUE;
    // Combinative blending (albedo, normals, selfIllumination)
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[1] = desc.RenderTarget[0];
    desc.RenderTarget[4] = desc.RenderTarget[0];

    // Additive blending only on BLUE (data)
    desc.RenderTarget[5].BlendEnable = TRUE;
    desc.RenderTarget[5].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[5].SrcBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[5].DestBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[5].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_BLUE;
    desc.RenderTarget[5].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[5].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[5].DestBlendAlpha = D3D11_BLEND_ONE;
    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_MIST]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_MIST], "BLEND_MIST");

    
    //BLENDING CONFIG FOR PAINT
    memset(&desc, 0x00, sizeof(desc));
    desc.IndependentBlendEnable = TRUE;
    // Maxmix blending (paintGlow)
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;

    //BLUE and ALPHA (data2)
    desc.RenderTarget[1].BlendEnable = TRUE;
    desc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_MAX;
    desc.RenderTarget[1].SrcBlend = D3D11_BLEND_SRC_COLOR;
    desc.RenderTarget[1].DestBlend = D3D11_BLEND_DEST_COLOR;
    desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_BA;
    desc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_MAX;
    desc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    
    // Combinative blending only (normals)
    desc.RenderTarget[2].BlendEnable = TRUE;
    desc.RenderTarget[2].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[2].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[2].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[2].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
    desc.RenderTarget[2].SrcBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[2].DestBlendAlpha = D3D11_BLEND_ONE;

    hr = Render::getDevice()->CreateBlendState(&desc, &blend_states[BLEND_CFG_PAINT]);
    if (FAILED(hr)) {return false;}
    setDbgName(blend_states[BLEND_CFG_PAINT], "BLEND_CFG_PAINT");

    return true;
}

void destroyBlendStates() {
  for (int i = 0; i < RSCFG_COUNT; ++i)
    SAFE_RELEASE(blend_states[i]);
}

void activateBlendConfig(enum BlendConfig cfg)
{
    UINT sampleMask = 0xffffffff;
    Render::getContext()->OMSetBlendState(
        blend_states[cfg], blendFactor[cfg], sampleMask );
}

bool createLine(Mesh& mesh)
{
    std::vector<VertexPosColor> vertices;
    vertices.resize(2);
    VertexPosColor *v = &vertices[0];
    
    v->Pos = XMFLOAT3(0, 0, 0.f);
    v->Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    ++v;
    v->Pos = XMFLOAT3(0, 0, 1.f);
    v->Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    ++v;
    
    return mesh.create((unsigned)vertices.size(), &vertices[0], 0, nullptr, Mesh::LINE_LIST);
}
bool createTexturedQuadXY(Mesh& mesh);

bool renderUtilsCreate()
{
    bool is_ok = true;
    is_ok &= ctes_object.create("object");
    is_ok &= ctes_camera.create("camera");
    is_ok &= ctes_light.create("light");
    is_ok &= ctes_ptlight.create("ptlight");
    is_ok &= ctes_dirlight.create("dirlight");
    is_ok &= ctes_volLight.create("volLights");
    is_ok &= ctes_mist.create("mist");
    is_ok &= ctes_render.create("seldom");
    is_ok &= ctes_texture.create("texture");
    is_ok &= ctes_global.create("global");
    is_ok &= ctes_paint.create("paint");
    is_ok &= boneBuffer.create(1024*12, DXGI_FORMAT_R32G32B32A32_FLOAT, "bones");
    RenderConstsMirror::update();
    
    is_ok &= createSamplers();
    is_ok &= createDepthStencilStates();
    is_ok &= createRasterizationStates();
    is_ok &= createBlendStates();
    
    is_ok &= createWiredUnitCube(mesh_cube_wire_unit);
    is_ok &= createWiredCube(mesh_cube_wire);
    is_ok &= createWiredCubeSplit(mesh_cube_wire_split);
    is_ok &= createViewVolumeWire(mesh_view_volume_wire);
    is_ok &= createViewVolume(mesh_view_volume);
    is_ok &= createTexturedQuadXY(mesh_textured_quad_xy);
    is_ok &= createLine(mesh_line);
    is_ok &= createAxis(mesh_axis);
    is_ok &= createStar(mesh_star, 0.1f);
    is_ok &= createGrid(mesh_grid, 10);
    is_ok &= createTexturedQuadXYCentered(mesh_textured_quad_xy_centered);
    is_ok &= createTexturedQuadXZCentered(mesh_textured_quad_xz_centered);
    is_ok &= createStackedQuadXZCentered(mesh_stacked_quad_xz_centered);
    is_ok &= createIcosahedron(mesh_icosahedron);
    is_ok &= createIcosahedronWireFrame(mesh_icosahedron_wire);

    //global textures: buffers t16 through t63
    Texture::getManager().getByName("grass")->activate(16);
    Texture::getManager().getByName("grass_normals")->activate(17);
    Texture::getManager().getByName("rock")->activate(18);
    Texture::getManager().getByName("rock_normals")->activate(19);
    Texture::getManager().getByName("flowers1")->activate(20);
    Texture::getManager().getByName("flowers1_normals")->activate(21);
    Texture::getManager().getByName("flowers2")->activate(22);
    Texture::getManager().getByName("flowers2_normals")->activate(23);

    //auxiliary textures: buffers t64 onwards
    Texture::getManager().getByName("whiteNoise")->activate(64);
    Texture::getManager().getByName("normalNoise")->activate(65);
    Texture::getManager().getByName("perlinNoise")->activate(66);
    Texture::getManager().getByName("grassVariation")->activate(67);
    
    return is_ok;
}

void renderUtilsDestroy()
{
    delete whiteTextureCube;
    destroyBlendStates();
    destroyRasterizationStates();
    destroyDepthStencilStates();
    destroySamplers();

    ctes_object.destroy();
    ctes_camera.destroy();
    ctes_light.destroy();
    ctes_ptlight.destroy();
    ctes_dirlight.destroy();
    ctes_volLight.destroy();
    ctes_mist.destroy();
    ctes_paint.destroy();
    ctes_render.destroy();
	ctes_texture.destroy();
    ctes_global.destroy();
	boneBuffer.destroy();

    mesh_icosahedron.destroy();
    mesh_icosahedron_wire.destroy();
    mesh_textured_quad_xy_centered.destroy();
    mesh_textured_quad_xz_centered.destroy();
    mesh_textured_quad_xy.destroy();
    mesh_line.destroy();
    mesh_view_volume.destroy();
    mesh_view_volume_wire.destroy();
    mesh_cube_wire.destroy();
    mesh_cube_wire_split.destroy();
    mesh_axis.destroy();
    mesh_grid.destroy();
    mesh_star.destroy();
}

void setTextureData(uint32_t nFrames, uint32_t framesPerRow, float elapsed)
{
	auto& ctes = ctes_texture.get();
	ctes.TextureNFrames = nFrames;
	ctes.TextureFramesPerRow = framesPerRow;

	if (elapsed == 0.0f){
		ctes.TextureFrameNum = 0.0f;
	}
	else{
		ctes.TextureFrameNum = elapsed / 0.0333f;
	}
}

void setTextureDataRasterFont(uint32_t nFrames, uint32_t framesPerRow, int value)
{
	auto& ctes = ctes_texture.get();
	ctes.TextureNFrames = nFrames;
	ctes.TextureFramesPerRow = framesPerRow;
	ctes.TextureFrameNum = float(value);
}

void updateTextureData()
 {
	if (ctes_texture.isDirty()) {
		ctes_texture.uploadToGPU();
		ctes_texture.activateInVS(SCTES_Texture::SLOT);
		
	}
}

void setWorldMatrix(XMMATRIX world) {
	ctes_object.get().World = world;
	ctes_object.uploadToGPU();
}


void activateObjectConstants()
{
    ctes_object.activateInVS(SCTES_Object::SLOT);
    ctes_object.activateInPS(SCTES_Object::SLOT);
}

void activateCamera(const render::Camera& camera)
{
    ctes_camera.activateInVS(SCTES_Camera::SLOT);
    ctes_camera.activateInPS(SCTES_Camera::SLOT);
    auto& ctes = ctes_camera.get();
    ctes.ViewProjection = camera.getViewProjection();
    ctes.CameraView = camera.getView();
    ctes.CameraWorldPos = camera.getPosition();
    ctes.CameraWorldFront = camera.getFront();
    ctes.CameraWorldLeft = -camera.getRight();
    ctes.CameraWorldUp = camera.getUp();
    ctes.CameraZFar = camera.getZFar();
    ctes.CameraZNear = camera.getZNear();
    ctes.CameraPadding = 0.f;

    D3D11_VIEWPORT vp = camera.getViewport();
    ctes.CameraHalfXRes = vp.Width / 2.f;
    ctes.CameraHalfYRes = vp.Height / 2.f;

    // tan( fov/2 ) = ( yres/2 ) / view_d
    ctes.CameraViewD = (vp.Height / 2.f) / tanf(camera.getFov() * 0.5f);

    ctes_camera.uploadToGPU();
}


bool createGrid(Mesh& mesh, int nsamples)
{
    std::vector< VertexPosColor > vtxs;

    XMFLOAT4 c1 = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    XMFLOAT4 c2 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f);

    float fsamples = (float)nsamples;
    VertexPosColor v;

    // The lines from -x to +x
    for (int z = -nsamples; z <= nsamples; ++z) {
        v.Color = (z % 5) ? c2 : c1;
        v.Pos = XMFLOAT3(-fsamples, 0.f, (float)z);
        vtxs.push_back(v);
        v.Pos = XMFLOAT3(fsamples, 0.f, (float)z);
        vtxs.push_back(v);
    }  

    // The lines from -z to +z
    for (int x = -nsamples; x <= nsamples; ++x) {
        v.Color = (x % 5) ? c2 : c1;
        v.Pos = XMFLOAT3((float)x, 0.f, -fsamples);
        vtxs.push_back(v);
        v.Pos = XMFLOAT3((float)x, 0.f, fsamples);
        vtxs.push_back(v);
    }

    return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, Mesh::LINE_LIST, utils::zero_v, utils::zero_v);
}
bool createDownPointer(Mesh &mesh, float w, float h)
{

  std::vector< VertexPosUVNormal > vertices;
  vertices.resize(6);
  VertexPosUVNormal *v = &vertices[0];
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Normal = XMFLOAT3(0.f,  1.f, 0.f); v->UV = XMFLOAT2(0,0); ++v;
  v->Pos = XMFLOAT3(  w,   h,   w); v->Normal = XMFLOAT3(  w,  0.f,   w); v->UV = XMFLOAT2(1,1); ++v;
  v->Pos = XMFLOAT3(  w,   h,  -w); v->Normal = XMFLOAT3(  w,  0.f,  -w); v->UV = XMFLOAT2(0,1); ++v;
  v->Pos = XMFLOAT3( -w,   h,  -w); v->Normal = XMFLOAT3( -w,  0.f,  -w); v->UV = XMFLOAT2(1,1); ++v;
  v->Pos = XMFLOAT3( -w,   h,   w); v->Normal = XMFLOAT3( -w,  0.f,   w); v->UV = XMFLOAT2(1,0); ++v;
  v->Pos = XMFLOAT3(0.f, h/2, 0.f); v->Normal = XMFLOAT3(0.f, -1.f, 0.f); v->UV = XMFLOAT2(0,0); ++v;

  const std::vector<Mesh::index_t> indices = {
      0, 1, 2,
      0, 2, 3,
      0, 3, 4,
      0, 4, 1,
      2, 1, 5,
      3, 2, 5,
      4, 3, 5,
      1, 4, 5,
  };
    return mesh.create(
      (unsigned)vertices.size(), &vertices[0],
      (unsigned)indices.size(), &indices[0],
      Mesh::TRIANGLE_LIST,
      XMVectorSet(-w,0,-w,1), XMVectorSet(w,h,w,1));
}
bool createAxis(Mesh& mesh, float length)
{

  std::vector< VertexPosColor > vtxs;
  vtxs.resize(12);
  VertexPosColor *v = &vtxs[0];

  const float opp = .25f;

  // X Axis
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f);     v->Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(length, 0.f, 0.f);  v->Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f);     v->Color = XMFLOAT4(0.f, 1.f, 1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(-length*opp, 0.f, 0.f);  v->Color = XMFLOAT4(0.f, 1.f, 1.f, 1.f); ++v;

  // Y Axis
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f);     v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, length, 0.f);  v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f);     v->Color = XMFLOAT4(1.f, 0.f, 1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, -length*opp, 0.f);  v->Color = XMFLOAT4(1.f, 0.f, 1.f, 1.f); ++v;

  // Z Axis
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f);     v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, length);  v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f);     v->Color = XMFLOAT4(1.f, 1.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, -length*opp);  v->Color = XMFLOAT4(1.f, 1.f, 0.f, 1.f); ++v;

  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, Mesh::LINE_LIST, utils::zero_v, utils::zero_v);
}

bool createStar(Mesh& mesh, float len)
{
    std::vector< VertexPosColor > vtxs;
    vtxs.resize(14);
    VertexPosColor *v = &vtxs[0];

    auto lc = std::sqrt(2)*.5f*len;

    v->Pos = XMFLOAT3( len,  0.f,  0.f);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3(-len,  0.f,  0.f);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( 0.f,  len,  0.f);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( 0.f, -len,  0.f);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( 0.f,  0.f,  len);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( 0.f,  0.f, -len);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3(  lc,   lc,   lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( -lc,  -lc,  -lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3(  lc,   lc,  -lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( -lc,  -lc,   lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( -lc,   lc,   lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3(  lc,  -lc,  -lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3( -lc,   lc,  -lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;
    v->Pos = XMFLOAT3(  lc,  -lc,   lc);  v->Color = XMFLOAT4(1,1,1,1); ++v;

    return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr,
        Mesh::LINE_LIST, utils::zero_v, utils::zero_v);
}

bool createPlanePUNT(Mesh& mesh, float xl, float zf, float xr, float zb, float y, float texSize)
{
    std::vector< VertexPosUVNormalTangent > vertices;
    vertices.resize(4);
    VertexPosUVNormalTangent *v = &vertices[0];
    float f = 1/texSize;
    
	v->Pos = XMFLOAT3( xl, y,-zb); v->UV = XMFLOAT2( xl*f,-zb*f); v->Normal = XMFLOAT3(0,1,0); v++;
	v->Pos = XMFLOAT3( xl, y, zf); v->UV = XMFLOAT2( xl*f, zf*f); v->Normal = XMFLOAT3(0,1,0); v++;
	v->Pos = XMFLOAT3(-xr, y,-zb); v->UV = XMFLOAT2(-xr*f,-zb*f); v->Normal = XMFLOAT3(0,1,0); v++;
	v->Pos = XMFLOAT3(-xr, y, zf); v->UV = XMFLOAT2(-xr*f, zf*f); v->Normal = XMFLOAT3(0,1,0); v++;
    
    const std::vector< Mesh::index_t > indices = {0, 1, 2, 2, 1, 3};
    
    //Probably the tangent is easy to figure out but I'd rather trust a tested algorithm
    computeTangentSpace(vertices, indices);
    
    auto min = XMVectorMin(XMVectorSet(xl, y,-zb, 1), XMVectorSet(-xr, y, zf, 1));
    auto max = XMVectorMax(XMVectorSet(xl, y,-zb, 1), XMVectorSet(-xr, y, zf, 1));

    return mesh.create(
        (unsigned)vertices.size(), vertices.data(), (unsigned)indices.size(), indices.data(),
        Mesh::TRIANGLE_LIST, min, max);
}
bool createPlane(Mesh& mesh)
{

  std::vector< VertexPosColor > vtxs;
  vtxs.resize(4);
  VertexPosColor *v = &vtxs[0];

  // Set the color to white
  v->Pos = XMFLOAT3( 1.f, 0.f, -1.f); v->Color = XMFLOAT4(.95f,.95f,.95f,1); ++v;
  v->Pos = XMFLOAT3( 1.f, 0.f,  1.f); v->Color = XMFLOAT4(.85f,.85f,.85f,1); ++v;
  v->Pos = XMFLOAT3(-1.f, 0.f, -1.f); v->Color = XMFLOAT4(.85f,.85f,.85f,1); ++v;
  v->Pos = XMFLOAT3(-1.f, 0.f,  1.f); v->Color = XMFLOAT4(.95f,.95f,.95f,1); ++v;

  const Mesh::index_t idxs[] = {
    0, 1, 2, 3,
  };
  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 4, idxs, Mesh::TRIANGLE_STRIP);
}
bool createTexturedQuadXY(Mesh& mesh)
{
    std::vector< VertexPosUV > vtxs;
    vtxs.resize(4);
    VertexPosUV *v = &vtxs[0];
    v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3(1.f, 0.f, 0.f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(0.f, 1.f, 0.f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3(1.f, 1.f, 0.f); v->UV = XMFLOAT2(1, 1); ++v;
    return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, Mesh::TRIANGLE_STRIP);
}
bool createStackedQuadXZCentered(Mesh& mesh) {
    std::vector< VertexPosUV > vertices;
    vertices.resize(20);
    VertexPosUV *v = &vertices[0];
    v->Pos = XMFLOAT3(-0.5f, 0.f, -0.5f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f, 0.f, -0.5f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f, 0.f,  0.5f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f, 0.f,  0.5f); v->UV = XMFLOAT2(1, 1); ++v;
    v->Pos = XMFLOAT3(-0.5f,-1.f, -0.5f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f,-1.f, -0.5f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f,-1.f,  0.5f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f,-1.f,  0.5f); v->UV = XMFLOAT2(1, 1); ++v;
    v->Pos = XMFLOAT3(-0.5f,-2.f, -0.5f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f,-2.f, -0.5f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f,-2.f,  0.5f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f,-2.f,  0.5f); v->UV = XMFLOAT2(1, 1); ++v;
    v->Pos = XMFLOAT3(-0.5f,-3.f, -0.5f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f,-3.f, -0.5f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f,-3.f,  0.5f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f,-3.f,  0.5f); v->UV = XMFLOAT2(1, 1); ++v;
    v->Pos = XMFLOAT3(-0.5f,-4.f, -0.5f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f,-4.f, -0.5f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f,-4.f,  0.5f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f,-4.f,  0.5f); v->UV = XMFLOAT2(1, 1); ++v;
    
#define QUAD(n) 0+4*n, 1+4*n, 2+4*n, 1+4*n, 3+4*n, 2+4*n

    std::vector< Mesh::index_t > indices = {
        QUAD(0), QUAD(1), QUAD(2), QUAD(3), QUAD(4)
    };

    return mesh.create(
        (unsigned)vertices.size(), vertices.data(),
        (unsigned)indices.size(), indices.data(),
        Mesh::TRIANGLE_LIST);
}

bool createTexturedQuadXZCentered(Mesh& mesh) {
    std::vector< VertexPosUV > vtxs;
    vtxs.resize(4);
    VertexPosUV *v = &vtxs[0];
    v->Pos = XMFLOAT3(-0.5f, 0.f, -0.5f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f, 0.f, -0.5f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f, 0.f,  0.5f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f, 0.f,  0.5f); v->UV = XMFLOAT2(1, 1); ++v;
    return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, Mesh::TRIANGLE_STRIP);
}
bool createTexturedQuadXYCentered(Mesh& mesh) {
    std::vector< VertexPosUV > vtxs;
    vtxs.resize(4);
    VertexPosUV *v = &vtxs[0];
    v->Pos = XMFLOAT3(-0.5f, -0.5f, 0.f); v->UV = XMFLOAT2(0, 0); ++v;
    v->Pos = XMFLOAT3( 0.5f, -0.5f, 0.f); v->UV = XMFLOAT2(1, 0); ++v;
    v->Pos = XMFLOAT3(-0.5f,  0.5f, 0.f); v->UV = XMFLOAT2(0, 1); ++v;
    v->Pos = XMFLOAT3( 0.5f,  0.5f, 0.f); v->UV = XMFLOAT2(1, 1); ++v;
    return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, Mesh::TRIANGLE_STRIP);
}
bool createPUNTBox(Mesh& mesh, XMVECTOR _max, XMVECTOR _min)
{
    XMVECTOR min = XMVectorMin(_max, _min);
    XMVECTOR max = XMVectorMax(_max, _min);

    std::vector< VertexPosUVNormalTangent > vertices;
    vertices.resize(6*4);
    VertexPosUVNormalTangent *v = &vertices[0];
    std::vector< Mesh::index_t > indices;
    indices.reserve(12*3);
    
    float x = XMVectorGetX(max);
    float y = XMVectorGetY(max);
    float z = XMVectorGetZ(max);
    float a = XMVectorGetX(min);
    float b = XMVectorGetY(min);
    float c = XMVectorGetZ(min);
    
    Mesh::index_t i=0;
    
    //bottom face
    v->Pos = XMFLOAT3(a, b, c); v->UV = XMFLOAT2(0,0); v->Normal = XMFLOAT3( 0,-1, 0); v++;
    v->Pos = XMFLOAT3(a, b, z); v->UV = XMFLOAT2(0,1); v->Normal = XMFLOAT3( 0,-1, 0); v++;
    v->Pos = XMFLOAT3(x, b, c); v->UV = XMFLOAT2(1,0); v->Normal = XMFLOAT3( 0,-1, 0); v++;
    v->Pos = XMFLOAT3(x, b, z); v->UV = XMFLOAT2(1,1); v->Normal = XMFLOAT3( 0,-1, 0); v++;
    indices.push_back(i+2); indices.push_back(i+0); indices.push_back(i+1);
    indices.push_back(i+1); indices.push_back(i+3); indices.push_back(i+2);
    i+=4;
    
    //top face
    v->Pos = XMFLOAT3(a, y, c); v->UV = XMFLOAT2(0,0); v->Normal = XMFLOAT3( 0, 1, 0); v++;
    v->Pos = XMFLOAT3(a, y, z); v->UV = XMFLOAT2(0,1); v->Normal = XMFLOAT3( 0, 1, 0); v++;
    v->Pos = XMFLOAT3(x, y, c); v->UV = XMFLOAT2(1,0); v->Normal = XMFLOAT3( 0, 1, 0); v++;
    v->Pos = XMFLOAT3(x, y, z); v->UV = XMFLOAT2(1,1); v->Normal = XMFLOAT3( 0, 1, 0); v++;
    indices.push_back(i+1); indices.push_back(i+0); indices.push_back(i+2);
    indices.push_back(i+2); indices.push_back(i+3); indices.push_back(i+1);
    i+=4;
    
    //right face
    v->Pos = XMFLOAT3(a, b, c); v->UV = XMFLOAT2(0,0); v->Normal = XMFLOAT3(-1, 0, 0); v++;
    v->Pos = XMFLOAT3(a, y, c); v->UV = XMFLOAT2(0,1); v->Normal = XMFLOAT3(-1, 0, 0); v++;
    v->Pos = XMFLOAT3(a, b, z); v->UV = XMFLOAT2(1,0); v->Normal = XMFLOAT3(-1, 0, 0); v++;
    v->Pos = XMFLOAT3(a, y, z); v->UV = XMFLOAT2(1,1); v->Normal = XMFLOAT3(-1, 0, 0); v++;
    indices.push_back(i+2); indices.push_back(i+0); indices.push_back(i+1);
    indices.push_back(i+1); indices.push_back(i+3); indices.push_back(i+2);
    i+=4;
    
    //left face
    v->Pos = XMFLOAT3(x, b, c); v->UV = XMFLOAT2(0,0); v->Normal = XMFLOAT3( 1, 0, 0); v++;
    v->Pos = XMFLOAT3(x, y, c); v->UV = XMFLOAT2(0,1); v->Normal = XMFLOAT3( 1, 0, 0); v++;
    v->Pos = XMFLOAT3(x, b, z); v->UV = XMFLOAT2(1,0); v->Normal = XMFLOAT3( 1, 0, 0); v++;
    v->Pos = XMFLOAT3(x, y, z); v->UV = XMFLOAT2(1,1); v->Normal = XMFLOAT3( 1, 0, 0); v++;
    indices.push_back(i+1); indices.push_back(i+0); indices.push_back(i+2);
    indices.push_back(i+2); indices.push_back(i+3); indices.push_back(i+1);
    i+=4;
    
    
    //front face
    v->Pos = XMFLOAT3(a, b, c); v->UV = XMFLOAT2(0,0); v->Normal = XMFLOAT3( 0, 0,-1); v++;
    v->Pos = XMFLOAT3(a, y, c); v->UV = XMFLOAT2(0,1); v->Normal = XMFLOAT3( 0, 0,-1); v++;
    v->Pos = XMFLOAT3(x, b, c); v->UV = XMFLOAT2(1,0); v->Normal = XMFLOAT3( 0, 0,-1); v++;
    v->Pos = XMFLOAT3(x, y, c); v->UV = XMFLOAT2(1,1); v->Normal = XMFLOAT3( 0, 0,-1); v++;
    indices.push_back(i+1); indices.push_back(i+0); indices.push_back(i+2);
    indices.push_back(i+2); indices.push_back(i+3); indices.push_back(i+1);
    i+=4;
    
    //back face
    v->Pos = XMFLOAT3(a, b, z); v->UV = XMFLOAT2(0,0); v->Normal = XMFLOAT3( 0, 0, 1); v++;
    v->Pos = XMFLOAT3(a, y, z); v->UV = XMFLOAT2(0,1); v->Normal = XMFLOAT3( 0, 0, 1); v++;
    v->Pos = XMFLOAT3(x, b, z); v->UV = XMFLOAT2(1,0); v->Normal = XMFLOAT3( 0, 0, 1); v++;
    v->Pos = XMFLOAT3(x, y, z); v->UV = XMFLOAT2(1,1); v->Normal = XMFLOAT3( 0, 0, 1); v++;
    indices.push_back(i+2); indices.push_back(i+0); indices.push_back(i+1);
    indices.push_back(i+1); indices.push_back(i+3); indices.push_back(i+2);
    i+=4;
    
    computeTangentSpace(vertices, indices);
    
    return mesh.create(
        (unsigned)vertices.size(), &vertices[0],
        (unsigned)indices.size(), &indices[0],
        Mesh::TRIANGLE_LIST,
        min, max);
}
bool createWiredUnitCube(Mesh& mesh) {

  std::vector< VertexPosColor > vtxs;
  vtxs.resize(8);
  VertexPosColor *v = &vtxs[0];

  // Set the color to white
  for (auto& it : vtxs)
    it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

  v->Pos = XMFLOAT3(-1.f, -1.f, -1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(+1.f, -1.f, -1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(-1.f, +1.f, -1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(+1.f, +1.f, -1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(-1.f, -1.f, +1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(+1.f, -1.f, +1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(-1.f, +1.f, +1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(+1.f, +1.f, +1.f); v->Color=Color::WHITE; ++v;

  const Mesh::index_t idxs[] = {
    0, 1, 2, 3, 4, 5, 6, 7
    , 0, 2, 1, 3, 4, 6, 5, 7
    , 0, 4, 1, 5, 2, 6, 3, 7
  };
  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 24, idxs, Mesh::LINE_LIST,
      utils::zero_v, utils::one_v);
}
bool createWiredCube(Mesh& mesh) {

  std::vector< VertexPosColor > vtxs;
  vtxs.resize(8);
  VertexPosColor *v = &vtxs[0];

  // Set the color to white
  for (auto& it : vtxs)
    it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 0.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(0.f, 1.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 1.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 0.f, 1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(0.f, 1.f, 1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 1.f, 1.f); v->Color=Color::WHITE; ++v;

  const Mesh::index_t idxs[] = {
    0, 1, 2, 3, 4, 5, 6, 7
    , 0, 2, 1, 3, 4, 6, 5, 7
    , 0, 4, 1, 5, 2, 6, 3, 7
  };
  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 24, idxs, Mesh::LINE_LIST,
      utils::zero_v, utils::one_v);
}
bool createWiredCubeSplit(Mesh& mesh) {

  std::vector< VertexPosColor > vtxs;
  vtxs.resize(14);
  VertexPosColor *v = &vtxs[0];

  // Set the color to white
  for (auto& it : vtxs)
    it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 0.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(0.f, 1.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 1.f, 0.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 0.f, 1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(0.f, 1.f, 1.f); v->Color=Color::WHITE; ++v;
  v->Pos = XMFLOAT3(1.f, 1.f, 1.f); v->Color=Color::WHITE; ++v;
  
  v->Pos = XMFLOAT3(0.f, 0.5f, 0.5f); v->Color=Color::GRAY; ++v;
  v->Pos = XMFLOAT3(1.f, 0.5f, 0.5f); v->Color=Color::GRAY; ++v;
  v->Pos = XMFLOAT3(0.5f, 0.f, 0.5f); v->Color=Color::GRAY; ++v;
  v->Pos = XMFLOAT3(0.5f, 1.f, 0.5f); v->Color=Color::GRAY; ++v;
  v->Pos = XMFLOAT3(0.5f, 0.5f, 0.f); v->Color=Color::GRAY; ++v;
  v->Pos = XMFLOAT3(0.5f, 0.5f, 1.f); v->Color=Color::GRAY; ++v;

  const Mesh::index_t idxs[] = {
    0, 1, 2, 3, 4, 5, 6, 7
    , 0, 2, 1, 3, 4, 6, 5, 7
    , 0, 4, 1, 5, 2, 6, 3, 7
    , 8,9,10,11,12,13
  };
  return mesh.create((unsigned)vtxs.size(), &vtxs[0], ARRAYSIZE(idxs),
      idxs, Mesh::LINE_LIST, utils::zero_v, utils::one_v);
}
bool createVolumeCube(Mesh &mesh){

	VertexPosColor vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	};
	Mesh::index_t indices[] =
	{
		3, 0, 1,
		2, 3, 1,

		0, 4, 5,
		1, 0, 5,

		3, 7, 4,
		0, 3, 4,

		1, 5, 6,
		2, 1, 6,

		2, 6, 7,
		3, 2, 7,

		6, 5, 4,
		7, 6, 4,
	};

	return mesh.create(8, vertices, 36, indices, Mesh::TRIANGLE_LIST,
        getVertexDecl<VertexPosColor>());
}
bool createRay(Mesh& meshRay, float longitude){
	std::vector< VertexPosColor > vtxs;
	vtxs.resize(2);
	VertexPosColor *v = &vtxs[0];

	// Axis Z
	v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(1.f, 0.f, 0.f, 0.f); ++v;
	v->Pos = XMFLOAT3(0.f, 0.f, longitude); v->Color = XMFLOAT4(1.f, 0.f, 0.f, 0.f); ++v;

	return meshRay.create((unsigned)vtxs.size(), &vtxs[0], 0,
        nullptr, Mesh::LINE_LIST, utils::zero_v,utils::zero_v);
}
bool createRay(Mesh& meshRay, float longitude, XMFLOAT4 colorRGB){
	std::vector< VertexPosColor > vtxs;
	vtxs.resize(2);
	VertexPosColor *v = &vtxs[0];

	// Axis Z
	v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = colorRGB; ++v;
	v->Pos = XMFLOAT3(0.f, 0.f, longitude); v->Color = colorRGB; ++v;
	
	return meshRay.create((unsigned)vtxs.size(), &vtxs[0], 0,
        nullptr, Mesh::LINE_LIST, utils::zero_v, utils::zero_v) ;

}

bool createIcosahedronWireFrame(Mesh& mesh, float size)
{
    static const float bX = .525731112119133606f*1.2584085f;
    static const float bZ = .850650808352039932f*1.2584085f;
    const float X = bX*size;
    const float Z = bZ*size;
    
    const float vs[12][7] = {
        { -X, 0.0, Z ,1,1,1,0}, {  X, 0.0, Z ,1,1,1,0}, { -X, 0.0, -Z ,1,1,1,0}, {  X, 0.0, -Z ,1,1,1,0},
        {  0.0, Z, X ,1,1,1,0}, { 0.0, Z, -X ,1,1,1,0}, {  0.0, -Z, X ,1,1,1,0}, { 0.0, -Z, -X ,1,1,1,0},
        {  Z, X, 0.0 ,1,1,1,0}, { -Z, X, 0.0 ,1,1,1,0}, {  Z, -X, 0.0 ,1,1,1,0}, { -Z, -X, 0.0 ,1,1,1,0}
    };
    static const Mesh::index_t idxs[20][6] = {
        {0,4,4,1,1,0}, {0,9,9,4,4,1}, {9,5,5,4,4,9}, {4,5,5,8,8,5}, {4,8,8,1,1,4},
        {8,10,10,1,1,8}, {8,3,3,10,10,8}, {5,3,3,8,8,5}, {5,2,2,3,3,5}, {2,7,7,3,3,2},
        {7,10,10,3,3,7}, {7,6,6,10,10,7}, {7,11,11,6,6,7}, {11,0,0,6,6,11}, {0,1,1,6,6,0},
        {6,1,1,10,10,6}, {9,0,0,11,11,9}, {9,11,11,2,2,9}, {9,2,2,5,5,9}, {7,2,2,11,11,7} };
    return mesh.create(12, (const VertexPosColor*) vs, 20 * 6, &idxs[0][0], Mesh::LINE_LIST);
}
bool createIcosahedron(Mesh& mesh, float size) {
  static const float bX = .525731112119133606f*1.2584085f;
  static const float bZ = .850650808352039932f*1.2584085f;
  const float X = bX*size;
  const float Z = bZ*size;
  const float vs[12][3] = {
      { -X, 0.0, Z }, {  X, 0.0, Z }, { -X, 0.0, -Z }, {  X, 0.0, -Z },
      {  0.0, Z, X }, { 0.0, Z, -X }, {  0.0, -Z, X }, { 0.0, -Z, -X },
      {  Z, X, 0.0 }, { -Z, X, 0.0 }, {  Z, -X, 0.0 }, { -Z, -X, 0.0 }
  };
  static const Mesh::index_t idxs[20][3] = {
      { 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
      { 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
      { 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
      { 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 } };
  return mesh.create(12, (const VertexPos*) vs, 20 * 3, &idxs[0][0], Mesh::TRIANGLE_LIST,
      -one_v, one_v);
}

bool createViewVolumeWire(Mesh& mesh) {
    std::vector< VertexPos > vtxs;
    vtxs.resize(8);
    VertexPos* v = &vtxs[0];

    // Axis X
    v->Pos = XMFLOAT3(-1.f, -1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(1.f, -1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(-1.f, 1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(1.f, 1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(-1.f, -1.f, 1.f); ++v;
    v->Pos = XMFLOAT3(1.f, -1.f, 1.f); ++v;
    v->Pos = XMFLOAT3(-1.f, 1.f, 1.f); ++v;
    v->Pos = XMFLOAT3(1.f, 1.f, 1.f); ++v;

    const Mesh::index_t idxs[] = {
      0, 1, 1, 3, 3, 2, 2, 0,
      4, 5, 5, 7, 7, 6, 6, 4,
      0, 4, 1, 5, 2, 6, 3, 7,
    };
    return mesh.create((unsigned)vtxs.size(), &vtxs[0],
        sizeof(idxs) / sizeof(Mesh::index_t), idxs, Mesh::LINE_LIST,
        -one_v, one_v);
}
bool createViewVolume(Mesh& mesh) {
    std::vector< VertexPos > vtxs;
    vtxs.resize(8);
    VertexPos* v = &vtxs[0];

    // Axis X
    v->Pos = XMFLOAT3(-1.f, -1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(1.f, -1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(-1.f, 1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(1.f, 1.f, 0.f); ++v;
    v->Pos = XMFLOAT3(-1.f, -1.f, 1.f); ++v;
    v->Pos = XMFLOAT3(1.f, -1.f, 1.f); ++v;
    v->Pos = XMFLOAT3(-1.f, 1.f, 1.f); ++v;
    v->Pos = XMFLOAT3(1.f, 1.f, 1.f); ++v;

    const Mesh::index_t idxs[] = {
      0, 2, 1, 1, 2, 3,
      4, 5, 6, 5, 7, 6,
      0, 1, 4, 1, 5, 4,
      2, 6, 7, 2, 7, 3,
      0, 4, 2, 4, 6, 2,
      1, 3, 5, 5, 3, 7,
    };
    return mesh.create((unsigned)vtxs.size(), &vtxs[0],
        sizeof(idxs) / sizeof(Mesh::index_t), idxs, Mesh::TRIANGLE_LIST,
        -one_v, one_v);
}

//#define FUSTRUM_SPLIT_Z

void drawViewVolume(const Camera& camera, const XMVECTOR& color)
{
#ifdef FUSTRUM_SPLIT_Z
    for (int i = 0; i < 10; ++i) {
      float ratio = (i + 1) / 10.0f;
      XMMATRIX scale = XMMatrixScaling(1.f, 1.f, ratio);
      XMMATRIX inv_view_proj = XMMatrixInverse(nullptr, camera.getViewProjection());
      setObjectConstants(scale * inv_view_proj, color, color);
      mesh_view_volume_wire.activateAndRender();
    }
#else
    XMMATRIX inv_view_proj = XMMatrixInverse(nullptr, camera.getViewProjection());
      setObjectConstants(inv_view_proj, color, color);
      mesh_view_volume_wire.activateAndRender();
#endif
}

void setObjectConstants(XMMATRIX world,
    Color tint, Color selfIllumination,
    const CMesh* mesh, unsigned bone0, const Material* material)
{
    auto& ctes = ctes_object.get();
    ctes.World = world;
    ctes.FirstBone = bone0;
    ctes.Tint = tint;
    uint32_t si = selfIllumination;
    ctes.SelfIllumination = selfIllumination;
    ctes.DiffuseAsSelfIllumination = 0;
    ctes.ObjectPaintableAmount = 1;
    if (material != nullptr) {
        ctes.AlphaAsSpecular = material->usesAlphaAsSpecular();

        auto matTint = material->getTint();
        if (matTint != 0) {
            if (tint != 0) {
                ctes.Tint = matTint.blend(tint);
            } else {
                ctes.Tint = matTint;
            }
        }
        
        auto matSIll = material->getSelfIlluminationTint();
        if (matSIll != 0) {
            if (selfIllumination != 0) {
                ctes.SelfIllumination = matSIll.blend(selfIllumination);
            } else {
                ctes.SelfIllumination = matSIll;
            }
        }

        ctes.BaseSpecular = material->getBaseSpecular();
        ctes.DiffuseAsSelfIllumination = material->getDiffuseAsIlluminationFactor();
        ctes.ObjectPaintableAmount = std::min(
            ctes.ObjectPaintableAmount,
            material->getPaintClamp());
    } else {
        ctes.AlphaAsSpecular = false;
        ctes.BaseSpecular = 0;
        ctes.DiffuseAsSelfIllumination = 0;
    }
    if (mesh != nullptr) {  
        ctes.MotionBlurAmount = mesh->getMotionBlurAmount();
        ctes.SelfIlluminationClamp = mesh->getSelfIlluminationClamp();
        ctes.DiffuseAsSelfIllumination += mesh->getDiffuseAsSelfIllumination();
        ctes.ObjectPaintableAmount = std::min(
            ctes.ObjectPaintableAmount,
            mesh->getPaintableAmount());
    } else {
        ctes.MotionBlurAmount = 0;
        ctes.ObjectPaintableAmount = 0;
        ctes.SelfIlluminationClamp = 1;
    }
    ctes_object.uploadToGPU();
}

void drawTexture2DAnim(
	pixelRect src, pixelRect dst,
	const Texture* texture, bool alpha, float delta_secs)
{
	const Technique* technique = Technique::getManager().getByName("hud");
	technique->activate();
	if (alpha) activateBlendConfig(BLEND_CFG_ADDITIVE_BY_SRC_ALPHA);
	activateZConfig(zConfig_e::ZCFG_TEST_ALL);
	// Activate the texture
	if (texture != nullptr) { texture->activate(0); }
	setTextureData(16, 4, delta_secs);
	updateTextureData();
	// Activate a orthogonal camera view-projection matrix
	XMMATRIX& curr_ViewProjection = ctes_camera.get().ViewProjection;
	XMMATRIX prev_ViewProjection = curr_ViewProjection;
	curr_ViewProjection = XMMatrixOrthographicOffCenterRH(
		float(dst.x), float(dst.x + dst.w), float(dst.y + dst.h), float(dst.y), -1.f, 1.f);
	ctes_camera.uploadToGPU();

	setObjectConstants(XMMatrixScaling(float(src.w), float(src.h), 1.0f) *
		XMMatrixTranslation(float(src.x), float(src.y), 0.0f));
	mesh_textured_quad_xy.activateAndRender();

	// Restore old view-projection
	curr_ViewProjection = prev_ViewProjection;
	ctes_camera.uploadToGPU();
	Texture::deactivate(0);
	if (alpha) activateBlendConfig(BLEND_CFG_DEFAULT);
}

void drawTexture2D(
    pixelRect src, pixelRect dst,
    const Texture* texture,
    const Technique* tech,
    bool alpha, Color tint)
{

    const Technique* technique = tech != nullptr ? tech :
        Technique::getManager().getByName("textured");
    technique->activate();
    
	if (alpha) activateBlendConfig(BLEND_CFG_COMBINATIVE);
    activateZConfig(zConfig_e::ZCFG_TEST_ALL);

    // Activate the texture
    if (texture != nullptr) {texture->activate(0);}
    
    // Activate a orthogonal camera view-projection matrix
    auto& ctes = ctes_camera.get();
    ctes.CameraXRes = float(dst.w);
    ctes.CameraYRes = float(dst.h);
    ctes.CameraHalfXRes = ctes.CameraXRes/2;
    ctes.CameraHalfYRes = ctes.CameraYRes/2;
    XMMATRIX& curr_ViewProjection = ctes.ViewProjection;
    XMMATRIX prev_ViewProjection = curr_ViewProjection;
    curr_ViewProjection = XMMatrixOrthographicOffCenterRH(
        float(dst.x), float(dst.x+dst.w), float(dst.y + dst.h), float(dst.y), -1.f, 1.f);
    ctes_camera.uploadToGPU();
    
    setObjectConstants(XMMatrixScaling(float(src.w), float(src.h), 1.0f) *
        XMMatrixTranslation(float(src.x), float(src.y), 0.f), tint);
    mesh_textured_quad_xy.activateAndRender();
    
    // Restore old view-projection
    curr_ViewProjection = prev_ViewProjection;
    ctes_camera.uploadToGPU();

    Texture::deactivate( 0 );

	if (alpha) activateBlendConfig(BLEND_CFG_DEFAULT);
}

void drawTextureFadeOut(pixelRect src, pixelRect dst, const Texture* texture, float value)
{
	const Technique* technique = Technique::getManager().getByName("fadeout");
	technique->activate();
	activateBlendConfig(BLEND_CFG_COMBINATIVE);
	if (texture != nullptr) { texture->activate(0); }
	XMMATRIX& curr_ViewProjection = ctes_camera.get().ViewProjection;
	XMMATRIX prev_ViewProjection = curr_ViewProjection;
	curr_ViewProjection = XMMatrixOrthographicOffCenterRH(
		float(dst.x), float(dst.x + dst.w), float(dst.y + dst.h), float(dst.y), -1.f, 1.f);
	ctes_camera.uploadToGPU();
	auto& ctesObject = ctes_object.get();
	ctesObject.World = XMMatrixScaling(float(src.w), float(src.h), 1.0f) *
		XMMatrixTranslation(float(src.x), float(src.y), 0.f);
	ctesObject.Tint = XMVectorSet(0, 0, 0, value);
	ctes_object.uploadToGPU();
	mesh_textured_quad_xy.activateAndRender();
	curr_ViewProjection = prev_ViewProjection;
	ctes_camera.uploadToGPU();
	Texture::deactivate(0);
	activateBlendConfig(BLEND_CFG_DEFAULT);
}

void drawLeafHUD(pixelRect src, pixelRect dst, float rot, const Texture* texture)
{
	const Technique* technique = Technique::getManager().getByName("textured");   
    technique->activate();    
	activateBlendConfig(BLEND_CFG_COMBINATIVE);
    // Activate the texture
    texture->activate(0);
    // Activate a orthogonal camera view-projection matrix
    XMMATRIX& curr_ViewProjection = ctes_camera.get().ViewProjection;
    XMMATRIX prev_ViewProjection = curr_ViewProjection;
    curr_ViewProjection = XMMatrixOrthographicOffCenterRH(
        float(dst.x), float(dst.x+dst.w), float(dst.y + dst.h), float(dst.y), -1.f, 1.f);
    ctes_camera.uploadToGPU(); 
    // Update the world matrix to match the params
    auto& ctesObject = ctes_object.get();
    ctesObject.World = XMMatrixScaling(float(src.w), float(src.h), 1.0f) * 
		XMMatrixRotationZ(rot) * XMMatrixTranslation(float(src.x), float(src.y), 0.f);
    ctesObject.Tint = zero_v;
    ctes_object.uploadToGPU();
	mesh_textured_quad_xy_centered.activateAndRender();
    // Restore old view-projection
    curr_ViewProjection = prev_ViewProjection;
    ctes_camera.uploadToGPU();
    Texture::deactivate( 0 );
	activateBlendConfig(BLEND_CFG_DEFAULT);
}

void drawEnergyBar(pixelRect src, pixelRect dst, float energy)
{
	const Technique* technique = Technique::getManager().getByName("gradientbar");
	technique->activate();
	activateBlendConfig(BLEND_CFG_COMBINATIVE);
	// Activate the texture
	Texture::getManager().getByName("energy")->activate(0);
	//Texture::getManager().getByName("testbar")->activate(0);
	// Activate a orthogonal camera view-projection matrix
	XMMATRIX& curr_ViewProjection = ctes_camera.get().ViewProjection;
	XMMATRIX prev_ViewProjection = curr_ViewProjection;
	curr_ViewProjection = XMMatrixOrthographicOffCenterRH(
		float(dst.x), float(dst.x+dst.w), float(dst.y + dst.h), float(dst.y), -1.f, 1.f);
	ctes_camera.uploadToGPU();
	// Update the world matrix to match the params
	auto& ctesObject = ctes_object.get();
	ctesObject.World = XMMatrixScaling(float(src.w), float(src.h), 1.0f) *
        XMMatrixTranslation(float(src.x), float(src.y), 0.f);
	ctesObject.Tint = XMVectorSet(0, 0, 0, energy/100);
	ctes_object.uploadToGPU();
	mesh_textured_quad_xy.activateAndRender();
	// Restore old view-projection
	curr_ViewProjection = prev_ViewProjection;
	ctes_camera.uploadToGPU();
	Texture::deactivate(0);
	activateBlendConfig(BLEND_CFG_DEFAULT);
}

void drawText(pixelRect src, pixelRect dst, std::string score)
{
	const Technique* technique = Technique::getManager().getByName("rasterfont");
	technique->activate();
	activateBlendConfig(BLEND_CFG_COMBINATIVE);
	Texture::getManager().getByName("rasterfont")->activate(0);
	for (int order = 0; order != score.size(); order++){
		int value = (int)score[order];
		value -= 32;
		setTextureDataRasterFont(224, 16, value);
		updateTextureData();
		// Activate a orthogonal camera view-projection matrix
		XMMATRIX& curr_ViewProjection = ctes_camera.get().ViewProjection;
		XMMATRIX prev_ViewProjection = curr_ViewProjection;
		curr_ViewProjection = XMMatrixOrthographicOffCenterRH(
			float(dst.x), float(dst.x + dst.w), float(dst.y + dst.h), float(dst.y), -1.f, 1.f);
		ctes_camera.uploadToGPU();
		// Update the world matrix to match the params
		auto& ctesObject = ctes_object.get();
		ctesObject.World = XMMatrixScaling(float(src.w), float(src.h), 1.0f) *
			XMMatrixTranslation(float(src.x) + float(src.w * order), float(src.y), 0.f);
		ctesObject.Tint = zero_v;
		ctes_object.uploadToGPU();
		mesh_textured_quad_xy.activateAndRender();
		// Restore old view-projection
		curr_ViewProjection = prev_ViewProjection;
		ctes_camera.uploadToGPU();
	}
	Texture::deactivate(0);
	activateBlendConfig(BLEND_CFG_DEFAULT);
}

void activateLight(const Camera& light)
{
    ctes_light.activateInVS(SCTES_Light::SLOT);    // as set in the shader.fx!!
    ctes_light.activateInPS(SCTES_Light::SLOT);    // as set in the shader.fx!!
    auto& ctes = ctes_light.get();
    XMStoreFloat3(&ctes.LightWorldPos, light.getPosition());
    ctes.LightZFar = light.getZFar();
    ctes.LightViewProjection = light.getViewProjection();
    XMMATRIX offset = XMMatrixTranslation(0.5f, 0.5f, 0.f);
    XMMATRIX scale = XMMatrixScaling(0.5f, -0.5f, 1.f);
    XMMATRIX tmx = scale * offset;
    ctes.LightViewProjectionOffset = light.getViewProjection() * tmx;
    ctes_light.uploadToGPU();
}

void activatePtLightCB()
{
    ctes_ptlight.activateInVS(SCTES_PtLight::SLOT);
    ctes_ptlight.activateInPS(SCTES_PtLight::SLOT);
}

void activatePtLight(const CPtLight* ptLight, XMVECTOR pos, CCubeShadow* shadow)
{
    auto& ctes = ctes_ptlight.get();
    auto color = ptLight->getColor();
    auto radius = ptLight->getRadius();
    ctes.PtLightColor = XMVectorSetW(color, ptLight->getIntensity());
    ctes.PtLightWorldPos = pos;
    ctes.PtLightMaxRadius = radius;
    ctes.PtLightInvDeltaRadius = 1.0f / (radius - radius * ptLight->getDecay());
    ctes.PtLightShadowIntensity = 1.f-ptLight->getShadowIntensity();
    ctes.PtLightShadowResolution = shadow == nullptr ? 0 : shadow->getResolution();
    ctes.PtLightShadowBlur = ptLight->getShadowFocus();
    ctes.PtLightShadowJittering = ptLight->getShadowJittering();
    ctes.PtLightSpecularAmountModifier = ptLight->getSpecularAmountModifier();
    ctes.PtLightSpecularPowerFactor = ptLight->getSpecularPowerFactor();
    ctes_ptlight.uploadToGPU();
}

void activateDirLightCB()
{
    ctes_dirlight.activateInVS(SCTES_DirLight::SLOT);
    ctes_dirlight.activateInPS(SCTES_DirLight::SLOT);
}

void activateDirLight(const CDirLight* dirLight, const Camera* camera, const CShadow* shadow)
{
    auto& ctes = ctes_dirlight.get();
    float radius = dirLight->getRadius();
    float zNear = camera->getZNear();
    float zFar = camera->getZFar();
    bool ortho = camera->isOrthographic();
    float tanFov = std::tan(camera->getFov()/2);
    float w = camera->getW();
    float h = camera->getH();
    float radiusAtZNear = ortho? std::min(w, h)/2 : tanFov*zNear;
    float radiusAtZFar = ortho? radiusAtZNear : tanFov*zFar;
    float decay = dirLight->getDecay();
    auto color = dirLight->getColor();
    ctes.DirLightColor = XMVectorSetW(color, dirLight->getIntensity());
    XMStoreFloat3(&ctes.DirLightWorldPos, camera->getPosition()+dirLight->getOffset());
    ctes.DirLightMaxRadius = radius > 0 ? radius :
        ortho?
            //Corner of the prism -> sqrt(w*h) (pitagoras)
            //hipotenuse origin->corner at zFar = sqrt(corner*corner + zFar*zFar) 
            sqrt(sq(w) + sq(h) + sq(zFar)) :
            //Max radius to cover the whole fustrum
            sqrt(sq(radiusAtZFar) + sq(zFar));
    ctes.DirLightInvDeltaRadius = 1.0f / (radius - radius * decay);
    XMStoreFloat3(&ctes.DirLightFront, camera->getFront());
    ctes.DirLightZNear = zNear;
    ctes.DirLightZDiff = zFar-zNear;
    ctes.DirLightSpotRadiusNear = radiusAtZNear;
    ctes.DirLightSpotRadiusDiff = radiusAtZFar - ctes.DirLightSpotRadiusNear;
    ctes.DirLightSpotDecay = dirLight->getSpotDecay();
    ctes.DirLightIsSpotlight = dirLight->isSpotlight();
    ctes.DirLightShadowIntensity = (1-dirLight->getShadowIntensity())*color.af();
    ctes.DirLightShadowResolution = shadow == nullptr ? 0 : shadow->getResolution();
    ctes.DirLightShadowFocus = dirLight->getShadowFocus();
    ctes.DirLightShadowJittering = dirLight->getShadowJittering();
    ctes.DirLightSpecularAmountModifier = dirLight->getSpecularAmountModifier();
    ctes.DirLightSpecularPowerFactor = dirLight->getSpecularPowerFactor();
    ctes_dirlight.uploadToGPU();
}

void activatePaintCB()
{
    ctes_paint.activateInPS(SCTES_Paint::SLOT);
}

void activatePaintGroup(const CPaintGroup* p)
{
    auto& ctes = ctes_paint.get();
    ctes.PaintColor = p->getColor();
    ctes.PaintIntensity = p->getIntensity();
    ctes.PaintDecay = p->getDecay();
    ctes.PaintSIIntensity = p->getSIIntensity();
    ctes.PaintFireY = PaintManager::getFireLevel();
    ctes_paint.uploadToGPU();
}

void activateMistCB()
{
    ctes_mist.activateInVS(SCTES_Mist::SLOT);
    ctes_mist.activateInPS(SCTES_Mist::SLOT);
}

void activateMist(const CMist* mist, unsigned level)
{
    auto& cb = ctes_mist.get();
    XMStoreFloat3(&cb.MistColorTop, mist->getColorTop());
    XMStoreFloat3(&cb.MistColorBottom, mist->getColorBottom());
    cb.MistSize.x = mist->getWidth() / mist->getUnitWorldSize();
    cb.MistSize.y = mist->getLength() / mist->getUnitWorldSize();
    cb.MistOffset.x = mist->getOffsetX() / mist->getWidth();
    cb.MistOffset.y = mist->getOffsetZ() / mist->getLength();
	cb.MistChaotic = mist->getChaotic();
	cb.MistDarkenAlpha = mist->getDarkenAlpha();
	cb.MistFactor = mist->getFactor();
	cb.MistAdd = mist->getMinimun();
	cb.MistLayerDecay = mist->getLayerDecay();
	cb.MistIntensity = mist->getIntensity();
	cb.MistDepthTolerance = mist->getDepthTolerance();
	cb.MistSqSqrt = mist->getSqSqrt();
	cb.MistLevel = level;
    ctes_mist.uploadToGPU();
}

void activateVolPtLightCB()
{
    ctes_volLight.activateInVS(SCTES_VolLight::SLOT);
    ctes_volLight.activateInPS(SCTES_VolLight::SLOT);
}

void activateVolPtLight(const CVolPtLight* light, const XMVECTOR& pos)
{
    auto radius = light->getRadius();
    auto& cb = ctes_volLight.get();
    XMStoreFloat3(&cb.VolLightPos, pos);
    cb.VolLightColor = light->getColor();
    cb.VolLightDensity = light->getDensity();
    cb.VolLightOccludedAddend = light->getOccludedAddend();
    cb.VolLightIlluminatedAddend = light->getIlluminatedAddend();
    cb.VolLightRayDecay = light->getRayDecay();
    cb.VolLightRadius = radius;
    cb.VolLightInvDeltaRadius = 1.0f / (radius - radius * light->getDecay());
    cb.VolLightWeight = light->getWeight();
    cb.VolLightNormalShadeMin = light->getNormalShadeMin();
    cb.VolLightMaxSamples= light->getMaxSamples();
    ctes_volLight.uploadToGPU();
}

void drawLine(XMVECTOR src, XMVECTOR target)
{
    XMMATRIX mtx = XMMatrixIdentity();
    XMVECTOR delta = target - src;
    
    XMVectorSetW(delta, 0.f);
    XMVectorSetW(src, 1.f);
    mtx.r[2] = delta;
    mtx.r[3] = src;
    ctes_object.get().World = mtx;
    ctes_object.uploadToGPU();
    mesh_line.activateAndRender();
}


void createSphere(Mesh& mesh, float radius, unsigned latitudes, unsigned longitudes)
{
	unsigned nVertices = ((latitudes-2) * longitudes) + 2;
	unsigned nIndices  = 3*(((latitudes-3)*(longitudes)*2) + (longitudes*2));

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	std::vector<VertexPos> vertices(nVertices);
	std::vector<Mesh::index_t> indices(nIndices);

	XMVECTOR currVertPos = zAxis_v;

	vertices[0].Pos.x = 0.0f;
	vertices[0].Pos.y = 0.0f;
	vertices[0].Pos.z = 1.0f;

	for(unsigned i = 0; i < latitudes-2; ++i)
	{
		spherePitch = (i+1) * (M_PIf/(latitudes-1));
		XMMATRIX rotX = XMMatrixRotationX(spherePitch);
		for(unsigned j = 0; j < longitudes; ++j)
		{
			sphereYaw = j * (M_TAUf/(longitudes));
			XMMATRIX rotY = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal( zAxis_v, (rotX * rotY) );	
			currVertPos = XMVector3Normalize( currVertPos );
			vertices[i*longitudes+j+1].Pos.x = XMVectorGetX(currVertPos);
			vertices[i*longitudes+j+1].Pos.y = XMVectorGetY(currVertPos);
			vertices[i*longitudes+j+1].Pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[nVertices-1].Pos.x =  0.0f;
	vertices[nVertices-1].Pos.y =  0.0f;
	vertices[nVertices-1].Pos.z = -1.0f;

	int k = 0;
	for(unsigned l = 0; l < longitudes-1; ++l) {
		indices[k] = 0;
		indices[k+1] = l+1;
		indices[k+2] = l+2;
		k += 3;
	}

	indices[k] = 0;
	indices[k+1] = longitudes;
	indices[k+2] = 1;
	k += 3;

	for(unsigned i = 0; i < latitudes-3; ++i) {
		for(unsigned j = 0; j < longitudes-1; ++j) {
			indices[k]   = i*longitudes+j+1;
			indices[k+1] = (i+1)*longitudes+j+1;
			indices[k+2] = i*longitudes+j+2;

			indices[k+3] = (i+1)*longitudes+j+1;
			indices[k+4] = (i+1)*longitudes+j+2;
			indices[k+5] = i*longitudes+j+2;

			k += 6; // next quad
		}

		indices[k]   = (i*longitudes)+longitudes;
		indices[k+1] = ((i+1)*longitudes)+longitudes;
		indices[k+2] = (i*longitudes)+1;

		indices[k+3] = ((i+1)*longitudes)+longitudes;
		indices[k+4] = ((i+1)*longitudes)+1;
		indices[k+5] = (i*longitudes)+1;

		k += 6;
	}

	for(unsigned l = 0; l < longitudes-1; ++l) {
		indices[k] = nVertices-1;
		indices[k+1] = (nVertices-1)-(l+1);
		indices[k+2] = (nVertices-1)-(l+2);
		k += 3;
	}

	indices[k] = nVertices-1;
	indices[k+1] = (nVertices-1)-longitudes;
	indices[k+2] = nVertices-2;

    for(auto& v : vertices) {v.Pos.x*=radius; v.Pos.y*=radius; v.Pos.z*=radius;}
    mesh.create(nVertices, vertices.data(), nIndices, indices.data(), Mesh::TRIANGLE_LIST);
}

XMVECTOR RenderConstsMirror::GlobalLightDirection = XMVectorSet(0.35f,-1.55f,-0.7f,1);
bool RenderConstsMirror::dirty = true;
float RenderConstsMirror::GlobalLightStrength = 0.265f;
float RenderConstsMirror::SkyBoxBlend = 0.f;
float RenderConstsMirror::SkyBoxBright = 0.15f;
float RenderConstsMirror::AmbientMax = 0.35f;
float RenderConstsMirror::AmbientMin = 0.05f;
float RenderConstsMirror::ResolveAlbedo = 0.0f;
float RenderConstsMirror::ResolveAlbedoLight = 1.25f;
float RenderConstsMirror::ResolveLight = 0.0f;
float RenderConstsMirror::ResolveSpecular = 0.7f;
float RenderConstsMirror::PowerSpecular = 20.f;
float RenderConstsMirror::ResolveSelfIll = 1.4f;
float RenderConstsMirror::ResolveSaturation = 0.500f;
float RenderConstsMirror::ResolveBrightness = 0.506f;
float RenderConstsMirror::ResolveContrast = 0.475f;

void RenderConstsMirror::update() {
    dirty = false;
    auto& ctes = ctes_render.get();  
    XMStoreFloat3(&ctes.GlobalLightDirection, -XMVector3Normalize(RenderConstsMirror::GlobalLightDirection));
    ctes.GlobalLightStrength        = RenderConstsMirror::GlobalLightStrength;
    ctes.SkyBoxBlend                = RenderConstsMirror::SkyBoxBlend;
    ctes.SkyBoxBright               = RenderConstsMirror::SkyBoxBright;
    ctes.AmbientDiff                = RenderConstsMirror::AmbientMax - RenderConstsMirror::AmbientMin;
    ctes.AmbientMin                 = RenderConstsMirror::AmbientMin;
    ctes.ResolveAlbedo              = RenderConstsMirror::ResolveAlbedo;
    ctes.ResolveAlbedoLight         = RenderConstsMirror::ResolveAlbedoLight;
    ctes.ResolveLight               = RenderConstsMirror::ResolveLight;
    ctes.ResolveSpecular            = RenderConstsMirror::ResolveSpecular;
    ctes.PowerSpecular              = RenderConstsMirror::PowerSpecular;
    ctes.ResolveSelfIll             = RenderConstsMirror::ResolveSelfIll;
    ctes.ResolveSaturation          = RenderConstsMirror::ResolveSaturation;
    ctes.ResolveBrightness          = RenderConstsMirror::ResolveBrightness;
    ctes.ResolveContrast            = RenderConstsMirror::ResolveContrast;
    ctes_render.uploadToGPU();
    ctes_render.activateInPS(SCTES_Render::SLOT);
};

XMVECTOR calculateTangent(XMVECTOR tan1, XMVECTOR tan2, XMFLOAT3 nf3)
{
    XMVECTOR n = DirectX::XMVectorSet(nf3.x, nf3.y, nf3.z, 1);
    
    // Gram-Schmidt normalization
    XMVECTOR tangentXMV = XMVector3Normalize(
        tan1 - n * XMVectorGetX(XMVector3Dot(n, tan1)));
    
    // Calculate handedness
    XMVECTOR nXt = XMVector3Cross(n, tan1);
    float tanw = XMVectorGetX(XMVector3Dot(nXt, tan2)) < 0 ? -1.f:1.f;
    return XMVectorSetW(tangentXMV, tanw);
}

size_t getCurrentBoneIndex()
{
    return boneBuffer.data.size();
}

void addBoneToBuffer(XMMATRIX m)
{
    auto& data (boneBuffer.data);
    data.push_back(m.r[0]);
    data.push_back(m.r[1]);
    data.push_back(m.r[2]);
    data.push_back(m.r[3]);
}
void uploadBonesToGPU()
{
    boneBuffer.uploadToGPU();
}

void activateBoneBuffer()
{
    boneBuffer.activateInVS(10);
}

void rewindBoneBuffer()
{
    boneBuffer.data.clear();
}


void updateGlobalConstants(float elapsed)
{
    auto& ctes = ctes_global.get();
    ctes.WorldTime+=elapsed;
    ctes_global.uploadToGPU();
}

bool createPlanePUNTWithUV(Mesh& mesh,
    float xl, float zf, float xr, float zb, float y,
    float ul, float vf, float ur, float vb)
{
    std::vector< VertexPosUVNormalTangent > vertices;
    vertices.resize(4);
    VertexPosUVNormalTangent *v = &vertices[0];
    
	v->Pos = XMFLOAT3(xl, y, zb); v->UV = XMFLOAT2(ul,vb); v->Normal = XMFLOAT3(0,1,0); v++;
	v->Pos = XMFLOAT3(xl, y, zf); v->UV = XMFLOAT2(ul,vf); v->Normal = XMFLOAT3(0,1,0); v++;
	v->Pos = XMFLOAT3(xr, y, zb); v->UV = XMFLOAT2(ur,vb); v->Normal = XMFLOAT3(0,1,0); v++;
	v->Pos = XMFLOAT3(xr, y, zf); v->UV = XMFLOAT2(ur,vf); v->Normal = XMFLOAT3(0,1,0); v++;
    
    const std::vector< Mesh::index_t > indices = {0, 1, 2, 2, 1, 3};
    
    //Probably the tangent is easy to figure out but I'd rather trust a tested algorithm
    computeTangentSpace(vertices, indices);
    
    auto min = XMVectorMin(XMVectorSet(xl, y, zb, 1), XMVectorSet(xr, y, zf, 1));
    auto max = XMVectorMax(XMVectorSet(xl, y, zb, 1), XMVectorSet(xr, y, zf, 1));

    return mesh.create(
        (unsigned)vertices.size(), vertices.data(), (unsigned)indices.size(), indices.data(),
        Mesh::TRIANGLE_LIST, min, max);
}


}