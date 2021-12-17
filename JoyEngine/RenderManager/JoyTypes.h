#ifndef JOY_TYPES_H
#define JOY_TYPES_H

#include <optional>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace JoyEngine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> transferFamily;

		[[nodiscard]] bool isComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 texCoord;

		//static VkVertexInputBindingDescription getBindingDescription()
		//{
		//	VkVertexInputBindingDescription bindingDescription{};
		//	bindingDescription.binding = 0;
		//	bindingDescription.stride = sizeof(Vertex);
		//	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		//	return bindingDescription;
		//}

		//static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
		//{
		//	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		//	attributeDescriptions[0].binding = 0;
		//	attributeDescriptions[0].location = 0;
		//	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		//	attributeDescriptions[0].offset = offsetof(Vertex, pos);

		//	attributeDescriptions[1].binding = 0;
		//	attributeDescriptions[1].location = 1;
		//	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		//	attributeDescriptions[1].offset = offsetof(Vertex, color);

		//	attributeDescriptions[2].binding = 0;
		//	attributeDescriptions[2].location = 2;
		//	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		//	attributeDescriptions[2].offset = offsetof(Vertex, normal);

		//	attributeDescriptions[3].binding = 0;
		//	attributeDescriptions[3].location = 3;
		//	attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		//	attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		//	return attributeDescriptions;
		//}
	};

	struct MVP
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct JoyData
	{
		glm::vec3 cameraWorldPos;
		glm::mat4 cameraProjMatrix;
		float time;
		float deltaTime;
	};
}

#endif //JOY_TYPES_H
