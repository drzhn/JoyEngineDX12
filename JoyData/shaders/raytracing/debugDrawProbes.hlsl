#include "CommonEngineStructs.h"

ConstantBuffer<ViewProjectionMatrixData> viewProjectionData;
ConstantBuffer<RaytracedProbesData> raytracedProbesData;

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, uint instanceId : SV_InstanceID)
{
	PSInput result;

	const float SCALE = 0.3;

	const float3 worldPosition = raytracedProbesData.gridMin + float3(
		(instanceId % raytracedProbesData.gridX),
		(instanceId / raytracedProbesData.gridX) % raytracedProbesData.gridY,
		(instanceId / raytracedProbesData.gridX / raytracedProbesData.gridY % raytracedProbesData.gridZ)
	) * raytracedProbesData.cellSize;

	const float4x4 modelMatrix = {
		SCALE, 0, 0, worldPosition.x,
		0, SCALE, 0, worldPosition.y,
		0, 0, SCALE, worldPosition.z,
		0, 0, 0, 1
	};
	const float4x4 resMatrix = mul(viewProjectionData.proj, mul(viewProjectionData.view, modelMatrix));
	result.position = mul(resMatrix, float4(position, 1));
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;

	output.Color = float4(1, 0, 0, 1);

	return output;
}
