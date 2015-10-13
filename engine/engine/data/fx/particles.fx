#include "platform/shader_ctes.h"
#include "textured.fx"
#include "math.fx"

Texture2D txSpace   : register(t2);

//--------------------------------------------------------------------------------------
struct VS_PARTICLE_OUTPUT
{
  float4 Pos          : SV_POSITION;
  float2 UV           : TEXCOORD0;
  float3 wPos         : TEXCOORD1;
  float colorWeight   : TEXCOORD2;
  unorm float4 colorA : COLOR0;
  unorm float4 colorB : COLOR1;
  float dead 		  : TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_PARTICLE_OUTPUT VS(
  float4 Pos             : POSITION0
, float2 UV              : TEXCOORD0
, float3 InstancePos     : POSITION1
, float InstanceQz       : QZ
, float InstanceQw       : QW
, unorm float4 colorA    : COLOR0
, unorm float4 colorB    : COLOR1
, float  colorWeight     : COLORW
, uint   user            : USER0 
, float  InstanceFrame   : FRAME0   
, float  InstanceScale   : SCALE0   
)
{
	VS_PARTICLE_OUTPUT output = (VS_PARTICLE_OUTPUT)0;
	
	float4 q = {0,0, InstanceQz, InstanceQw};
	float3 vtx = rotate(Pos.xyz, q); 

	float3 wpos = InstancePos + InstanceScale * ( CameraWorldUp.xyz * vtx.y + CameraWorldLeft.xyz * vtx.x);
	
	output.Pos = mul(float4( wpos, 1 ), ViewProjection);
	
	// Animate the UVs.
	float nmod16 = fmod(floor(InstanceFrame), TextureNFrames);
	int   idx = int(nmod16);
	float coords_x = fmod(idx, TextureFramesPerRow);
	float coords_y = int( idx / TextureFramesPerRow);
	
	output.UV.x = (coords_x + UV.x) / TextureFramesPerRow;
	output.UV.y = (coords_y + UV.y) / TextureFramesPerRow;
	
	output.wPos = wpos;
	output.colorWeight = colorWeight;
	output.colorA = colorA;
	output.colorB = colorB;
	
	output.dead = user.x != 0 ? -1 : 1;
	
	return output;
}

float alphaByDepth(float3 wPos, uint3 ssPixel)
{
	float depth = dot(wPos.xyz - CameraWorldPos.xyz, CameraWorldFront.xyz) / CameraZFar;
	float worldDepth = txSpace.Load(ssPixel).w;
	return saturate( abs(worldDepth - depth) * 1000);
}

float4 mixSampleTint(float2 uv, float4 tint)
{
	float4 color = txDiffuse.Sample(samClampLinear, uv);
	float a = tint.a * color.a;
	return float4(color.rgb*tint.rgb*tint.a + color.rgb*(1-a), color.a);
}

float4 PSDefault(VS_PARTICLE_OUTPUT input) : SV_Target0
{
	clip(input.dead);
	float4 inColor = input.colorA * (1-input.colorWeight) + input.colorB * input.colorWeight;
	float4 color = mixSampleTint(input.UV, inColor);
	color.a = min(color.a, alphaByDepth(input.wPos, uint3(input.Pos.xy, 0)));
	return color;
}

float4 PSLuminanceAsAlpha(VS_PARTICLE_OUTPUT input) : SV_Target0
{
	clip(input.dead);
	float4 inColor = input.colorA * (1-input.colorWeight) + input.colorB * input.colorWeight;
	float4 color = mixSampleTint(input.UV, inColor);
	float l = luminance(color.rgb);
	color.a = min(l*color.a, alphaByDepth(input.wPos, uint3(input.Pos.xy, 0)));
	color.rgb /= l; // compensate loss from black as alpha
	return color;
}

float4 PSValueAsAlpha(VS_PARTICLE_OUTPUT input) : SV_Target0
{
	clip(input.dead);
	float4 inColor = input.colorA * (1-input.colorWeight) + input.colorB * input.colorWeight;
	float4 color = mixSampleTint(input.UV, inColor);
	float l = value(color.rgb);
	color.a = min(l*color.a, alphaByDepth(input.wPos, uint3(input.Pos.xy, 0)));
	color.rgb /= l; // compensate loss from black as alpha
	return color;
}


//Do not use! use this to try things before creating a new particle tech
float4 PSSandbox(VS_PARTICLE_OUTPUT input) : SV_Target0
{
	clip(input.dead);
	float4 inColor = input.colorA * (1-input.colorWeight) + input.colorB * input.colorWeight;
	float4 color = mixSampleTint(input.UV, inColor);
	float l = value(color.rgb);
	color.a = min(l*color.a, alphaByDepth(input.wPos, uint3(input.Pos.xy, 0)));
	color.rgb /= l; // compensate loss from black as alpha
	return color;
}
