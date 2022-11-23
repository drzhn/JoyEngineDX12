#include "CommonEngineStructs.h"

Texture2D<float4> colorTexture;
Texture2D<float4> normalsTexture;
Texture2D<float> depthTexture;


float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

float4 PSMain(float4 position : SV_POSITION) : SV_Target
{
	float4 color =  colorTexture.Load(float3(position.xy, 0));
	float3 worldNormal = normalsTexture.Load(float3(position.xy, 0)).rgb;
	float3 lightDirection = float3(1, 1, 1);
	color *= max(0.4, dot(worldNormal, lightDirection));

	return color;
}
