#include "RenderManager.h"

#include <memory>

#include "JoyContext.h"
#include "Utils/Assert.h"

//#include "MemoryManager/MemoryManager.h"
//#include "ResourceManager/ResourceManager.h"
#include "JoyTypes.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/DummyMaterialProvider.h"

#define GLM_FORCE_RADIANS

namespace JoyEngine
{
	void RenderManager::Init()
	{
		RECT rect;

		if (GetWindowRect(JoyContext::Graphics->GetHWND(), &rect))
		{
			m_width = rect.right - rect.left;
			m_height = rect.bottom - rect.top;
		}

		ASSERT(m_width != 0 && m_height != 0);

		m_viewport = {
			0.0f,
			0.0f,
			static_cast<float>(m_width),
			static_cast<float>(m_height),
			D3D12_MIN_DEPTH,
			D3D12_MAX_DEPTH
		};

		m_scissorRect = {
			0,
			0,
			static_cast<LONG>(m_width),
			static_cast<LONG>(m_height)
		};

		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, JoyContext::Graphics->GetDevice(),
		                                         FrameCount);

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ASSERT_SUCC(JoyContext::Graphics->GetFactory()->CreateSwapChainForHwnd(
			m_queue->GetQueue(), // Swap chain needs the queue so that it can force a flush on it.
			JoyContext::Graphics->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// This sample does not support fullscreen transitions.
		ASSERT_SUCC(JoyContext::Graphics->GetFactory()->MakeWindowAssociation(JoyContext::Graphics->GetHWND(), DXGI_MWA_NO_ALT_ENTER));
		ASSERT_SUCC(swapChain.As(&m_swapChain));

		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ComPtr<ID3D12Resource> swapchainResource;
			ASSERT_SUCC(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&swapchainResource)));

