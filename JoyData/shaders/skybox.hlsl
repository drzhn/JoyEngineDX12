#include "CommonEngineStructs.h"

Texture2D skyboxTexture;
Texture2D gBufferColorTexture;

SamplerState textureSampler : register(s0);

ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b1);

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;

	float4x4 invView = viewProjectionData.view;
	invView[0][3] = 0;
	invView[1][3] = 0;
	invView[2][3] = 0;

	position = mul(invView, position);

	result.position = mul(viewProjectionData.proj, float4(position, 1));
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;

	const float4 skyboxColor = skyboxTexture.Sample(textureSampler, input.uv);
	const float4 gBufferColor = gBufferColorTexture.Load(float3(input.position.xy, 0));
	output.Color = float4(skyboxColor.rgb, 1-gBufferColor.a);

	return output;
}
