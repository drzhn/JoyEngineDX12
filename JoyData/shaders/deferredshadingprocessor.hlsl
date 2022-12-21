#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
	float4 clipPos : TEXCOORD1;
};

ConstantBuffer<EngineData> engineData;

Texture2D<float4> colorTexture;
Texture2D<float4> normalsTexture;
Texture2D<float4> positionTexture;

ConstantBuffer<DirectionLightData> directionalLightData;
Texture2D<float> directionalLightShadowmap;

SamplerComparisonState PCFSampler;


inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

PSInput VSMain(uint id : SV_VertexID)
{
	const float2 uv = float2((id << 1) & 2, id & 2);

	PSInput result;
	result.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);;
	result.clipPos = ComputeNonStereoScreenPos(result.position);

	return result;
}

float4 ComputeClipSpacePosition(float2 positionNDC, float deviceDepth)
{
	float4 positionCS = float4(positionNDC * 2.0 - 1.0, deviceDepth, 1.0);
	positionCS.y = -positionCS.y;

	return positionCS;
}

float3 WorldPosFromDepth(float zbuffer_depth, float2 texcoord)
{
	float4 positionCS = ComputeClipSpacePosition(texcoord, zbuffer_depth);
	float4 hpositionWS = mul(engineData.cameraInvView, mul(engineData.cameraInvProj, positionCS));
	return hpositionWS.xyz / hpositionWS.w;
}


float4 PSMain(PSInput input) : SV_Target
{
	const float2 screenPosition = (input.clipPos.xy / input.clipPos.w);

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

	attenuation = worldPosition.a > 0 ? attenuation : 1;

	const float3 ret = color.rgb * attenuation;
	return float4(ret, color.a);
}
