#include "textured.fx"
#include "math.fx"

float4 FXIdentity(VS_TEXTURED_OUTPUT input) : SV_Target
{
    return txDiffuse.Sample(samClampLinear, input.UV);
}

float4 FXNegative(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
    return float4(1-color.rgb, color.a);
}

float4 FXNegativeA(VS_TEXTURED_OUTPUT input) : SV_Target
{
    return 1- txDiffuse.Sample(samClampLinear, input.UV);
}

float4 FXFlipH(VS_TEXTURED_OUTPUT input) : SV_Target
{
	input.UV.x = 1 - input.UV.x;
    return txDiffuse.Sample(samClampLinear, input.UV);
}

float4 FXFlipV(VS_TEXTURED_OUTPUT input) : SV_Target
{
	input.UV.y = 1 - input.UV.y;
    return txDiffuse.Sample(samClampLinear, input.UV);
}

float4 FXRotate(VS_TEXTURED_OUTPUT input) : SV_Target
{
    return txDiffuse.Sample(samClampLinear, 1-input.UV);
}

float4 FXGrayscale(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float maxC = max(color.r, max(color.g, color.b));
	float minC = min(color.r, min(color.g, color.b));
	color = (maxC + minC) *0.5f;
	color.a = 1;
    return color;
}

float4 FXRChannel(VS_TEXTURED_OUTPUT input) : SV_Target {return float4(txDiffuse.Sample(samClampLinear, input.UV).rrr, 1);}
float4 FXGChannel(VS_TEXTURED_OUTPUT input) : SV_Target {return float4(txDiffuse.Sample(samClampLinear, input.UV).ggg, 1);}
float4 FXBChannel(VS_TEXTURED_OUTPUT input) : SV_Target {return float4(txDiffuse.Sample(samClampLinear, input.UV).bbb, 1);}
float4 FXAChannel(VS_TEXTURED_OUTPUT input) : SV_Target {return float4(txDiffuse.Sample(samClampLinear, input.UV).aaa, 1);}
float4 FXInvAChannel(VS_TEXTURED_OUTPUT input) : SV_Target {
	float inva = 1 - txDiffuse.Sample(samClampLinear, input.UV).a;
	return float4(inva, inva, inva, 1);}

float4 FXRed(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.gb = 0;
    return color;
}

float4 FXGreen(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.rb = 0;
    return color;
}

float4 FXBlue(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.rg = 0;
    return color;
}

float4 FXYellow(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.b = 0;
    return color;
}

float4 FXMagenta(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.g = 0;
    return color;
}

float4 FXCyan(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	color.r = 0;
    return color;
}

float4 FXMonochrome(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float l = luminance(txDiffuse.Sample(samClampLinear, input.UV));
    return float4(l,l,l,1);
}

float4 FXHueShift(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float3 rgb = txDiffuse.Sample(samClampLinear, input.UV).rgb;
    return float4(float3(rgb.brg + rgb.gbr)/2, 1);
}

float4 FXLighten(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float3 rgb = txDiffuse.Sample(samClampLinear, input.UV).rgb;
    return float4(saturate(rgb*1.45), 1);
}

float4 FXPosterize8(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float r = color.r > 0.5f ? 1 : 0;
	float g = color.g > 0.5f ? 1 : 0;
	float b = color.b > 0.5f ? 1 : 0;
    return float4(r,g,b,1);
}

float4 FXPosterize9(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float r = color.r > 0.66f ? 1 : color.r > 0.33f ? 0.5f : 0;
	float g = color.g > 0.66f ? 1 : color.g > 0.33f ? 0.5f : 0;
	float b = color.b > 0.66f ? 1 : color.b > 0.33f ? 0.5f : 0;
    return float4(r,g,b,1);
}

float4 FXPosterize64(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float r = color.r > 0.75f ? 1 : color.r > 0.5f ? 0.66f : color.r > 0.25f ? 0.33f : 0;
	float g = color.g > 0.75f ? 1 : color.g > 0.5f ? 0.66f : color.g > 0.25f ? 0.33f : 0;
	float b = color.b > 0.75f ? 1 : color.b > 0.5f ? 0.66f : color.b > 0.25f ? 0.33f : 0;
    return float4(r,g,b,1);
}

float4 FXPosterize125(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float r = color.r > 0.8f ? 1 : color.r > 0.6f ? 0.75f : color.r > 0.4f ? 0.5f : color.r > 0.1f ? 0.25f : 0;
	float g = color.g > 0.8f ? 1 : color.g > 0.6f ? 0.75f : color.g > 0.4f ? 0.5f : color.g > 0.1f ? 0.25f : 0;
	float b = color.b > 0.8f ? 1 : color.b > 0.6f ? 0.75f : color.b > 0.4f ? 0.5f : color.b > 0.1f ? 0.25f : 0;
    return float4(r,g,b,1);
}

float4 Multiply(VS_TEXTURED_OUTPUT input) : SV_Target
{
    return txDiffuse.Sample(samClampLinear, input.UV) * Tint;
}