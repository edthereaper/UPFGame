#include "deferred.fx"

#include "random.fx"

Texture2D txWhiteNoise 					: register(t64);
Texture2D txNormalNoise 				: register(t65);

#define RANDOM_SAMPLE 1
#define RANDOM_HASH	  2
#define PLAIN_PCF	  3
#define NO_SMOOTH	  4
#define SMOOTH_METHOD_PT  NO_SMOOTH
#define SMOOTH_METHOD_DIR NO_SMOOTH

#define BLINNPHONG 1
#define PHONG 2
#define TESTINPUT 3
#define SPECULAR BLINNPHONG

#define BOBJENKINS 1
#define FRAC_SIN_DOT 2
#define HASH_METHOD BOBJENKINS


float getSpecular(float3 wPos, float3 L, float3 N, int3 pixel, float amountMod, float powerFactor)
{
	float specAmount = txNormal.Load(pixel).a + amountMod;
	specAmount = sqrt(clamp(specAmount,0,1000000));
	float power = PowerSpecular * powerFactor;
	#if SPECULAR == PHONG
		float3 E = normalize(CameraWorldPos.xyz - wPos);
		float3 R = reflect( -normalize(L), N );
		float specular = pow( saturate( dot( R, E ) ), power ) * specAmount;
	#elif SPECULAR == BLINNPHONG
		float3 E = normalize(CameraWorldPos.xyz - wPos);
		float3 H = normalize( normalize(L) + E );
		float specular = pow( saturate( dot( N, H ) ), power ) * specAmount;
	#elif SPECULAR == TESTINPUT
		float specular = specAmount;
	#else
		#error "No specular model selected!"
	#endif
	
	return clamp(specular, 0, 100000);
}

// Clip to a Light Fustrum
void clipLightFustrum(float4 pPos)
{
	pPos.xyz /= pPos.w;
	clip(pPos.xy);
	clip(1-pPos.xy);
	clip(pPos.z - 1e-3);
	clip(1 - pPos.z);
}

//////////////// DIRECTIONAL LIGHTS ////////////////

// Sample a 2D shadowmap
float tapAt(float2 texel, float depth)
{
	return txShadowMap.SampleCmpLevelZero(samPCFShadows, texel, depth);
}

// Generating shadows
float getShadowAt(float4 pPos, float2 uv, float resolution, float focusF, float jittering)
{
	float z = pPos.z;
	pPos.xyz /= pPos.w;
	
	float2 center = pPos.xy;
	float depth = pPos.z - 1e-3; // anti-acne
	
	float zF = z/LightZFar;
	float texel = 1/resolution;
	//penumbra size: zF=0 -> texel. zF=1 -> focusF*texel
	float amp = 2*((focusF - 1)*texel) * zF * zF + texel;
	
#if SMOOTH_METHOD_DIR == RANDOM_SAMPLE
	//Apply the jittering factor to all sampling.
	//If jittering=0, it'll sample the same texel every time => less cache misses
	float4 j0 = (1-jittering) + jittering * txWhiteNoise.Sample(samWrapLinear, normalize(center));
	float4 j1 = (1-jittering) + jittering * txWhiteNoise.Sample(samWrapLinear, j0.xy);
	float4 ja = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.xy); // ~N(1, td)
	float4 jb = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.zw); // ~N(1, td)
	float4 jc = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.wx); // ~N(1, td)
	float4 jd = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.yz); // ~N(1, td)
	float4 jai = (1-ja);// ~N(0, td)
	float4 jbi = (1-jb);// ~N(0, td)
	float4 jci = (1-jc);// ~N(0, td)
	float4 jdi = (1-jd);// ~N(0, td)
	
	//3x3 PCF
	float amount = 0;//tapAt(center, depth);
	amount += tapAt(center + amp * ja.xy * (jdi.yx + float2(+1,  0)) , depth);
	amount += tapAt(center + amp * ja.zw * (jdi.wz + float2( 0, +1)) , depth);
	amount += tapAt(center + amp * jb.xy * (jci.yx + float2(-1,  0)) , depth);
	amount += tapAt(center + amp * jb.zw * (jci.wz + float2( 0, -1)) , depth);
	amount += tapAt(center + amp * jc.xy * (jbi.yx + float2(+1, +1)) , depth);
	amount += tapAt(center + amp * jc.zw * (jbi.wz + float2(+1, -1)) , depth);
	amount += tapAt(center + amp * jd.xy * (jai.yx + float2(-1, +1)) , depth);
	amount += tapAt(center + amp * jd.zw * (jai.wz + float2(-1, -1)) , depth);
	amount /= 8;
	return amount;
