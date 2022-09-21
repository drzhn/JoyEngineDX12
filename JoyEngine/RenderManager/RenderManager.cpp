#include "RenderManager.h"

#include <memory>

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"


#include "Utils/Assert.h"

#include "ResourceManager/ResourceManager.h"
#include "Common/CommandQueue.h"
#include "Components/Light.h"
#include "ResourceManager/Mesh.h"
#include "Components/Camera.h"
#include "ResourceManager/SharedMaterial.h"
#include "Tonemapping.h"
#include "Common/HashDefs.h"

#include "Common/Time.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/DynamicBuffer.h"

#include "Utils/GraphicsUtils.h"

#define GLM_FORCE_RADIANS

namespace JoyEngine
{
	IMPLEMENT_SINGLETON(RenderManager)

	void RenderManager::Init()
	{
		m_width = GraphicsManager::Get()->GetWidth();
		m_height = GraphicsManager::Get()->GetHeight();

		ASSERT(m_width != 0 && m_height != 0);

		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, GraphicsManager::Get()->GetDevice(),
		                                         frameCount);

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = frameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = GraphicsManager::Get()->GetTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		//swapChainDesc.SampleDesc.Quality = 0;

		ComPtr<IDXGISwapChain1> swapChain;
		ASSERT_SUCC(GraphicsManager::Get()->GetFactory()->CreateSwapChainForHwnd(
			m_queue->GetQueue(), // Swap chain needs the queue so that it can force a flush on it.
			GraphicsManager::Get()->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		ASSERT_SUCC(GraphicsManager::Get()->GetFactory()->MakeWindowAssociation(GraphicsManager::Get()->GetHWND(), DXGI_MWA_NO_ALT_ENTER));
		ASSERT_SUCC(swapChain.As(&m_swapChain));

		//m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < frameCount; n++)
		{
			ComPtr<ID3D12Resource> swapchainResource;
			ASSERT_SUCC(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&swapchainResource)));

			m_swapchainRenderTargets[n] = std::make_unique<RenderTexture>(
				swapchainResource,
				m_width,
				m_height,
				ldrRTVFormat,
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_HEAP_TYPE_DEFAULT);
		}

		m_hdrRenderTarget = std::make_unique<RenderTexture>(
			m_width,
			m_height,
			hdrRTVFormat,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT
		);

		m_depthAttachment = std::make_unique<DepthTexture>(
			m_width,
			m_height,
			DXGI_FORMAT_R32_TYPELESS,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_HEAP_TYPE_DEFAULT);


		m_engineDataBuffer = std::make_unique<DynamicBuffer<::EngineData>>(frameCount);


		m_tonemapping = std::make_unique<Tonemapping>(
			m_width, m_height,
			m_hdrRenderTarget.get(),
			hdrRTVFormat, ldrRTVFormat, depthFormat);

		m_raytracing = std::make_unique<Raytracing>();

		{
			D3D12_CPU_DESCRIPTOR_HANDLE imguiCpuHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE imguiGpuHandle;

			DescriptorManager::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_imguiDescriptorIndex,
			                                             imguiCpuHandle,
			                                             imguiGpuHandle);

