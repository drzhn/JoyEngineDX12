#ifndef JOY_TYPES_H
#define JOY_TYPES_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace JoyEngine
{
	enum LightType
	{
		Point = 0,
		Spot = 1,
		Capsule = 2
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


#pragma pack(push,16)
	struct LightData
	{
		float intensity;
		float radius;
		float height;
		float angle;
		glm::mat4 model;
		glm::mat4 viewProj;
	};
#pragma pack(pop)

	struct JoyData
	{
		glm::vec3 cameraWorldPos;
		glm::mat4 cameraProjMatrix;
		float time;
		float deltaTime;
	};
}

#endif //JOY_TYPES_H
