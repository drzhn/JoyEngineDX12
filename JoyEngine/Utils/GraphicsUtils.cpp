#include "GraphicsUtils.h"

#include "CommonEngineStructs.h"
#include "Common/HashDefs.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/ResourceView.h"

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


	void GraphicsUtils::Barrier(
		ID3D12GraphicsCommandList* commandList,
		ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter)
	{
		const D3D12_RESOURCE_BARRIER barrier = Transition(
			pResource,
			stateBefore,
			stateAfter);
		commandList->ResourceBarrier(1, &barrier);
	}

	void GraphicsUtils::UAVBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* pResource)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV = {
			.pResource = pResource
		};
		commandList->ResourceBarrier(1, &barrier);
	}

	void GraphicsUtils::AttachViewToGraphics(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle
	)
	{
		commandList->SetGraphicsRootDescriptorTable(
			rootParameterIndex, gpuHandle);
	}

	void GraphicsUtils::AttachViewToGraphics(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		const ResourceView* view
	)
	{
		commandList->SetGraphicsRootDescriptorTable(
			rootParameterIndex, view->GetGPUHandle());
	}

	void GraphicsUtils::AttachViewToGraphics(
		ID3D12GraphicsCommandList* commandList,
		const ResourceHandle<GraphicsPipeline>& pipeline,
		const char* paramName,
		const ResourceView* view
	)
	{
		commandList->SetGraphicsRootDescriptorTable(
			pipeline->GetBindingIndexByHash(strHash(paramName)), view->GetGPUHandle());
	}

	void GraphicsUtils::AttachViewToGraphics(
		ID3D12GraphicsCommandList* commandList,
		const GraphicsPipeline* pipeline,
		const char* paramName,
		const ResourceView* view
	)
	{
		commandList->SetGraphicsRootDescriptorTable(
			pipeline->GetBindingIndexByHash(strHash(paramName)), view->GetGPUHandle());
	}

	void GraphicsUtils::AttachViewToCompute(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle
	)
	{
		commandList->SetComputeRootDescriptorTable(
			rootParameterIndex, gpuHandle);
	}

	void GraphicsUtils::AttachViewToCompute(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		const ResourceView* view
	)
	{
		commandList->SetComputeRootDescriptorTable(
			rootParameterIndex, view->GetGPUHandle());
	}

	void GraphicsUtils::AttachViewToCompute(
		ID3D12GraphicsCommandList* commandList,
		const ResourceHandle<ComputePipeline>& pipeline,
		const char* paramName,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		commandList->SetComputeRootDescriptorTable(
			pipeline->GetBindingIndexByHash(strHash(paramName)), gpuHandle);
	}


	void GraphicsUtils::AttachViewToCompute(
		ID3D12GraphicsCommandList* commandList,
		const ResourceHandle<ComputePipeline>& pipeline,
		const char* paramName,
		const ResourceView* view)
	{
		commandList->SetComputeRootDescriptorTable(
			pipeline->GetBindingIndexByHash(strHash(paramName)), view->GetGPUHandle());
	}

	void GraphicsUtils::SetViewportAndScissor(
		ID3D12GraphicsCommandList* commandList,
		uint32_t width,
		uint32_t height)
	{
		const D3D12_VIEWPORT viewport = {
			0.0f,
			0.0f,
			static_cast<float>(width),
			static_cast<float>(height),
			D3D12_MIN_DEPTH,
			D3D12_MAX_DEPTH
		};
		const D3D12_RECT scissorRect = {
			0,
			0,
			static_cast<LONG>(width),
			static_cast<LONG>(height)
		};
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
	}

	void GraphicsUtils::ProcessEngineBindings(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		const std::map<uint32_t, EngineBindingType>& bindings,
		const uint32_t* modelIndex,
		const ViewProjectionMatrixData* viewProjectionMatrix
	)
	{
		for (const auto& pair : bindings)
		{
			const auto type = pair.second;
			const auto rootIndex = pair.first;

			switch (type)
			{
			case EngineBindingType::ObjectIndexData:
				{
					ASSERT(modelIndex != nullptr);
					commandList->SetGraphicsRoot32BitConstants(
						rootIndex,
						sizeof(uint32_t) / 4,
						modelIndex,
						0);
					break;
				}
			case EngineBindingType::ModelMatrixData:
				{
					commandList->SetGraphicsRootDescriptorTable(
						rootIndex,
						EngineMaterialProvider::Get()->GetObjectMatricesDataView(frameIndex)->GetGPUHandle()
					);
					break;
				}
			case EngineBindingType::ViewProjectionMatrixData:
				{
					commandList->SetGraphicsRoot32BitConstants(
						rootIndex,
						sizeof(ViewProjectionMatrixData) / 4,
						viewProjectionMatrix,
						0);
					break;
				}
			case EngineBindingType::EngineDataGraphics:
				{
					AttachViewToGraphics(commandList, rootIndex,
					                     EngineMaterialProvider::Get()->GetEngineDataView(frameIndex));
					break;
				}
			case EngineBindingType::EngineDataCompute:
				{
					AttachViewToCompute(commandList, rootIndex,
					                    EngineMaterialProvider::Get()->GetEngineDataView(frameIndex));
					break;
				}
			default:
				ASSERT(false);
			}
		}
	}
}
