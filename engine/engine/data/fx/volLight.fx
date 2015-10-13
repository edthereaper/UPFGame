#include "textured.fx"
#include "math.fx"
#include "platform/fx_ctes.h"

//#define _DEBUG

Texture2D txNormal: register(t0);
Texture2D txSpace : register(t1);

struct PROJECTION {
	float4 sPos : SV_Position;
	//float4 wPos : TEXCOORD0;
};

PROJECTION VSProject(float4 iPos : POSITION) {
	PROJECTION ret;
	float4 wPos = mul(iPos, World);
	ret.sPos = mul(wPos, ViewProjection);
	//ret.wPos = wPos;
	return ret;
}

float traceVolLightRay(float3 wPos, float3 deltaWPos, float nSamples, float2 resolution, float occludedAdd, float illuminatedAdd, float decay)
{
	static const float TOLERANCE = 0;
	float lightAmount = 0;
	float illuminationDecay = 1;
	for (uint i = 0; illuminationDecay >= TOLERANCE && i < uint(nSamples); ++i) {  
		float depth = dot(float4(wPos,1) - CameraWorldPos, CameraWorldFront) / CameraZFar;
		float2 ssPos = saturate(UVofWPos(wPos, ViewProjection));
		float expectedDepth = txSpace.Load(uint3(resolution * ssPos,0)).w;
		bool occluded = depth >= expectedDepth;
		lightAmount += (occluded? occludedAdd : illuminatedAdd) * illuminationDecay ;
		illuminationDecay *= decay;
		wPos += deltaWPos;
	}
	return lightAmount;
}

float4 VolumetricLight(PROJECTION i) : SV_Target
{
	static const uint MIN_SAMPLES = 14; //Too few samples lead to "black holes" in the light center
	
	const float2 resolution = float2(CameraXRes, CameraYRes);
	uint3 pixel = uint3(i.sPos.xy,0);
	float2 texel = i.sPos.xy/resolution;
	
	float4 space = txSpace.Load(pixel);
    float3 tint = VolLightColor.rgb * VolLightColor.a + (1-VolLightColor.a);
	
	float dTexel = length(texel - UVofWPos(VolLightPos.xyz, ViewProjection));
	float nSamples = (VolLightMaxSamples-MIN_SAMPLES) * dTexel + MIN_SAMPLES;

	float3 wPos = VolLightPos.xyz;
	float3 dir = space.xyz - VolLightPos.xyz;
	float  distanceToLight = length(dir);
	float3 deltaWPos = dir/(nSamples*VolLightDensity);
	
	float attenuation = saturate((VolLightRadius - distanceToLight)*VolLightInvDeltaRadius);
	clip(attenuation);
	
	float lightAmount = traceVolLightRay(wPos, deltaWPos, nSamples, resolution,
		VolLightOccludedAddend, VolLightIlluminatedAddend, VolLightRayDecay);
	
	// Basic diffuse lighting
	float3 N = txNormal.Load(pixel).xyz * 2 - 1.;
	float3 L = dir / distanceToLight;
	float normalShade = saturate(saturate(dot(N, L))+VolLightNormalShadeMin);
	return float4(tint*saturate(lightAmount * VolLightWeight / nSamples)*attenuation*normalShade,1);
}