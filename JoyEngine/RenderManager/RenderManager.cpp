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
#include "ResourceManager/DynamicCpuBuffer.h"
#include "ResourceManager/Material.h"
#include "SceneManager/Transform.h"

#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

#define GLM_FORCE_RADIANS

namespace JoyEngine
{
	IMPLEMENT_SINGLETON(RenderManager)

	void RenderManager::Init()
	{
		TIME_PERF("RenderManager init")

		static_assert(sizeof(EngineData) == 176);

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

		for (UINT n = 0; n < frameCount; n++)
		{
			ComPtr<ID3D12Resource> swapchainResource;
			ASSERT_SUCC(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&swapchainResource)));

			m_swapchainRenderTargets[n] = std::make_unique<RenderTexture>(
				swapchainResource,
				m_width,
				m_height,
				swapchainFormat,
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_HEAP_TYPE_DEFAULT);
		}

		m_mainColorRenderTarget = std::make_unique<RenderTexture>(
			m_width,
			m_height,
			hdrRTVFormat,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT
		);

		m_gbuffer = std::make_unique<RTVGbuffer>(m_width, m_height);

		m_skybox = std::make_unique<Skybox>(m_gbuffer->GetColorSRV());

		m_engineDataBuffer = std::make_unique<DynamicCpuBuffer<EngineData>>(frameCount);

		m_tonemapping = std::make_unique<Tonemapping>(
			m_width, m_height,
			m_mainColorRenderTarget.get(),
			hdrRTVFormat, swapchainFormat, depthFormat);

		m_raytracing = std::make_unique<Raytracing>(
			m_sharedMaterials,
			GetMainColorFormat(),
			GetGBufferFormat(),
			GetGBufferFormat(),
			GetSwapchainFormat(),
			m_width,
			m_height);

		{
			D3D12_CPU_DESCRIPTOR_HANDLE imguiCpuHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE imguiGpuHandle;

			DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::SRV_CBV_UAV, m_imguiDescriptorIndex,
			                                             imguiCpuHandle,
			                                             imguiGpuHandle);

			ImGui_ImplDX12_Init(GraphicsManager::Get()->GetDevice(), frameCount,
			                    swapchainFormat, DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			                    imguiCpuHandle,
			                    imguiGpuHandle);
		}
	}

	void RenderManager::Start() const
	{
		m_queue->WaitQueueIdle();
		m_raytracing->UploadSceneData();
		m_raytracing->PrepareBVH();
	}


	void RenderManager::Stop()
	{
		m_queue->WaitQueueIdle();

		m_raytracing = nullptr;
		m_tonemapping = nullptr;
		m_queue = nullptr;

		{
			ImGui_ImplDX12_Shutdown();
		}
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


	bool g_drawRaytracedImage = true;

	void RenderManager::Update()
	{
		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);

		m_queue->ResetForFrame(m_currentFrameIndex);

		const auto commandList = m_queue->GetCommandList(m_currentFrameIndex);

		auto swapchainResource = m_swapchainRenderTargets[m_currentFrameIndex]->GetImage().Get();
		auto hdrRTVResource = m_mainColorRenderTarget->GetImage().Get();

		auto swapchainRTVHandle = m_swapchainRenderTargets[m_currentFrameIndex]->GetRTV()->GetCPUHandle();
		auto hdrRTVHandle = m_mainColorRenderTarget->GetRTV()->GetCPUHandle();

		ASSERT(m_currentCamera != nullptr);
		const glm::mat4 mainCameraViewMatrix = m_currentCamera->GetViewMatrix();
		const glm::mat4 mainCameraProjMatrix = m_currentCamera->GetProjMatrix();

		ViewProjectionMatrixData mainCameraMatrixVP = {
			.view = mainCameraViewMatrix,
			.proj = mainCameraProjMatrix
		};

		{
			m_engineDataBuffer->Lock(m_currentFrameIndex);

			const auto data = static_cast<EngineData*>(m_engineDataBuffer->GetPtr());
			data->cameraWorldPos = m_currentCamera->GetTransform()->GetPosition();
			data->time = Time::GetTime();
			data->cameraInvProj = glm::inverse(mainCameraProjMatrix);
			data->cameraInvView = glm::inverse(mainCameraViewMatrix);
			data->cameraNear = m_currentCamera->GetNear();
			data->cameraFar = m_currentCamera->GetFar();
			data->cameraFovRadians = m_currentCamera->GetFovRadians();
			data->screenWidth = m_width;
			data->screenHeight = m_height;

			m_engineDataBuffer->Unlock();
		}

		UpdateObjectMatrices();

		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);

		// Set main viewport-scissor rects
		GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

		//Drawing G-Buffer
		{
			m_gbuffer->BarrierToWrite(commandList);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2] = {
				m_gbuffer->GetColorRTV()->GetCPUHandle(),
				m_gbuffer->GetNormalsRTV()->GetCPUHandle(),
			};
			auto dsvHandle = m_gbuffer->GetDepthDSV()->GetCPUHandle();

			commandList->OMSetRenderTargets(
				2,
				rtvHandles,
				FALSE, &dsvHandle);

			constexpr float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
			commandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
			commandList->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			RenderSceneForSharedMaterial(commandList, &mainCameraMatrixVP, EngineMaterialProvider::Get()->GetGBufferWriteSharedMaterial());

			m_gbuffer->BarrierToRead(commandList);
		}

		// Deferred shading 
		{
			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, nullptr);

			const auto& sm = EngineMaterialProvider::Get()->GetDeferredShadingProcessorSharedMaterial();

			commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "colorTexture", m_gbuffer->GetColorSRV());
			GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "normalsTexture", m_gbuffer->GetNormalsSRV());
			//GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "depthTexture", m_gbuffer->GetDepthSRV());

			commandList->DrawIndexedInstanced(
				3,
				1,
				0, 0, 0);

			m_skybox->DrawSky(commandList, m_currentFrameIndex, &mainCameraMatrixVP);

		}

		if (g_drawRaytracedImage)
		{
			m_raytracing->ProcessRaytracing(commandList, m_engineDataBuffer->GetView(m_currentFrameIndex));

			m_raytracing->DebugDrawRaytracedImage(commandList);
		}

		// HDR->LDR
		{
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
				&swapchainRTVHandle,
				FALSE, nullptr);

			m_tonemapping->Render(commandList, m_swapchainRenderTargets[m_currentFrameIndex].get());


			DrawGui(commandList, &mainCameraMatrixVP);


			GraphicsUtils::Barrier(commandList,
			                       swapchainResource,
			                       D3D12_RESOURCE_STATE_RENDER_TARGET,
			                       D3D12_RESOURCE_STATE_PRESENT);
			GraphicsUtils::Barrier(commandList,
			                       hdrRTVResource,
			                       D3D12_RESOURCE_STATE_GENERIC_READ,
			                       D3D12_RESOURCE_STATE_RENDER_TARGET);
		}


		ASSERT_SUCC(commandList->Close());

		m_queue->Execute(m_currentFrameIndex);

		UINT presentFlags = GraphicsManager::Get()->GetTearingSupport() ? DXGI_PRESENT_ALLOW_TEARING : 0;

		// Present the frame.
		ASSERT_SUCC(m_swapChain->Present(0, presentFlags));

		m_trianglesCount = 0;
	}

	void RenderManager::DrawGui(ID3D12GraphicsCommandList* commandList, const ViewProjectionMatrixData* viewProjectionData) const
	{
		// Draw axis gizmo
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

			GraphicsUtils::ProcessEngineBindings(commandList, m_currentFrameIndex, sm->GetGraphicsPipeline()->GetEngineBindings(), nullptr, viewProjectionData);

			commandList->DrawInstanced(
				6,
				1,
				0, 0);
		}


		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos({0, 0});
		ImGui::SetNextWindowSize({300, 150});
		{
			ImGui::Begin("Stats:");
			ImGui::Text("Screen: %dx%d", m_width, m_height);
			ImGui::Text("Num triangles %d", m_trianglesCount);
			const glm::vec3 camPos = m_currentCamera->GetTransform()->GetPosition();
			ImGui::Text("Camera: %.3f %.3f %.3f", camPos.x, camPos.y, camPos.z);
			ImGui::Checkbox("Draw raytraced image", &g_drawRaytracedImage);
			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	}

	void RenderManager::UpdateObjectMatrices() const
	{
		DynamicCpuBuffer<ObjectMatricesData>* objectMatrices = EngineMaterialProvider::Get()->GetObjectMatricesDataBuffer();
		objectMatrices->Lock(m_currentFrameIndex);
		ObjectMatricesData* data = objectMatrices->GetPtr();
		for (auto const& sm : m_sharedMaterials)
		{
			for (const auto& mr : sm->GetMeshRenderers())
			{
				data->data[*mr->GetTransform()->GetIndex()] = mr->GetTransform()->GetModelMatrix();
			}
		}

		objectMatrices->Unlock();
	}

	void RenderManager::RenderEntireSceneWithMaterials(
		ID3D12GraphicsCommandList* commandList,
		const ViewProjectionMatrixData* viewProjectionData
	) const
	{
		for (auto const& sm : m_sharedMaterials)
		{
			commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

			for (const auto& mr : sm->GetMeshRenderers())
			{
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
				commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

				for (const auto param : mr->GetMaterial()->GetRootParams())
				{
					const uint32_t index = param.first;
					GraphicsUtils::AttachViewToGraphics(commandList, index, param.second);
				}

				GraphicsUtils::ProcessEngineBindings(
					commandList,
					m_currentFrameIndex,
					sm->GetGraphicsPipeline()->GetEngineBindings(),
					mr->GetTransform()->GetIndex(),
					viewProjectionData);

				commandList->DrawIndexedInstanced(
					mr->GetMesh()->GetIndexCount(),
					1,
					0, 0, 0);
				m_trianglesCount += mr->GetMesh()->GetIndexCount() / 3;
			}
		}
	}

	void RenderManager::RenderSceneForSharedMaterial(
		ID3D12GraphicsCommandList* commandList,
		const ViewProjectionMatrixData* viewProjectionData,
		SharedMaterial* sharedMaterial
	) const
	{
		commandList->SetPipelineState(sharedMaterial->GetGraphicsPipeline()->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sharedMaterial->GetGraphicsPipeline()->GetRootSignature().Get());

		for (const auto& mr : sharedMaterial->GetMeshRenderers())
		{
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
			commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

			for (const auto param : mr->GetMaterial()->GetRootParams())
			{
				const uint32_t index = param.first;
				GraphicsUtils::AttachViewToGraphics(commandList, index, param.second);
			}

			GraphicsUtils::ProcessEngineBindings(
				commandList, 
				m_currentFrameIndex, 
				sharedMaterial->GetGraphicsPipeline()->GetEngineBindings(), 
				mr->GetTransform()->GetIndex(), 
				viewProjectionData);

			commandList->DrawIndexedInstanced(
				mr->GetMesh()->GetIndexCount(),
				1,
				0, 0, 0);
			m_trianglesCount += mr->GetMesh()->GetIndexCount() / 3;
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