#elif SMOOTH_METHOD_DIR == RANDOM_HASH
	
#if HASH_METHOD == FRAC_SIN_DOT
	float r = (1-jittering) + jittering * 2 * frac(sin(dot(uv, float2(12.98, 78.23))) * 43758.54535);
	float ri = 1 - r;
	
	float amount = 0;//tapAt(center, depth);
	amount += tapAt(center + amp * r * (ri + float2(+1,  0)) , depth);
	amount += tapAt(center + amp * r * (ri + float2( 0, +1)) , depth);
	amount += tapAt(center + amp * r * (ri + float2(-1,  0)) , depth);
	amount += tapAt(center + amp * r * (ri + float2( 0, -1)) , depth);
	amount += tapAt(center + amp * r * (ri + float2(+1, +1)) , depth);
	amount += tapAt(center + amp * r * (ri + float2(+1, -1)) , depth);
	amount += tapAt(center + amp * r * (ri + float2(-1, +1)) , depth);
	amount += tapAt(center + amp * r * (ri + float2(-1, -1)) , depth);
	return amount / 8;
#elif HASH_METHOD == BOBJENKINS
	const uint2 h = hash(uv).xy;
	const float4 r = (1-jittering) + jittering * 2 * (float4(extractUnorm_16(h.x), extractUnorm_16(h.y)));
	const float4 ri = 1 - r;
	
	//3x3 PCF
	float amount = 0; //tapAt(center, depth);
	amount += tapAt(center + amp * r.xy * ( ri.wz + float2(+1,  0)) , depth);
	amount += tapAt(center + amp * r.yz * ( ri.wx + float2( 0, +1)) , depth);
	amount += tapAt(center + amp * r.zx * ( ri.yw + float2(-1,  0)) , depth);
	amount += tapAt(center + amp * r.wz * ( ri.yx + float2( 0, -1)) , depth);
	amount += tapAt(center + amp * r.yw * ( ri.xz + float2(+1, +1)) , depth);
	amount += tapAt(center + amp * r.xw * ( ri.zy + float2(+1, -1)) , depth);
	amount += tapAt(center + amp * r.yx * ( ri.zw + float2(-1, +1)) , depth);
	amount += tapAt(center + amp * r.wy * ( ri.zx + float2(-1, -1)) , depth);
	return amount / 8;
#endif // HASH_METHOD

#elif SMOOTH_METHOD_DIR == PLAIN_PCF
	float amount = tapAt(center, depth);
	amount += tapAt(center + amp * float2(+1,  0) , depth);
	amount += tapAt(center + amp * float2( 0, +1) , depth);
	amount += tapAt(center + amp * float2(-1,  0) , depth);
	amount += tapAt(center + amp * float2( 0, -1) , depth);
	amount += tapAt(center + amp * float2(+1, +1) , depth);
	amount += tapAt(center + amp * float2(+1, -1) , depth);
	amount += tapAt(center + amp * float2(-1, +1) , depth);
	amount += tapAt(center + amp * float2(-1, -1) , depth);
	return amount / 9;
#else
	return tapAt(center, depth);
#endif //SMOOTH_METHOD_DIR 
}

struct DIRLIGHT_DATA {
	float4 pPos;
	float3 wPos;
	float3 pixel;
	float diffuseAmount;
	float attenuation;
	float specular;
};

