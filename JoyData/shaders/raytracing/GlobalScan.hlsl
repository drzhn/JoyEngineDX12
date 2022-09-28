// GLOBAL SCAN
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

    const uint element = data[groupId * THREADS_PER_BLOCK + threadId];
    const uint blockSum = blockSumsData[groupId];
    AllMemoryBarrierWithGroupSync();
    data[groupId * THREADS_PER_BLOCK + threadId] = element + blockSum;
}
