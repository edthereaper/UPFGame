//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "platform/shader_ctes.h"

Texture2D txDiffuse : register(t0);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);


//--------------------------------------------------------------------------------------
// Gradient Technique
//--------------------------------------------------------------------------------------

struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
};

VS_TEXTURED_OUTPUT VSTextured( float4 Pos : POSITION, float2 UV : TEXCOORD0 )
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
    output.Pos = mul(Pos, World);
    output.Pos = mul(output.Pos, ViewProjection);
    output.UV = UV;	
    return output;
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.a = Tint.a*color.a;
	return color;
}