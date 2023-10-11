#include "MemoryManager.h"

#include <string>
#include <iostream>
#include <fstream>

#include "d3dx12.h"


#include "Common/HashDefs.h"
#include "DescriptorManager/DescriptorManager.h"


#include "GraphicsManager/GraphicsManager.h"
#include "EngineDataProvider/EngineDataProvider.h"

#include "ResourceManager/Texture.h"
#include "Utils/Assert.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

#define GPU_BUFFER_ALLOCATION_SIZE (256*1024*1024) // 256 MB
#define GPU_TEXTURE_ALLOCATION_SIZE (128*1024*1024) // 128 MB
#define GPU_RT_DS_ALLOCATION_SIZE (128*1024*1024) // 128 MB

#define CPU_UPLOAD_ALLOCATION_SIZE (256*1024*1024) // 256 MB
#define CPU_READBACK_ALLOCATION_SIZE (256*1024*1024) // 256 MB


namespace JoyEngine
{
	uint64_t g_debugMaxResourceSizeAllocated = 0;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT g_subresourceFootprints[24];

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

	std::string ParseAllocatorStats(const LinearMemoryAllocator* allocator)
	{
		uint64_t aligned = allocator->GetAlignedBytesAllocated();
		return "Requested " + ParseByteNumber(aligned) +
			", used " + std::to_string(static_cast<float>(aligned) / static_cast<float>(allocator->GetSize()) * 100) + "%\n";
	}

