Texture2D<float> ssaoTexture : register(t0);
Texture2D<float4> colorTexture : register(t1);

SamplerState gsamLinearWrap : register(s0);


float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

float PSMain(float4 position : SV_POSITION) : SV_Target
{
	const float4 color = colorTexture.Load(float3(position.xy, 0));
	const float ssao = ssaoTexture.Load(float3(position.xy, 0));

	const float d = LinearEyeDepth(depth);
	const float fog = ComputeFog(d);

	output.Color = lerp(fogColor, color, fog);

	return output;
}
