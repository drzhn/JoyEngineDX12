#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
	//float2 uv : TEXCOORD0;
	//float4 clipPos : TEXCOORD1;
};

struct PSOutput
{
	//float4 Color: SV_Target;
};

ConstantBuffer<MVP> mvp : register(b0);

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal: NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
	result.position = mul(resMatrix, float4(position, 1));
	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;
	//output.Color = float4(1, 1, 1, 1);
	return output;
}