			m_renderTargets[n] = std::make_unique<Texture>(
				swapchainResource,
				m_width,
				m_height,
				DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_HEAP_TYPE_DEFAULT);
		}

		m_depthAttachment = std::make_unique<Texture>(
			m_width, m_height,
			DXGI_FORMAT_D32_FLOAT,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_HEAP_TYPE_DEFAULT);

		m_positionAttachment = std::make_unique<RenderTexture>(
			m_width, m_height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT);

		m_normalAttachment = std::make_unique<RenderTexture>(
			m_width, m_height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT);
	}


	void RenderManager::Stop()
	{
		m_queue = nullptr;
	}

	void RenderManager::RegisterSharedMaterial(SharedMaterial* meshRenderer)
	{
		m_sharedMaterials.insert(meshRenderer);
	}

	void RenderManager::UnregisterSharedMaterial(SharedMaterial* meshRenderer)
	{
		if (m_sharedMaterials.find(meshRenderer) == m_sharedMaterials.end())
		{
			ASSERT(false);
		}
		m_sharedMaterials.erase(meshRenderer);
	}

	void RenderManager::RegisterCamera(Camera* camera)
	{
		m_currentCamera = camera;
		//m_commonDescriptorSetProvider->SetCamera(camera);
	}

	void RenderManager::UnregisterCamera(Camera* camera)
	{
		ASSERT(m_currentCamera == camera);
		m_currentCamera = nullptr;
	}

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

	void RenderManager::Update()
	{
		m_queue->ResetForFrame(m_currentFrameIndex);

		const auto commandList = m_queue->GetCommandList();
		auto dsvHandle = m_depthAttachment->GetResourceView()->GetHandle();
		auto rtvHandle = m_renderTargets[m_currentFrameIndex]->GetResourceView()->GetHandle();

		auto positionHandle = m_positionAttachment->GetResourceView()->GetHandle();
		auto normalHandle = m_normalAttachment->GetResourceView()->GetHandle();

		// Set necessary state.
		commandList->RSSetViewports(1, &m_viewport);
		commandList->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		D3D12_RESOURCE_BARRIER barrier1 = Transition(
			m_renderTargets[m_currentFrameIndex]->GetImage().Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier1);

		ASSERT(m_currentCamera != nullptr);
		glm::mat4 view = m_currentCamera->GetViewMatrix();
		glm::mat4 proj = m_currentCamera->GetProjMatrix();

		{
			// Drawing GBUFFER textures

			D3D12_CPU_DESCRIPTOR_HANDLE gbufferHandles[] = {positionHandle, normalHandle};
			commandList->OMSetRenderTargets(
				2,
				gbufferHandles,
				FALSE,
				&dsvHandle);

			const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
			commandList->ClearRenderTargetView(positionHandle, clearColor, 0, nullptr);
			commandList->ClearRenderTargetView(normalHandle, clearColor, 0, nullptr);
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			auto sm = JoyContext::DummyMaterials->GetGBufferSharedMaterial();

			for (auto const& s : m_sharedMaterials)
			{
				commandList->SetPipelineState(sm->GetPipelineObject().Get());
				commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
				for (const auto& mr : s->GetMeshRenderers())
				{
					//commandList->SetDescriptorHeaps(
					//	mr->GetMaterial()->GetHeaps().size(),
					//	mr->GetMaterial()->GetHeaps().data());

					commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
					commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

					MVP mvp{
						(mr->GetTransform()->GetModelMatrix()),
						(view),
						(proj)
					};
					//for (auto param : mr->GetMaterial()->GetRootParams())
					//{
					//	uint32_t index = param.first;
					//	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = param.second->GetGPUDescriptorHandleForHeapStart();

					//	commandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
					//}
					commandList->SetGraphicsRoot32BitConstants(0, sizeof(MVP) / 4, &mvp, 0);
					commandList->DrawIndexedInstanced(
						mr->GetMesh()->GetIndexSize(),
						1,
						0, 0, 0);
				}
			}
		}

		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER positionToReadBarrier = Transition(
			m_positionAttachment->GetImage().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &positionToReadBarrier);

		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER normalToReadBarrier = Transition(
			m_normalAttachment->GetImage().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &normalToReadBarrier);


		{
			// Drawing main color

			commandList->OMSetRenderTargets(
				1,
				&rtvHandle,
				FALSE, &dsvHandle);

			const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			for (auto const& sm : m_sharedMaterials)
			{
				commandList->SetPipelineState(sm->GetPipelineObject().Get());
				commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
				for (const auto& mr : sm->GetMeshRenderers())
				{
					commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
					commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

					MVP mvp{
						mr->GetTransform()->GetModelMatrix(),
						view,
						proj
					};
					for (auto param : mr->GetMaterial()->GetRootParams())
					{
						uint32_t index = param.first;
						D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = param.second->GetGPUDescriptorHandleForHeapStart();
						ID3D12DescriptorHeap* heaps[1] = {param.second};
						commandList->SetDescriptorHeaps(1, heaps);
						commandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
					}

					ID3D12DescriptorHeap* heaps1[1] = {m_positionAttachment->GetAttachmentView()->GetHeap()};
					commandList->SetDescriptorHeaps(
						1,
						heaps1);
					commandList->SetGraphicsRootDescriptorTable(3, m_positionAttachment->GetAttachmentView()->GetGPUHandle());

					ID3D12DescriptorHeap* heaps2[1] = {m_normalAttachment->GetAttachmentView()->GetHeap()};
					commandList->SetDescriptorHeaps(
						1,
						heaps2);
					commandList->SetGraphicsRootDescriptorTable(4, m_normalAttachment->GetAttachmentView()->GetGPUHandle());

					commandList->SetGraphicsRoot32BitConstants(2, sizeof(MVP) / 4, &mvp, 0);
					commandList->DrawIndexedInstanced(
						mr->GetMesh()->GetIndexSize(),
						1,
						0, 0, 0);
				}
			}
		}

		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER barrier2 = Transition(
			m_renderTargets[m_currentFrameIndex]->GetImage().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier2);


		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER positionToRenderBarrier = Transition(
			m_positionAttachment->GetImage().Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &positionToRenderBarrier);

		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER normalToRenderBarrier = Transition(
			m_normalAttachment->GetImage().Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &normalToRenderBarrier);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		// Present the frame.
		ASSERT_SUCC(m_swapChain->Present(0, 0));

		// Update the frame index.
		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);
	}


	float RenderManager::GetAspect() const noexcept
	{
		return static_cast<float>(m_width) / static_cast<float>(m_height);
	}

	float RenderManager::GetWidth() const noexcept
	{
		return static_cast<float>(m_width);
	}

	float RenderManager::GetHeight() const noexcept
	{
		return static_cast<float>(m_height);
	}
}
