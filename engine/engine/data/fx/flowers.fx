#include "platform/shader_ctes.h"
#include "textured.fx"

Texture2D txFlowers : register(t24);

struct DATA 
{
	float4 sPos  : SV_POSITION;
	float3 wPos  : TEXCOORD1;
	float2 UV    : TEXCOORD0;
};

DATA VSBasic(float3 vPos : POSITION0,
	float3 iPos : POSITION1, float2 uv : TEXCOORD0, float2 sca : SCALE, uint frame : FRAME)
{
	DATA ret = (DATA)0;
	ret.wPos = iPos + vPos;
	ret.sPos = mul(float4( ret.wPos, 1 ), ViewProjection);
	return ret;
}
float4 PSBasic(DATA data) : SV_Target
{
	return Tint;
}

DATA VSFlower(float3 vPos : POSITION0,
	float3 iPos : POSITION1, float2 uv : TEXCOORD0, float2 sca : SCALE, uint frame : FRAME)
{
	DATA ret = (DATA)0;
	ret.wPos = iPos + sca.xyx * ( CameraWorldUp.xyz * vPos.y + CameraWorldLeft.xyz * vPos.x);
	ret.sPos = mul(float4( ret.wPos, 1 ), ViewProjection);
	
	// UV for frame
	ret.UV = (float2(uint(frame % 4), uint(frame / 16)) + uv);
	return ret;
}
float4 PSFlower(DATA data) : SV_Target
{
	return txFlowers.Sample(samClampLinear, data.UV);
}