DIRLIGHT_DATA calculateDirLight(float4 pos)
{
	DIRLIGHT_DATA ret = (DIRLIGHT_DATA)0;
	
	ret.pixel = uint3(pos.xy, 0);
	
	ret.wPos = txSpace.Load(ret.pixel).xyz;
	// Move to homogeneous space of the light
	ret.pPos  = mul(float4(ret.wPos, 1), LightViewProjectionOffset);
	clipLightFustrum(ret.pPos); //Early clip => better? 
	
	float3 N = txNormal.Load(ret.pixel).xyz * 2 - 1.;
	
	// Basic diffuse lighting
	float3 diff = ret.wPos - DirLightWorldPos.xyz;
	float  distanceToLight = length(diff);
	float3 L = -diff / distanceToLight;
	ret.diffuseAmount = saturate(dot(N, L));
	
	// Attenuation based on distance to light origin
	float attenuation = saturate((DirLightMaxRadius - distanceToLight) * DirLightInvDeltaRadius);
	
	// Attenuation based on distance to projection center
	float zProj = dot(diff, DirLightFront.xyz);
	clip(zProj);
	float3 center = DirLightWorldPos.xyz + zProj*DirLightFront.xyz;
	float distanceToCenter = length(ret.wPos - center);
	
	float proportion = DirLightZDiff==0 ? 0 : (zProj - DirLightZNear) / DirLightZDiff;
	float rProj = DirLightSpotRadiusNear + proportion * DirLightSpotRadiusDiff;
	float invDeltaSpotRadius = 1.0f / (rProj - rProj * DirLightSpotDecay);
	
	float spotlight = DirLightIsSpotlight ? saturate((rProj - distanceToCenter) * invDeltaSpotRadius):1;
	ret.attenuation = min(spotlight, attenuation);
	
	ret.specular = getSpecular(ret.wPos, L, N, ret.pixel,
		DirLightSpecularAmountModifier, DirLightSpecularPowerFactor);
	
	return ret;
}

float4 getDirLight(DIRLIGHT_DATA data)
{
	return float4(DirLightColor.rgb * data.diffuseAmount, data.specular) * data.attenuation * DirLightColor.a;
}

float4 PSDirLight(float4 pos : SV_Position) :SV_Target0
{
	return getDirLight(calculateDirLight(pos));
}

float4 PSDirLightShadow(float4 pos : SV_Position) :SV_Target0
{
	DIRLIGHT_DATA data = calculateDirLight(pos);
	float2 texel = data.pixel.xy*0.5 / float2(CameraHalfXRes, CameraHalfYRes);
	float shadow = txShadowMap.Sample(samClampLinear, texel).x;
	data.attenuation *= shadow;
	return getDirLight(data);
}

float PSShadowBuffer(float4 pos : SV_Position) :SV_Target0
{
	int3 pixel = uint3(pos.xy, 0);
	float3 wPos = txSpace.Load(pixel).xyz;
	float4 pPos  = mul(float4(wPos, 1), LightViewProjectionOffset);
	clipLightFustrum(pPos); //Early clip => better? 
	return saturate(getShadowAt(pPos, pos.xy, DirLightShadowResolution, DirLightShadowFocus, DirLightShadowJittering)) + DirLightShadowIntensity;
}

//////////////// DISTANCE TO LIGHT ////////////////

struct VS_WORLD_OUTPUT
{
	float4 oPos : SV_POSITION;
	float4 wPos : TEXCOORD0;
};

// Get World regular vertex
VS_WORLD_OUTPUT VSWorld(float4 iPos : POSITION)
{
    VS_WORLD_OUTPUT output = (VS_WORLD_OUTPUT)0;
	output.wPos = mul(iPos, World);
	output.oPos = mul(output.wPos, ViewProjection);
	return output;
}
// Get World Instanced vertex
VS_WORLD_OUTPUT VSWorldInstanced(float4 iPos : POSITION, matrix InstanceWorld : ITRANSFORM)
{
    VS_WORLD_OUTPUT output = (VS_WORLD_OUTPUT)0;
	output.wPos = mul(iPos, InstanceWorld);
	output.oPos = mul(output.wPos, ViewProjection);
	output.oPos.x = output.oPos.x;
	return output;
}
// Get World skinned vertex
VS_WORLD_OUTPUT VSWorldSkinned(
	float4 Pos : POSITION, 
	uint4  bone_ids : BONEIDS,
	float4 weights  : WEIGHTS )
{
	matrix skin_mtx = skinMatrix(bone_ids, weights);
    VS_WORLD_OUTPUT output = (VS_WORLD_OUTPUT)0;
	output.wPos = mul(Pos, skin_mtx);
	output.oPos = mul(output.wPos, ViewProjection);
	return output;
}
float PSDistanceToLight(in VS_WORLD_OUTPUT input) : SV_Target0
{
	float3 d = LightWorldPos.xyz - input.wPos.xyz;
	return length(d)/LightZFar;
}

