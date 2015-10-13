#include "platform/shader_ctes.h"
#include "textured.fx"
#include "math.fx"
#include "project.fx"

Texture2D txNormal 						: register(t1);
Texture2D txSpace  						: register(t2);
Texture2D txLight     					: register(t3);
Texture2D txSelfIllumination			: register(t4);
Texture2D txAmbient						: register(t5);
Texture2D txData1  						: register(t6);
Texture2D txData2						: register(t7);
Texture2D txPaint						: register(t8);
Texture2D txNormal2						: register(t9);

Texture2D txTransfH0Diffuse				: register(t16);
Texture2D txTransfH1Diffuse				: register(t20);
Texture2D txTransfH2Diffuse				: register(t22);
Texture2D txTransfH0Normals				: register(t17);
Texture2D txTransfH1Normals				: register(t21);
Texture2D txTransfH2Normals				: register(t23);

Texture2D txTransfV0Diffuse				: register(t18);
Texture2D txTransfV0Normals				: register(t19);

Texture2D txShadowMap 					: register(t6);
TextureCube txShadowCube 				: register(t6);


SamplerComparisonState samPCFShadows 	: register(s3);

struct VS_DEFERRED_OUTPUT
{
    float4 Pos       : SV_POSITION;
    float2 UV        : TEXCOORD0;
    float4 wPos      : TEXCOORD1;
    float3 wNormal   : NORMAL;
    float4 wTangent  : TANGENT;
	unorm float4 tint : COLOR0;
	unorm float4 selfIllumination : COLOR1;
};

//////////////// TRANSFORMATION UTILS

float getVertFactor(float normalY)
{
	float vertFactor = saturate(normalY);
	return vertFactor * vertFactor;
}

Texture2D txNoise : register(t67);

struct VAR_DATA {
	float v;
	float w;
};
	
VAR_DATA getVarData(float2 xz)
{
	VAR_DATA ret;
	ret.v = txNoise.Sample(samWrapLinear, xz*0.01).r;
	
	ret.w = sq(abs(ret.v*2-1));
	return ret;
}

static const float PAINTH0_SIZE = 4;
static const float PAINTH1_SIZE = 4;
static const float PAINTH2_SIZE = 4;

float3 getGroundTexture(float2 xz, float2 uv)
{
	VAR_DATA var = getVarData(xz);
	float3 s0 = txTransfH0Diffuse.Sample(samWrapLinear, uv*PAINTH0_SIZE).rgb;
	float3 s1 = txTransfH1Diffuse.Sample(samWrapLinear, uv*PAINTH1_SIZE).rgb;
	float3 s2 = txTransfH2Diffuse.Sample(samWrapLinear, uv*PAINTH2_SIZE).rgb;
	float3 sV = var.v < 0.5 ? s1 : s2;
	
	return (1-var.w) * s0 + var.w * sV;
}

float3 getGroundNormal(float2 xz, float2 uv)
{
	VAR_DATA var = getVarData(xz);
	float3 s0 = txTransfH0Normals.Sample(samWrapLinear, uv*PAINTH0_SIZE).rgb;
	float3 s1 = txTransfH1Normals.Sample(samWrapLinear, uv*PAINTH1_SIZE).rgb;
	float3 s2 = txTransfH2Normals.Sample(samWrapLinear, uv*PAINTH2_SIZE).rgb;
	float3 sV = var.v < 0.5 ? s1 : s2;
	
	return (1-var.w) * s0 + var.w * sV;
}

//////////////// DEFERRED RENDER ////////////////

// Regular vertex
VS_DEFERRED_OUTPUT VS(float4 Pos : POSITION, float2 UV : TEXCOORD0, float3 Normal : NORMAL, float4 Tangent : TANGENT)
{
    VS_DEFERRED_OUTPUT output = (VS_DEFERRED_OUTPUT)0;
    float4 wPos = mul(Pos, World);
    output.Pos = mul(wPos, ViewProjection);
    output.wNormal = mul(Normal, (float3x3)World);
    output.UV = UV;
    output.wPos = wPos;
	// Rotate the tangent keeping the w value
    output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
    output.wTangent.w = Tangent.w;
    return output;
}

