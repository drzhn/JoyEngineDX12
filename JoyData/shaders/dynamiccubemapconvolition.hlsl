struct GS_OUTPUT
{
	float4 Pos : SV_POSITION;
	uint RTIndex : SV_RenderTargetArrayIndex;
};

float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

float4 PSMain() : SV_TARGET
{
}
