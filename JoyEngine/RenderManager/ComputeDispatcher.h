#ifndef COMPUTE_DISPATCHER_H
#define COMPUTE_DISPATCHER_H
#include <memory>

#include "Common/CommandQueue.h"

namespace JoyEngine
{
	class ComputeDispatcher
	{
	public:
		ComputeDispatcher();
		[[nodiscard]] ID3D12GraphicsCommandList4* GetCommandList() noexcept;
		void ExecuteAndWait() const;
	private:
		std::unique_ptr<CommandQueue> m_queue;
		ID3D12GraphicsCommandList4* m_currentCommandList;
	};
}
#endif // COMPUTE_DISPATCHER_H
