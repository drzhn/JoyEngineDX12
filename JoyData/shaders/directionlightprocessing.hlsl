struct PSOutput
{
	float4 Color: SV_Target;
};

struct DirectionLightData
{
	float3 direction;
	float intensity;
	float ambient;
};

ConstantBuffer<DirectionLightData> lightData : register(b0);
Texture2D normalTexture : register(t0);


float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

PSOutput PSMain(float4 position : SV_POSITION)
{
	PSOutput output;

	const float3 worldNormal = normalTexture.Load(float3(position.xy, 0));

	const float diff = max(dot(worldNormal, -lightData.direction), 0.0);

	output.Color = (diff+lightData.ambient) * lightData.intensity;

	return output;
}
