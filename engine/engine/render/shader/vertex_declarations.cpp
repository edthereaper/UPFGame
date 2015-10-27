#include "mcv_platform.h"
#include "vertex_declarations.h"
using namespace utils;

namespace render {

unsigned bytesOfFormat(DXGI_FORMAT fmt) {
    switch (fmt) {
        case DXGI_FORMAT_R16_FLOAT: return 2;
        case DXGI_FORMAT_R32_FLOAT: return 4;
        case DXGI_FORMAT_R32G32_FLOAT: return 8;
        case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
        case DXGI_FORMAT_R32_UINT: return 4;
        case DXGI_FORMAT_R16G16_SINT: return 4;
        case DXGI_FORMAT_R16G16_UINT: return 4;
        case DXGI_FORMAT_R8G8B8A8_UINT: return 4;
        case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
    }
    fatal("bytesOfFormat: Unknown fmt");
    return 0;
}

unsigned VertexDecl::getBytesPerVertex(unsigned stream) const
{
    unsigned bytes = 0;
    for (unsigned i = 0; i < nelems; ++i) {
        if (elems[i].InputSlot == stream || stream < 0)
        bytes += bytesOfFormat(elems[i].Format);
    }
    return bytes;
}

#define DEF_VTX_DECL(x) VertexDecl x(x##_layout, ARRAYSIZE(x##_layout)) 

// POSITION
D3D11_INPUT_ELEMENT_DESC vdcl_position_layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT    , 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position);

// POSITION --- COLOR
D3D11_INPUT_ELEMENT_DESC vdcl_position_color_layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT    , 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position_color);

// POSITION --- UV
D3D11_INPUT_ELEMENT_DESC vdcl_position_uv_layout[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position_uv);

// POSITION --- UV ---- NORMAL
D3D11_INPUT_ELEMENT_DESC vdcl_position_uv_normal_layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position_uv_normal);

// POSITION --- UV --- NORMAL --- TANGENT
D3D11_INPUT_ELEMENT_DESC vdcl_position_uv_normal_tangent_layout[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position_uv_normal_tangent);

// POSITION --- UV --- NORMAL --- TANGENT
// INSTANCE TRANSFORM
D3D11_INPUT_ELEMENT_DESC vdcl_punt_instance_layout[] =
{
  { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
  { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
  { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA,   0 },
  { "TANGENT",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA,   0 },
  { "ITRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "ITRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "ITRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "ITRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "ITINT"     , 0, DXGI_FORMAT_R8G8B8A8_UNORM,     1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "ISELFILL"  , 0, DXGI_FORMAT_R8G8B8A8_UNORM,     1, 68, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "IUSERDATA" , 0, DXGI_FORMAT_R16G16_SINT,        1, 72, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
  { "IPADDING"  , 0, DXGI_FORMAT_R32_FLOAT,          1, 76, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
};
DEF_VTX_DECL(vdcl_punt_instance);

// POSITION --- UV --- NORMAL --- TANGENT --- BONE-IDS --- WEIGHT
D3D11_INPUT_ELEMENT_DESC vdcl_skin_layout[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "BONEIDS",  0, DXGI_FORMAT_R8G8B8A8_UINT,      0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "WEIGHTS",  0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_skin);

// ------------------------------------------------------
D3D11_INPUT_ELEMENT_DESC vdcl_particle_layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
	{ "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "QZ",       0, DXGI_FORMAT_R32_FLOAT,          1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "QW",       0, DXGI_FORMAT_R32_FLOAT,          1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR",	  0, DXGI_FORMAT_R8G8B8A8_UNORM,	 1, 20, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR",	  1, DXGI_FORMAT_R8G8B8A8_UNORM,	 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLORW",   0, DXGI_FORMAT_R32_FLOAT,          1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "FRAME",    0, DXGI_FORMAT_R32_FLOAT,          1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "SCALE",    0, DXGI_FORMAT_R32_FLOAT,          1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "USER",     0, DXGI_FORMAT_R16G16_UINT,        1, 40, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
};
DEF_VTX_DECL(vdcl_particle);

D3D11_INPUT_ELEMENT_DESC vdcl_paint_layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT    , 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "ICENTER",  0, DXGI_FORMAT_R32G32B32_FLOAT,    1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "IRADIUS",  0, DXGI_FORMAT_R32_FLOAT,          1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
};
DEF_VTX_DECL(vdcl_paint);

D3D11_INPUT_ELEMENT_DESC vdcl_flower_layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "FRAME"   , 0, DXGI_FORMAT_R32_UINT,        1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SCALE"   , 0, DXGI_FORMAT_R32G32_FLOAT,    1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "LIFE"    , 0, DXGI_FORMAT_R32_FLOAT,       1, 20, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
};
DEF_VTX_DECL(vdcl_flower);

template<> VertexDecl* getVertexDecl<VertexPos>()                   {return &vdcl_position;}
template<> VertexDecl* getVertexDecl<VertexPosColor>()              {return &vdcl_position_color;}
template<> VertexDecl* getVertexDecl<VertexPosUV>()                 {return &vdcl_position_uv;}
template<> VertexDecl* getVertexDecl<VertexPosUVNormal>()           {return &vdcl_position_uv_normal;}
template<> VertexDecl* getVertexDecl<VertexPosUVNormalTangent>()    {return &vdcl_position_uv_normal_tangent;}
template<> VertexDecl* getVertexDecl<VertexPUNTInstance>()          {return &vdcl_punt_instance;}
template<> VertexDecl* getVertexDecl<VertexSkin>()                  {return &vdcl_skin;}
template<> VertexDecl* getVertexDecl<VertexParticleUData>()			{return &vdcl_particle; }
template<> VertexDecl* getVertexDecl<VertexPaintData>()			    {return &vdcl_paint; }
template<> VertexDecl* getVertexDecl<VertexFlowerData>()			{return &vdcl_flower; }

}