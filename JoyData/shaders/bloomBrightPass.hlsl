#include "CommonEngineStructs.h"

ConstantBuffer<HDRDownScaleConstants> constants: register(b0);
Texture2D<float4> HDRDownScaleTex : register(t0);
StructuredBuffer<float> AvgLum : register(t1);
RWTexture2D<float4> Bloom : register(u0);

static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);


[numthreads(1024, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 CurPixel = uint2(dispatchThreadId.x % constants.Res.x,
	                       dispatchThreadId.x / constants.Res.x);
	// Skip out of bound pixels
	if (CurPixel.y < constants.Res.y)
	{
		float4 color = HDRDownScaleTex.Load(int3(CurPixel, 0));
		float Lum = dot(color, LUM_FACTOR);
		float avgLum = AvgLum[0];
		// Find the color scale
		float colorScale = saturate(Lum - avgLum * constants.fBloomThreshold);
		// Store the scaled bloom value
		Bloom[CurPixel.xy] = color * colorScale;
	}
}