	void MemoryManager::Init()
	{
		TIME_PERF("MemoryManager init")

		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeGpuBuffer] = std::make_unique<LinearMemoryAllocator>(
			D3D12_HEAP_TYPE_DEFAULT,
			DeviceAllocatorTypeGpuBuffer,
			GPU_BUFFER_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeTextures] = std::make_unique<LinearMemoryAllocator>(
			D3D12_HEAP_TYPE_DEFAULT,
			DeviceAllocatorTypeTextures,
			GPU_TEXTURE_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeRtDsTextures] = std::make_unique<LinearMemoryAllocator>(
			D3D12_HEAP_TYPE_DEFAULT,
			DeviceAllocatorTypeRtDsTextures,
			GPU_RT_DS_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeCpuUploadBuffer] = std::make_unique<LinearMemoryAllocator>(
			D3D12_HEAP_TYPE_UPLOAD,
			DeviceAllocatorTypeCpuUploadBuffer,
			CPU_UPLOAD_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_allocators[DeviceAllocatorTypeCpuReadbackBuffer] = std::make_unique<LinearMemoryAllocator>(
			D3D12_HEAP_TYPE_READBACK,
			DeviceAllocatorTypeCpuReadbackBuffer,
			CPU_READBACK_ALLOCATION_SIZE,
			GraphicsManager::Get()->GetDevice());

		m_uploadStagingBuffer = std::make_unique<Buffer>(128 * 1024 * 1024, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		m_readbackStagingBuffer = std::make_unique<Buffer>(128 * 1024 * 1024, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK);
	}

	void MemoryManager::PrintStats() const
	{
		Logger::Log(("Biggest resource allocated: " + ParseByteNumber(g_debugMaxResourceSizeAllocated) + "\n").c_str());
		Logger::Log(("GPU buffer allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeGpuBuffer].get())).c_str());
		Logger::Log(("GPU textures allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeTextures].get())).c_str());
		Logger::Log(("GPU RT DS textures allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeRtDsTextures].get())).c_str());
		Logger::Log(("CPU upload buffer allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeCpuUploadBuffer].get())).c_str());
		Logger::Log(("CPU readback buffer allocator: " + ParseAllocatorStats(m_allocators[DeviceAllocatorTypeCpuReadbackBuffer].get())).c_str());
	}

	void MemoryManager::LoadDataToImage(
		std::ifstream& stream,
		uint64_t offset,
		Texture* gpuImage) const
	{
		uint64_t resourceSize;
		D3D12_RESOURCE_DESC resourceDesc = gpuImage->GetImageResource().Get()->GetDesc();
		GraphicsManager::Get()->GetDevice()->GetCopyableFootprints(
			&resourceDesc,
			0,
			resourceDesc.MipLevels,
			0,
			g_subresourceFootprints,
			nullptr,
			nullptr,
			&resourceSize);
		if (resourceSize > g_debugMaxResourceSizeAllocated)
		{
			g_debugMaxResourceSizeAllocated = resourceSize;
		}

		uint32_t bytesPer4x4Block = 16;

		switch (resourceDesc.Format)
		{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC1_TYPELESS:
			bytesPer4x4Block = 8;
			break;

		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC6H_TYPELESS:
			bytesPer4x4Block = 16;
			break;
		default:
			ASSERT(false); // TODO later maybe never
		}

		const MappedAreaHandle ptr = m_uploadStagingBuffer->Map();
		stream.clear();

		stream.seekg(offset);

		for (uint32_t i = 0; i < resourceDesc.MipLevels; i++)
		{
			uint64_t mipDataOffset = g_subresourceFootprints[i].Offset;

			for (uint32_t y = 0; y < g_subresourceFootprints[i].Footprint.Height / 4; y++)
			{
				uint32_t rowSize = g_subresourceFootprints[i].Footprint.Width / 4 * bytesPer4x4Block;
				stream.read(static_cast<char*>(ptr.GetPtr()) + mipDataOffset, rowSize);
				mipDataOffset += std::max(rowSize, g_subresourceFootprints[i].Footprint.RowPitch);
			}
		}

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);

		for (uint32_t i = 0; i < resourceDesc.MipLevels; i++)
		{
			D3D12_TEXTURE_COPY_LOCATION src = {
				m_uploadStagingBuffer->GetBufferResource().Get(),
				D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
				{
					g_subresourceFootprints[i]
				}
			};


			D3D12_TEXTURE_COPY_LOCATION dst = {
				gpuImage->GetImageResource().Get(),
				D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
				i
			};

			commandList->CopyTextureRegion(
				&dst,
				0, 0, 0,
				&src,
				nullptr);
		}


		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			gpuImage->GetImageResource().Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ
		);
		commandList->ResourceBarrier(1, &barrier);

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

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBufferResource().Get(),
		                       state,
		                       D3D12_RESOURCE_STATE_COPY_SOURCE);

		commandList->CopyBufferRegion(
			m_readbackStagingBuffer->GetBufferResource().Get(),
			0,
			gpuBuffer->GetBufferResource().Get(),
			0,
			bufferSize);

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBufferResource().Get(),
		                       D3D12_RESOURCE_STATE_COPY_SOURCE,
		                       state);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();

		const MappedAreaHandle bufferMappedPtr = m_readbackStagingBuffer->Map();

		memcpy(ptr, bufferMappedPtr.GetPtr(), bufferSize);
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


		LinearMemoryAllocator* allocator = nullptr;

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
		const MappedAreaHandle bufferMappedPtr = m_uploadStagingBuffer->Map();
		stream.clear();
		stream.seekg(offset);
		stream.read(static_cast<char*>(bufferMappedPtr.GetPtr()), bufferSize);

		LoadDataToBufferInternal(bufferSize, gpuBuffer, 0);
	}

	void MemoryManager::LoadDataToBuffer(
		void* ptr,
		uint64_t bufferSize,
		const Buffer* gpuBuffer,
		uint64_t bufferOffset) const
	{
		const MappedAreaHandle bufferMappedPtr = m_uploadStagingBuffer->Map();

		memcpy(bufferMappedPtr.GetPtr(), ptr, bufferSize);

		LoadDataToBufferInternal(bufferSize, gpuBuffer, bufferOffset);
	}

	void MemoryManager::LoadDataToBufferInternal(
		uint64_t bufferSize,
		const Buffer* gpuBuffer,
		uint64_t bufferOffset) const
	{
		if (bufferSize > g_debugMaxResourceSizeAllocated)
		{
			g_debugMaxResourceSizeAllocated = bufferSize;
		}

		m_queue->ResetForFrame();

		const auto commandList = m_queue->GetCommandList(0);

		const D3D12_RESOURCE_STATES state = gpuBuffer->GetCurrentResourceState();

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBufferResource().Get(),
		                       state,
		                       D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->CopyBufferRegion(
			gpuBuffer->GetBufferResource().Get(),
			bufferOffset,
			m_uploadStagingBuffer->GetBufferResource().Get(),
			0,
			bufferSize);

		GraphicsUtils::Barrier(commandList, gpuBuffer->GetBufferResource().Get(),
		                       D3D12_RESOURCE_STATE_COPY_DEST,
		                       state);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}
}
