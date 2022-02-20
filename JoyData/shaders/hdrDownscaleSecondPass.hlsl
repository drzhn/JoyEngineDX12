#define MAX_GROUPS 64

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

RWStructuredBuffer<float> AverageLum : register(u0);
RWStructuredBuffer<float> PrevAverageLum : register(u1);

// Group shared memory to store the intermediate results
groupshared float SharedAvgFinal[MAX_GROUPS];

[numthreads(MAX_GROUPS, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3
            groupThreadId : SV_GroupThreadID,
            uint3 dispatchThreadId : SV_DispatchThreadID)
{
	// Fill the shared memory with the 1D values
	float avgLum = 0.0;
	if (dispatchThreadId.x < GroupSize)
	{
		avgLum = AverageLum[dispatchThreadId.x];
	}
	SharedAvgFinal[dispatchThreadId.x] = avgLum;
	GroupMemoryBarrierWithGroupSync(); // Sync before next step
	// Downscale from 64 to 16
	if (dispatchThreadId.x % 4 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 1 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 1] : avgLum;
		stepAvgLum += dispatchThreadId.x + 2 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 2] : avgLum;
		stepAvgLum += dispatchThreadId.x + 3 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 3] : avgLum;
		// Store the results
		avgLum = stepAvgLum;
		SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
	}
	GroupMemoryBarrierWithGroupSync(); // Sync before next step
	// Downscale from 16 to 4
	if (dispatchThreadId.x % 16 == 0)
	{
		// Calculate the luminance sum for this step
		float stepAvgLum = avgLum;
		stepAvgLum += dispatchThreadId.x + 4 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 4] : avgLum;
		stepAvgLum += dispatchThreadId.x + 8 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 8] : avgLum;
		stepAvgLum += dispatchThreadId.x + 12 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 12] : avgLum;
		// Store the results
		avgLum = stepAvgLum;
		SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
	}
	GroupMemoryBarrierWithGroupSync(); // Sync before next step
	// Downscale from 4 to 1
	if (dispatchThreadId.x == 0)
	{
		// Calculate the average luminace
		float fFinalLumValue = avgLum;
		fFinalLumValue += dispatchThreadId.x + 16 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 16] : avgLum;
		fFinalLumValue += dispatchThreadId.x + 32 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 32] : avgLum;
		fFinalLumValue += dispatchThreadId.x + 48 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 48] : avgLum;
		fFinalLumValue /= 64.0;


		// Calculate the adaptive luminance
		float fAdaptedAverageLum = lerp(PrevAverageLum[0], fFinalLumValue, Adaptation);
		// Store the final value
		AverageLum[0] = fAdaptedAverageLum;// max(fAdaptedAverageLum, 0.0001);
		PrevAverageLum[0] = fAdaptedAverageLum;
	}
}
