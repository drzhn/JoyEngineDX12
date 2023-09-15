#include "CommandQueue.h"

#include "Utils/Assert.h"

namespace JoyEngine
{
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type, const ComPtr<ID3D12Device5>& device, uint32_t frameCount)
	{
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {
			type,
			0,
			D3D12_COMMAND_QUEUE_FLAG_NONE,
			0
		};
		ASSERT_SUCC(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		m_queueEntries.resize(frameCount);

		for (uint32_t i = 0; i < frameCount; i++)
		{
			ASSERT_SUCC(device->CreateCommandAllocator( type, IID_PPV_ARGS(&m_queueEntries[i].allocator)));
			m_queueEntries[i].fenceValue = 0;

			ASSERT_SUCC(device->CreateCommandList(
				0,
				type,
				m_queueEntries[0].allocator.Get(),
				nullptr,
				IID_PPV_ARGS(&m_queueEntries[i].commandList)));

			ASSERT_SUCC(m_queueEntries[i].commandList->Close());
		}


		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ASSERT_SUCC(device->CreateFence(
				m_currentFenceValue,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&m_fence)));

			m_currentFenceValue++;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
			{
				ASSERT_SUCC(HRESULT_FROM_WIN32(GetLastError()));
			}

			// Wait for the command list to execute; we are reusing the same command 
			// list in our main loop but for now, we just want to wait for setup to 
			// complete before continuing.
			WaitQueueIdle();
		}
	}

	CommandQueue::~CommandQueue()
	{
		WaitQueueIdle();

		CloseHandle(m_fenceEvent);
	}

	// Wait for pending GPU work to complete.
	void CommandQueue::WaitQueueIdle()
	{
		// Schedule a Signal command in the queue.
		ASSERT_SUCC(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));

		// Wait until the fence has been processed.
		ASSERT_SUCC(m_fence->SetEventOnCompletion(m_currentFenceValue, m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		m_currentFenceValue++;
	}

	void CommandQueue::ResetForFrame(uint32_t frameIndex) const
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ASSERT_SUCC(m_queueEntries[frameIndex].allocator->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		ASSERT_SUCC(m_queueEntries[frameIndex].commandList->Reset(m_queueEntries[frameIndex].allocator.Get(), nullptr));
	}

	void CommandQueue::Execute(uint32_t frameIndex) const
	{
		ID3D12CommandList* ppCommandLists[] = {m_queueEntries[frameIndex].commandList.Get()};
		m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

		// Schedule a Signal command in the queue.
		ASSERT_SUCC(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));
	}

	void CommandQueue::WaitForFence(uint32_t frameIndex)
	{
		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_fence->GetCompletedValue() < m_queueEntries[frameIndex].fenceValue)
		{
			ASSERT_SUCC(m_fence->SetEventOnCompletion(m_queueEntries[frameIndex].fenceValue, m_fenceEvent));
			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_currentFenceValue++;

		m_queueEntries[frameIndex].fenceValue = m_currentFenceValue;
	}

	ID3D12GraphicsCommandList4* CommandQueue::GetCommandList(uint32_t frameIndex) const noexcept
	{
		return m_queueEntries[frameIndex].commandList.Get();
	}

	ID3D12CommandQueue* CommandQueue::GetQueue() const noexcept
	{
		return m_commandQueue.Get();
	}
}
