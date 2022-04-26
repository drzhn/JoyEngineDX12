#include "MemoryManager.h"

#include <string>
#include <iostream>
#include <fstream>

#include "d3dx12.h"


#include "JoyContext.h"


#include "GraphicsManager/GraphicsManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"

#include "ResourceManager/Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "ResourceManager/Texture.h"
#include "Utils/Assert.h"

#define GPU_BUFFER_ALLOCATION_SIZE 128*1024*1024 // 128 MB
#define GPU_TEXTURE_ALLOCATION_SIZE 256*1024*1024 // 256 MB
#define GPU_RT_DS_ALLOCATION_SIZE 128*1024*1024 // 128 MB

#define CPU_BUFFER_ALLOCATION_SIZE 64*1024*1024 // 64 MB


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
		//ID3D12DescriptorHeap* heaps1[1] = {view->GetHeap()};
		//commandList->SetDescriptorHeaps(
		//	1,
		//	heaps1);
		//D3D12_GPU_DESCRIPTOR_HANDLE null = {0};
		//commandList->SetComputeRootDescriptorTable(
		//	rootParameterIndex, view->GetGPUHandle());
	}

	void MemoryManager::Init()
	{
		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, JoyContext::Graphics->GetDevice());

		D3D12_HEAP_FLAGS flagBuffer =
			D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES |
			D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
		D3D12_HEAP_FLAGS flagTexture =
			D3D12_HEAP_FLAG_DENY_BUFFERS |
			D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
		D3D12_HEAP_FLAGS flagRT_DS =
			D3D12_HEAP_FLAG_DENY_BUFFERS |
			D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

		m_gpuHeapAllocators.insert(
			{
				flagBuffer,
				std::make_unique<DeviceLinearAllocator>(
					D3D12_HEAP_TYPE_DEFAULT,
					flagBuffer,
					GPU_BUFFER_ALLOCATION_SIZE,
					JoyContext::Graphics->GetDevice())
			});
		m_gpuHeapAllocators.insert({
			flagTexture,
			std::make_unique<DeviceLinearAllocator>(
				D3D12_HEAP_TYPE_DEFAULT,
				flagTexture,
				GPU_TEXTURE_ALLOCATION_SIZE,
				JoyContext::Graphics->GetDevice())
		});
		m_gpuHeapAllocators.insert({
			flagRT_DS,
			std::make_unique<DeviceLinearAllocator>(
				D3D12_HEAP_TYPE_DEFAULT,
				flagRT_DS,
				GPU_RT_DS_ALLOCATION_SIZE,
				JoyContext::Graphics->GetDevice())
		});


		m_cpuHeapAllocators.insert({
			flagBuffer,
			std::make_unique<DeviceLinearAllocator>(
				D3D12_HEAP_TYPE_UPLOAD,
				flagBuffer,
				CPU_BUFFER_ALLOCATION_SIZE,
				JoyContext::Graphics->GetDevice())
		});
	}

	void MemoryManager::LoadDataToImage(
		std::ifstream& stream,
		uint64_t offset,
		uint32_t RowPitch,
		uint32_t SlicePitch,
		uint32_t width,
		uint32_t height,
		Texture* gpuImage,
		uint32_t mipMapsCount) const
	{
		const uint64_t imageSize = GetRequiredIntermediateSize(gpuImage->GetImage().Get(), 0, 1);

		Buffer stagingBuffer = Buffer(imageSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, imageSize);
		stream.clear();

		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), imageSize);

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = ptr->GetMappedPtr();
		textureData.RowPitch = RowPitch; //width * 4;
		textureData.SlicePitch = SlicePitch; //textureData.RowPitch * height;

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);

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

		std::vector<ResourceView> mipViews;

		//if (mipMapsCount > 1)
		//{
		//	commandList->SetComputeRootSignature(JoyContext::EngineMaterials->GetMipsGenerationComputePipeline()->GetRootSignature().Get());
		//	commandList->SetPipelineState(JoyContext::EngineMaterials->GetMipsGenerationComputePipeline()->GetPipelineObject().Get());

		//	for (uint32_t i = 0; i < 4; i++)
		//	{
		//		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		//		desc.Format = gpuImage->GetFormat();
		//		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		//		desc.Texture2D = {
		//			i + 1,
		//			0
		//		};
		//		mipViews.emplace_back(desc, gpuImage->GetImage().Get());

		//		AttachView(commandList, i, &mipViews[i]);
		//	}

		//	AttachView(commandList, 4, gpuImage->GetResourceView());
		//	AttachView(commandList, 5, Texture::GetPointSampler());


		//	commandList->SetComputeRoot32BitConstant(6, width >> 1, 0);
		//	commandList->SetComputeRoot32BitConstant(6, height >> 1, 1);
		//	commandList->Dispatch((width >> 1) / 8, (height >> 1) / 8, 1);
		//}
		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}

	//ComPtr<ID3D12Resource> MemoryManager::CreateResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE* clearValue)
	//{
	//}

	void MemoryManager::ChangeResourceState(
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter
	)
	{
		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);


		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			resource,
			stateBefore,
			stateAfter
		);
		commandList->ResourceBarrier(1, &barrier);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}


	void MemoryManager::LoadDataToBuffer(
		std::ifstream& stream,
		uint64_t offset,
		uint64_t bufferSize,
		Buffer* gpuBuffer) const
	{
		Buffer stagingBuffer = Buffer(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

		std::unique_ptr<BufferMappedPtr> ptr = stagingBuffer.GetMappedPtr(0, bufferSize);
		stream.clear();
		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), bufferSize);

		D3D12_SUBRESOURCE_DATA bufferData = {};
		bufferData.pData = ptr->GetMappedPtr();
		bufferData.RowPitch = bufferSize;
		bufferData.SlicePitch = bufferSize;

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);

		D3D12_RESOURCE_STATES state = gpuBuffer->GetCurrentResourceState();

		const D3D12_RESOURCE_BARRIER barrierBefore = Transition(
			gpuBuffer->GetBuffer().Get(),
			state,
			D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->ResourceBarrier(1, &barrierBefore);

		commandList->CopyBufferRegion(
			gpuBuffer->GetBuffer().Get(),
			0,
			stagingBuffer.GetBuffer().Get(),
			0,
			bufferSize);

		const D3D12_RESOURCE_BARRIER barrierAfter = Transition(
			gpuBuffer->GetBuffer().Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			state);

		commandList->ResourceBarrier(1, &barrierAfter);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}
}
