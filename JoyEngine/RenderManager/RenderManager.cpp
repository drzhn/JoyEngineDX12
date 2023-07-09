#include "RenderManager.h"

#include <memory>

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"


#include "Utils/Assert.h"

#include "Common/CommandQueue.h"
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
#include "ResourceManager/Material.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"

#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

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

		ASSERT_SUCC(
			GraphicsManager::Get()->GetFactory()->MakeWindowAssociation(GraphicsManager::Get()->GetHWND(),
				DXGI_MWA_NO_ALT_ENTER));
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

		m_skybox = std::make_unique<Skybox>();

		m_tonemapping = std::make_unique<Tonemapping>(
			this,
			m_mainColorRenderTarget.get(),
			hdrRTVFormat, swapchainFormat, depthFormat);

		m_raytracing = std::make_unique<RaytracedLightProbes>(
			m_sharedMaterials,
			GetMainColorFormat(),
			GetGBufferFormat(),
			GetGBufferFormat(),
			GetSwapchainFormat(),
			GetDepthFormat(),
			frameCount,
			m_width,
			m_height);

		m_transformProvider = std::make_unique<TransformProvider>(frameCount);
		m_lightSystem = std::make_unique<ClusteredLightSystem>(frameCount);

		// IMGUI initialization
		{
			D3D12_CPU_DESCRIPTOR_HANDLE imguiCpuHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE imguiGpuHandle;

			DescriptorManager::Get()->AllocateDescriptor(
				DescriptorHeapType::SRV_CBV_UAV,
				m_imguiDescriptorIndex,
				imguiCpuHandle,
				imguiGpuHandle);

			ImGui_ImplDX12_Init(
				GraphicsManager::Get()->GetDevice(), frameCount,
				swapchainFormat,
				DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
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
		if (!m_sharedMaterials.contains(sm))
		{
			ASSERT(false);
		}
		m_sharedMaterials.erase(sm);
	}

	void RenderManager::RegisterCamera(Camera* camera)
	{
		m_currentCamera = camera;
		m_lightSystem->SetCamera(m_currentCamera);
	}

	void RenderManager::UnregisterCamera(Camera* camera)
	{
		ASSERT(m_currentCamera == camera);
		m_currentCamera = nullptr;
	}


	bool g_drawRaytracedImage = false;
	bool g_drawProbes = true;

	void RenderManager::PreUpdate()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void RenderManager::Update()
	{
		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);

		m_queue->ResetForFrame(m_currentFrameIndex);

		const auto commandList = m_queue->GetCommandList(m_currentFrameIndex);

		auto swapchainResource = m_swapchainRenderTargets[m_currentFrameIndex]->GetImageResource().Get();
		auto hdrRTVResource = m_mainColorRenderTarget->GetImageResource().Get();

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
			DynamicCpuBuffer<EngineData>* engineDataBuffer = EngineMaterialProvider::Get()->GetEngineDataBuffer();


			const auto data = static_cast<EngineData*>(engineDataBuffer->GetPtr(m_currentFrameIndex));
			data->cameraWorldPos = m_currentCamera->GetGameObject().GetTransform()->GetPosition();
			data->time = Time::GetTime();
			data->cameraInvProj = glm::inverse(mainCameraProjMatrix);
			data->cameraInvView = glm::inverse(mainCameraViewMatrix);
			data->cameraNear = m_currentCamera->GetNear();
			data->cameraFar = m_currentCamera->GetFar();
			data->cameraFovRadians = m_currentCamera->GetFovRadians();
			data->screenWidth = m_width;
			data->screenHeight = m_height;
			data->cameraAspect = GetAspect();

		}

		m_transformProvider->Update(m_currentFrameIndex);
		m_lightSystem->Update(m_currentFrameIndex);

		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);

		// Drawing shadows
		{
			m_lightSystem->RenderDirectionalShadows(
				commandList,
				m_currentFrameIndex,
				EngineMaterialProvider::Get()->GetGBufferWriteSharedMaterial());
		}

		// Set main viewport-scissor rects
		GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

		//Drawing G-Buffer
		{
			m_gbuffer->BarrierColorToWrite(commandList);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3] = {
				m_gbuffer->GetColorRTV()->GetCPUHandle(),
				m_gbuffer->GetNormalsRTV()->GetCPUHandle(),
				m_gbuffer->GetPositionRTV()->GetCPUHandle(),
			};
			auto dsvHandle = m_gbuffer->GetDepthDSV()->GetCPUHandle();
			constexpr float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
			commandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
			commandList->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
			commandList->ClearRenderTargetView(rtvHandles[2], clearColor, 0, nullptr);
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			commandList->OMSetRenderTargets(
				1,
				&rtvHandles[0],
				FALSE, nullptr);

			m_skybox->DrawSky(commandList, m_currentFrameIndex, &mainCameraMatrixVP);

			commandList->OMSetRenderTargets(
				3,
				rtvHandles,
				FALSE, &dsvHandle);

			RenderSceneForSharedMaterial(commandList, &mainCameraMatrixVP,
			                             EngineMaterialProvider::Get()->GetGBufferWriteSharedMaterial());

			m_gbuffer->BarrierColorToRead(commandList);
		}

		// Process raytracing
		{
			m_raytracing->ProcessRaytracing(commandList, m_currentFrameIndex, &mainCameraMatrixVP, m_skybox->GetSkyboxTextureDataSrv());


			auto raytracedRTVHandle = m_raytracing->GetShadedRenderTexture()->GetRTV()->GetCPUHandle();

			GraphicsUtils::Barrier(commandList, m_raytracing->GetShadedRenderTexture()->GetImageResource().Get(),
			                       D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

			GraphicsUtils::SetViewportAndScissor(commandList, m_raytracing->GetRaytracedTextureWidth(), m_raytracing->GetRaytracedTextureHeight());

			commandList->OMSetRenderTargets(
				1,
				&raytracedRTVHandle,
				FALSE, nullptr);

			RenderDeferredShading(commandList, m_raytracing->GetGBuffer(), &mainCameraMatrixVP);

			GraphicsUtils::Barrier(commandList, m_raytracing->GetShadedRenderTexture()->GetImageResource().Get(),
			                       D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			m_raytracing->GenerateProbeIrradiance(commandList, m_currentFrameIndex);
		}

		GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

		// Deferred shading 
		{
			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, nullptr);

			RenderDeferredShading(commandList, m_gbuffer.get(), &mainCameraMatrixVP);
		}

		if (g_drawRaytracedImage)
		{
			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, nullptr);

			m_raytracing->DebugDrawRaytracedImage(commandList);
		}

		if (g_drawProbes)
		{
			auto dsvHandle = m_gbuffer->GetDepthDSV()->GetCPUHandle();
			//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, &dsvHandle);

			m_raytracing->DebugDrawProbes(commandList, m_currentFrameIndex, &mainCameraMatrixVP);
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

			commandList->OMSetRenderTargets(
				1,
				&swapchainRTVHandle,
				FALSE, nullptr);

			m_tonemapping->Render(commandList, m_currentFrameIndex,
			                      m_swapchainRenderTargets[m_currentFrameIndex].get());


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

	void RenderManager::DrawGui(ID3D12GraphicsCommandList* commandList,
	                            const ViewProjectionMatrixData* viewProjectionData) const
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

			GraphicsUtils::ProcessEngineBindings(commandList, m_currentFrameIndex,
			                                     sm->GetGraphicsPipeline()->GetEngineBindings(), nullptr,
			                                     viewProjectionData);

			commandList->DrawInstanced(
				6,
				1,
				0, 0);
		}

		ImGui::SetNextWindowPos({0, 0});
		ImGui::SetNextWindowSize({300, 75});
		{
			ImGui::Begin("Stats:");
			//ImGui::Text("Screen: %dx%d", m_width, m_height);
			ImGui::Text("Num triangles %d", m_trianglesCount);
			const glm::vec3 camPos = m_currentCamera->GetGameObject().GetTransform()->GetPosition();
			ImGui::Text("Camera: %.3f %.3f %.3f", camPos.x, camPos.y, camPos.z);
			//ImGui::Checkbox("Draw raytraced image", &g_drawRaytracedImage);
			ImGui::End();
		}
		ImGui::SetNextWindowPos({0, 75});
		ImGui::SetNextWindowSize({300, 75});
		{
			bool useDDGI = m_raytracing->GetRaytracedProbesDataPtr()->useDDGI == 1;
			ImGui::Begin("DDGI:");
			ImGui::Checkbox("Draw probes", &g_drawProbes);
			ImGui::Checkbox("Use GI", &useDDGI);
			ImGui::End();
			m_raytracing->GetRaytracedProbesDataPtr()->useDDGI = useDDGI ? 1 : 0;
		}
		ImGui::SetNextWindowSize({300, 150});
		ImGui::SetNextWindowPos({0, 150});
		{
			HDRDownScaleConstants* constants = m_tonemapping->GetConstantsPtr();
			ImGui::Begin("Tonemapping:");
			ImGui::Checkbox("Use tonemapping", reinterpret_cast<bool*>(&(constants->UseTonemapping)));
			ImGui::Checkbox("Use gamma correction", reinterpret_cast<bool*>(&(constants->UseGammaCorrection)));
			ImGui::SliderFloat("MiddleGrey", &constants->MiddleGrey, 0.f, 10.f);
			ImGui::SliderFloat("LumWhiteSqr", &constants->LumWhiteSqr, 0.f, 100.f);
			ImGui::End();
			m_tonemapping->UpdateConstants(m_currentFrameIndex);
		}
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
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
					mr->GetGameObject().GetTransformIndexPtr(),
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
				mr->GetGameObject().GetTransformIndexPtr(),
				viewProjectionData);

			commandList->DrawIndexedInstanced(
				mr->GetMesh()->GetIndexCount(),
				1,
				0, 0, 0);
			m_trianglesCount += mr->GetMesh()->GetIndexCount() / 3;
		}
	}

	void RenderManager::RenderDeferredShading(
		ID3D12GraphicsCommandList* commandList,
		const AbstractGBuffer* gBuffer,
		const ViewProjectionMatrixData* cameraVP) const
	{
		const auto& sm = EngineMaterialProvider::Get()->GetDeferredShadingProcessorSharedMaterial();

		commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "colorTexture", gBuffer->GetColorSRV());
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "normalsTexture", gBuffer->GetNormalsSRV());
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "positionTexture", gBuffer->GetPositionSRV());
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "directionalLightData", m_lightSystem->GetDirectionalLightDataView(m_currentFrameIndex));
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "directionalLightShadowmap", m_lightSystem->GetDirectionalShadowmapView());
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "PCFSampler", EngineSamplersProvider::GetDepthPCFSampler());


		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "raytracedProbesData", m_raytracing->GetRaytracedProbesDataView(m_currentFrameIndex));
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "linearBlackBorderSampler", EngineSamplersProvider::GetLinearBlackBorderSampler());

		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "probeIrradianceTexture", m_raytracing->GetProbeIrradianceTexture()->GetSRV());
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "probeDepthTexture", m_raytracing->GetProbeDepthTexture()->GetSRV());

		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "clusteredEntryData", m_lightSystem->GetClusterEntryDataView(m_currentFrameIndex));
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "clusteredItemData", m_lightSystem->GetClusterItemDataView(m_currentFrameIndex));
		GraphicsUtils::AttachViewToGraphics(commandList, sm->GetGraphicsPipeline(), "lightData", m_lightSystem->GetLightDataView(m_currentFrameIndex));

		GraphicsUtils::ProcessEngineBindings(commandList, m_currentFrameIndex,
		                                     sm->GetGraphicsPipeline()->GetEngineBindings(), nullptr,
		                                     cameraVP);

		commandList->DrawIndexedInstanced(
			3,
			1,
			0, 0, 0);
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

	float RenderManager::GetWidth_f() const noexcept
	{
		return static_cast<float>(m_width);
	}

	float RenderManager::GetHeight_f() const noexcept
	{
		return static_cast<float>(m_height);
	}

	uint32_t RenderManager::GetWidth() const noexcept
	{
		return m_width;
	}

	uint32_t RenderManager::GetHeight() const noexcept
	{
		return m_height;
	}
}
