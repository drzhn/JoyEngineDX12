#ifndef COMMON_ENGINE_STRUCTS
#define COMMON_ENGINE_STRUCTS

#ifdef ENGINE
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

// ========= COMMON DEFINES ========
#define RADIX 8
#define BUCKET_SIZE 256 // 2 ^ RADIX
#define BLOCK_SIZE 512
#define THREADS_PER_BLOCK 1024
#define WARP_SIZE 32
#define ELEM_PER_THREAD  1

#define DATA_ARRAY_COUNT (ELEM_PER_THREAD * THREADS_PER_BLOCK * BLOCK_SIZE) // 1*512*1024 = 524288

#define MAX_FLOAT 0x7F7FFFFF // just a big float
#define MAX_UINT 0xFFFFFFFF

#define INTERNAL_NODE 0
#define LEAF_NODE 1

#define MATERIAL_SIZE 512
#define OBJECT_SIZE 512
#define LIGHT_SIZE 256

#define DDGI_RAYS_COUNT 192
#define DDGI_PROBE_DATA_RESOLUTION 16
#define DDGI_PROBE_IRRADIANCE_SAMPLES 4

// ========= CONTEXT DEPENDENT STRUCTS =========

#ifdef ENGINE

#define PI 3.14159265359f

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

enum LightType
{
	Point = 0,
	Spot = 1,
	Capsule = 2,
	Direction = 3
};
#elif SHADER

#define PI 3.14159265359

#endif

// ========= COMMON STRUCTS =========

struct Vertex
{
	VEC3 pos;
	VEC3 color;
	VEC3 normal;
	VEC2 texCoord;
};

struct ViewProjectionMatrixData
{
	MAT4 view;
	MAT4 proj;
};

struct DirectionalLightInfo
{
	VEC3 direction;
	float intensity;

	MAT4 view;
	MAT4 proj;

	float ambient;
	float bias;
	UINT1 shadowmapSize;
	UINT1 packedColor; // TODO
};

struct LightInfo
{
	UINT1 packedColor;
	float radius;
	float intensity;
	float _dummy0; // TODO pack radius and height for spotlight
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

struct StandardMaterial
{
	UINT1 diffuseTextureIndex;
	UINT1 normalTextureIndex;
	UINT1 _dummy0;
	UINT1 _dummy1;
};

struct StandardMaterialData
{
	StandardMaterial data[MATERIAL_SIZE];
};

struct ObjectIndexData
{
	UINT1 data;
};

struct TextureIndexData
{
	UINT1 data;
};

struct ObjectMatricesData
{
	MAT4 data[OBJECT_SIZE];
};

struct LightData
{
	LightInfo data[LIGHT_SIZE];
};

struct HDRDownScaleConstants
{
	// Resolution of the down scaled target: x - width, y - height
	UINT2 Res;
	// Total pixel in the downscaled image
	UINT1 Domain;
	// Number of groups dispached on the first pass
	UINT1 GroupSize;

	float AdaptationSpeed; // AdaptationSpeed factor
	UINT1 UseGammaCorrection;
	float MiddleGrey; // = 3.0f;
	float LumWhiteSqr; // = 9.0f;

	VEC3 LumFactor;
	UINT1 UseTonemapping;
};

struct MipMapGenerationData
{
	UINT2 TexelSize;
	UINT1 SrcMipLevel; // Texture level of source mip
	UINT1 NumMipLevels; // Number of OutMips to write: [1, 4]
};


// Raytracing structs

//#define CAMERA_TRACE

struct RaytracedProbesData
{
	VEC3 gridMin;
	float cellSize;

	UINT1 gridX;
	UINT1 gridY;
	UINT1 gridZ;
	float _dummy0;
};

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
	UINT1 materialIndex;
	UINT1 objectIndex;

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
