#include "common.hlsl"


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

ConstantBuffer<HDRDownScaleConstants> constants: register(b0);
Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);

groupshared float4 SharedInput[groupthreads];

[numthreads(groupthreads, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex)
{
	int2 coord = int2(Gid.x, GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y);
	coord = clamp(coord, int2(0, 0), int2(constants.Res.x - 1, constants.Res.y - 1));
	SharedInput[GI] = Input.Load(int3(coord, 0));
	GroupMemoryBarrierWithGroupSync();
	// Vertical blur
	if (GI >= kernelhalf && GI < (groupthreads - kernelhalf) &&
		((GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.y)
			< constants.Res.y))
	{
		float4 vOut = 0;
		[unroll]
		for (int i = -kernelhalf; i <= kernelhalf; ++i)
		{
			vOut += SharedInput[GI + i] * SampleWeights[i +
				kernelhalf];
		}
		Output[coord] = float4(vOut.rgb, 1.0f);
	}
}
