// BLOCK SUM
#include "CommonEngineStructs.h"

RWStructuredBuffer<uint> data; // size = BUCKET_SIZE * BLOCK_SIZE
RWStructuredBuffer<uint> blockSumsData; // size = BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE)

groupshared uint scanTile[THREADS_PER_BLOCK / WARP_SIZE];
groupshared uint blockSumsTile[(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE)) / WARP_SIZE];

[numthreads(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE),1,1)]
void CSMain(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    // our data has THREADS_PER_BLOCK * BLOCK_SIZE elements.
    // each block in LocalRadixSort step produces 2 ^ RADIX = BUCKET_SIZE elements
    // so, whole number of sizesData array will be BLOCK_SIZE * BUCKET_SIZE elements 
    // here each block process THREADS_PER_BLOCK elements
    // so, number of blocks will be BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE)
    const uint blockSize = BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE);

    const uint threadId = tid.x;
    const uint warpId = threadId / WARP_SIZE;
    const uint laneId = threadId % WARP_SIZE;

    const uint element = blockSumsData[threadId];
    AllMemoryBarrierWithGroupSync();
    const uint wavePrefix = WavePrefixSum(element);

    if (laneId == WARP_SIZE - 1)
    {
        scanTile[warpId] = wavePrefix + element;
    }
    GroupMemoryBarrierWithGroupSync();

    if (threadId < blockSize / WARP_SIZE)
    {
        const uint warpSum = scanTile[threadId];
        GroupMemoryBarrier();
        const uint warpPrefix = WavePrefixSum(warpSum);
        scanTile[threadId] = warpPrefix;
    }

    GroupMemoryBarrierWithGroupSync();
    blockSumsData[threadId] = wavePrefix + scanTile[warpId];
}