//////////////// POINT LIGHTS ////////////////

float tapCubeAt(float3 dir, float depth)
{
	//Theoretically, but has some error that causes acne:
	//	return txShadowMap.SampleCmpLevelZero(samPCFShadows, dir, depth); 
	//So we amplify and eliminate this error like this:
	float sample = txShadowCube.Sample(samBorderLinear, dir).x;
    
	float amplifiedError = (depth-sample)*100;
    return (1-amplifiedError)<=(1e-3f) ?0:1;
}

//Generating cube shadows
float getCubeShadowAt(float3 wPos, float2 uv, float resolution, float blurF, float jittering)
{
	float3 d = LightWorldPos - wPos;
	float3 absd  = abs(d);
	bool xFace = (absd.x > absd.y && absd.x > absd.z);
	bool yFace = (absd.z > absd.x && absd.z > absd.y);
	
	d.xy = -d.xy;
	float z= length(d)/LightZFar;
	z -= 1e-3; // anti-acne
	
	float texel = 20/resolution;
	//penumbra size: z=0 -> texel. z=1 -> blurF*texel
	float amp = ((blurF - 1)*texel) * z * z + texel;
	
	//PCF sampling offsets
	float k = 0;
	float3 a = xFace ? float3(0,1,0) : yFace ? float3(1,0,0) : float3(1,0,0);
	float3 b = xFace ? float3(0,0,1) : yFace ? float3(0,1,0) : float3(0,0,1);
	float2 rsampleUV = xFace ? float2(d.y,d.z) : yFace ? float2(d.x,d.y) : float2(d.x,d.z);

#if SMOOTH_METHOD_PT == RANDOM_SAMPLE
	//Apply the jittering factor to all sampling.
	//If jittering=0, it'll sample the same texel every time => less cache misses
	float4 j0 = (1-jittering) + jittering * txWhiteNoise.Sample(samWrapLinear, normalize(rsampleUV));
	float4 j1 = (1-jittering) + jittering * txWhiteNoise.Sample(samWrapLinear, j0.xy);
	float4 ja = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.xy); // ~N(1, td)
	float4 jb = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.zw); // ~N(1, td)
	float4 jc = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.wx); // ~N(1, td)
	float4 jd = (1-jittering) + jittering * 2 * txNormalNoise.Sample(samWrapLinear, j1.yz); // ~N(1, td)
	float4 jai = (1-ja);// ~N(0, td)
	float4 jbi = (1-jb);// ~N(0, td)
	float4 jci = (1-jc);// ~N(0, td)
	float4 jdi = (1-jd);// ~N(0, td
	
	//3x3 PCF
	float amount = 0;//tapCubeAt(d, z);
	amount += tapCubeAt(d + amp * ja.xyz * ( jdi.xyz + (+a+b)) , z);
	amount += tapCubeAt(d + amp * ja.zwx * ( jdi.zwx + (-a+b)) , z);
	amount += tapCubeAt(d + amp * jb.xyz * ( jci.xyz + (+a-b)) , z);
	amount += tapCubeAt(d + amp * jb.zwx * ( jci.zwx + (-a-b)) , z);
	amount += tapCubeAt(d + amp * jc.xyz * ( jbi.xyz + (+a  )) , z);
	amount += tapCubeAt(d + amp * jc.zwx * ( jbi.zwx + (  +b)) , z);
	amount += tapCubeAt(d + amp * jd.xyz * ( jai.xyz + (-a  )) , z);
	amount += tapCubeAt(d + amp * jd.zwx * ( jai.zwx + (  -b)) , z);
	return amount / 8;
#elif SMOOTH_METHOD_PT == RANDOM_HASH
	
#if HASH_METHOD == FRAC_SIN_DOT
	float r = (1-jittering) + jittering * 2 * frac(sin(dot(uv, float2(12.98, 78.23))) * 43758.54535);
	float ri = 1 - r;
	
	//3x3 PCF
	float amount = 0;//tapCubeAt(d, z);
	amount += tapCubeAt(d + amp * r * ( ri + (+a+b)) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (-a+b)) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (+a-b)) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (-a-b)) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (+a  )) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (  +b)) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (-a  )) , z);
	amount += tapCubeAt(d + amp * r * ( ri + (  -b)) , z);
	return amount / 8;
