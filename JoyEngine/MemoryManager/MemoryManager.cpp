#include "MemoryManager.h"

#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include "JoyContext.h"


#include "GraphicsManager/GraphicsManager.h"
//#include "ResourceManager/Buffer.h"
//#include "GPUMemoryManager.h"
//#include "Common/Resource.h"
#include "d3dx12.h"
#include "ResourceManager/Buffer.h"
#include "Utils/Assert.h"
//#include "Utils/FileUtils.h"

namespace JoyEngine
{
	void MemoryManager::Init()
	{
		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, JoyContext::Graphics->GetDevice());
	}


	void MemoryManager::LoadDataToImage(
		std::ifstream& stream,
		uint64_t offset,
		uint32_t width,
		uint32_t height,
		ComPtr<ID3D12Resource> gpuImage) const
	{
		const uint64_t imageSize = GetRequiredIntermediateSize(gpuImage.Get(), 0, 1);

		Buffer stagingBuffer = Buffer(imageSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, imageSize);
		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), imageSize);

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = ptr->GetMappedPtr();
		textureData.RowPitch = width * 4; // TODO: WHAT IF TEXTURE COMPRESSED???
		textureData.SlicePitch = textureData.RowPitch * height;

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList();
		UpdateSubresources(
			commandList, 
			gpuImage.Get(), 
			stagingBuffer.GetBuffer().Get(),
			0, 
			0, 
			1, 
			&textureData);
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			gpuImage.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ
		);
		commandList->ResourceBarrier(1, &barrier);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		m_queue->WaitQueueIdle();
	}


	//void MemoryManager::LoadDataToImage(
	//	const unsigned char* data,
	//	uint32_t width,
	//	uint32_t height,
	//	ComPtr<ID3D12Resource> gpuImage)
	//{
	//	//VkDeviceSize imageSize = width * height * 4;
	//	//Buffer stagingBuffer = Buffer(
	//	//	imageSize,
	//	//	VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	//	//std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, imageSize);
	//	//memcpy(ptr->GetMappedPtr(), data, imageSize);
	//	//TransitionImageLayout(
	//	//	gpuImage,
	//	//	VK_FORMAT_R8G8B8A8_SRGB,
	//	//	VK_IMAGE_LAYOUT_UNDEFINED,
	//	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	//	//);
	//	//CopyBufferToImage(stagingBuffer.GetBuffer(), gpuImage, width, height);
	//	//TransitionImageLayout(
	//	//	gpuImage,
	//	//	VK_FORMAT_R8G8B8A8_SRGB,
	//	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	//	//);
	//}

	void MemoryManager::LoadDataToBuffer(
		std::ifstream& stream,
		uint64_t offset,
		uint64_t bufferSize,
		ComPtr<ID3D12Resource> gpuBuffer)
	{
		Buffer stagingBuffer = Buffer(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, bufferSize);
		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), bufferSize);

		D3D12_SUBRESOURCE_DATA bufferData = {};
		bufferData.pData = ptr->GetMappedPtr();
		bufferData.RowPitch = bufferSize;
		bufferData.SlicePitch = bufferSize;

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList();
		UpdateSubresources(
			commandList,
			gpuBuffer.Get(),
			stagingBuffer.GetBuffer().Get(),
			0,
			0,
			1,
			&bufferData);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		m_queue->WaitQueueIdle();
	}
}
