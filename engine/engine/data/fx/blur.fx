#include "textured.fx"
#include "platform/fx_ctes.h"

Texture2D txAux : register(t1);

float4 PSBlur5(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color =
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 2 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 1 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV                          ) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 1 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 2 * BlurAmp);
	return color / 5.;
}

float4 PSBlur(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color =
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 3 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 2 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 1 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV                          ) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 1 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 2 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 3 * BlurAmp);
	return color / 7.;
}

float4 PSBlurBleed(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float nSamples = 1;
	float4 right = 
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 3 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 2 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV + BlurDelta * 1 * BlurAmp);
	right /=3;
	
	if (right.x > color.x) {color+=right; nSamples++;}
	
	float4 left = 
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 1 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 2 * BlurAmp) +
		txDiffuse.Sample(samClampLinear, input.UV - BlurDelta * 3 * BlurAmp);
	left /=3;
	if (left.x > color.x) {color+=left; nSamples++;}
	
	
	return color / nSamples;
}

float4 PSBlurDepth(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float depth = txAux.Sample(samClampLinear, input.UV).a;
	
	float nSamples = 1;
	float2 texel1 = BlurDelta * BlurAmp;
	float2 texel2 = texel1*2;
	float2 texel3 = texel1*3;
	
	float2 uvpt1 = input.UV + texel1;
	float2 uvpt2 = input.UV + texel2;
	float2 uvpt3 = input.UV + texel3;
	float2 uvmt1 = input.UV - texel1;
	float2 uvmt2 = input.UV - texel2;
	float2 uvmt3 = input.UV - texel3;
	
	float4 right = 
		txDiffuse.Sample(samClampLinear, uvpt1) +
		txDiffuse.Sample(samClampLinear, uvpt2) +
		txDiffuse.Sample(samClampLinear, uvpt3);
	right /=3;
	
	float2 testDepth;
	
	testDepth.x = 
		txAux.Sample(samClampLinear, uvpt1).a +
		txAux.Sample(samClampLinear, uvpt2).a +
		txAux.Sample(samClampLinear, uvpt3).a;
	
	float4 left = 
		txDiffuse.Sample(samClampLinear, uvmt1) +
		txDiffuse.Sample(samClampLinear, uvmt2) +
		txDiffuse.Sample(samClampLinear, uvmt3);
	left /=3;
	testDepth.y = 
		txAux.Sample(samClampLinear, uvmt1).a +
		txAux.Sample(samClampLinear, uvmt2).a +
		txAux.Sample(samClampLinear, uvmt3).a;
		
	depth = depth * CameraZFar;
	testDepth = (testDepth * CameraZFar) / 3;
	
	bool2 tolerance = bool2(abs(testDepth - depth) < BlurTolerance);
	
	//DEBUG
#ifdef _DEBUG
	//Use the green and bue channels to see what's going on
	//LightBuffer only uses the x component
	color += float4(0, tolerance.r,tolerance.g,1);
#endif
	
	if (tolerance.r) {color+=right; nSamples++;}
	if (tolerance.g) {color+=left; nSamples++;}
	return color / nSamples;
}

float4 PSBlurDepthBehind(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float depth = txAux.Sample(samClampLinear, input.UV).a;
	
	float nSamples = 1;
	float2 texel1 = BlurDelta * BlurAmp;
	float2 texel2 = texel1*2;
	float2 texel3 = texel1*3;
	
	float2 uvpt  = input.UV + texel1*1.5;
	float2 uvmt  = input.UV - texel1*1.5;
	float2 uvpt1 = input.UV + texel1;
	float2 uvpt2 = input.UV + texel2;
	float2 uvpt3 = input.UV + texel3;
	float2 uvmt1 = input.UV - texel1;
	float2 uvmt2 = input.UV - texel2;
	float2 uvmt3 = input.UV - texel3;
	
	float4 right = 
		txDiffuse.Sample(samClampLinear, uvpt1)*0.75+
		txDiffuse.Sample(samClampLinear, uvpt2)*0.50+
		txDiffuse.Sample(samClampLinear, uvpt3)*0.25;
	right /= 1.5;
	
	float2 testDepth;
	
	testDepth.x = txAux.Sample(samClampLinear, uvpt).a;
	
	float4 left = 
		txDiffuse.Sample(samClampLinear, uvmt1)*0.75+
		txDiffuse.Sample(samClampLinear, uvmt2)*0.50+
		txDiffuse.Sample(samClampLinear, uvmt3)*0.25;
	left /= 1.5;
	testDepth.y = txAux.Sample(samClampLinear, uvmt).a;
		
	depth = depth * CameraZFar;
	testDepth = (testDepth * CameraZFar);
	
	bool2 tolerance = bool2((testDepth - depth) < BlurTolerance);
	
	//DEBUG
#ifdef _DEBUG
	//Use the green and bue channels to see what's going on
	//LightBuffer only uses the x component
	color += float4(0, tolerance.r,tolerance.g,1);
#endif

	color+=right * tolerance.r;
	nSamples += 1*tolerance.r;
	
	color+=left * tolerance.g;
	nSamples += 1*tolerance.g;
	
	return color / nSamples;
}