#elif HASH_METHOD == BOBJENKINS
	const uint2 h = hash(uv).xy;
	const float4 r = (1-jittering) + jittering * 2 * (float4(extractUnorm_16(h.x), extractUnorm_16(h.y)));
	const float4 ri = 1 - r;
	
	//3x3 PCF
	float amount = 0;//tapCubeAt(d, z);
	amount += tapCubeAt(d + amp * r.xyz * ( ri.ywz + (+a+b)) , z);
	amount += tapCubeAt(d + amp * r.yzx * ( ri.xwy + (-a+b)) , z);
	amount += tapCubeAt(d + amp * r.zxy * ( ri.yxw + (+a-b)) , z);
	amount += tapCubeAt(d + amp * r.wzy * ( ri.wyx + (-a-b)) , z);
	amount += tapCubeAt(d + amp * r.ywz * ( ri.xyz + (+a  )) , z);
	amount += tapCubeAt(d + amp * r.xwy * ( ri.yzx + (  +b)) , z);
	amount += tapCubeAt(d + amp * r.yxw * ( ri.zxy + (-a  )) , z);
	amount += tapCubeAt(d + amp * r.wyx * ( ri.wzy + (  -b)) , z);
	return amount / 8;

#endif //HASH_METHOD

#elif SMOOTH_METHOD_PT == PLAIN_PCF
	float amount = tapCubeAt(d, z);
	amount += tapCubeAt(d + amp * ( +a+b) , z);
	amount += tapCubeAt(d + amp * ( -a+b) , z);
	amount += tapCubeAt(d + amp * ( +a-b) , z);
	amount += tapCubeAt(d + amp * ( -a-b) , z);
	amount += tapCubeAt(d + amp * ( +a  ) , z);
	amount += tapCubeAt(d + amp * (   +b) , z);
	amount += tapCubeAt(d + amp * ( -a  ) , z);
	amount += tapCubeAt(d + amp * (   -b) , z);
	return amount / 9;
#else
	return tapCubeAt(d, z);
#endif //SMOOTH_METHOD_PT
}


struct PTLIGHT_DATA {
	float3 wPos;
	float3 pixel;
	float attenuation;
	float diffuseAmount;
	float specular;
};

PTLIGHT_DATA calculatePtLight(float4 pos)
{
	PTLIGHT_DATA ret = (PTLIGHT_DATA)0;

	ret.pixel = uint3(pos.xy, 0);
	float3 N = txNormal.Load(ret.pixel).xyz * 2 - 1.;
	
	ret.wPos = txSpace.Load(ret.pixel).xyz;
	
	// Basic diffuse lighting
	float3 L = PtLightWorldPos.xyz - ret.wPos;
	float distanceToLight = length(L);
	L = L / distanceToLight;
	ret.diffuseAmount = saturate(dot(N, L));
	
	ret.specular = getSpecular(ret.wPos, L, N, ret.pixel,
		PtLightSpecularAmountModifier, PtLightSpecularPowerFactor);
	
	// Attenuation based on distance:   1 - [( r - rmin ) / ( rmax - rmin )]
	ret.attenuation = saturate((PtLightMaxRadius - distanceToLight) * PtLightInvDeltaRadius);
	
	return ret;
}


float4 getPtLight(PTLIGHT_DATA data)
{
	return float4(PtLightColor.rgb * data.diffuseAmount, data.specular) * data.attenuation * PtLightColor.a;
}

float4 PSPointLight(float4 pos : SV_Position) :SV_Target0
{
	return getPtLight(calculatePtLight(pos));
}

float4 PSPointLightShadow(float4 pos : SV_Position) :SV_Target0
{
	PTLIGHT_DATA data = calculatePtLight(pos);
	float2 texel = data.pixel.xy*0.5 / float2(CameraHalfXRes, CameraHalfYRes);
	float shadow = txShadowMap.Sample(samClampLinear, texel).x;
	data.attenuation *= shadow;
	return getPtLight(data);
}

float PSPtShadowBuffer(float4 pos : SV_Position) :SV_Target0
{
	int3 pixel = uint3(pos.xy, 0);
	float3 wPos = txSpace.Load(pixel).xyz;
	return saturate(getCubeShadowAt(wPos, pos.xy, PtLightShadowResolution, PtLightShadowBlur, PtLightShadowJittering) + PtLightShadowIntensity);
}