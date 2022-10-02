// AABB Merger
#include "CommonEngineStructs.h"

ConstantBuffer<BVHConstructorData> data : register(b0);

StructuredBuffer<uint> sortedTriangleIndices; // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<AABB> triangleAABB; // size = THREADS_PER_BLOCK * BLOCK_SIZE

StructuredBuffer<InternalNode> internalNodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE - 1
StructuredBuffer<LeafNode> leafNodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE

RWStructuredBuffer<uint> atomicsData; // size = THREADS_PER_BLOCK * BLOCK_SIZE 
RWStructuredBuffer<AABB> BVHData; // size = THREADS_PER_BLOCK * BLOCK_SIZE - 1


inline AABB MergeAABB(AABB left, AABB right)
{
    AABB ret;
    ret.min = float3(
        min(left.min.x, right.min.x),
        min(left.min.y, right.min.y),
        min(left.min.z, right.min.z)
    );

    ret.max = float3(
        max(left.max.x, right.max.x),
        max(left.max.y, right.max.y),
        max(left.max.z, right.max.z)
    );

    ret._dummy0 = 0;
    ret._dummy1 = 0;
    return ret;
}

[numthreads(THREADS_PER_BLOCK,1,1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    const uint threadId = id.x;
    const uint _trianglesCount = data.trianglesCount;
    AllMemoryBarrierWithGroupSync();

    if (threadId < _trianglesCount)
    {
        uint parent = leafNodes[threadId].parent;
        while (parent != 0xFFFFFFFF)
        {
            uint old = 0;
            InterlockedCompareExchange(atomicsData[parent], 0, 1, old);
            if (old == 0)
            {
                break;
            }

            const uint leftId = internalNodes[parent].leftNode;
            const uint leftType = internalNodes[parent].leftNodeType;
            const uint rightId = internalNodes[parent].rightNode;
            const uint rightType = internalNodes[parent].rightNodeType;

            AABB leftAABB;
            if (leftType == INTERNAL_NODE)
            {
                leftAABB = BVHData[leftId];
            }
            else
            {
                leftAABB = triangleAABB[sortedTriangleIndices[leftId]];
            }
            AABB rightAABB;
            if (rightType == INTERNAL_NODE)
            {
                rightAABB = BVHData[rightId];
            }
            else
            {
                rightAABB = triangleAABB[sortedTriangleIndices[rightId]];
            }

            BVHData[parent] = MergeAABB(leftAABB, rightAABB);

            parent = internalNodes[parent].parent;
        }
    }
}