VS_DEFERRED_OUTPUT VSInstanced(
	float4 Pos : POSITION, float2 UV : TEXCOORD0, float3 Normal : NORMAL, float4 Tangent : TANGENT,
	matrix InstanceWorld : ITRANSFORM,
	unorm float4 InstanceTint : ITINT,
	unorm float4 InstanceSelfIllumination : ISELFILL )
{
    VS_DEFERRED_OUTPUT output = (VS_DEFERRED_OUTPUT)0;
    float4 wPos = mul(Pos, InstanceWorld);
    output.Pos = mul(wPos, ViewProjection);
    output.wNormal = mul(Normal, (float3x3)InstanceWorld);
    output.UV = UV;
    output.wPos = wPos;
	// Rotate the tangent keeping the w value
    output.wTangent.xyz = mul(Tangent.xyz, (float3x3)InstanceWorld);
    output.wTangent.w = Tangent.w;
    output.tint = InstanceTint;
    output.selfIllumination = InstanceSelfIllumination;
    return output;
}


// Skinned vertex
VS_DEFERRED_OUTPUT VSSkin(
	float4 Pos      : POSITION ,
	float2 UV       : TEXCOORD0 ,
	float3 Normal   : NORMAL ,
	float4 Tangent  : TANGENT ,
	uint4  bone_ids : BONEIDS ,
	float4 weights  : WEIGHTS
	)
{
	matrix skin_mtx = skinMatrix(bone_ids, weights);

    VS_DEFERRED_OUTPUT output = (VS_DEFERRED_OUTPUT)0;
	float4 skinned_pos = mul(Pos, skin_mtx);
    float4 wPos = skinned_pos;
    output.Pos = mul(skinned_pos, ViewProjection);
    output.UV = UV;
    output.wPos = wPos;
    output.wNormal = mul(Normal, (float3x3)skin_mtx);
    output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
    output.wTangent.w = Tangent.w;
    return output;
}

void PSGBuffer(
    in VS_DEFERRED_OUTPUT input,
	out float4 albedo : SV_Target0,
	out float4 normal : SV_Target1,
    out float4 light : SV_Target2,
    out float4 space : SV_Target3,
    out float4 selfIllumination : SV_Target4,
    out float4 data1 : SV_Target5,
    out float4 data2 : SV_Target6,
    out float4 normal2 : SV_Target7
)
{
	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);
	clip( color.a - 0.05);
	
	float4 tint = float4(
		Tint.rgb*Tint.a + input.tint.rgb*input.tint.a,
		max(Tint.a, input.tint.a));
	
	albedo = float4(color.rgb*(tint.rgb*tint.a) + color.rgb*(1-tint.a), color.a);
	
	float4 sIllTex = txSelfIllumination.Sample(samWrapLinear, input.UV);
	float4 sIll = sIllTex;
	sIll.rgb += albedo.rgb*DiffuseAsSelfIllumination;
	sIll.rgb *= sIll.a;
	float sIllWeight = saturate(value(sIllTex)*4);
	sIll.rgb += (1-sIllWeight) * SelfIllumination.rgb*SelfIllumination.a;
	sIll.rgb += (1-sIllWeight) * input.selfIllumination.rgb*input.selfIllumination.a;
	selfIllumination = sIll;
	
	light = 0;
	
	float3   in_normal = normalize(input.wNormal);
	float3   in_tangent = normalize(input.wTangent.xyz);
	float3   in_binormal = cross(in_normal, in_tangent) * -input.wTangent.w;
	float3x3 TBN = float3x3(in_tangent, in_binormal, in_normal);
	
	// Convert the range 0...1 from the texture to range -1 ..1 
	float4 normalSample = txNormal.Sample(samWrapLinear, input.UV);
	float3 normal_tangent_space = normalSample.xyz * 2 - 1.;
	float3 wnormal_per_pixel = mul(normal_tangent_space, TBN);
	
	float specularAmount = (1- normalSample.a + color.a * AlphaAsSpecular) + BaseSpecular;
	
	normal = float4((wnormal_per_pixel+1)*.5, specularAmount);
	unorm float depth = dot(input.wPos - CameraWorldPos, CameraWorldFront) / CameraZFar;
	
	space = float4(input.wPos.xyz, depth*color.a);
	
	float siClamp = sIllWeight != 0 ? 1 : SelfIlluminationClamp;
	data1 = float4(MotionBlurAmount, siClamp, 0, ObjectPaintableAmount);
	data2 = float4(abs(input.UV.xy % 1), 0, 0);
	
	float3 transfHN = getGroundNormal(input.wPos.xz, input.UV);
	float3 transfVN = txTransfV0Normals.Sample(samWrapLinear, input.UV).xyz;
	float vertFactor = getVertFactor(in_normal.y);
	float3 paintNormal = (vertFactor * transfHN + (1 - vertFactor) * transfVN);
	wnormal_per_pixel = mul(paintNormal*2-1, TBN);
	normal2 = float4((wnormal_per_pixel+1)*.5, vertFactor);
}

