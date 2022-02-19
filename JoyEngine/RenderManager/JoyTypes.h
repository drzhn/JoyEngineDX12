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

	struct JoyData
	{
		glm::vec4 perspectiveValues;
		glm::vec3 cameraWorldPos;
		float time;
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
	};

}

#endif //JOY_TYPES_H
