#include "CommonEngineStructs.h"

ConstantBuffer<MVP> mvp : register(b0);

StructuredBuffer<AABB> BVHData; // size = THREADS_PER_BLOCK * BLOCK_SIZE - 1

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
	PSInput input;

	AABB aabb = BVHData[instanceId];

	float4 pos[24] = {
		float4(aabb.min.x, aabb.min.y, aabb.min.z, 1),
		float4(aabb.min.x, aabb.min.y, aabb.max.z, 1),
		float4(aabb.min.x, aabb.min.y, aabb.min.z, 1),
		float4(aabb.min.x, aabb.max.y, aabb.min.z, 1),
		float4(aabb.min.x, aabb.min.y, aabb.min.z, 1),
		float4(aabb.max.x, aabb.min.y, aabb.min.z, 1),

		float4(aabb.max.x, aabb.max.y, aabb.max.z, 1),
		float4(aabb.max.x, aabb.max.y, aabb.min.z, 1),
		float4(aabb.max.x, aabb.max.y, aabb.max.z, 1),
		float4(aabb.max.x, aabb.min.y, aabb.max.z, 1),
		float4(aabb.max.x, aabb.max.y, aabb.max.z, 1),
		float4(aabb.min.x, aabb.max.y, aabb.max.z, 1),

		float4(aabb.max.x, aabb.min.y, aabb.min.z, 1),
		float4(aabb.max.x, aabb.max.y, aabb.min.z, 1),
		float4(aabb.max.x, aabb.min.y, aabb.min.z, 1),
		float4(aabb.max.x, aabb.min.y, aabb.max.z, 1),

		float4(aabb.min.x, aabb.max.y, aabb.min.z, 1),
		float4(aabb.max.x, aabb.max.y, aabb.min.z, 1),
		float4(aabb.min.x, aabb.max.y, aabb.min.z, 1),
		float4(aabb.min.x, aabb.max.y, aabb.max.z, 1),

		float4(aabb.min.x, aabb.min.y, aabb.max.z, 1),
		float4(aabb.min.x, aabb.max.y, aabb.max.z, 1),
		float4(aabb.min.x, aabb.min.y, aabb.max.z, 1),
		float4(aabb.max.x, aabb.min.y, aabb.max.z, 1),
	};

	float4 position = pos[id];
	float4x4 resMatrix = mul(mvp.proj, mul(mvp.view, mvp.model));

	input.position = mul(resMatrix, position);
	input.color = float4(1, 0, 0, 1);
	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	return input.color;
}
