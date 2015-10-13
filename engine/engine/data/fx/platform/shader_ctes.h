#ifndef RENDER_SHADER_CTES_H_
#define RENDER_SHADER_CTES_H_

#ifdef WIN32
#include "shader_platform.h"
#else
#include "platform/shader_platform.h"
#endif

//TODO: CONVERT COLORS INTO unorm float4

//Updated per object
cbuffer SCTES_Object SHADER_REGISTER(b0)
{
    SHADER_SLOT(0);
    matrix World;
    float4 Tint;
    float4 SelfIllumination;
    uint FirstBone;
	HSLSBool AlphaAsSpecular;
    float DiffuseAsSelfIllumination;
    float BaseSpecular;
    float MotionBlurAmount;
    float SelfIlluminationClamp;
	float ObjectPaintableAmount;
	float SCTES_Object_Padding[1];
};

//Updated per camera
cbuffer SCTES_Camera SHADER_REGISTER(b1)
{
    SHADER_SLOT(1);
    matrix ViewProjection;
    matrix CameraView;
    float4 CameraWorldPos;
    float4 CameraWorldFront;
    float4 CameraWorldUp;
    float4 CameraWorldLeft;
    float  CameraZNear, CameraZFar, CameraViewD;
    float  CameraPadding;
    float  CameraHalfXRes, CameraHalfYRes, CameraXRes, CameraYRes;
};

//Updated per frame
cbuffer SCTES_Global SHADER_REGISTER(b2)
{
    SHADER_SLOT(2);
    float  WorldTime;
    float  SCTES_Global_padding[3];
};

//Updated rarely
cbuffer SCTES_Render SHADER_REGISTER(b3)
{
    SHADER_SLOT(3);
	
	//Ambient Light
	float3 GlobalLightDirection;
	float GlobalLightStrength;
	
	//Skybox
    float  SkyBoxBlend;
    float  SkyBoxBright;
	
	//Resolve
    float  AmbientDiff;
    float  AmbientMin;
	
    float  ResolveAlbedo;
    float  ResolveAlbedoLight;
    float  ResolveLight;
    float  ResolveSelfIll;

    float  ResolveSpecular;
    float  PowerSpecular;    

    float  ResolveSaturation;
    float  ResolveBrightness;
    float  ResolveContrast;
           
    float  SCTES_Render_PADDING[3];
};

//Updated per frame
EXTERN Buffer<float4> boneBuffer SHADER_REGISTER(t10);

//Updated per light
cbuffer SCTES_Light SHADER_REGISTER(b4)
{
    SHADER_SLOT(4);
    matrix LightViewProjection;
    matrix LightViewProjectionOffset;
    float3 LightWorldPos;
    float  LightZFar;
};
//Updated per light
cbuffer SCTES_PtLight SHADER_REGISTER(b5)
{
    SHADER_SLOT(5);
    float4 PtLightWorldPos;
    float4 PtLightColor;
    float  PtLightMaxRadius, PtLightInvDeltaRadius;
    float  PtLightShadowIntensity;
    uint   PtLightShadowResolution;
    float  PtLightShadowBlur;
    float  PtLightShadowJittering;
    float  PtLightSpecularAmountModifier;
    float  PtLightSpecularPowerFactor;
};
//Updated per light
cbuffer SCTES_DirLight SHADER_REGISTER(b5)
{
    SHADER_SLOT(5);
    float3 DirLightWorldPos;
    float DirLightZNear;
    float3 DirLightFront;
    float DirLightZDiff;
    float4 DirLightColor;
    float DirLightMaxRadius, DirLightInvDeltaRadius;
    float DirLightSpotRadiusNear, DirLightSpotRadiusDiff;
    float DirLightSpotDecay;
    HSLSBool DirLightIsSpotlight;
    float  DirLightShadowIntensity;
    uint DirLightShadowResolution;
    float DirLightShadowFocus;
    float DirLightShadowJittering;
    float DirLightSpecularAmountModifier;
    float DirLightSpecularPowerFactor;
};


//Updated per environment item
cbuffer SCTES_VolLight SHADER_REGISTER(b5)
{
    SHADER_SLOT(5);
	float4 VolLightColor;
	float3 VolLightPos;
	float  VolLightWeight;
	float  VolLightDensity;
	float  VolLightRayDecay;
	float  VolLightOccludedAddend;
	float  VolLightIlluminatedAddend;
    float  VolLightRadius;
    float  VolLightInvDeltaRadius;
    float  VolLightNormalShadeMin;
    uint   VolLightMaxSamples;
};

//Updated per environment item
cbuffer SCTES_Mist SHADER_REGISTER(b5)
{
    SHADER_SLOT(5);
    float3 MistColorTop;
    float MistIntensity;
    float3 MistColorBottom;
    float MistLayerDecay;
    float2 MistOffset;
    float2 MistSize;
    float MistDarkenAlpha;
    float MistFactor;
    float MistAdd;
    bool  MistChaotic;
    float MistDepthTolerance;
    float MistSqSqrt;
    uint MistLevel;
    float SCTES_Mist_Padding[1];
};


//Updated per paint group
cbuffer SCTES_Paint SHADER_REGISTER(b5)
{
    SHADER_SLOT(5);
    float4 PaintColor;
	float  PaintIntensity;
	float  PaintDecay;
	float  PaintSIIntensity;
	float  PaintFireY;
};

cbuffer SCTES_Texture SHADER_REGISTER(b6)
{
	SHADER_SLOT(6);
	uint TextureNFrames;
	uint TextureFramesPerRow;
	float TextureFrameNum;
	float SCTES_Texture_padding[1];
};

#endif
