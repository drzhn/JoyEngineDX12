#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include <vector>

#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class CommandQueue
	{
	public:
		CommandQueue() = delete;
		CommandQueue(D3D12_COMMAND_LIST_TYPE type, const ComPtr<ID3D12Device2>& device, uint32_t frameCount = 1);
		~CommandQueue();

		void WaitQueueIdle();
		void ResetForFrame(uint32_t frameIndex = 1) const;
		void Execute() const;
		void WaitForFence(uint32_t frameIndex = 1);

		[[nodiscard]] ID3D12CommandQueue* GetQueue() const noexcept { return m_commandQueue.Get(); }
		[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const noexcept { return m_commandList.Get(); }
	private:
		struct QueueAllocatorEntry
		{
			ComPtr<ID3D12CommandAllocator> allocator = nullptr;
			uint64_t fenceValue = 0;
		};

		std::vector<QueueAllocatorEntry> m_queueEntries;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12Fence> m_fence;

		uint32_t m_currentFenceValue = 0;
		HANDLE m_fenceEvent;
	};
}

#endif // COMMAND_QUEUE_H
