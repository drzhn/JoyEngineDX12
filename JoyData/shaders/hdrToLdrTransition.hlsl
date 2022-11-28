#include "CommonEngineStructs.h"

ConstantBuffer<HDRDownScaleConstants> Constants;
Texture2D<float4> HdrTexture;
StructuredBuffer<float> AvgLum;


float4 ToneMapping(float4 HDRColor)
{
	// Find the luminance scale for the current pixel
	float LScale = dot(HDRColor.rgb, Constants.LumFactor);
	LScale *= Constants.MiddleGrey / AvgLum[0];
	LScale = (LScale + LScale * LScale / Constants.LumWhiteSqr) / (1.0 + LScale);
	// Apply the luminance scale to the pixels color
	return HDRColor * LScale;
}

struct PSInput
{
	float4 position : SV_POSITION;
};

PSInput VSMain(uint id : SV_VertexID)
{
	float2 uv = float2((id << 1) & 2, id & 2);
	PSInput input;
	input.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);

	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	float4 color = HdrTexture.Load(float3(input.position.xy, 0));

	const float gamma = 2.2;
	color.rgb = lerp(color.rgb, pow(color.rgb, float3(1, 1, 1) * (1.0 / gamma)), Constants.UseGammaCorrection);

	color = lerp(color, ToneMapping(color), Constants.UseTonemapping);

	return color;
}
