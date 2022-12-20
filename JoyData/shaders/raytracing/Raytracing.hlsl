// Raytracing core
#include "CommonEngineStructs.h"

//ConstantBuffer<EngineData> engineData;
//ConstantBuffer<ViewProjectionMatrixData> viewProjectionData;
ConstantBuffer<RaytracedProbesData> raytracedProbesData;

StructuredBuffer<uint> sortedTriangleIndices; // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<AABB> triangleAABB; // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<InternalNode> internalNodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE - 1
StructuredBuffer<LeafNode> leafNodes; // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<AABB> bvhData; // size = THREADS_PER_BLOCK * BLOCK_SIZE - 1
StructuredBuffer<Triangle> triangleData; // size = THREADS_PER_BLOCK * BLOCK_SIZE

ConstantBuffer<StandardMaterialData> materials;
Texture2D<float4> textures[];
SamplerState linearClampSampler;

RWTexture2D<float4> colorTexture;
RWTexture2D<float4> normalsTexture;
RWTexture2D<float4> positionTexture;

struct Ray
{
	float3 origin;
	float3 dir;
	float3 inv_dir;
};

struct RaycastResult
{
	float distance;
	uint triangleIndex;
	float2 uv;
};

RaycastResult RayTriangleIntersection(float3 orig, float3 dir, float3 v0, float3 v1, float3 v2)
{
	RaycastResult result;

	const float3 e1 = v1 - v0;
	const float3 e2 = v2 - v0;

	const float3 pvec = cross(dir, e2);
	const float det = dot(e1, pvec);

	if (det < 1e-8 && det > -1e-8)
	{
		result.distance = MAX_FLOAT;
		return result;
	}

	const float inv_det = 1 / det;
	const float3 tvec = orig - v0;
	const float u = dot(tvec, pvec) * inv_det;

	const float3 qvec = cross(tvec, e1);
	const float v = dot(dir, qvec) * inv_det;
	if ((v < 0 || u + v > 1) || (u < 0 || u > 1))
	{
		result.distance = MAX_FLOAT;
		return result;
	}

	result.distance = dot(e2, qvec) * inv_det;
	result.uv = float2(u, v);
	return result;
}

bool RayBoxIntersection(AABB b, Ray r)
{
	const float3 t1 = (b.min - r.origin) * r.inv_dir;
	const float3 t2 = (b.max - r.origin) * r.inv_dir;

	const float3 tmin1 = min(t1, t2);
	const float3 tmax1 = max(t1, t2);

	const float tmin = max(tmin1.x, max(tmin1.y, tmin1.z));
	const float tmax = min(tmax1.x, min(tmax1.y, tmax1.z));

	return tmax > tmin && tmax > 0;
}

RaycastResult CheckTriangle(uint triangleIndex, Ray ray, RaycastResult result)
{
	if (RayBoxIntersection(triangleAABB[triangleIndex], ray))
	{
		const Triangle t = triangleData[triangleIndex];
		RaycastResult newResult = RayTriangleIntersection(ray.origin, ray.dir, t.a, t.b, t.c);
		if (newResult.distance < result.distance)
		{
			newResult.triangleIndex = triangleIndex;
			return newResult;
		}
		return result;
	}
	return result;
}

