cbuffer HDRDownScaleConstants : register(b0)
{
// Resolution of the down scaled target: x - width, y - height
uint2 Res : packoffset(c0);
// Total pixel in the downscaled image
uint Domain : packoffset(c0.z);
// Number of groups dispached on the first pass
uint GroupSize : packoffset(c0.w);
float Adaptation : packoffset(c1); // Adaptation factor
float fBloomThreshold : packoffset(c1.y); // Bloom threshold percentage
}
Texture2D<float4> HDRDownScaleTex : register(t0);
StructuredBuffer<float> AvgLum : register(t1);
RWTexture2D<float4> Bloom : register(u0);

static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);


[numthreads(1024, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 CurPixel = uint2(dispatchThreadId.x % Res.x,
		dispatchThreadId.x / Res.x);
	// Skip out of bound pixels
	if (CurPixel.y < Res.y)
	{
		float4 color = HDRDownScaleTex.Load(int3(CurPixel, 0));
		float Lum = dot(color, LUM_FACTOR);
		float avgLum = AvgLum[0];
		// Find the color scale
		float colorScale = saturate(Lum - avgLum * fBloomThreshold);
		// Store the scaled bloom value
		Bloom[CurPixel.xy] = color * colorScale;
	}
}