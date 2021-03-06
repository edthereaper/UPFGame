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
	float3 iPos : POSITION1, float2 uv : TEXCOORD0, float2 sca : SCALE)
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
	float3 iPos : POSITION1, float2 uv : TEXCOORD0, float2 sca : SCALE, uint2 user : USER)
{
	DATA ret = (DATA)0;
	float3 up = float3(World._21, World._22, World._23);
	float3 left = float3(World._11, World._12, World._13);
	ret.wPos = iPos + sca.xyx * ( up * vPos.y + left * vPos.x);
	ret.sPos = mul(float4( ret.wPos, 1 ), ViewProjection);
	
	uint frame = user.y % 16;
	uint reflex = user.y / 16;
	uv.x = uv.x * (1-reflex) + (1-uv.x) * reflex;
	uv.y = 1 - uv.y;
	
	// UV for frame
	ret.UV = (float2(uint(frame % 4), uint(frame / 4)) + uv)/4;
	return ret;
}
void PSFlower(DATA data, out float4 pixel : SV_Target)
{
	float4 color = txFlowers.Sample(samClampLinear, data.UV);
	clip(color.a <= 0.001);
	color.a *= saturate(data.sPos.z*data.sPos.w);
	pixel = color * color.a;
}