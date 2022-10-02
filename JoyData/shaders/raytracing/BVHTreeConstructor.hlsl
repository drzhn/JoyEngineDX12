// BVH TREE CONSTRUCTOR
#include "CommonEngineStructs.h"

ConstantBuffer<BVHConstructorData> data : register(b0);

StructuredBuffer<uint> sortedMortonCodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE

RWStructuredBuffer<InternalNode> internalNodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE - 1
RWStructuredBuffer<LeafNode> leafNodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE

inline uint clz32(uint v)
{
    return 31 - firstbithigh(v);
}

inline int delta(int x, int y, int numObjects)
{
    if (x >= 0 && x <= numObjects - 1 && y >= 0 && y <= numObjects - 1)
    {
        const uint x_code = sortedMortonCodes[x];
        const uint y_code = sortedMortonCodes[y];
        // we guarantee that x_code != y_code
        return clz32(x_code ^ y_code);
    }
    return -1;
}

inline int2 DetermineRange(int numObjects, int idx)
{
    const int d = sign(delta(idx, idx + 1, numObjects) - delta(idx, idx - 1, numObjects));
    const int dmin = delta(idx, idx - d, numObjects);
    uint lmax = 2;
    while (delta(idx, idx + lmax * d, numObjects) > dmin)
        lmax = lmax * 2;
    int l = 0;
    for (uint t = lmax / 2; t >= 1; t /= 2)
    {
        if (delta(idx, idx + (l + t) * d, numObjects) > dmin)
            l += t;
    }

    const int j = idx + l * d;
    int2 range = int2(min(idx, j), max(idx, j));
    return range;
}

inline int FindSplit(int first, int last)
{
    // Identical Morton codes => split the range in the middle.

    const uint firstCode = sortedMortonCodes[first];
    const uint lastCode = sortedMortonCodes[last];

    if (firstCode == lastCode)
        return (first + last) >> 1;

    // Calculate the number of highest bits that are the same
    // for all objects, using the count-leading-zeros intrinsic.

    const int commonPrefix = clz32(firstCode ^ lastCode);

    // Use binary search to find where the next bit differs.
    // Specifically, we are looking for the highest object that
    // shares more than commonPrefix bits with the first one.
 
    int split = first; // initial guess
    int step = last - first;

    do
    {
        step = (step + 1) >> 1; // exponential decrease
        const int newSplit = split + step; // proposed new position

        if (newSplit < last)
        {
            const uint splitCode = sortedMortonCodes[newSplit];
            const int splitPrefix = clz32(firstCode ^ splitCode);
            if (splitPrefix > commonPrefix)
                split = newSplit; // accept proposal
        }
    }
    while (step > 1);

    return split;
}

[numthreads(THREADS_PER_BLOCK,1,1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    const uint threadId = id.x;
    const uint _trianglesCount = data.trianglesCount;
    AllMemoryBarrierWithGroupSync();

    if (threadId == 0)
    {
        internalNodes[threadId].parent = 0xFFFFFFFF;
    }

    if (threadId < _trianglesCount - 1)
    {
        int2 range = DetermineRange(_trianglesCount, threadId);
        const int first = range.x;
        const int last = range.y;

        // Determine where to split the range.

        const int split = FindSplit(first, last);

        internalNodes[threadId].index = threadId;

        // Select childA.
        if (split == first)
        {
            const LeafNode node = {
                threadId,
                split
            };
            leafNodes[split] = node;
            internalNodes[threadId].leftNode = split;
            internalNodes[threadId].leftNodeType = LEAF_NODE;
        }
        else
        {
            internalNodes[split].parent = threadId;
            internalNodes[threadId].leftNode = split;
            internalNodes[threadId].leftNodeType = INTERNAL_NODE;
        }

        // Select childB.
        if (split + 1 == last)
        {
            const LeafNode node = {
                threadId,
                split + 1
            };
            leafNodes[split + 1] = node;
            internalNodes[threadId].rightNode = split + 1;
            internalNodes[threadId].rightNodeType = LEAF_NODE;
        }
        else
        {
            internalNodes[split + 1].parent = threadId;
            internalNodes[threadId].rightNode = split + 1;
            internalNodes[threadId].rightNodeType = INTERNAL_NODE;
        }
    }
}