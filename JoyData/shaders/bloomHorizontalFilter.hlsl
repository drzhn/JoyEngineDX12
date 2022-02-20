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

static const float SampleWeights[13] = {
	0.002216,
	0.008764,
	0.026995,
	0.064759,
	0.120985,
	0.176033,
	0.199471,
	0.176033,
	0.120985,
	0.064759,
	0.026995,
	0.008764,
	0.002216,
};
#define kernelhalf 6
#define groupthreads 128

Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);

groupshared float4 SharedInput[groupthreads];

[numthreads(groupthreads, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex)
{
	int2 coord = int2(GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x, Gid.y);
	coord = clamp(coord, int2(0, 0), int2(Res.x - 1, Res.y - 1));
	SharedInput[GI] = Input.Load(int3(coord, 0));
	GroupMemoryBarrierWithGroupSync();
	// Horizontal blur
	if (GI >= kernelhalf && GI < (groupthreads - kernelhalf) &&
		((Gid.x * (groupthreads - 2 * kernelhalf) + GI - kernelhalf)
			< Res.x))
	{
		float4 vOut = 0;
		[unroll]
		for (int i = -kernelhalf; i <= kernelhalf; ++i)
			vOut += SharedInput[GI + i] * SampleWeights[i +
				kernelhalf];
		Output[coord] = float4(vOut.rgb, 1.0f);
	}
}
