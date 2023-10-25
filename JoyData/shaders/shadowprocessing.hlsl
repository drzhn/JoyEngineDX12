#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
};

struct PSOutput
{
};

ConstantBuffer<ObjectIndexData> objectIndex : register(b0);
ConstantBuffer<ViewProjectionMatrixData> viewProjectionData : register(b1);

StructuredBuffer<MAT4> objectMatricesData : register(t0);

PSInput VSMain(float3 position : POSITION)
{
	PSInput result;

	const float4x4 resMatrix = mul(viewProjectionData.proj, mul(viewProjectionData.view, objectMatricesData[objectIndex.data]));

	result.position = mul(resMatrix, float4(position, 1));
	return result;
}

PSOutput PSMain(PSInput input)
{
	PSOutput output;
	return output;
}
