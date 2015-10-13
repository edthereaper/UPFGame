#include "textured.fx"
#include "math.fx"
#include "random.fx"
#include "platform/fx_ctes.h"

#define BOBJENKINS 1
#define FRAC_SIN_DOT 2
#define HASH_METHOD BOBJENKINS

Texture2D txNormal 	 		: register(t1);
Texture2D txSpace 		    : register(t2);

float doAmbientOcclusion(in float2 uv, in float3 wPos, in float3 norm, float centerDepth)
{
	const float4 space = txSpace.Sample(samClampLinear, saturate(uv), 0);
	const float3 diff = space.xyz - wPos;
	const float3 v = normalize(diff);
	const float d = length(diff)*SSAOScale;
	
	float occludeFactor = max(0,dot(norm,v)-SSAOBias);
	float distanceFactor = SSAOIntensity/(1+d);
	//float depthFactor = SSAODepthTolerance / (SSAODepthTolerance - abs(space.w*CameraZFar - centerDepth));
	//depthFactor*=depthFactor;
	bool depthFactor = abs(space.w*CameraZFar - centerDepth) < SSAODepthTolerance;
	
	return occludeFactor * distanceFactor * depthFactor;
}

float getSSAO(float2 uv)
{
	float4 wPos = txSpace.Sample(samClampLinear, uv, 0);
	
	float3 n = txNormal.Sample(samClampLinear, uv).xyz*2-1;
	
#if HASH_METHOD == FRAC_SIN_DOT
	const float r0 = (1-SSAOJitter) + SSAOJitter * 2 * frac(sin(dot(uv, float2(12.98, 78.23))) * 43758.54535)-1;
	const float r1 = (1-SSAOJitter) + SSAOJitter * 2 * frac(sin(dot(uv, float2(74.35, 26.82))) * 82593.29674)-1;
	const float4 rand = float2(r0, r1, r0, r1);
#elif HASH_METHOD == BOBJENKINS
	const uint2 h = hash(uv).xy;
	const float4 rand = float4(extractUnorm_16(h.x), extractUnorm_16(h.y));
#endif

	float ao = 0.0f;
	float viewZ = wPos.w*CameraZFar;
	float radius = SSAORadius / viewZ;
	
	static const float2 vec[4] = {
		float2(1,0),float2(-1,0), float2(0,1),float2(0,-1)};
	static const float f[4][4] = {
		{0.25f, 0.50f, 0.75f, 1.00f,},
		{0.75f, 0.25f, 1.00f, 0.50f,},
		{0.50f, 1.00f, 0.25f, 0.75f,},
		{1.00f, 0.75f, 0.50f, 0.25f,},
		};
	for (uint j = 0; j < 4; ++j) {
		float2 coord1a = reflect(vec[j],rand.xy)*radius;
		float2 coord1b = reflect(vec[j],rand.xy)*radius;
		float2 coord2a = float2(
			coord1a.x*0.707 - coord1a.y*0.707,
			coord1a.x*0.707 + coord1a.y*0.707);
		float2 coord2b = float2(
			coord1b.x*0.707 - coord1b.y*0.707,
			coord1b.x*0.707 + coord1b.y*0.707);
		
		ao += doAmbientOcclusion(uv + coord1a*f[j][0], wPos.xyz, n, viewZ);
		ao += doAmbientOcclusion(uv + coord2a*f[j][1], wPos.xyz, n, viewZ);
		ao += doAmbientOcclusion(uv + coord1b*f[j][2], wPos.xyz, n, viewZ);
		ao += doAmbientOcclusion(uv + coord2b*f[j][3], wPos.xyz, n, viewZ);
	}
	ao/= 16;	
	return ao;
}

float SSAO(VS_TEXTURED_OUTPUT input) : SV_Target
{	
	float ao = getSSAO(input.UV);	
	return ao;
}
