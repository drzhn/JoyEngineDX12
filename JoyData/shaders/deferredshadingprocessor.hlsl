#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
};

ConstantBuffer<EngineData> engineData: register(b0);
ConstantBuffer<DirectionLightData> directionalLightData: register(b1);
ConstantBuffer<RaytracedProbesData> raytracedProbesData: register(b2);

Texture2D<float4> colorTexture;
Texture2D<float4> normalsTexture;
Texture2D<float4> positionTexture;


Texture2D<float> directionalLightShadowmap;

Texture2D<float3> probeIrradianceTexture;

SamplerState linearBlackBorderSampler;
SamplerComparisonState PCFSampler;


PSInput VSMain(uint id : SV_VertexID)
{
	const float2 uv = float2((id << 1) & 2, id & 2);

	PSInput result;
	result.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);

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

float2 GetProbeTextureUV(float3 gridID, float3 worldNormal)
{
	const float2 probeId2D = float2(
		gridID.x + raytracedProbesData.gridX * gridID.y,
		gridID.z
		);

	float2 probeTextureSize = float2(raytracedProbesData.gridX * raytracedProbesData.gridY * (DDGI_PROBE_DATA_RESOLUTION + 2),
		raytracedProbesData.gridZ * (DDGI_PROBE_DATA_RESOLUTION + 2));

	const float2 probeUV = (float32x3_to_oct(worldNormal) + float2(1, 1)) / 2.0;

	float2 textureUV =
		probeId2D * (DDGI_PROBE_DATA_RESOLUTION + 2) +
		float2(1, 1) +
		probeUV * DDGI_PROBE_DATA_RESOLUTION;

	return float2(textureUV.x / probeTextureSize.x, textureUV.y / probeTextureSize.y);
}


float3 SampleProbeGrid(float3 worldPosition, float3 worldNormal)
{
	float3 ret = float3(0, 0, 0);
	float3 gridPos = (worldPosition - raytracedProbesData.gridMin) / raytracedProbesData.cellSize;

	//gridPos = float3(
	//	clamp(gridPos.x, -1.0, float(raytracedProbesData.gridX) + 1.0),
	//	clamp(gridPos.y, -1.0, float(raytracedProbesData.gridY) + 1.0),
	//	clamp(gridPos.z, -1.0, float(raytracedProbesData.gridZ) + 1.0)
	//);

	const float3 gridCage[8] = {
		float3(ceil(gridPos.x), ceil(gridPos.y), ceil(gridPos.z)),
		float3(ceil(gridPos.x), ceil(gridPos.y), floor(gridPos.z)),
		float3(ceil(gridPos.x), floor(gridPos.y), ceil(gridPos.z)),
		float3(ceil(gridPos.x), floor(gridPos.y), floor(gridPos.z)),
		float3(floor(gridPos.x), ceil(gridPos.y), ceil(gridPos.z)),
		float3(floor(gridPos.x), ceil(gridPos.y), floor(gridPos.z)),
		float3(floor(gridPos.x), floor(gridPos.y), ceil(gridPos.z)),
		float3(floor(gridPos.x), floor(gridPos.y), floor(gridPos.z))
	};

	float wholeWeight = 0;

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		float3 probePos = gridCage[i];
		float3 probeColor = probeIrradianceTexture.Sample(linearBlackBorderSampler, GetProbeTextureUV(probePos, worldNormal)); 
		//float backProbeMultiplier = pow(max(0.0001, (dot(normalize(probePos - gridPos), worldNormal) + 1.0) * 0.5), 2) + 0.2;
		//float backProbeMultiplier = max(0.000, dot(normalize(probePos - gridPos), worldNormal));

		float backProbeMultiplier = pow(max(0.000, dot(normalize(probePos - gridPos), worldNormal)), 1.2);

		float weight =
			(1 - abs(probePos.x - gridPos.x)) *
			(1 - abs(probePos.y - gridPos.y)) *
			(1 - abs(probePos.z - gridPos.z));

		weight *= backProbeMultiplier;

		ret += probeColor * weight;

		wholeWeight += weight;
	}

	ret /= wholeWeight;

	return ret;
}

float4 PSMain(PSInput input) : SV_Target
{
	const float4 color = colorTexture.Load(float3(input.position.xy, 0));
	const float4 worldNormal = normalsTexture.Load(float3(input.position.xy, 0));
	const float4 worldPosition = positionTexture.Load(float3(input.position.xy, 0));

	const float4x4 resMatrix = mul(directionalLightData.proj, directionalLightData.view);
	float4 posShadowMap = mul(resMatrix, float4(worldPosition.rgb, 1.0));
	float3 UVD = posShadowMap.xyz / posShadowMap.w;
	UVD.xy = 0.5 * UVD.xy + 0.5;
	UVD.y = 1.0 - UVD.y;

	float bias = directionalLightData.bias; // max(0.05 * (dot(worldNormal, directionalLightData.direction)), 0.005);
	UVD.z -= bias;

	float shadowAttenuation = 0;
	const float2 texelSize = 1.0 / float2(2048, 2048); // TODO move to directionalLightData struct
	const int softShadowSize = 2;
	for (int x = -softShadowSize; x <= softShadowSize; ++x)
	{
		for (int y = -softShadowSize; y <= softShadowSize; ++y)
		{
			shadowAttenuation += directionalLightShadowmap.SampleCmpLevelZero(
				PCFSampler, UVD.xy + float2(x, y) * texelSize, UVD.z);
		}
	}

	shadowAttenuation /= (softShadowSize * 2 + 1) * (softShadowSize * 2 + 1);
	shadowAttenuation = max(directionalLightData.ambient, shadowAttenuation);

	float lambertAttenuation = max(0, dot(worldNormal.rgb, -directionalLightData.direction));

	// we use world position alpha chanel as an info about if this pixel is skybox or not.
	shadowAttenuation = worldPosition.a > 0 ? shadowAttenuation : 1;
	lambertAttenuation = worldPosition.a > 0 ? lambertAttenuation : 1;
	const float3 sampledProbeGridColor = worldPosition.a > 0 ? SampleProbeGrid(worldPosition, worldNormal) : 0;

	const float3 ret =
		color.rgb * shadowAttenuation * lambertAttenuation +
		color.rgb * sampledProbeGridColor;
	return float4(ret, 1);
}
