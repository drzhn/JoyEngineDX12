// LOCAL RADIX SORT
#include "CommonEngineStructs.h"

ConstantBuffer<BufferSorterData> data : register(b0);

StructuredBuffer<uint> keysData : register(t0); // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<uint> valuesData : register(t1); // size = THREADS_PER_BLOCK * BLOCK_SIZE


RWStructuredBuffer<uint> sortedBlocksKeysData : register(u0); // size = THREADS_PER_BLOCK * BLOCK_SIZE
RWStructuredBuffer<uint> sortedBlocksValuesData : register(u1); // size = THREADS_PER_BLOCK * BLOCK_SIZE

RWStructuredBuffer<uint> offsetsData : register(u2); // size = BLOCK_SIZE * BUCKET_SIZE
RWStructuredBuffer<uint> sizesData : register(u3); // size = BLOCK_SIZE * BUCKET_SIZE


groupshared uint sortTile[THREADS_PER_BLOCK];
groupshared uint valuesTile[THREADS_PER_BLOCK];

groupshared uint scanTile[THREADS_PER_BLOCK / WARP_SIZE];
groupshared uint falseTotal;

groupshared uint radixTile[THREADS_PER_BLOCK];
groupshared uint offsetsTile[BUCKET_SIZE];
groupshared uint sizesTile[BUCKET_SIZE];


inline uint IntraBlockScan(uint threadId, bool pred0)
{
	const uint warpIdx = threadId / WARP_SIZE;
	const uint laneIdx = threadId % WARP_SIZE;
	const uint warpResult = WavePrefixCountBits(pred0);
	const uint predResult = pred0;

	GroupMemoryBarrierWithGroupSync();
	if (laneIdx == WARP_SIZE - 1)
	{
		scanTile[warpIdx] = warpResult + predResult;
	}
	GroupMemoryBarrierWithGroupSync();

	if (threadId < WARP_SIZE)
	{
		const uint prefixSum = scanTile[threadId];
		scanTile[threadId] = WavePrefixSum(prefixSum);
	}
	GroupMemoryBarrierWithGroupSync();

	return warpResult + scanTile[warpIdx];
}

[numthreads(THREADS_PER_BLOCK,1,1)]
void CSMain(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
	const uint threadId = tid.x;
	const uint groupId = gid.x;

	sortTile[threadId] = keysData[groupId * THREADS_PER_BLOCK + threadId];
	valuesTile[threadId] = valuesData[groupId * THREADS_PER_BLOCK + threadId];

	AllMemoryBarrierWithGroupSync();

	for (uint shift = data.bitOffset; shift < data.bitOffset + RADIX; shift++)
	{
		uint predResult = 0;

		const uint key = sortTile[threadId];
		const uint value = valuesTile[threadId];

		const bool pred = (key >> shift) & 1;

		predResult += pred;

		GroupMemoryBarrierWithGroupSync();

		const uint trueBefore = IntraBlockScan(threadId, pred);

		// GroupMemoryBarrierWithGroupSync();

		if (threadId == THREADS_PER_BLOCK - 1)
		{
			falseTotal = THREADS_PER_BLOCK - (trueBefore + predResult);
		}
		GroupMemoryBarrierWithGroupSync();

		sortTile[pred ? trueBefore + falseTotal : threadId - trueBefore] = key;
		valuesTile[pred ? trueBefore + falseTotal : threadId - trueBefore] = value;

		GroupMemoryBarrierWithGroupSync();
	}

	GroupMemoryBarrierWithGroupSync();

	const uint key = sortTile[threadId];
	const uint value = valuesTile[threadId];
	GroupMemoryBarrierWithGroupSync();

	sortedBlocksKeysData[groupId * THREADS_PER_BLOCK + threadId] = key;
	sortedBlocksValuesData[groupId * THREADS_PER_BLOCK + threadId] = value;

	radixTile[threadId] = (key >> data.bitOffset) & (BUCKET_SIZE - 1);

	if (threadId < BUCKET_SIZE)
	{
		offsetsTile[threadId] = 0;
		sizesTile[threadId] = 0;
	}
	GroupMemoryBarrierWithGroupSync();

	if (threadId > 0 && radixTile[threadId - 1] != radixTile[threadId])
	{
		offsetsTile[radixTile[threadId]] = threadId;
	}
	GroupMemoryBarrierWithGroupSync();

	if (threadId > 0 && radixTile[threadId - 1] != radixTile[threadId])
	{
		uint radix = radixTile[threadId - 1];
		sizesTile[radix] = threadId - offsetsTile[radix];
	}
	if (threadId == THREADS_PER_BLOCK - 1)
	{
		uint radix = radixTile[THREADS_PER_BLOCK - 1];
		sizesTile[radix] = THREADS_PER_BLOCK - offsetsTile[radix];
	}

	GroupMemoryBarrierWithGroupSync();
	if (threadId < BUCKET_SIZE)
	{
		offsetsData[groupId * BUCKET_SIZE + threadId] = offsetsTile[threadId];
		sizesData[groupId + threadId * BLOCK_SIZE] = sizesTile[threadId];
	}
}
