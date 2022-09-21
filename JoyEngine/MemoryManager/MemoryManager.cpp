#include "MemoryManager.h"

#include <string>
#include <iostream>
#include <fstream>

#include "d3dx12.h"


#include "Common/HashDefs.h"
#include "DescriptorManager/DescriptorManager.h"


#include "GraphicsManager/GraphicsManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "CommonEngineStructs.h"

#include "ResourceManager/Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "ResourceManager/Texture.h"
#include "Utils/Assert.h"
#include "Utils/GraphicsUtils.h"

#define GPU_BUFFER_ALLOCATION_SIZE 256*1024*1024 // 256 MB
#define GPU_TEXTURE_ALLOCATION_SIZE 256*1024*1024 // 256 MB
#define GPU_RT_DS_ALLOCATION_SIZE 32*1024*1024 // 128 MB

#define CPU_UPLOAD_ALLOCATION_SIZE 256*1024*1024 // 64 MB
#define CPU_READBACK_ALLOCATION_SIZE 256*1024*1024 // 64 MB


namespace JoyEngine
{
	uint64_t g_maxResourceSizeAllocated = 0;

	std::string ParseByteNumber(uint64_t bytes)
	{
		if (bytes == 0) return "0 b";
		uint64_t gb = bytes / (1 << 30);
		uint64_t mb = bytes % (1 << 30) / (1 << 20);
		uint64_t kb = bytes % (1 << 20) / (1 << 10);
		uint64_t b = bytes % (1 << 10);
		return
			(gb > 0 ? std::to_string(gb) + " gb " : "") +
			(mb > 0 ? std::to_string(mb) + " mb " : "") +
			(kb > 0 ? std::to_string(kb) + " kb " : "") +
			(b > 0 ? std::to_string(b) + " b" : "");
	}

	std::string ParseAllocatorStats(const DeviceLinearAllocator* allocator)
	{
		uint64_t aligned = allocator->GetAlignedBytesAllocated();
		return "Requested " + ParseByteNumber(aligned) +
			", ratio " + std::to_string(static_cast<float>(aligned) / static_cast<float>(allocator->GetSize()) * 100) + "%\n";
	}

	void AttachView(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		const ResourceView* view
	)
	{
		commandList->SetComputeRootDescriptorTable(
			rootParameterIndex, view->GetGPUHandle());
	}

	IMPLEMENT_SINGLETON(MemoryManager)

