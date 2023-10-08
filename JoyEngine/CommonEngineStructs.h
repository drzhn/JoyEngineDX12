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
#define DDGI_WEIGHT_EPSILON 0.0001

#define NUM_CLUSTERS_X 10
#define NUM_CLUSTERS_Y 10
#define NUM_CLUSTERS_Z 24
#define LIGHTS_PER_CLUSTER 32
#define CLUSTER_ITEM_DATA_SIZE 2048 // TODO make additional check in engine

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
#define BINDLESS 0

#endif

// ========= COMMON STRUCTS =========

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};

struct Color
{
	VEC4 data;
};

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
	UINT1 packedColor;
};

struct LightInfo
{
	UINT1 packedColor;
	float radius;
	float intensity;
	UINT1 transformIndex;
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
	float cameraAspect;
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

struct ObjectMatricesData
{
	MAT4 data[OBJECT_SIZE];
};

struct LightData
{
	LightInfo data[LIGHT_SIZE];
};

struct ClusterEntry // TODO WTF
{
	UINT1 offset;
	UINT1 numLight;
	UINT1 _dummy0;
	UINT1 _dummy1;
};

struct ClusterEntryData
{
	ClusterEntry data[NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z];
};

struct ClusterItem
{
	UINT1 lightIndex; // TODO WTF
	UINT1 _dummy0;
	UINT1 _dummy1;
	UINT1 _dummy2;
};

struct ClusterItemData
{
	ClusterItem data[CLUSTER_ITEM_DATA_SIZE];
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
//#define HW_CAMERA_TRACE

struct HardwareRayPayload
{
	VEC4 color;
	VEC4 normals;
	VEC4 position;
};

struct RaytracedProbesData
{
	VEC3 gridMin;
	float cellSize;

	UINT1 gridX;
	UINT1 gridY;
	UINT1 gridZ;
	UINT1 useDDGI;

	UINT1 skyboxTextureIndex;
	UINT1 dummy_0;
	UINT1 dummy_1;
	UINT1 dummy_2;
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

struct TrianglePayload
{
	UINT1 triangleIndex;
	UINT1 meshIndex;
	UINT1 _dummy0;
	UINT1 _dummy1;
};

struct MeshData
{
	UINT1 materialIndex;
	UINT1 verticesIndex;
	UINT1 indicesIndex;
	UINT1 _dummy;
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
