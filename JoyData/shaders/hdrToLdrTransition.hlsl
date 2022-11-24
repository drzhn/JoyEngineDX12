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

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

struct PSInput
{
	float4 position : SV_POSITION;
	float4 clipPos : TEXCOORD0;
};

PSInput VSMain(uint id : SV_VertexID)
{
	float2 uv = float2((id << 1) & 2, id & 2);
	PSInput input;
	input.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	input.clipPos = ComputeNonStereoScreenPos(input.position);

	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	const float2 screenPosition = (input.clipPos.xy / input.clipPos.w);

	float4 color = HdrTexture.Load(float3(input.position.xy, 0));

	//color += BloomScale * BloomTexture.Sample(LinearSampler, screenPosition);

	if (Constants.UseSrgbConversion)
	{
		float gamma = 2.2;
		color.rgb = pow(color.rgb, float3(1, 1, 1) * (1.0 / gamma));
	}

	if (Constants.UseTonemapping)
	{
		color = ToneMapping(color);
	}

	return color;
}