inline float3 applySBC(float3 startColor, float3 sbc)
{
    float s = sbc.r * 2;
    float b = sbc.g * 2 - 1;
    float c = sbc.b * 2;
 
    float3 outputColor = startColor;
    outputColor = (outputColor - 0.5f) * (c) + 0.5f;
    outputColor = outputColor + b;        
    float3 intensity = dot(outputColor, float3(0.299,0.587,0.114));
    outputColor = lerp(intensity, outputColor, s);
 
    return outputColor;
}

float4 PSResolve(VS_TEXTURED_OUTPUT input) :SV_Target0
{
	float2 resolution = float2(CameraXRes, CameraYRes);
	float2 texel = input.UV;
	uint3 pixel = uint3(resolution*texel, 0);
	float3 albedo = txDiffuse.Load(pixel).rgb;
	float3 data = txData1.Load(pixel).gba;
	float selfIlluminationClamp = data.r;
	float mistMask = (1-data.g);
	
	float3 normal = txNormal.Load(pixel).rgb*2-1;
	
	float4 data2 = txData2.Load(pixel);
	float2 uv = data2.xy;
	float4 paint = txPaint.Load(pixel);
	float paintAmount = data2.b;
	float3 transfHD = getGroundTexture(txSpace.Load(pixel).xz, uv);
	float3 transfVD = txTransfV0Diffuse.Sample(samWrapLinear, uv).rgb;
	
	float vertFactor = paint.a;
	float3 paintTexture = vertFactor * transfHD + (1 - vertFactor) * transfVD;
	
	float4 lightSample = txLight.Load(pixel);
	float3 light = lightSample.rgb;
	float specular = lightSample.a * mistMask;
	
	float4 siSample = txSelfIllumination.Sample(samClampLinear, texel);
	float3 selfIllumination = clamp(siSample.rgb * siSample.a * ResolveSelfIll, 0, selfIlluminationClamp);
	float3 paintSI = paint.rgb;
	paintSI = clamp(paintSI, 0, 1-data2.a+0.025);
	selfIllumination += paintSI;
	
	static const float INV_TINT_PAINT = 0.9;
	static const float TINT_PAINT = 1-INV_TINT_PAINT;
	
	float ambientSample = txAmbient.Load(pixel).r * mistMask;
	float ambientGeometry = (1-dot(normal, GlobalLightDirection));
	float3 ambient = max(ambientGeometry*ambientGeometry*GlobalLightStrength, ambientSample*0.9)+ambientSample*0.1;
	ambient = AmbientDiff * (1 - saturate(ambient)) + AmbientMin;
	albedo = paintAmount * (paintTexture *INV_TINT_PAINT + TINT_PAINT * paint.rgb) + (1 - paintAmount) * albedo;
	albedo += selfIllumination;
	light +=  selfIllumination;
	
	float3 output =
		ResolveAlbedo * albedo +
		ResolveAlbedoLight * albedo * (light + ambient) +
		ResolveLight * light +
		ResolveSpecular * specular;
	output = applySBC(output, float3(ResolveSaturation, ResolveBrightness, ResolveContrast));
	
	return float4(output, 1);
}