	void MemoryManager::Init()
	{
		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeGpuBuffer] = std::make_unique<DeviceLinearAllocator>(
			D3D12_HEAP_TYPE_DEFAULT,
			DeviceAllocatorTypeGpuBuffer,
			GPU_BUFFER_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeTextures] = std::make_unique<DeviceLinearAllocator>(
			D3D12_HEAP_TYPE_DEFAULT,
			DeviceAllocatorTypeTextures,
			GPU_TEXTURE_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeRtDsTextures] = std::make_unique<DeviceLinearAllocator>(
			D3D12_HEAP_TYPE_DEFAULT,
			DeviceAllocatorTypeRtDsTextures,
			GPU_RT_DS_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeCpuUploadBuffer] = std::make_unique<DeviceLinearAllocator>(
			D3D12_HEAP_TYPE_UPLOAD,
			DeviceAllocatorTypeCpuUploadBuffer,
			CPU_UPLOAD_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeCpuReadbackBuffer] = std::make_unique<DeviceLinearAllocator>(
			D3D12_HEAP_TYPE_READBACK,
			DeviceAllocatorTypeCpuReadbackBuffer,
			CPU_READBACK_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_uploadStagingBuffer = std::make_unique<Buffer>(128 * 1024 * 1024, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		m_readbackStagingBuffer = std::make_unique<Buffer>(128 * 1024 * 1024, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK);
	}

	void MemoryManager::PrintStats() const
	{
		OutputDebugStringA(("Biggest resource allocated: " + ParseByteNumber(g_maxResourceSizeAllocated) + "\n").c_str());
		OutputDebugStringA(("GPU buffer allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeGpuBuffer].get())).c_str());
		OutputDebugStringA(("GPU textures allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeTextures].get())).c_str());
		OutputDebugStringA(("GPU RT DS textures allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeRtDsTextures].get())).c_str());
		OutputDebugStringA(("CPU upload buffer allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeCpuUploadBuffer].get())).c_str());
		OutputDebugStringA(("CPU readback buffer allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeCpuReadbackBuffer].get())).c_str());
	}

	void MemoryManager::LoadDataToImage(
		std::ifstream& stream,
		uint64_t offset,
		Texture* gpuImage,
		uint32_t mipMapsCount) const
	{
		uint64_t resourceSize = 0;
		uint64_t rowSize = 0;
		D3D12_RESOURCE_DESC resourceDesc = gpuImage->GetImage().Get()->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		GraphicsManager::Get()->GetDevice()->GetCopyableFootprints(
			&resourceDesc,
			0,
			1,
			0,
			&footprint,
			nullptr,
			&rowSize,
			&resourceSize);

		if (resourceSize > g_maxResourceSizeAllocated)
		{
			g_maxResourceSizeAllocated = resourceSize;
		}


		std::unique_ptr<BufferMappedPtr> ptr = m_uploadStagingBuffer->GetMappedPtr(0, resourceSize);
		stream.clear();

		stream.seekg(offset);
		stream.read(static_cast<char*>(ptr->GetMappedPtr()), resourceSize);

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);


		D3D12_TEXTURE_COPY_LOCATION src = {
			m_uploadStagingBuffer->GetBuffer().Get(),
			D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			{
				footprint
			}
		};


		D3D12_TEXTURE_COPY_LOCATION dst = {
			gpuImage->GetImage().Get(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			0
		};

		commandList->CopyTextureRegion(
			&dst,
			0, 0, 0,
			&src,
			nullptr);

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			gpuImage->GetImage().Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ
		);
		commandList->ResourceBarrier(1, &barrier);

		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);


		if (mipMapsCount > 1)
		{
			std::vector<ResourceView> mipViews(4);

			ResourceHandle<ComputePipeline> mipMapGenerationPipeline = EngineMaterialProvider::Get()->GetMipsGenerationComputePipeline();

			commandList->SetComputeRootSignature(mipMapGenerationPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(mipMapGenerationPipeline->GetPipelineObject().Get());


			AttachView(
				commandList,
				mipMapGenerationPipeline->GetBindingIndexByHash(strHash("SrcMip")),
				gpuImage->GetSRV());

			AttachView(
				commandList,
				mipMapGenerationPipeline->GetBindingIndexByHash(strHash("BilinearClamp")),
				EngineSamplersProvider::GetLinearClampSampler());

			uint32_t dispatchCount = ((mipMapsCount - 1) + 3) / 4;

			for (uint32_t dispatchIndex = 0; dispatchIndex < dispatchCount; dispatchIndex++)
			{
				uint32_t mipLevelsToGenerate = (mipMapsCount - 1) - dispatchIndex * 4;
				if (mipLevelsToGenerate > 4) mipLevelsToGenerate = 4;

				for (uint32_t i = 0; i < mipLevelsToGenerate; i++)
				{
					uint32_t mipIndex = dispatchIndex * 4 + i + 1;
					D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
					desc.Format = gpuImage->GetFormat();
					desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					desc.Texture2D = {
						mipIndex,
						0
					};
					mipViews.emplace(mipViews.begin() + i, desc, gpuImage->GetImage().Get());

					AttachView(
						commandList,
						mipMapGenerationPipeline->GetBindingIndexByName("OutMip" + std::to_string(i + 1)), // I do not sorry
						&mipViews[i]);
				}


				MipMapGenerationData generationData{
					{gpuImage->GetWidth() >> (1 + (dispatchIndex * 4)), gpuImage->GetHeight() >> (1 + (dispatchIndex * 4))},
					dispatchIndex * 4,
					mipLevelsToGenerate
				};

				commandList->SetComputeRoot32BitConstants(
					mipMapGenerationPipeline->GetBindingIndexByHash(strHash("MipMapGenerationData")),
					sizeof(MipMapGenerationData) / 4,
					&generationData,
					0);

				commandList->Dispatch(
					(gpuImage->GetWidth() >> (1 + (dispatchIndex * 4))) / 8,
					(gpuImage->GetHeight() >> (1 + (dispatchIndex * 4))) / 8,
					1);

				barrier = CD3DX12_RESOURCE_BARRIER::UAV(gpuImage->GetImage().Get());
				commandList->ResourceBarrier(1, &barrier);
			}
		}
		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}

	void MemoryManager::ReadbackDataFromBuffer(
		void* ptr, 
		uint64_t bufferSize, 
		const Buffer* gpuBuffer) const
	{
		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);

		const D3D12_RESOURCE_STATES state = gpuBuffer->GetCurrentResourceState();

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBuffer().Get(),
			state,
			D3D12_RESOURCE_STATE_COPY_SOURCE);

		commandList->CopyBufferRegion(
			m_readbackStagingBuffer->GetBuffer().Get(),
			0,
			gpuBuffer->GetBuffer().Get(),
			0,
			bufferSize);

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBuffer().Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			state);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();

		const std::unique_ptr<BufferMappedPtr> bufferMappedPtr = m_readbackStagingBuffer->GetMappedPtr(0, bufferSize);

		memcpy(ptr, bufferMappedPtr->GetMappedPtr(), bufferSize);
	}

	ComPtr<ID3D12Resource> MemoryManager::CreateResource(
		D3D12_HEAP_TYPE heapType,
		const D3D12_RESOURCE_DESC* resourceDesc,
		D3D12_RESOURCE_STATES initialResourceState,
		const D3D12_CLEAR_VALUE* clearValue) const
	{
		ComPtr<ID3D12Resource> resource;

		const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = GraphicsManager::Get()->GetDevice()->GetResourceAllocationInfo(
			0, 1, resourceDesc);


		DeviceLinearAllocator* allocator = nullptr;

		if (heapType == D3D12_HEAP_TYPE_DEFAULT)
		{
			if (resourceDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				allocator = m_allocators[DeviceAllocatorTypeGpuBuffer].get();
			}
			else
			{
				if (resourceDesc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ||
					resourceDesc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				{
					allocator = m_allocators[DeviceAllocatorTypeRtDsTextures].get();
				}
				else
				{
					allocator = m_allocators[DeviceAllocatorTypeTextures].get();
				}
			}
		}
		else if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		{
			allocator = m_allocators[DeviceAllocatorTypeCpuUploadBuffer].get();
		}
		else if (heapType == D3D12_HEAP_TYPE_READBACK)
		{
			allocator = m_allocators[DeviceAllocatorTypeCpuReadbackBuffer].get();
		}
		else
		{
			ASSERT(false);
		}

		ASSERT_SUCC(GraphicsManager::Get()->GetDevice()->CreatePlacedResource(
			allocator->GetHeap(),
			allocator->Allocate(allocationInfo.SizeInBytes),
			resourceDesc,
			initialResourceState,
			clearValue,
			IID_PPV_ARGS(&resource)
		));

		return resource;
	}
	

	void MemoryManager::LoadDataToBuffer(
		std::ifstream& stream,
		uint64_t offset,
		uint64_t bufferSize,
		const Buffer* gpuBuffer) const
	{
		const std::unique_ptr<BufferMappedPtr> bufferMappedPtr = m_uploadStagingBuffer->GetMappedPtr(0, bufferSize);
		stream.clear();
		stream.seekg(offset);
		stream.read(static_cast<char*>(bufferMappedPtr->GetMappedPtr()), bufferSize);

		LoadDataToBufferInternal(bufferSize, gpuBuffer);
	}

	void MemoryManager::LoadDataToBuffer(
		void* ptr,
		uint64_t bufferSize,
		const Buffer* gpuBuffer) const
	{
		const std::unique_ptr<BufferMappedPtr> bufferMappedPtr = m_uploadStagingBuffer->GetMappedPtr(0, bufferSize);

		memcpy(bufferMappedPtr->GetMappedPtr(), ptr, bufferSize);

		LoadDataToBufferInternal(bufferSize, gpuBuffer);
	}

	void MemoryManager::LoadDataToBufferInternal(
		uint64_t bufferSize,
		const Buffer* gpuBuffer) const
	{
		if (bufferSize > g_maxResourceSizeAllocated)
		{
			g_maxResourceSizeAllocated = bufferSize;
		}

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);

		const D3D12_RESOURCE_STATES state = gpuBuffer->GetCurrentResourceState();

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBuffer().Get(),
		                       state,
		                       D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->CopyBufferRegion(
			gpuBuffer->GetBuffer().Get(),
			0,
			m_uploadStagingBuffer->GetBuffer().Get(),
			0,
			bufferSize);

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBuffer().Get(),
		                       D3D12_RESOURCE_STATE_COPY_DEST,
		                       state);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}
}
