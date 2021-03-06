#ifndef JOY_TYPES_H
#define JOY_TYPES_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace JoyEngine
{
	inline glm::vec3 shadowTransformsForward[6]
	{
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(-1.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, -1.0, 0.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, -1.0)
	};

	inline glm::vec3 shadowTransformsUp[6]
	{
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0)
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

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 texCoord;
	};

	struct MVP
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct DirectionLightData
	{
		glm::vec3 direction;
		float intensity;
		float ambient;
	};

	struct LightData
	{
		float intensity;
		float radius;
		float height;
		float angle;

		glm::mat4 view[6];
		glm::mat4 proj;
	};

	struct CubemapConvolutionConstants
	{
		glm::mat4 model;
		glm::mat4 view[6];
		glm::mat4 projection;
	};

	struct JoyData
	{
		glm::vec4 perspectiveValues;
		glm::vec3 cameraWorldPos;
		float time;
		glm::mat4 cameraInvProj;
	};

	struct HDRDownScaleConstants
	{
		// Resolution of the down scaled target: x - width, y - height
		glm::uvec2 Res;
		// Total pixel in the downscaled image
		uint32_t Domain;
		// Number of groups dispached on the first pass
		uint32_t GroupSize;
		float Adaptation; // Adaptation factor
		float fBloomThreshold; // Bloom threshold percentage
	};

	struct SSAOData
	{
		glm::vec4 offsetVectors[14];
		glm::vec2 invRenderTargetSize;
		uint32_t isHorizontal;
		uint32_t _dummy;
		glm::vec4 blurWeights[3];
	};

	struct MipMapGenerationData
	{
		glm::uvec2 TexelSize;
		uint32_t SrcMipLevel;	// Texture level of source mip
		uint32_t NumMipLevels;	// Number of OutMips to write: [1, 4]
	};
}

#endif //JOY_TYPES_H
