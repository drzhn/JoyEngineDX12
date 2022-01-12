#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <fstream>

#include "Common/CommandQueue.h"

namespace JoyEngine
{
	class JoyEngine;

	class MemoryManager
	{
	public:
		MemoryManager() = default;

		void Init();

		void Update();

		void LoadDataToBuffer(std::ifstream& stream, uint64_t offset, uint64_t bufferSize, ComPtr<ID3D12Resource> gpuBuffer);

		void LoadDataToImage(
			const unsigned char* data,
			uint32_t width,
			uint32_t height,
			ComPtr<ID3D12Resource> gpuImage);

		void LoadDataToImage(
			std::ifstream& stream,
			uint64_t offset,
			uint32_t width,
			uint32_t height,
			ComPtr<ID3D12Resource> gpuImage) const;

	private:
		//void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		//VkCommandBuffer BeginSingleTimeCommands();

		//void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


		//void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		//void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	private:
		std::unique_ptr<CommandQueue> m_queue;

	};
}

#endif
