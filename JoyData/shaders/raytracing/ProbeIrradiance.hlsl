#include "CommonEngineStructs.h"

ConstantBuffer<RaytracedProbesData> raytracedProbesData;

Texture2D raytracingTexture;
RWTexture2D<float3> irradianceTexture;

float2 signNotZero(float2 v)
{
	return float2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

float3 oct_to_float32x3(float2 e)
{
	float3 v = float3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0)
	{
		v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
	}
	return normalize(v);
}

inline float madfrac(float A, float B)
{
	return ((A) * (B)-floor((A) * (B)));
}

inline float3 sphericalFibonacci(float i, float n)
{
	float PHI = sqrt(5) * 0.5 + 0.5;
	float phi = 2.0 * PI * madfrac(i, PHI - 1);
	float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
	float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));

	return normalize(float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta));
}

inline int2 GetPixelId(int2 realPixelId)
{
	int2 pixelId = int2(
		min(DDGI_PROBE_IRRADIANCE_RESOLUTION - 1, max(0, realPixelId.x - 1)),
		min(DDGI_PROBE_IRRADIANCE_RESOLUTION - 1, max(0, realPixelId.y - 1))
		);

	int2 t = pixelId;

	if (realPixelId.y == 0)
	{
		pixelId.x = DDGI_PROBE_IRRADIANCE_RESOLUTION - 1 - t.x;
		pixelId.y = (realPixelId.x == 0 || realPixelId.x == DDGI_PROBE_IRRADIANCE_RESOLUTION + 1) ? DDGI_PROBE_IRRADIANCE_RESOLUTION - 1 : 0;
	}

	if (realPixelId.y == DDGI_PROBE_IRRADIANCE_RESOLUTION + 1)
	{
		pixelId.x = DDGI_PROBE_IRRADIANCE_RESOLUTION - 1 - t.x;
		pixelId.y = realPixelId.x == 0 || realPixelId.x == DDGI_PROBE_IRRADIANCE_RESOLUTION + 1 ? 0 : DDGI_PROBE_IRRADIANCE_RESOLUTION - 1;
	}

	if (realPixelId.x == 0)
	{
		pixelId.y = DDGI_PROBE_IRRADIANCE_RESOLUTION - 1 - t.y;
		pixelId.x = realPixelId.y == 0 || realPixelId.y == DDGI_PROBE_IRRADIANCE_RESOLUTION + 1 ? DDGI_PROBE_IRRADIANCE_RESOLUTION - 1 : 0;
	}

	if (realPixelId.x == DDGI_PROBE_IRRADIANCE_RESOLUTION + 1)
	{
		pixelId.y = DDGI_PROBE_IRRADIANCE_RESOLUTION - 1 - t.y;
		pixelId.x = realPixelId.y == 0 || realPixelId.y == DDGI_PROBE_IRRADIANCE_RESOLUTION + 1 ? 0 : DDGI_PROBE_IRRADIANCE_RESOLUTION - 1;
	}

	return pixelId;
}

[numthreads(DDGI_PROBE_IRRADIANCE_RESOLUTION + 2, DDGI_PROBE_IRRADIANCE_RESOLUTION + 2, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
{
	const uint probeId1D =
		groupId.x * raytracedProbesData.gridY * raytracedProbesData.gridZ +
		groupId.y * raytracedProbesData.gridZ +
		groupId.z;

	const uint2 probeId2D = uint2(
		groupId.x + raytracedProbesData.gridX * groupId.y,
		groupId.z
		);

	const uint2 probeRealPixelID = groupThreadId.xy;

	const int2 probePixelID = GetPixelId(probeRealPixelID);

	const float2 uv = ((float2(probePixelID)+float2(0.5, 0.5)) / DDGI_PROBE_IRRADIANCE_RESOLUTION - float2(0.5, 0.5)) * 2;

	const float3 direction = oct_to_float32x3(uv);

	float4 irradiance = float4(0, 0, 0, 0);

	for (int i = 0; i < DDGI_RAYS_COUNT; i++)
	{
		const float3 rayDir = sphericalFibonacci(i, DDGI_RAYS_COUNT);
		const float3 rayRadiance = raytracingTexture.Load(int3(probeId1D, i, 0)).rgb;

		const float weight = max(0, dot(direction, rayDir));
		irradiance += float4(rayRadiance * weight, weight);
	}

	const float3 weightedIrradiance = float3(irradiance.rgb / irradiance.w);
	const float3 prevIrradiance = irradianceTexture[probeId2D * (DDGI_PROBE_IRRADIANCE_RESOLUTION + 2) + probeRealPixelID];
	const float3 newIrradiance = lerp(prevIrradiance, weightedIrradiance, 0.25);

	irradianceTexture[probeId2D * (DDGI_PROBE_IRRADIANCE_RESOLUTION + 2) + probeRealPixelID] = newIrradiance;
}
