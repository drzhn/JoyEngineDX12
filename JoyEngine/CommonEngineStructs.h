#ifndef COMMON_ENGINE_STRUCTS
#define COMMON_ENGINE_STRUCTS

#ifdef ENGINE
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#ifdef ENGINE
#define VEC2 glm::vec2
#define VEC3 glm::vec3
#define VEC4 glm::vec4
#define MAT4 glm::mat4
#define UINT1 uint32_t
#define UINT2 glm::uvec2
#define UINT3 glm::uvec3
#endif

#ifdef SHADER
#define VEC2 float2
#define VEC3 float3
#define VEC4 float4
#define MAT4 float4x4
#define UINT1 uint
#define UINT2 uint2
#define UINT3 uint3
#endif


#ifdef ENGINE
inline glm::vec3 shadowTransformsForward[6]
{
	VEC3(1.0, 0.0, 0.0),
	VEC3(-1.0, 0.0, 0.0),
	VEC3(0.0, 1.0, 0.0),
	VEC3(0.0, -1.0, 0.0),
	VEC3(0.0, 0.0, 1.0),
	VEC3(0.0, 0.0, -1.0)
};

inline glm::vec3 shadowTransformsUp[6]
{
	VEC3(0.0, 1.0, 0.0),
	VEC3(0.0, 1.0, 0.0),
	VEC3(0.0, 0.0, 1.0),
	VEC3(0.0, 0.0, 1.0),
	VEC3(0.0, 1.0, 0.0),
	VEC3(0.0, 1.0, 0.0)
};

struct MeshHeader
{
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t materialIndex;
};

enum LightType
{
	Point = 0,
	Spot = 1,
	Capsule = 2,
	Direction = 3
};
#endif


#define RADIX 8
#define BUCKET_SIZE 256 // 2 ^ RADIX
#define BLOCK_SIZE 512
#define THREADS_PER_BLOCK 1024
#define WARP_SIZE 32
#define ELEM_PER_THREAD  1

#define DATA_ARRAY_COUNT ELEM_PER_THREAD*THREADS_PER_BLOCK*BLOCK_SIZE // 1*512*1024 = 524288

#define MAX_FLOAT 0x7F7FFFFF // just a big float
#define MAX_UINT 0xFFFFFFFF

#define INTERNAL_NODE 0
#define LEAF_NODE 1

struct Vertex
{
	VEC3 pos;
	VEC3 color;
	VEC3 normal;
	VEC2 texCoord;
};

struct ModelMatrixData
{
	MAT4 model;
};

struct ViewProjectionMatrixData
{
	MAT4 view;
	MAT4 proj;
};

struct DirectionLightData
{
	VEC3 direction;
	float intensity;
	float ambient;
};

struct LightData
{
	float intensity;
	float radius;
	float height;
	float angle;

	MAT4 view[6];
	MAT4 proj;
};

struct CubemapConvolutionConstants
{
	MAT4 model;
	MAT4 view[6];
	MAT4 projection;
};

struct EngineData
{
	VEC3 cameraWorldPos;
	float time;

	MAT4 cameraInvProj;
	MAT4 cameraInvView;

	float cameraNear;
	float cameraFar;
	float cameraFovRadians;
	UINT1 screenWidth;

	UINT1 screenHeight;
	UINT1 _dummy0;
	UINT1 _dummy1;
	UINT1 _dummy2;
};

struct HDRDownScaleConstants
{
	// Resolution of the down scaled target: x - width, y - height
	UINT2 Res;
	// Total pixel in the downscaled image
	UINT1 Domain;
	// Number of groups dispached on the first pass
	UINT1 GroupSize;
	float Adaptation; // Adaptation factor
	float fBloomThreshold; // Bloom threshold percentage
};

//struct SSAOData
//{
//	VEC4 offsetVectors[14];
//	VEC2 invRenderTargetSize;
//	UINT1 isHorizontal;
//	UINT1 _dummy;
//	VEC4 blurWeights[3];
//};

struct MipMapGenerationData
{
	UINT2 TexelSize;
	UINT1 SrcMipLevel; // Texture level of source mip
	UINT1 NumMipLevels; // Number of OutMips to write: [1, 4]
};


// Raytracing structs

struct BufferSorterData
{
	UINT1 bitOffset;
};

struct BVHConstructorData
{
	UINT1 trianglesCount;
};

struct AABB
{
	VEC3 min;
	float _dummy0;
	VEC3 max;
	float _dummy1;
};

struct Triangle
{
	VEC3 a;
	float _dummy0;
	VEC3 b;
	float _dummy1;
	VEC3 c;
	float _dummy2;
	VEC2 a_uv;
	VEC2 b_uv;
	VEC2 c_uv;
	VEC2 _dummy3;

	VEC3 a_normal;
	float _dummy4;
	VEC3 b_normal;
	float _dummy5;
	VEC3 c_normal;
	float _dummy6;
};

struct InternalNode
{
	UINT1 leftNode;
	UINT1 leftNodeType;
	UINT1 rightNode;
	UINT1 rightNodeType;
	UINT1 parent;
	UINT1 index;
};

struct LeafNode
{
	UINT1 parent;
	UINT1 index;
};


#endif //COMMON_ENGINE_STRUCTS
