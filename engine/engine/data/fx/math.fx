static const float PI = 3.14159265359f;
static const float sqrt2 = 1.41421356237f;

static const float2 poissonDisk24[24] = {
	float2(  0.4677722f,  -0.6681575f	),
	float2(  0.05449407f, -0.8545988f	),
	float2(  0.3233855f,  -0.245416f	),
	float2( -0.00494891f, -0.2873039f	),
	float2(  0.6396217f,  -0.385691f	),
	float2( -0.4025818f,  -0.8795565f	),
	float2( -0.2375164f,  -0.5243279f	),
	float2( -0.4531612f,  -0.2970622f	),
	float2( -0.7475112f,  -0.4188458f	),
	float2( -0.1190095f,   0.1415305f	),
	float2( -0.4838489f,   0.2183606f	),
	float2(  0.260049f,    0.2809722f	),
	float2( -0.4276491f,   0.6337907f	),
	float2( -0.09241122f,  0.5543184f	),
	float2(  0.3728536f,   0.8436955f	),
	float2(  0.5847111f,   0.05194861f	),
	float2(  0.4641155f,   0.535556f	),
	float2(  0.7423707f,   0.3867652f	),
	float2(  0.9394412f,   0.1463991f	),
	float2(  0.9801205f,  -0.1776777f	),
	float2(  0.04088021f,  0.9351649f	),
	float2( -0.8808203f,   0.04750568f	),
	float2( -0.7648985f,   0.412887f	),
	float2( -0.2960058f,   0.9487777f	),
};


float2 UVofWPos(float3 wPos, matrix viewProjection)
{
	float3 ssPos = mul(float4(wPos, 1), viewProjection).xyz;
	ssPos.y = -ssPos.y;
	ssPos /= ssPos.z;
	ssPos +=1;
	ssPos /= 2;
	return ssPos.xy;
}

//Quaternion rotation
float3 rotate(float3 v, float4 q)
{
    float3 vq = q.xyz;
    float3 vqcrossv = cross(vq, v);
    float3 wvplusvqcrossv = q.w * v + vqcrossv;
    float3 vqcrosswvplusvqcrossv = cross(vq, wvplusvqcrossv);
    float3 res = q.w * wvplusvqcrossv + vqcrosswvplusvqcrossv;
    res = dot(vq, v) * vq + res;
    return res;
}

float luminance(float4 color)
{
	float maxC = max(color.r, max(color.g, color.b));
	float minC = min(color.r, min(color.g, color.b));
	return (maxC + minC) * 0.5f;
}
float luminance(float3 color)
{
	float maxC = max(color.r, max(color.g, color.b));
	float minC = min(color.r, min(color.g, color.b));
	return (maxC + minC) * 0.5f;
}

float value(float4 color)
{
	return max(color.r, max(color.g, color.b));
}

float value(float3 color)
{
	return max(color.r, max(color.g, color.b));
}

float  sq(float  a) {return a*a;}
float2 sq(float2 a) {return a*a;}
float3 sq(float3 a) {return a*a;}
float4 sq(float4 a) {return a*a;}
int    sq(int    a) {return a*a;}
int2   sq(int2   a) {return a*a;}
int3   sq(int3   a) {return a*a;}
int4   sq(int4   a) {return a*a;}
uint   sq(uint   a) {return a*a;}
uint2  sq(uint2  a) {return a*a;}
uint3  sq(uint3  a) {return a*a;}
uint4  sq(uint4  a) {return a*a;}