#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
};

ConstantBuffer<EngineData> engineData;

Texture2D<float4> colorTexture;
Texture2D<float4> normalsTexture;
Texture2D<float4> positionTexture;

ConstantBuffer<DirectionLightData> directionalLightData;
Texture2D<float> directionalLightShadowmap;

SamplerComparisonState PCFSampler;


PSInput VSMain(uint id : SV_VertexID)
{
	const float2 uv = float2((id << 1) & 2, id & 2);

	PSInput result;
	result.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);;

	return result;
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

	float directShadow = 0;
	const float2 texelSize = 1.0 / float2(2048, 2048);
	const int softShadowSize = 2;
	for (int x = -softShadowSize; x <= softShadowSize; ++x)
	{
		for (int y = -softShadowSize; y <= softShadowSize; ++y)
		{
			directShadow += directionalLightShadowmap.SampleCmpLevelZero(
				PCFSampler, UVD.xy + float2(x, y) * texelSize, UVD.z);
		}
	}

	directShadow /= (softShadowSize * 2 + 1) * (softShadowSize * 2 + 1);

	float attenuation = max(directionalLightData.ambient, min(directShadow, dot(worldNormal.rgb, -directionalLightData.direction)));

	// we use world position alpha chanel as an info about if this pixel is skybox or not.
	attenuation = worldPosition.a > 0 ? attenuation : 1;

	const float3 ret = color.rgb * attenuation;
	return float4(ret, 1);
}
