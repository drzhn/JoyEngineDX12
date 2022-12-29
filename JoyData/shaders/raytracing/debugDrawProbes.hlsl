#include "CommonEngineStructs.h"

ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b0);
ConstantBuffer<RaytracedProbesData> raytracedProbesData: register(b1);

Texture2D irradianceTexture;
SamplerState linearClampSampler;


struct PSInput
{
	float4 position : SV_POSITION;
	float4 worldNormal : COLOR0;
	uint instanceId : SV_InstanceID;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, uint instanceId : SV_InstanceID)
{
	PSInput result;

	const float SCALE = 0.4;

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
	result.worldNormal = mul(modelMatrix, float4(normal, 0));
	result.instanceId = instanceId;

	return result;
}

// Returns 1
float2 signNotZero(float2 v)
{
	return float2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

// Assume normalized input. Output is on [-1, 1] for each component.
float2 float32x3_to_oct(in float3 v)
{
	// Project the sphere onto the octahedron, and then onto the xy plane
	float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;

	const uint3 probeId3D = uint3(
		(input.instanceId % raytracedProbesData.gridX),
		(input.instanceId / raytracedProbesData.gridX) % raytracedProbesData.gridY,
		(input.instanceId / raytracedProbesData.gridX / raytracedProbesData.gridY % raytracedProbesData.gridZ)
	);

	const uint2 probeId2D = uint2(
		probeId3D.x + raytracedProbesData.gridX * probeId3D.y,
		probeId3D.z
	);

	float2 probeTextureSize = float2(raytracedProbesData.gridX * raytracedProbesData.gridY * (DDGI_PROBE_IRRADIANCE_RESOLUTION + 2),
	                                 raytracedProbesData.gridZ * (DDGI_PROBE_IRRADIANCE_RESOLUTION + 2));

	float2 probeUV = float32x3_to_oct(input.worldNormal) / 2.0 + float2(0.5, 0.5);

	float2 textureUV =
		probeId2D * (DDGI_PROBE_IRRADIANCE_RESOLUTION + 2) +
		float2(1, 1) +
		probeUV * DDGI_PROBE_IRRADIANCE_RESOLUTION;

	textureUV = float2(textureUV.x / probeTextureSize.x, textureUV.y / probeTextureSize.y);

	float4 irradiance = irradianceTexture.Sample(linearClampSampler, textureUV);

	//output.Color = float4(probeId3D, 1);

	output.Color = irradiance;
	return output;
}
