#include "textured.fx"
#include "platform/fx_ctes.h"

Texture2D txMix    : register(t1);
Texture2D txWeight : register(t2);

float4 MixConstant(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 src = txDiffuse.Sample(samClampLinear, input.UV);
    float4 mix = txMix.Sample(samClampLinear, input.UV);
	
	return src*(MixFactor) + mix*(1-MixFactor);
}

float4 Mix(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 src = txDiffuse.Sample(samClampLinear, input.UV);
    float4 mix = txMix.Sample(samClampLinear, input.UV);
    float4 weightV = txWeight.Sample(samClampLinear, input.UV) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	
	float factor = saturate(MixBaseFactor + weight * MixFactor);
	return src*(factor) + mix*(1-factor);
}

float4 MixSq(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 src = txDiffuse.Sample(samClampLinear, input.UV);
    float4 mix = txMix.Sample(samClampLinear, input.UV);
    float4 weightV = txWeight.Sample(samClampLinear, input.UV) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	
	float factor = saturate(MixBaseFactor + weight * weight * MixFactor);
	return src*(factor) + mix*(1-factor);
}

float4 MixCube(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 src = txDiffuse.Sample(samClampLinear, input.UV);
    float4 mix = txMix.Sample(samClampLinear, input.UV);
    float4 weightV = txWeight.Sample(samClampLinear, input.UV) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	
	float factor = saturate(MixBaseFactor + weight * weight * weight * MixFactor);
	return src*(factor) + mix*(1-factor);
}

float4 MixClipRange(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 src = txDiffuse.Sample(samClampLinear, input.UV);
    float4 mix = txMix.Sample(samClampLinear, input.UV);
    float4 weightV = txWeight.Sample(samClampLinear, input.UV) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	float factor = (weight >= MixRangeMax || weight <= MixRangeMin) ? 0 :
		saturate(MixBaseFactor + weight * MixFactor);
		
	return src*(factor) + mix*(1-factor);
}

float4 MixClipRangeSq(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 src = txDiffuse.Sample(samClampLinear, input.UV);
    float4 mix = txMix.Sample(samClampLinear, input.UV);
    float4 weightV = txWeight.Sample(samClampLinear, input.UV) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	float factor = (weight >= MixRangeMax || weight <= MixRangeMin) ? 0 :
		saturate(MixBaseFactor + weight * weight * MixFactor);
	
	return src*(factor) + mix*(1-factor);
}

//Negative of weight
//MixRangeMax = pupil radius
//MixRangeMin = iris radius
float4 MixNegVignette(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float2 uv = input.UV;

    float4 src = txDiffuse.Sample(samClampLinear, uv);
    float4 mix = txMix.Sample(samClampLinear, uv);
    float4 weightV = (1-txWeight.Sample(samClampLinear, uv)) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	float factor = saturate(MixBaseFactor + weight * MixFactor);
		
	float radius = length(uv*2-1);
	float vignette = saturate((radius-MixRangeMin)/(MixRangeMax-MixRangeMin));
	factor *= vignette;
	return src*(factor) + mix*(1-factor);
}

//Negative of weight
//MixRangeMax = pupil radius
//MixRangeMin = iris radius
float4 MixNegVignetteSq(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float2 uv = input.UV;

    float4 src = txDiffuse.Sample(samClampLinear, uv);
    float4 mix = txMix.Sample(samClampLinear, uv);
    float4 weightV = (1-txWeight.Sample(samClampLinear, uv)) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	float factor = saturate(MixBaseFactor + weight * MixFactor);
		
	float radius = length(uv*2-1);
	float vignette = saturate((radius-MixRangeMin)/(MixRangeMax-MixRangeMin));
	factor *= vignette*vignette;
	return src*(factor) + mix*(1-factor);
}

//Negative of weight
//MixRangeMax = pupil radius
//MixRangeMin = iris radius
float4 MixNegSqVignette(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float2 uv = input.UV;

    float4 src = txDiffuse.Sample(samClampLinear, uv);
    float4 mix = txMix.Sample(samClampLinear, uv);
    float4 weightV = (1-txWeight.Sample(samClampLinear, uv)) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	float factor = saturate(MixBaseFactor + weight * weight * MixFactor);
		
	float radius = length(uv*2-1);
	float vignette = saturate((radius-MixRangeMin)/(MixRangeMax-MixRangeMin));
	factor *= vignette;
	return src*(factor) + mix*(1-factor);
}

//Negative of weight
//MixRangeMax = pupil radius
//MixRangeMin = iris radius
float4 MixNegSqVignetteSq(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float2 uv = input.UV;

    float4 src = txDiffuse.Sample(samClampLinear, uv);
    float4 mix = txMix.Sample(samClampLinear, uv);
    float4 weightV = (1-txWeight.Sample(samClampLinear, uv)) * MixVector;
	float weight = weightV.x + weightV.y + weightV.z + weightV.w;
	float factor = saturate(MixBaseFactor + weight * weight * MixFactor);
		
	float radius = length(uv*2-1);
	float vignette = saturate((radius-MixRangeMin)/(MixRangeMax-MixRangeMin));
	factor *= vignette*vignette;
	return src*(factor) + mix*(1-factor);
}