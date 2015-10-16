#include "platform/shader_ctes.h"

Texture2D txNormal 			: register(t1);
Texture2D txSpace 			: register(t2);
Texture2D txData 			: register(t3);

Texture2D txTransfHDiffuse	: register(t16);
Texture2D txTransfVDiffuse	: register(t18);

struct PROJECTED_DATA 
{
	float4 sPos   : SV_POSITION;
	float3 center : TEXCOORD0;  
	float  radius : TEXCOORD1;
};

PROJECTED_DATA VSProjectPaint(float4 iPos : POSITION, float3 c : ICENTER, float r : IRADIUS)
{
	PROJECTED_DATA ret = (PROJECTED_DATA)0;
	iPos.xyz = iPos.xyz*r + c;
	ret.sPos = mul(iPos, ViewProjection);
	ret.center = c;
	ret.radius = r;
	return ret;
}

float4 PSPaintWire(PROJECTED_DATA input) : SV_TARGET {
	return Tint;
}
void PSPaint(PROJECTED_DATA i,
	out float4 paint : SV_TARGET0,
	out float4 data2 : SV_TARGET1,
	out float4 normal : SV_TARGET2
) 
{
	uint3 pixel = uint3(i.sPos.xy, 0);
	float3 wPos = txSpace.Load(pixel).xyz;
	float objectPaintableAmount = txData.Load(pixel).a;
	float dist = distance(i.center, wPos);
	
	float burnFactor = saturate(((wPos.y+0.5) - PaintFireY)*10*PaintDecay);
	float burnedArea = 1-saturate(abs(wPos.y - PaintFireY)*0.85*PaintDecay);
	
	float deltaRadius = i.radius * (1 - PaintDecay);
	float paintAmount = saturate((i.radius - dist) / deltaRadius) * objectPaintableAmount * burnFactor;
	float4 normalSample = txNormal.Load(pixel);
	
	
	float3 paintColor = PaintColor.rgb * PaintColor.a  * (1-burnedArea) + 
		float3(0.9, 0.2, 0) * burnedArea;
	paint = float4(paintColor* paintAmount * PaintSIIntensity, normalSample.a);
	data2 = float4(1,1, paintAmount * PaintIntensity, paintAmount*PaintIntensity);
	normal = float4(normalSample.xyz, paintAmount*.25);
}