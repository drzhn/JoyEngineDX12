// PRE SCAN
#include "CommonEngineStructs.h"

RWStructuredBuffer<uint> data; // size = BUCKET_SIZE * BLOCK_SIZE
RWStructuredBuffer<uint> blockSumsData; // size = BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE)

groupshared uint scanTile[THREADS_PER_BLOCK / WARP_SIZE];
groupshared uint blockSumsTile[(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE)) / WARP_SIZE];


[numthreads(THREADS_PER_BLOCK,1,1)]
void CSMain(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    const uint threadId = tid.x;
    const uint groupId = gid.x;
    const uint warpId = threadId / WARP_SIZE;
    const uint laneId = threadId % WARP_SIZE;

    const uint element = data[groupId * THREADS_PER_BLOCK + threadId];
    AllMemoryBarrierWithGroupSync();
    const uint wavePrefix = WavePrefixSum(element);

    if (laneId == WARP_SIZE - 1)
    {
        scanTile[warpId] = wavePrefix + element;
    }
    GroupMemoryBarrierWithGroupSync();

    if (threadId < THREADS_PER_BLOCK / WARP_SIZE)
    {
        const uint warpSum = scanTile[threadId];
        GroupMemoryBarrier();
        const uint warpPrefix = WavePrefixSum(warpSum);
        scanTile[threadId] = warpPrefix;

        if (threadId == THREADS_PER_BLOCK / WARP_SIZE - 1)
        {
            blockSumsData[groupId] = warpPrefix + warpSum;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    data[groupId * THREADS_PER_BLOCK + threadId] = wavePrefix + scanTile[warpId];
}