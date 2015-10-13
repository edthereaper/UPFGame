#include "textured.fx"
#include "platform/fx_ctes.h"

Texture2D txSpace : register(t1);
Texture2D txData  : register(t2);
float4 PSMotionBlur(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float2 sPos1 = input.UV *2 - 1;
	float3 wPos1 = txSpace.Sample(samClampLinear, input.UV).xyz;
	float4 sPos0 = mul(float4(wPos1,1), MotionBlurPrevViewProjection);  
	sPos0 /= sPos0.w;
	sPos0.y = -sPos0.y;
	
	float objectAmount =  txData.Sample(samClampLinear, input.UV).r;
	float amp = MotionBlurAmp * objectAmount;
	
    float2 vel = (sPos1.xy - sPos0.xy)*amp/(2.f * MotionBlurNSamples);
   
	float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float4 blur = color;
	float2 sPos = sPos1 + vel;
	float totalWeight = 1;
	for(uint i = 1; i < MotionBlurNSamples; ++i, sPos -= vel) {  
		float2 texel = 0.5*(sPos+1);
		float sampleAmount = txData.Sample(samClampLinear, texel).r;
		blur += sampleAmount * txDiffuse.Sample(samClampLinear, texel);
		totalWeight += sampleAmount;
	}
	
	return (color + blur / totalWeight)/2;
}