#include "CommonEngineStructs.h"

Texture2D diffuse : register(t0);
Texture2D normal : register(t1);
SamplerState textureSampler : register(s0);
ConstantBuffer<ModelMatrixData> modelData : register(b0);
ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b1);


struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 clipPos : TEXCOORD1;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal: NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(viewProjectionData.proj, mul(viewProjectionData.view, modelData.model));
	result.position = mul(resMatrix, float4(position, 1));
	result.clipPos = ComputeNonStereoScreenPos(result.position);
	//result.clipPos.xy /= result.clipPos.w;
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;
	const float2 screenPosition = (input.clipPos.xy / input.clipPos.w);
	const float4 diffuseColor = diffuse.Sample(textureSampler, input.uv);
	const float4 normalColor = normal.Sample(textureSampler, input.uv);

	output.Color = diffuseColor;// *normalColor;

	return output;
}
