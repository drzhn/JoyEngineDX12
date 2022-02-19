Texture2D<float4> colorTexture : register(t0);
StructuredBuffer<float> AvgLum : register(t1);

static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);
static const float MiddleGrey = 2.0f;
static const float LumWhiteSqr = 4.0f;

float4 ToneMapping(float4 HDRColor)
{
	// Find the luminance scale for the current pixel
	float LScale = dot(HDRColor, LUM_FACTOR);
	LScale *= MiddleGrey / AvgLum[0];
	LScale = (LScale + LScale * LScale / LumWhiteSqr) / (1.0 + LScale);
	// Apply the luminance scale to the pixels color
	return HDRColor * LScale;
}

float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

float4 PSMain(float4 position : SV_POSITION) : SV_Target
{
	float4 color = colorTexture.Load(float3(position.xy, 0));
	color = ToneMapping(color);
	return color;
}
