//struct EngineData
//{
//	float4 perspectiveValues;
//	float3 cameraWorldPos;
//	float time;
//};
//
//ConstantBuffer<EngineData> data : register(b0);
//Texture2D<float> depthTexture : register(t0);
Texture2D<float4> colorTexture : register(t0);


float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

float4 PSMain(float4 position : SV_POSITION) : SV_Target
{
	const float4 color = colorTexture.Load(float3(position.xy, 0));

	return color;
}
