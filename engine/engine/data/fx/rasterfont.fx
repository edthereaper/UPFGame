//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "platform/shader_ctes.h"

Texture2D txDiffuse : register(t0);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS( float4 Pos : POSITION0, float2 UV : TEXCOORD0)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);

  // Animate the UV's.
  float nmod16 = fmod(TextureFrameNum, TextureNFrames);
  int   idx = int(nmod16);
  float coords_x = fmod(TextureFrameNum, TextureFramesPerRow);
  float coords_y = int( TextureFrameNum / TextureFramesPerRow) * 1.15f;
  
  output.UV.x = (coords_x + UV.x) / TextureFramesPerRow;
  output.UV.y = (coords_y + UV.y) / TextureFramesPerRow;
  
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
    return color*(Tint*Tint.a) + color*(1-Tint.a);
}
