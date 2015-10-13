#ifdef WIN32

#define Buffer render::Buffer

#define float2              DirectX::XMFLOAT2
#define float3              DirectX::XMFLOAT3
#define float4              DirectX::XMVECTOR
#define matrix              DirectX::XMMATRIX
#define cbuffer             struct 
#define SHADER_REGISTER(x)  
#define SHADER_SLOT(x)      static const unsigned SLOT = x
//Ensure bool := 4 bytes
#define HSLSBool            uint32_t 
#define uint				uint32_t
#define EXTERN              extern

#else
// We are in HLSL
#define SHADER_REGISTER(x)  : register(x)
#define SHADER_SLOT(x)
#define HSLSBool            bool
#define EXTERN 

#endif