#include "textured.fx"
#include "platform/fx_ctes.h"
#include "math.fx"

Texture2D txAux : register(t1);

float4 Smudge(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = txDiffuse.Sample(samClampLinear, input.UV)
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2( 1, 0) * SmudgeAmp) 
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2( 0, 1) * SmudgeAmp) 
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2(-1, 0) * SmudgeAmp) 
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2( 0,-1) * SmudgeAmp) 
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2( sqrt2, sqrt2) * SmudgeAmp * 0.5) * sqrt2 * 0.5
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2(-sqrt2, sqrt2) * SmudgeAmp * 0.5) * sqrt2 * 0.5
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2( sqrt2,-sqrt2) * SmudgeAmp * 0.5) * sqrt2 * 0.5
		+ txDiffuse.Sample(samClampLinear, input.UV +  float2(-sqrt2,-sqrt2) * SmudgeAmp * 0.5) * sqrt2 * 0.5
		;
	return color / (5. + 2 * sqrt2);
}