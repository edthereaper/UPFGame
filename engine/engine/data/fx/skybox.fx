#include "platform/shader_ctes.h"

TextureCube skyMapA 		: register(t32);
TextureCube skyMapB 		: register(t33);
SamplerState samWrapLinear 	: register(s0);
SamplerState samClampLinear : register(s1);

struct VS_SKYBOX_OUTPUT
{
	float4 Pos	: SV_POSITION;
	float3 UVW	: TEXCOORD;
};

VS_SKYBOX_OUTPUT VS(float3 Pos : POSITION)
{
    VS_SKYBOX_OUTPUT output = (VS_SKYBOX_OUTPUT)0;
    output.Pos = mul(float4(Pos, 1.0f), World);
	//Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
    output.Pos = mul(output.Pos, ViewProjection).xyww;
	output.UVW = Pos;
	output.UVW.x = 1 - output.UVW.x;
	return output;
}

float4 PS(VS_SKYBOX_OUTPUT input) : SV_Target
{
    float4 colorA = skyMapA.Sample(samClampLinear, input.UVW);
    float4 colorB = skyMapB.Sample(samClampLinear, input.UVW);
	float4 color = SkyBoxBlend*colorA + (1-SkyBoxBlend)*colorB;
    return float4((color*(Tint*Tint.a) + color*(1-Tint.a)).rgb, 1);
}

/*
	Output into deferred render targets to prevent being affected by illumination, etc
*/
void PSDefer(VS_SKYBOX_OUTPUT input,
	out float4 albedo : SV_Target0,
	out float4 normal : SV_Target1,
    out float4 acc_light : SV_Target2,
	out float4 space : SV_Target3,
	out float4 selfIll : SV_Target4,
    out float4 data1 : SV_Target5,
    out float4 data2 : SV_Target6,
    out float4 normal2 : SV_Target7
	)
{
    float4 colorA = skyMapA.Sample(samClampLinear, input.UVW);
    float4 colorB = skyMapB.Sample(samClampLinear, input.UVW);
	float4 color = SkyBoxBlend*colorA + (1-SkyBoxBlend)*colorB;
    color = color*(Tint*Tint.a) + color*(1-Tint.a);
	albedo = float4(color.rgba);
	acc_light = float4(SkyBoxBright, SkyBoxBright, SkyBoxBright, 0); 	
	normal = float4(0.5f, 0.5f, 0.5f, 0.5f); 	// no normal,
    normal2 = float4(0.5f, 0.5f, 0.5f, 0.5f); 	// no normal,
	float3 pos = input.UVW;
	pos.x = 1 - pos.x;
	pos *= 1e30f;
	space = float4(pos, 1);// furthest from camera
	selfIll = 0;
	data1 = float4(1,1,0,0);
	data2 = 0;
}