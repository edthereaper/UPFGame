#include "platform/shader_ctes.h"

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 wPos : TEXCOORD1;
    float4 Color : COLOR0;
};

VS_OUTPUT VS( float4 Pos : POSITION, float4 Color : COLOR )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.wPos = mul( Pos, World );
    output.Pos = mul( output.wPos, ViewProjection );
    output.Color = Color;
    return output;
}

float4 PS( VS_OUTPUT input ) : SV_Target
{
	float a = Tint.a;
    return input.Color.rgba*(1-a)+Tint*a;
}

float4 PSTint( VS_OUTPUT input ) : SV_Target
{
    return Tint;
}

/*
	Output into deferred render targets to prevent being affected by illumination, etc
*/
void PSDefer( VS_OUTPUT input,
	out float4 albedo : SV_Target0,
	out float4 normal : SV_Target1,
    out float4 acc_light : SV_Target2,
	out float4 space : SV_Target3,
	out float4 sIll : SV_Target4)
{
    float4 color = Tint;
	albedo = color;
	normal = 0.5f;
	acc_light = float4(1,1,1,0);
	sIll = 0;
	space = float4(input.wPos.xyz, 1);//depth = depthdot(input.wPos - CameraWorldPos, CameraWorldFront) / CameraZFar;
}

