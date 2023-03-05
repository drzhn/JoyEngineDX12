struct PSOutput
{
	float4 Color: SV_Target;
};

Texture2D<float4> shadedColorTexture : register(t1);

float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

PSOutput PSMain(float4 position : SV_POSITION)
{
	PSOutput output;

	const float4 raytracing = shadedColorTexture.Load(float3(position.xy, 0));

	output.Color = raytracing;

	return output;
}
