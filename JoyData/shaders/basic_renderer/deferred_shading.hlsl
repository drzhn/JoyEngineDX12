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
	shadowAttenuation = max(0.1, shadowAttenuation);

	float lambertAttenuation = max(1, dot(worldNormal.rgb, -directionalLightData.direction));

	// we use world position alpha chanel as an info about if this pixel is skybox or not.
	const float3 directionalLightAttenuation = worldPosition.a > 0 ? UnpackColor(directionalLightData.packedColor).rgb * shadowAttenuation : 1;
	lambertAttenuation = worldPosition.a > 0 ? lambertAttenuation : 1;

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
		color.rgb * lightColor;
	return float4(ret, 1);
}
