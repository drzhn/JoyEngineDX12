#include "CommonEngineStructs.h"

Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
SamplerState TextureSampler : register(s0);
ConstantBuffer<ObjectIndexData> objectIndex : register(b0);
ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b1);
StructuredBuffer<MAT4> objectMatricesData : register(t2);


struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 worldNormal : COLOR1;
	float4 worldPosition : COLOR2;

	float3x3 TBN : COLOR3;
};

struct PSOutput
{
	float4 Color: SV_Target0;
	float4 Normal: SV_Target1;
	float4 Position: SV_Target2;
};

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

PSInput VSMain(float3 position : POSITION, float3 tangent: TANGENT, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;

	const float4x4 resMatrix = mul(viewProjectionData.proj, mul(viewProjectionData.view, objectMatricesData[objectIndex.data]));

	float4 transformedNormal = normalize(mul(objectMatricesData[objectIndex.data], float4(normal * 2 - 1, 0)));
	float4 transformedTangent = normalize(mul(objectMatricesData[objectIndex.data], float4(tangent * 2 - 1, 0)));

	// todo Get sign of tangent from .w component of vertex input
	float3 transformedBitangent = cross(transformedTangent.xyz, transformedNormal.xyz);

	result.position = mul(resMatrix, float4(position, 1));
	result.worldNormal = transformedNormal;
	result.worldPosition = mul(objectMatricesData[objectIndex.data], float4(position, 1));
	result.uv = uv;
	result.TBN = transpose(float3x3(transformedTangent.xyz, transformedBitangent, transformedNormal.xyz));

	return result;
}

PSOutput PSMain(PSInput input)
{
	PSOutput output;

	float3 normal = NormalMap.Sample(TextureSampler, input.uv);
	normal = length(normal) < FLT_EPSILON ? float3(0, 0, 1) : normalize(normal * 2.0 - 1.0);
	normal = mul(input.TBN, normal);
	output.Normal = float4(normal, 1);

	output.Color = DiffuseMap.Sample(TextureSampler, input.uv);
	output.Position = input.worldPosition;

	return output;
}