			ImGui_ImplDX12_Init(GraphicsManager::Get()->GetDevice(), frameCount,
			                    ldrRTVFormat, DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			                    imguiCpuHandle,
			                    imguiGpuHandle);
		}
	}

	void RenderManager::Start()
	{
		m_queue->WaitQueueIdle();
	}


	void RenderManager::Stop()
	{
		m_queue->WaitQueueIdle();


		m_tonemapping = nullptr;
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
		//ASSERT(light->GetLightType() != Direction);
		//m_lights.insert(light);
	}

	void RenderManager::UnregisterLight(Light* light)
	{
		//if (m_lights.find(light) == m_lights.end())
		//{
		//	ASSERT(false);
		//}
		//m_lights.erase(light);
	}

	void RenderManager::RegisterDirectionLight(Light* light)
	{
		//ASSERT(light->GetLightType() == Direction);
		//m_directionLight = light;
	}

	void RenderManager::UnregisterDirectionLight(Light* light)
	{
		//ASSERT(m_directionLight == light);
		//m_directionLight = nullptr;
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

	void RenderManager::Update()
	{
		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);

		m_queue->ResetForFrame(m_currentFrameIndex);


		const auto commandList = m_queue->GetCommandList(m_currentFrameIndex);

		auto swapchainResource = m_swapchainRenderTargets[m_currentFrameIndex]->GetImage().Get();
		auto hdrRTVResource = m_hdrRenderTarget->GetImage().Get();
		auto depthResource = m_depthAttachment->GetImage().Get();

		auto dsvHandle = m_depthAttachment->GetDSV()->GetCPUHandle();
		auto ldrRTVHandle = m_swapchainRenderTargets[m_currentFrameIndex]->GetRTV()->GetCPUHandle();
		auto hdrRTVHandle = m_hdrRenderTarget->GetRTV()->GetCPUHandle();

		ASSERT(m_currentCamera != nullptr);
		const glm::mat4 mainCameraViewMatrix = m_currentCamera->GetViewMatrix();
		const glm::mat4 mainCameraProjMatrix = m_currentCamera->GetProjMatrix();

		{
			m_engineDataBuffer->Lock(m_currentFrameIndex);

			const auto data = static_cast<::EngineData*>(m_engineDataBuffer->GetPtr());
			data->cameraWorldPos = m_currentCamera->GetTransform()->GetPosition();
			data->time = Time::GetTime();
			data->perspectiveValues = glm::vec4(
				1.0f / mainCameraProjMatrix[0][0],
				1.0f / mainCameraProjMatrix[1][1],
				mainCameraProjMatrix[3][2],
				mainCameraProjMatrix[2][2]
			);
			data->cameraInvProj = glm::inverse(mainCameraProjMatrix);
			data->cameraInvView = glm::inverse(mainCameraViewMatrix);

			m_engineDataBuffer->Unlock();
		}

		//Drawing main color
		{
			GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, &dsvHandle);

			constexpr float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
			commandList->ClearRenderTargetView(hdrRTVHandle, clearColor, 0, nullptr);
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			RenderEntireSceneWithMaterials(commandList, mainCameraViewMatrix, mainCameraProjMatrix, true);
		}

		// HDR->LDR

		GraphicsUtils::Barrier(commandList,
		                       hdrRTVResource,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET,
		                       D3D12_RESOURCE_STATE_GENERIC_READ);
		GraphicsUtils::Barrier(commandList,
		                       swapchainResource,
		                       D3D12_RESOURCE_STATE_PRESENT,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET);

		GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

		commandList->OMSetRenderTargets(
			1,
			&ldrRTVHandle,
			FALSE, nullptr);

		m_tonemapping->Render(commandList, m_swapchainRenderTargets[m_currentFrameIndex].get());

		DrawGui(commandList);

		{
			auto sm = EngineMaterialProvider::Get()->GetGizmoAxisDrawerSharedMaterial();

			commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

			float gizmoViewportWidth = static_cast<float>(m_width) / 5.f;
			float gizmoViewportHeight = static_cast<float>(m_height) / 5.f;

			const D3D12_VIEWPORT viewport = {
				static_cast<float>(m_width) - gizmoViewportWidth + (gizmoViewportWidth - gizmoViewportHeight) / 2,
				0.0f,
				gizmoViewportWidth,
				gizmoViewportHeight,
				D3D12_MIN_DEPTH,
				D3D12_MAX_DEPTH
			};
			const D3D12_RECT scissorRect = {
				0,
				0,
				static_cast<LONG>(m_width),
				static_cast<LONG>(m_height)
			};
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);

			::MVP mvp{
				glm::identity<glm::mat4>(),
				mainCameraViewMatrix,
				mainCameraProjMatrix
			};

			ProcessEngineBindings(commandList, sm->GetGraphicsPipeline()->GetEngineBindings(), &mvp);

			commandList->DrawInstanced(
				6,
				1,
				0, 0);
		}


		GraphicsUtils::Barrier(commandList,
		                       swapchainResource,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET,
		                       D3D12_RESOURCE_STATE_PRESENT);
		GraphicsUtils::Barrier(commandList,
		                       hdrRTVResource,
		                       D3D12_RESOURCE_STATE_GENERIC_READ,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET);


		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(m_currentFrameIndex);

		UINT presentFlags = GraphicsManager::Get()->GetTearingSupport() ? DXGI_PRESENT_ALLOW_TEARING : 0;

		// Present the frame.
		ASSERT_SUCC(m_swapChain->Present(0, presentFlags));

		m_trianglesCount = 0;
	}

	void RenderManager::DrawGui(ID3D12GraphicsCommandList* commandList) const
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos({0, 0});
		ImGui::SetNextWindowSize({300, 100});
		{
			ImGui::Begin("Stats:");
			ImGui::Text("Num triangles %d", m_trianglesCount);
			const glm::vec3 camPos = m_currentCamera->GetTransform()->GetPosition();
			ImGui::Text("Camera: %.3f %.3f %.3f", camPos.x, camPos.y, camPos.z);
			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	}

	void RenderManager::ProcessEngineBindings(
		ID3D12GraphicsCommandList* commandList,
		const std::map<uint32_t, EngineBindingType>& bindings,
		::MVP* mvp
	) const
	{
		for (const auto& pair : bindings)
		{
			const auto type = pair.second;
			const auto rootIndex = pair.first;

			switch (type)
			{
			case ModelViewProjection:
				{
					ASSERT(mvp != nullptr)
					commandList->SetGraphicsRoot32BitConstants(rootIndex, sizeof(::MVP) / 4, mvp, 0);
					break;
				}
			//case LightAttachment:
			//	{
			//		if (isDrawingMainColor)
			//		{
			//			AttachViewToGraphics(commandList, rootIndex, m_lightingAttachment->GetRTV());
			//		}
			//		break;
			//	}
			//case EnvironmentCubemap:
			//	{
			//		if (isDrawingMainColor)
			//		{
			//			AttachViewToGraphics(commandList, rootIndex, m_cubemap->GetCubemapTexture()->GetRTV());
			//		}
			//		break;
			//	}
			//case EnvironmentConvolutedCubemap:
			//	{
			//		if (isDrawingMainColor)
			//		{
			//			AttachViewToGraphics(commandList, rootIndex, m_cubemap->GetCubemapConvolutedTexture()->GetRTV());
			//		}
			//		break;
			//	}
			//case EngineData:
			//	{
			//		AttachViewToGraphics(commandList, rootIndex, m_engineDataBufferView.get());
			//		break;
			//	}
			default:
				ASSERT(false);
			}
		}
	}

	void RenderManager::RenderEntireScene(
		ID3D12GraphicsCommandList* commandList,
		glm::mat4 view,
		glm::mat4 proj
	) const
	{
		for (auto const& s : m_sharedMaterials)
		{
			for (const auto& mr : s->GetMeshRenderers())
			{
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
				commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

				::MVP mvp{
					mr->GetTransform()->GetModelMatrix(),
					view,
					proj
				};
				uint32_t var = 5;
				commandList->SetGraphicsRoot32BitConstants(0, sizeof(::MVP) / 4, &mvp, 0);
				commandList->DrawIndexedInstanced(
					mr->GetMesh()->GetIndexCount(),
					1,
					0, 0, 0);
			}
		}
	}

	void RenderManager::RenderEntireSceneWithMaterials(
		ID3D12GraphicsCommandList* commandList,
		glm::mat4 view,
		glm::mat4 proj,
		bool isDrawingMainColor
	) const
	{
		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);

		for (auto const& sm : m_sharedMaterials)
		{
			commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

			for (const auto& mr : sm->GetMeshRenderers())
			{
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
				commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

				::MVP mvp{
					mr->GetTransform()->GetModelMatrix(),
					view,
					proj
				};
				for (auto param : mr->GetMaterial()->GetRootParams())
				{
					const uint32_t index = param.first;
					GraphicsUtils::AttachViewToGraphics(commandList, index, param.second);
				}

				ProcessEngineBindings(commandList, sm->GetGraphicsPipeline()->GetEngineBindings(), &mvp);

				commandList->DrawIndexedInstanced(
					mr->GetMesh()->GetIndexCount(),
					1,
					0, 0, 0);
				m_trianglesCount += mr->GetMesh()->GetIndexCount() / 3;
			}
		}
	}

	void RenderManager::CopyRTVResource(
		ID3D12GraphicsCommandList* commandList,
		ID3D12Resource* rtvResource,
		ID3D12Resource* copyResource
	)
	{
		GraphicsUtils::Barrier(commandList, rtvResource,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

		GraphicsUtils::Barrier(commandList, copyResource,
		                       D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);


		commandList->CopyResource(
			copyResource,
			rtvResource
		);

		GraphicsUtils::Barrier(commandList, copyResource,
		                       D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

		GraphicsUtils::Barrier(commandList, rtvResource,
		                       D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
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
