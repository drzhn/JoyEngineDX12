#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
};

ConstantBuffer<EngineData> engineData: register(b0);
ConstantBuffer<DirectionalLightInfo> directionalLightData: register(b1);
ConstantBuffer<RaytracedProbesData> raytracedProbesData: register(b2);
ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b3);

StructuredBuffer<ClusterEntry> clusteredEntryData: register(t0);
StructuredBuffer<UINT1> clusteredItemData : register(t1);
StructuredBuffer<LightInfo> lightData : register(t2);
StructuredBuffer<MAT4> objectMatricesData : register(t3);


Texture2D<float4> colorTexture;
Texture2D<float4> normalsTexture;
Texture2D<float4> positionTexture;


Texture2D<float> directionalLightShadowmap;

Texture2D<float3> probeIrradianceTexture;
Texture2D<float2> probeDepthTexture;

SamplerState linearBlackBorderSampler;
SamplerComparisonState PCFSampler;

#define DDGI_LINEAR_BLENDING

float4 UnpackColor(UINT1 packedColor)
{
	float4 color;

	color.r = ((packedColor >> 24) & 255) / 255.0f;
	color.g = ((packedColor >> 16) & 255) / 255.0f;
	color.b = ((packedColor >> 8) & 255) / 255.0f;
	color.a = ((packedColor >> 0) & 255) / 255.0f;

	return color;
}

inline float sqr(float x)
{
	return x * x;
}

float PointLightAttenuation(float distance, float radius, float max_intensity, float falloff)
{
	const float s = distance / radius;
	const float s2 = sqr(s);
	const float attenuation = max_intensity * sqr(1 - s2) / (1 + falloff * s);
	return s >= 1.0 ? 0 : attenuation;
}

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

	float totalWeight = 0;

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		const float3 probeCoord = gridCage[i];
		const float3 probeWorldPos = raytracedProbesData.gridMin + probeCoord * raytracedProbesData.cellSize;
		const float distanceToProbe = length(worldPosition - probeWorldPos);
		const float2 textureUV = GetProbeTextureUV(probeCoord, worldNormal);
		const float3 probeColor = probeIrradianceTexture.Sample(linearBlackBorderSampler, textureUV);

		const float backProbeMultiplier = pow(max(0.000, dot(normalize(probeCoord - gridPos), worldNormal)), 1.2);

		const float2 temp = probeDepthTexture.Sample(linearBlackBorderSampler, textureUV);
		const float mean = temp.x;
		const float variance = abs(sqr(temp.x) - temp.y);

		// http://www.punkuser.net/vsm/vsm_paper.pdf; equation 5
		// Need the max in the denominator because biasing can cause a negative displacement
		float chebyshev_weight = variance / (variance + sqr(max(distanceToProbe - mean, 0.0)));

		// Increase contrast in the weight 
		chebyshev_weight = max(pow(chebyshev_weight, 3), 0.0);

		chebyshev_weight = (distanceToProbe <= mean) ? 1.0 : chebyshev_weight;

		float weight =
			(1 - abs(probeCoord.x - gridPos.x)) *
			(1 - abs(probeCoord.y - gridPos.y)) *
			(1 - abs(probeCoord.z - gridPos.z));

		weight *= backProbeMultiplier * chebyshev_weight;

		// Avoid zero weight
		weight = max(0.000001, weight);

		ret += probeColor * weight;

		totalWeight += weight;
	}

	ret /= totalWeight;

	return ret;
}

float4 PSMain(PSInput input) : SV_Target
{
	const float4 color = colorTexture.Load(float3(input.position.xy, 0));
	const float4 worldNormal = normalsTexture.Load(float3(input.position.xy, 0));
	const float4 worldPosition = positionTexture.Load(float3(input.position.xy, 0));

	const float4x4 resMatrix = mul(directionalLightData.proj, directionalLightData.view);
	float4 posShadowMap = mul(resMatrix, float4(worldPosition.xyz, 1.0));
	float3 UVD = posShadowMap.xyz / posShadowMap.w;
	UVD.xy = 0.5 * UVD.xy + 0.5;
	UVD.y = 1.0 - UVD.y;

	float bias = directionalLightData.bias; // max(0.05 * (dot(worldNormal, directionalLightData.direction)), 0.005);
	UVD.z -= bias;

	float shadowAttenuation = 0;
	const float2 texelSize = 1.0 / float2(directionalLightData.shadowmapSize, directionalLightData.shadowmapSize);
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
	const float3 directionalLightAttenuation = worldPosition.a > 0 ? UnpackColor(directionalLightData.packedColor).rgb * shadowAttenuation : 1;
	lambertAttenuation = worldPosition.a > 0 ? lambertAttenuation : 1;
	const float3 sampledProbeGridColor = worldPosition.a > 0 ? SampleProbeGrid(worldPosition, worldNormal) : 0;

	float3 lightColor = float3(0, 0, 0);
	{
		const float aspect = engineData.cameraAspect;
		const float distance = engineData.cameraFar - engineData.cameraNear;
		const float logDistance = log2(distance + 1);

		float4 viewPos = mul(viewProjectionData.view, float4(worldPosition.xyz, 1.0));
		uint clusterZ = floor(log2(viewPos.z + 1 - engineData.cameraNear) / logDistance * NUM_CLUSTERS_Z);

		const float nearH = 2 * viewPos.z * tan(engineData.cameraFovRadians / 2.f);
		const float nearW = aspect * nearH;

		uint clusterX = floor((viewPos.x + nearW / 2) / nearW * NUM_CLUSTERS_X);
		uint clusterY = floor((viewPos.y + nearH / 2) / nearH * NUM_CLUSTERS_Y);

		ClusterEntry entry = clusteredEntryData[clusterY + clusterX * NUM_CLUSTERS_Y + clusterZ * NUM_CLUSTERS_Y * NUM_CLUSTERS_X];
		for (int i = 0; i < entry.numLight; i++)
		{
			LightInfo info = lightData[clusteredItemData[entry.offset + i]];
			float4 lightViewPos = mul(viewProjectionData.view, mul(objectMatricesData[info.transformIndex], float4(0, 0, 0, 1)));
			const float d = length(viewPos.xyz - lightViewPos.xyz);
			const float pointLightAttenuation = PointLightAttenuation(d, info.radius, info.intensity, 4);
			lightColor += UnpackColor(info.packedColor).rgb * pointLightAttenuation;
		}
	}

	const float3 ret =
		color.rgb * directionalLightAttenuation * lambertAttenuation +
		color.rgb * sampledProbeGridColor * raytracedProbesData.useDDGI +
		color.rgb * lightColor;
	return float4(ret, 1);
}
