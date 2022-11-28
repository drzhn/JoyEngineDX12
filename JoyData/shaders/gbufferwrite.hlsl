#include "CommonEngineStructs.h"

Texture2D diffuse : register(t0);
Texture2D normal : register(t1);
SamplerState textureSampler : register(s0);
ConstantBuffer<ObjectIndexData> objectIndex : register(b0);
ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b1);
ConstantBuffer<ObjectMatricesData> objectMatricesData : register(b2);


struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 normal : COLOR1;
};

struct PSOutput
{
	float4 Color: SV_Target0;
	float4 Normal: SV_Target1;
};

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;

	const float4x4 resMatrix = mul(viewProjectionData.proj, mul(viewProjectionData.view, objectMatricesData.data[objectIndex.data]));

	result.position = mul(resMatrix, float4(position, 1));
	result.normal = normalize(mul(objectMatricesData.data[objectIndex.data], float4(normal, 0)));
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input)
{
	PSOutput output;

	output.Color = diffuse.Sample(textureSampler, input.uv);
	output.Normal = input.normal;

	return output;
}
