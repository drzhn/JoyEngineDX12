#include "RenderManager.h"

#include <memory>

#include "JoyContext.h"
#include "Utils/Assert.h"

//#include "MemoryManager/MemoryManager.h"
//#include "ResourceManager/ResourceManager.h"
#include "JoyTypes.h"
#include "Components/Camera.h"
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
			D3D12_HEAP_TYPE_DEFAULT,
			false,
			true);

		m_positionAttachment = std::make_unique<RenderTexture>(
			m_width, m_height,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT);

		m_normalAttachment = std::make_unique<RenderTexture>(
			m_width, m_height,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT);

		m_lightingAttachment = std::make_unique<RenderTexture>(
			m_width, m_height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT);

		m_planeMesh = GUID::StringToGuid("7489a35d-1173-48cd-9ad0-606f13c33319");
	}


	void RenderManager::Stop()
	{
		m_queue = nullptr;
	}

	void RenderManager::RegisterSharedMaterial(SharedMaterial* sm)
	{
		m_sharedMaterials.insert(sm);
	}

	void RenderManager::UnregisterSharedMaterial(SharedMaterial* sm)
	{
		if (m_sharedMaterials.find(sm) == m_sharedMaterials.end())
		{
			ASSERT(false);
		}
		m_sharedMaterials.erase(sm);
	}

	void RenderManager::RegisterLight(Light* light)
	{
		ASSERT(light->GetLightType() != Direction);
		m_lights.insert(light);
	}

	void RenderManager::UnregisterLight(Light* light)
	{
		if (m_lights.find(light) == m_lights.end())
		{
			ASSERT(false);
		}
		m_lights.erase(light);
	}

	void RenderManager::RegisterDirectionLight(Light* light)
	{
		ASSERT(light->GetLightType() == Direction);
		m_directionLight = light;
	}

	void RenderManager::UnregisterDirectionLight(Light* light)
	{
		ASSERT(m_directionLight == light);
		m_directionLight = nullptr;
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
		auto lightHandle = m_lightingAttachment->GetResourceView()->GetHandle();

		// Set necessary state.
		SetViewportAndScissor(commandList, m_width, m_height);

		// Indicate that the back buffer will be used as a render target.
		D3D12_RESOURCE_BARRIER barrier1 = Transition(
			m_renderTargets[m_currentFrameIndex]->GetImage().Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier1);

		ASSERT(m_currentCamera != nullptr);
		const glm::mat4 mainCameraViewMatrix = m_currentCamera->GetViewMatrix();
		const glm::mat4 mainCameraProjMatrix = m_currentCamera->GetProjMatrix();

		// Drawing GBUFFER textures
		{
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
			commandList->SetPipelineState(sm->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());

			RenderEntireScene(commandList, mainCameraViewMatrix, mainCameraProjMatrix);
		}

		// Transition normal and position texture to generic read state
		{
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
		}


		//Light processing
		{
			// shadow maps generation

			auto sm = JoyContext::DummyMaterials->GetShadowProcessingSharedMaterial();

			commandList->SetPipelineState(sm->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			for (const auto& light : m_lights)
			{
				if (light->GetShadowmap() == nullptr) continue;

				D3D12_RESOURCE_BARRIER depthToDSVBarrier = Transition(
					light->GetShadowmap()->GetImage().Get(),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					D3D12_RESOURCE_STATE_DEPTH_WRITE);
				commandList->ResourceBarrier(1, &depthToDSVBarrier);

				auto shadowmapHandle = light->GetShadowmap()->GetResourceView()->GetHandle();

				SetViewportAndScissor(commandList, light->GetShadowmap()->GetWidth(), light->GetShadowmap()->GetHeight());

				commandList->OMSetRenderTargets(
					0,
					nullptr,
					FALSE,
					&shadowmapHandle);

				commandList->ClearDepthStencilView(shadowmapHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

				RenderEntireScene(commandList, light->GetViewMatrix(), light->GetProjMatrix());

				D3D12_RESOURCE_BARRIER depthToSrvBarrier = Transition(
					light->GetShadowmap()->GetImage().Get(),
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					D3D12_RESOURCE_STATE_GENERIC_READ);
				commandList->ResourceBarrier(1, &depthToSrvBarrier);
			}

			SetViewportAndScissor(commandList, m_width, m_height);

			commandList->OMSetRenderTargets(
				1,
				&lightHandle,
				FALSE,
				&dsvHandle);

			const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
			commandList->ClearRenderTargetView(lightHandle, clearColor, 0, nullptr);

			// Direction light
			if (m_directionLight != nullptr)
			{
				auto sm = JoyContext::DummyMaterials->GetDirectionLightProcessingSharedMaterial();

				commandList->SetPipelineState(sm->GetPipelineObject().Get());
				commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				ID3D12DescriptorHeap* descriptorHeap[1] = {m_normalAttachment->GetAttachmentView()->GetHeap()};
				commandList->SetDescriptorHeaps(
					1,
					descriptorHeap);
				commandList->SetGraphicsRootDescriptorTable(1, m_normalAttachment->GetAttachmentView()->GetGPUHandle());

				DirectionLightData lightData = {
					m_directionLight->GetTransform()->GetForward(),
					m_directionLight->GetIntensity(),
					m_directionLight->GetAmbient()
				};

				commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectionLightData) / 4, &lightData, 0);
				commandList->DrawInstanced(
					3,
					1,
					0, 0);
			}

			// Other lights
			{
				auto sm = JoyContext::DummyMaterials->GetLightProcessingSharedMaterial();

				commandList->SetPipelineState(sm->GetPipelineObject().Get());
				commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());

				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				commandList->IASetVertexBuffers(0, 1, m_planeMesh->GetVertexBufferView());
				commandList->IASetIndexBuffer(m_planeMesh->GetIndexBufferView());

				for (const auto& light : m_lights)
				{
					MVP mvp{
						light->GetTransform()->GetModelMatrix(),
						mainCameraViewMatrix,
						mainCameraProjMatrix,
					};

					AttachView(commandList, 1, m_positionAttachment->GetAttachmentView());
					AttachView(commandList, 2, m_normalAttachment->GetAttachmentView());

					if (light->GetShadowmap() != nullptr)
					{
						AttachView(commandList, 3, light->GetShadowmap()->GetAttachmentView());
						AttachView(commandList, 4, Texture::GetDepthPCFSampler());
					}

					AttachView(commandList, 5, light->GetLightDataBufferView());

					commandList->SetGraphicsRoot32BitConstants(0, sizeof(MVP) / 4, &mvp, 0);
					commandList->DrawIndexedInstanced(
						m_planeMesh->GetIndexSize(),
						1,
						0, 0, 0);
				}
			}
		}

		// Transition light texture to generic read state
		{
			D3D12_RESOURCE_BARRIER lightToReadBarrier = Transition(
				m_lightingAttachment->GetImage().Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_GENERIC_READ);
			commandList->ResourceBarrier(1, &lightToReadBarrier);
		}

		//Drawing main color
		{
			commandList->OMSetRenderTargets(
				1,
				&rtvHandle,
				FALSE, &dsvHandle);

			const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			// we've written depth in g-buffer generation step
			//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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
						mainCameraViewMatrix,
						mainCameraProjMatrix
					};
					for (auto param : mr->GetMaterial()->GetRootParams())
					{
						uint32_t index = param.first;
						D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = param.second->GetGPUDescriptorHandleForHeapStart();
						ID3D12DescriptorHeap* heaps[1] = {param.second};
						commandList->SetDescriptorHeaps(1, heaps);
						commandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
					}

					ID3D12DescriptorHeap* heaps1[1] = {m_lightingAttachment->GetAttachmentView()->GetHeap()};
					commandList->SetDescriptorHeaps(
						1,
						heaps1);
					commandList->SetGraphicsRootDescriptorTable(3, m_lightingAttachment->GetAttachmentView()->GetGPUHandle());

					commandList->SetGraphicsRoot32BitConstants(2, sizeof(MVP) / 4, &mvp, 0);
					commandList->DrawIndexedInstanced(
						mr->GetMesh()->GetIndexSize(),
						1,
						0, 0, 0);
				}
			}
		}

		// transition front buffer to present state
		// transition normal and position buffers back to render target state
		{
			// Indicate that the back buffer will now be used to present.
			D3D12_RESOURCE_BARRIER barrier2 = Transition(
				m_renderTargets[m_currentFrameIndex]->GetImage().Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT);
			commandList->ResourceBarrier(1, &barrier2);


			D3D12_RESOURCE_BARRIER positionToRenderBarrier = Transition(
				m_positionAttachment->GetImage().Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &positionToRenderBarrier);

			D3D12_RESOURCE_BARRIER normalToRenderBarrier = Transition(
				m_normalAttachment->GetImage().Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &normalToRenderBarrier);

			D3D12_RESOURCE_BARRIER lightToRenderBarrier = Transition(
				m_lightingAttachment->GetImage().Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &lightToRenderBarrier);
		}

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		// Present the frame.
		ASSERT_SUCC(m_swapChain->Present(0, 0));

		// Update the frame index.
		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);
	}

	void RenderManager::RenderEntireScene(
		ID3D12GraphicsCommandList* commandList,
		glm::mat4 view,
		glm::mat4 proj) const
	{
		for (auto const& s : m_sharedMaterials)
		{
			for (const auto& mr : s->GetMeshRenderers())
			{
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
				commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

				MVP mvp{
					mr->GetTransform()->GetModelMatrix(),
					view,
					proj
				};
				uint32_t var = 5;
				commandList->SetGraphicsRoot32BitConstants(0, sizeof(MVP) / 4, &mvp, 0);
				commandList->DrawIndexedInstanced(
					mr->GetMesh()->GetIndexSize(),
					1,
					0, 0, 0);
			}
		}
	}

	void RenderManager::SetViewportAndScissor(
		ID3D12GraphicsCommandList* commandList,
		uint32_t width,
		uint32_t height) const
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


	void RenderManager::AttachView(
		ID3D12GraphicsCommandList* commandList,
		uint32_t rootParameterIndex,
		const ResourceView* view
	) const
	{
		ID3D12DescriptorHeap* heaps1[1] = {view->GetHeap()};
		commandList->SetDescriptorHeaps(
			1,
			heaps1);
		D3D12_GPU_DESCRIPTOR_HANDLE null = {0};
		commandList->SetGraphicsRootDescriptorTable(
			rootParameterIndex, view->GetGPUHandle());
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
