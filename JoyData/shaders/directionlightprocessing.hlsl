#include "common.hlsl"

struct PSOutput
{
	float4 Color: SV_Target;
};

ConstantBuffer<DirectionLightData> lightData : register(b0);
ConstantBuffer<EngineData> engineData : register(b1);

Texture2D positionTexture: register(t0);
Texture2D normalTexture : register(t1);


float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

PSOutput PSMain(float4 position : SV_POSITION)
{
	PSOutput output;

	const float3 worldNormal = normalTexture.Load(float3(position.xy, 0));
	const float3 worldPos = positionTexture.Load(float3(position.xy, 0));

	const float diff = max(dot(worldNormal, -lightData.direction), 0.0);

	float3 viewDir = normalize(engineData.cameraWorldPos - worldPos);
	float3 reflectDir = reflect(lightData.direction, worldNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	output.Color = (diff + spec + lightData.ambient) * lightData.intensity;

	return output;
}
