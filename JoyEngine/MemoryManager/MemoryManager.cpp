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
#include <glm/vec2.hpp>

#include "d3dx12.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "ResourceManager/Texture.h"
#include "Utils/Assert.h"
#include "Utils/DummyMaterialProvider.h"
//#include "Utils/FileUtils.h"

namespace JoyEngine
{
	inline D3D12_RESOURCE_BARRIER Transition(
		_In_ ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = subresource;
		return barrier;
	}

	void AttachView(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		const ResourceView* view
	)
	{
		ID3D12DescriptorHeap* heaps1[1] = {view->GetHeap()};
		commandList->SetDescriptorHeaps(
			1,
			heaps1);
		D3D12_GPU_DESCRIPTOR_HANDLE null = {0};
		commandList->SetComputeRootDescriptorTable(
			rootParameterIndex, view->GetGPUHandle());
	}

	void MemoryManager::Init()
	{
		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, JoyContext::Graphics->GetDevice());
	}

	void MemoryManager::LoadDataToImage(
		std::ifstream& stream,
		uint64_t offset,
		uint32_t width,
		uint32_t height,
		Texture* gpuImage) const
	{
		const uint64_t imageSize = GetRequiredIntermediateSize(gpuImage->GetImage().Get(), 0, 1);

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
			gpuImage->GetImage().Get(),
			stagingBuffer.GetBuffer().Get(),
			0,
			0,
			1,
			&textureData);

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			gpuImage->GetImage().Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ
		);
		commandList->ResourceBarrier(1, &barrier);

		commandList->SetComputeRootSignature(JoyContext::DummyMaterials->GetMipsGenerationComputePipeline()->GetRootSignature().Get());
		commandList->SetPipelineState(JoyContext::DummyMaterials->GetMipsGenerationComputePipeline()->GetPipelineObject().Get());

		std::vector<ResourceView> mipViews;
		for (uint32_t i = 0; i < 4; i++)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = gpuImage->GetFormat();
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D = {
				i + 1,
				0
			};
			mipViews.emplace_back(desc, gpuImage->GetImage().Get());

			AttachView(commandList, i, &mipViews[i]);
		}

		AttachView(commandList, 4, gpuImage->GetResourceView());
		AttachView(commandList, 5, Texture::GetPointSampler());


		commandList->SetComputeRoot32BitConstant(6, width >> 1, 0);
		commandList->SetComputeRoot32BitConstant(6, height >> 1, 1);
		commandList->Dispatch((width >> 1) / 8, (height >> 1) / 8, 1);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		m_queue->WaitQueueIdle();
	}

	void MemoryManager::ChangeResourceState(
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter
	)
	{
		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList();


		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			resource,
			stateBefore,
			stateAfter
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
		Buffer* gpuBuffer)
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

		D3D12_RESOURCE_STATES state = gpuBuffer->GetCurrentResourceState();

		const D3D12_RESOURCE_BARRIER barrierBefore = Transition(
			gpuBuffer->GetBuffer().Get(),
			state,
			D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->ResourceBarrier(1, &barrierBefore);

		UpdateSubresources(
			commandList,
			gpuBuffer->GetBuffer().Get(),
			stagingBuffer.GetBuffer().Get(),
			0,
			0,
			1,
			&bufferData);

		const D3D12_RESOURCE_BARRIER barrierAfter = Transition(
			gpuBuffer->GetBuffer().Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			state);

		commandList->ResourceBarrier(1, &barrierAfter);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		m_queue->WaitQueueIdle();
	}
}