inline RaycastResult TraceRay(Ray ray)
{
	RaycastResult result;
	result.distance = MAX_FLOAT;
	result.triangleIndex = 0;
	result.uv = float2(0, 0);

	uint stack[64];
	uint currentStackIndex = 0;
	stack[currentStackIndex] = 0;
	currentStackIndex = 1;

	while (currentStackIndex != 0)
	{
		currentStackIndex--;
		const uint index = stack[currentStackIndex];

		if (!RayBoxIntersection(bvhData[index], ray))
		{
			continue;
		}

		const uint leftIndex = internalNodes[index].leftNode;
		const uint leftType = internalNodes[index].leftNodeType;

		if (leftType == INTERNAL_NODE)
		{
			stack[currentStackIndex] = leftIndex;
			currentStackIndex++;
		}
		else
		{
			const uint triangleIndex = sortedTriangleIndices[leafNodes[leftIndex].index];
			result = CheckTriangle(triangleIndex, ray, result);
		}

		const uint rightIndex = internalNodes[index].rightNode;
		const uint rightType = internalNodes[index].rightNodeType;


		if (rightType == INTERNAL_NODE)
		{
			stack[currentStackIndex] = rightIndex;
			currentStackIndex++;
		}
		else
		{
			const uint triangleIndex = sortedTriangleIndices[leafNodes[rightIndex].index];
			result = CheckTriangle(triangleIndex, ray, result);
		}
	}

	return result;
}

//inline float DistanceToDepth(float distance)
//{
//	const float zNear = engineData.cameraNear;
//	const float zFar = engineData.cameraFar;
//	const float x = 1 - zFar / zNear;
//	const float y = zFar / zNear;
//	const float z = x / zFar;
//	const float w = y / zFar;
//
//	return (1.0 / distance - w) / z;
//}

inline float madfrac(float A, float B)
{
	return ((A) * (B) - floor((A) * (B)));
}

inline float3 sphericalFibonacci(float i, float n)
{
	float PHI = sqrt(5) * 0.5 + 0.5;
	float phi = 2.0 * PI * madfrac(i, PHI - 1);
	float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
	float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));

	return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

//[numthreads(32,32,1)]
//void CSMain(uint3 id : SV_DispatchThreadID)
//{
//	const float near = engineData.cameraNear;
//	const float fov = tan(engineData.cameraFovRadians / 2);
//	const float height = 2 * near * fov;
//	const float width = engineData.screenWidth * height / engineData.screenHeight;
//
//	const float3 originCamera = float3(0, 0, 0);
//	const float3 dirCamera = float3(
//		-width / 2 + width / engineData.screenWidth * (id.x + 0.5),
//		height / 2 - height / engineData.screenHeight * (id.y + 0.5),
//		near
//		);
//
//	const float3 origin = mul(engineData.cameraInvView, float4(originCamera, 1)).xyz;
//	const float3 dir = mul(engineData.cameraInvView, float4(dirCamera, 0)).xyz;
//
//}

[numthreads(DDGI_RAYS_COUNT,1,1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
{
	const uint2 id = uint2(
		groupId.x * raytracedProbesData.gridY * raytracedProbesData.gridZ + 
		groupId.y * raytracedProbesData.gridZ +
		groupId.z,
		groupThreadId.x
	);
	const float3 origin = raytracedProbesData.gridMin + float3(groupId) * raytracedProbesData.cellSize;
	const float3 dir = sphericalFibonacci(groupThreadId.x, DDGI_RAYS_COUNT);


	Ray ray;
	ray.origin = origin;
	ray.dir = normalize(dir);
	ray.inv_dir = 1 / ray.dir;

	RaycastResult result = TraceRay(ray);

	const Triangle t = triangleData[result.triangleIndex];
	const float2 uv = (1 - result.uv.x - result.uv.y) * t.a_uv + result.uv.x * t.b_uv + result.uv.y * t.c_uv;
	const float3 normal = (1 - result.uv.x - result.uv.y) * t.a_normal + result.uv.x * t.b_normal + result.uv.y * t.c_normal;
	const uint materialIndex = t.materialIndex;

	const float hasResult = result.distance != MAX_FLOAT;

	float4 color = textures[materials.data[materialIndex].diffuseTextureIndex].SampleLevel(linearClampSampler, uv, 0);
	colorTexture[id.xy] = float4(color.rgb, hasResult);
	positionTexture[id.xy] = float4(ray.origin + ray.dir * result.distance, hasResult);
	normalsTexture[id.xy] = float4(normalize(normal) * hasResult, 1);
}
