// GLOBAL RADIX SORT
#include "CommonEngineStructs.h"

ConstantBuffer<BufferSorterData> data : register(b0);

StructuredBuffer<uint> sortedBlocksKeysData; // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<uint> sortedBlocksValuesData; // size = THREADS_PER_BLOCK * BLOCK_SIZE

StructuredBuffer<uint> offsetsData; // size = BLOCK_SIZE * BUCKET_SIZE
StructuredBuffer<uint> sizesData; // size = BLOCK_SIZE * BUCKET_SIZE

RWStructuredBuffer<uint> sortedKeysData; // size = THREADS_PER_BLOCK * BLOCK_SIZE
RWStructuredBuffer<uint> sortedValuesData; // size = THREADS_PER_BLOCK * BLOCK_SIZE

groupshared uint offsetsTile[BUCKET_SIZE];
groupshared uint sizesTile[BUCKET_SIZE];

[numthreads(THREADS_PER_BLOCK,1,1)]
void CSMain(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    const uint threadId = tid.x;
    const uint groupId = gid.x;

    const uint key = sortedBlocksKeysData[groupId * THREADS_PER_BLOCK + threadId];
    const uint value = sortedBlocksValuesData[groupId * THREADS_PER_BLOCK + threadId];
    if (threadId < BUCKET_SIZE)
    {
        offsetsTile[threadId] = offsetsData[groupId * BUCKET_SIZE + threadId];
        sizesTile[threadId] = sizesData[groupId + threadId * BLOCK_SIZE];
    }
    AllMemoryBarrierWithGroupSync();

    const uint radix = (key >> data.bitOffset) & (BUCKET_SIZE - 1);
    const uint indexOutput = sizesTile[radix] + threadId - offsetsTile[radix];
    
    sortedKeysData[indexOutput] = key;
    sortedValuesData[indexOutput] = value;
}
