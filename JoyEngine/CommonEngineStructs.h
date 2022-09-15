#ifndef COMMON_ENGINE_STRUCTS
#define COMMON_ENGINE_STRUCTS

#ifdef ENGINE
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


struct Vertex
{
	VEC3 pos;
	VEC3 color;
	VEC3 normal;
	VEC2 texCoord;
};

struct MVP
{
	MAT4 model;
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

struct JoyData
{
	VEC4 perspectiveValues;
	VEC3 cameraWorldPos;
	float time;
	MAT4 cameraInvProj;
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

struct SSAOData
{
	VEC4 offsetVectors[14];
	VEC2 invRenderTargetSize;
	UINT1 isHorizontal;
	UINT1 _dummy;
	VEC4 blurWeights[3];
};

struct MipMapGenerationData
{
	UINT2 TexelSize;
	UINT1 SrcMipLevel; // Texture level of source mip
	UINT1 NumMipLevels; // Number of OutMips to write: [1, 4]
};

#endif //COMMON_ENGINE_STRUCTS
