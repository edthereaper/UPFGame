#ifndef RENDER_FX_CTES_H_
#define RENDER_FX_CTES_H_

#ifdef WIN32
#include "shader_platform.h"
#else
#include "platform/shader_platform.h"
#endif

//Updated per fx
cbuffer SCTES_Mix SHADER_REGISTER(b10)
{
    SHADER_SLOT(10);
    float4 MixVector;
    float  MixBaseFactor;
    float  MixFactor;
    float  MixRangeMax;
    float  MixRangeMin;
};

//Updated per fx phase
cbuffer SCTES_Blur SHADER_REGISTER(b11)
{
    SHADER_SLOT(11);
	float2 BlurDelta;
    float  BlurAmp;
    float  BlurTolerance;
};

cbuffer SCTES_MotionBlur SHADER_REGISTER(b11)
{
    SHADER_SLOT(11);
    matrix MotionBlurPrevViewProjection;
    float  MotionBlurAmp;
    uint   MotionBlurNSamples;
	float SCTES_MotionBlur_Padding[2];
};
          
//Updated per fx
cbuffer SCTES_Smudge SHADER_REGISTER(b11)
{
    SHADER_SLOT(11);
    float2  SmudgeAmp;
    float2  SmudgeTolerance;
};
       
//Updated per fx
cbuffer SCTES_SSAO SHADER_REGISTER(b11)
{
    SHADER_SLOT(11);
	float  SSAORadius;
	float  SSAOIntensity;
	float  SSAOScale;
	float  SSAOBias;
	float  SSAOJitter;
	float  SSAODepthTolerance;
	float  SCTES_SSAO_Padding[2];
};

#endif