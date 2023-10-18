#include "ComputeDispatcher.h"

#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	ComputeDispatcher::ComputeDispatcher(): m_currentCommandList(nullptr)
	{
		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE, GraphicsManager::Get()->GetDevice());
	}

	ID3D12GraphicsCommandList4* ComputeDispatcher::GetCommandList() noexcept
	{
		m_queue->ResetForFrame();

		m_currentCommandList = m_queue->GetCommandList(0);
		return m_currentCommandList;
	}

	void ComputeDispatcher::ExecuteAndWait() const
	{
		ASSERT_SUCC(m_currentCommandList->Close());

		m_queue->Execute(0);

		m_queue->WaitQueueIdle();
	}
}
