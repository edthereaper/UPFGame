#include "platform/shader_ctes.h"
#include "math.fx"

Texture2D txSpace : register(t2);
Texture2D txPerlinNoise : register(t66);

SamplerState samWrapLinear   : register(s0);
SamplerState samClampLinear  : register(s1);
SamplerState samBorderLinear : register(s2);

struct VS_PROJECTED_OUTPUT
{
  float4 SPos   : SV_POSITION;
  float4 WPos   : TEXCOORD0;
  uint   Level	: TEXCOORD1;
  float2 UV     : TEXCOORD2;
};

VS_PROJECTED_OUTPUT VSProjected( float4 Pos : POSITION, float2 UV : TEXCOORD0 )
{
    VS_PROJECTED_OUTPUT output = (VS_PROJECTED_OUTPUT)0;
    output.WPos = mul(Pos, World);
    output.SPos = mul(output.WPos, ViewProjection);
    output.UV = UV;
	output.Level = -Pos.y;
    return output;
}

float alphaByDepth(float3 wPos, uint3 pixel, float depthTolerance)
{
	float depth = dot(wPos.xyz - CameraWorldPos.xyz, CameraWorldFront.xyz) / CameraZFar;
	float worldDepth = txSpace.Load(pixel).w;
	return saturate( abs(worldDepth - depth) * depthTolerance);
}

void PSMist(VS_PROJECTED_OUTPUT input,
	out float4 albedo : SV_Target0,
	out float4 normals : SV_Target1,
	out float4 selfIllumination : SV_Target4,
	out float4 data : SV_Target5
	)
{
	float flip = MistChaotic ? -1 : 1;
	uint3 pixel = uint3(input.SPos.xy, 0);
	
	const uint level = MistLevel;//input.Level;
	float levelGradient = level/4.f;
	
	float3 tint = (1-levelGradient) * MistColorTop + levelGradient * MistColorBottom;
	
	const float2 mod[6] = {
		// N(1, 0.6/3)
		float2(1.27218,  	  0.918743*flip),
		float2(1.14305*flip,  0.782175),
		float2(1.05456*flip,  1.261051*flip),
		float2(0.84716,   	  0.678435),
		float2(0.4,           0.4*flip), // Make bottom layer very slow
		float2(1,             1), // Make bottom layer very slow
		};
		
	const float2 off[6] = {
		// N(0, 1/3)
		float2(0.731267, -0.328384),
		float2(0.313672,  0.034304),
		float2(-0.15573,  0.299663),
		float2(-0.05273, -0.329134),
		float2(0.038201, -0.170865),
		float2(0, 0),
		};
	
	float2 uv = input.UV + MistOffset * mod[level] + off[level];
	uv *= MistSize;
	
	float mist = txPerlinNoise.Sample(samWrapLinear, uv).r;
	
	mist = saturate(sq(mist)*MistSqSqrt) + saturate(sqrt(mist)*-MistSqSqrt) + (1-abs(MistSqSqrt))*mist;	
	mist = saturate(mist * MistFactor + MistAdd) * MistIntensity;
	
	tint *= (1-MistDarkenAlpha) + MistDarkenAlpha*(1-mist);
	mist *= alphaByDepth(input.WPos.xyz, pixel, MistDepthTolerance);
	
	mist *= saturate(levelGradient*MistLayerDecay) + saturate((1-levelGradient)*-MistLayerDecay) + (1-abs(MistLayerDecay));
	
	albedo = float4(tint, mist); // Mist
	
	normals = float4(0.5,1 * (CameraWorldPos.y > input.WPos.y ?1:-1),0.5, mist); //Obscure by alpha
	selfIllumination = float4(0,0,0,mist); //erase
	data = float4(0,0,mist,mist); //mask ambient
}