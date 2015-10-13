#include "platform/shader_ctes.h"

matrix getInfluence(uint boneId)
{
	int off = 4*boneId;
	return matrix(
		boneBuffer[FirstBone + off + 0],
		boneBuffer[FirstBone + off + 1],
		boneBuffer[FirstBone + off + 2],
		boneBuffer[FirstBone + off + 3]
		);
}


matrix skinMatrix(uint4 bone_ids, float4 weights)
{
	return
		getInfluence(bone_ids.x) * weights.x +
		getInfluence(bone_ids.y) * weights.y +
		getInfluence(bone_ids.z) * weights.z +
		getInfluence(bone_ids.w) * weights.w;
}

// Project regular vertex
void VSProject(float4 iPos : POSITION, out float4 oPos : SV_POSITION)
{
	float4 world_pos = mul(iPos, World);
	oPos = mul(world_pos, ViewProjection);
}

// Project regular vertex
void VSProjectInstanced(float4 iPos : POSITION, out float4 oPos : SV_POSITION, matrix InstanceWorld : ITRANSFORM)
{
	float4 world_pos = mul(iPos, InstanceWorld);
	oPos = mul(world_pos, ViewProjection);
}

// Project skinned vertex
void VSProjectSkinned(
	float4 Pos : POSITION, 
	uint4  bone_ids : BONEIDS,
	float4 weights  : WEIGHTS,
	out float4 oPos : SV_POSITION)
{
	matrix skin_mtx = skinMatrix(bone_ids, weights);
	float4 skinned_pos = mul(Pos, skin_mtx);
	oPos = mul(skinned_pos, ViewProjection);
}