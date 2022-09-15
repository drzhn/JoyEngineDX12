#include "CommonEngineStructs.h"

struct PSOutput
{
	float4 Color: SV_Target;
};

ConstantBuffer<EngineData> data : register(b0);
Texture2D<float> depthTexture : register(t0);
Texture2D<float4> colorTexture : register(t1);

inline float LinearEyeDepth(float depth)
{
	const float zNear = 0.1f;
	const float zFar = 1000.0f;
	float x = 1 - zFar / zNear;
	float y = zFar / zNear;
	float z = x / zFar;
	float w = y / zFar;
	return 1.0 / (z * depth + w);
}

float ComputeFog(float z)
{
	float fog = 0.0;
	fog = (30 - z) / (30 - 10);
	//fog = exp2(-0.1f * z);
	return saturate(fog);
}

float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

PSOutput PSMain(float4 position : SV_POSITION)
{
	PSOutput output;
	const float4 fogColor = float4(0.831, 0.4, 0.882, 0); // just nice pink color

	const float4 color = colorTexture.Load(float3(position.xy, 0));
	const float depth = depthTexture.Load(float3(position.xy, 0));

	const float d = LinearEyeDepth(depth);
	const float fog = ComputeFog(d);

	output.Color = lerp(fogColor, color, fog);

	return output;
}
