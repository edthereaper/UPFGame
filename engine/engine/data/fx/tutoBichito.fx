#include "platform/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txDepth   : register(t2);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
  float3 wPos : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(float4 Pos : POSITION0, float2 UV : TEXCOORD0)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  float3 wpos = Pos.xyz + ( CameraWorldUp.xyz * Pos.y + CameraWorldLeft.xyz * Pos.x);
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);
  output.UV = UV;	
  output.wPos = wpos;

  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_TEXTURED_OUTPUT input, in float4 iPosition : SV_Position) : SV_Target
{
  float my_depth = dot(input.wPos.xyz - CameraWorldPos.xyz, CameraWorldFront.xyz) / CameraZFar;
  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float pixel_detph = txDepth.Load(ss_load_coords).x;
  float delta_z = abs(pixel_detph - my_depth);
  delta_z = saturate(delta_z * 1000);
  float4 color = txDiffuse.Sample(samClampLinear, input.UV);
  color = float4(color.rgb*(Tint.rgb*Tint.a) + color.rgb*(1-Tint.a), color.a);
  color.a *= delta_z;
  return color;
}
