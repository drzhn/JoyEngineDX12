#include "RaytracedDDGIRenderer.h"

#include <memory>

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"


#include "Utils/Assert.h"

#include "Common/CommandQueue.h"
#include "ResourceManager/Mesh.h"
#include "Components/Camera.h"
#include "ResourceManager/SharedMaterial.h"
#include "RenderManager/Tonemapping.h"

#include "Common/Time.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "ResourceManager/Material.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"

#include "RenderManager/RaytracedDDGIRenderer/HW/HardwareRaytracedDDGIController.h"
#include "RenderManager/RaytracedDDGIRenderer/SW/SoftwareRaytracedDDGIController.h"

#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	enum class CurrentDDGIRaytracer : int
	{
		Software = 0,
		Hardware = 1
	};

	bool g_drawRaytracedImage = false;
	bool g_drawProbes = true;
	CurrentDDGIRaytracer g_currentRaytracer = CurrentDDGIRaytracer::Software;

	RaytracedDDGIRenderer::RaytracedDDGIRenderer(HWND windowHandle): IRenderer(windowHandle)
	{
		RECT rect;
		if (GetClientRect(windowHandle, &rect))
		{
			m_width = rect.right - rect.left;
			m_height = rect.bottom - rect.top;
		}
		else
		{
			ASSERT(false);
		}
	}

	void RaytracedDDGIRenderer::Init(Skybox* skybox)
	{
		TIME_PERF("RaytracedDDGIRenderer init")

		static_assert(sizeof(EngineData) == 176);

		ASSERT(m_width != 0 && m_height != 0);

		m_skybox = skybox;

		m_queue = std::make_unique<CommandQueue>(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			GraphicsManager::Get()->GetDevice(),
			FRAME_COUNT
		);

		// Describe and create the swap chain.
		const DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
			.Width = m_width,
			.Height = m_height,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc = {
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = FRAME_COUNT,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = GraphicsManager::Get()->GetTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u,
		};

		ComPtr<IDXGISwapChain1> swapChain;
		ASSERT_SUCC(
			GraphicsManager::Get()->GetFactory()->CreateSwapChainForHwnd(
				m_queue->GetQueue(), // Swap chain needs the queue so that it can force a flush on it.
				m_windowHandle,
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain
			));

		ASSERT_SUCC(
			GraphicsManager::Get()->GetFactory()->MakeWindowAssociation(
				m_windowHandle,
				DXGI_MWA_NO_ALT_ENTER));
		ASSERT_SUCC(swapChain.As(&m_swapChain));

		//m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		for (UINT n = 0; n < FRAME_COUNT; n++)
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
			hdrRenderTextureFormat,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT
		);

		m_gbuffer = std::make_unique<RTVGbuffer>(m_width, m_height);


		m_tonemapping = std::make_unique<Tonemapping>(
			this,
			m_mainColorRenderTarget.get(),
			hdrRenderTextureFormat, swapchainFormat, depthFormat
		);

		m_raytracingDataContainer = std::make_unique<RaytracedDDGIDataContainer>(
			m_sharedMaterials,
			FRAME_COUNT,
			GetHDRRenderTextureFormat(),
			GetDepthFormat()
		);

		m_softwareRaytracedDDGI = std::make_unique<SoftwareRaytracedDDGIController>(
			*m_raytracingDataContainer,
			GetHDRRenderTextureFormat(),
			GetSwapchainFormat(),
			m_width,
			m_height
		);

		m_hardwareRaytracedDDGI = std::make_unique<HardwareRaytracedDDGIController>(
			*m_raytracingDataContainer,
			GetHDRRenderTextureFormat(),
			GetSwapchainFormat(),
			m_width,
			m_height
		);

		m_lightSystem = std::make_unique<ClusteredLightSystem>(FRAME_COUNT);

		// IMGUI initialization
		{
			D3D12_CPU_DESCRIPTOR_HANDLE imguiCpuHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE imguiGpuHandle;

			DescriptorManager::Get()->AllocateDescriptor(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				m_imguiDescriptorIndex,
				imguiCpuHandle,
				imguiGpuHandle);

			ImGui_ImplDX12_Init(
				GraphicsManager::Get()->GetDevice(), FRAME_COUNT,
				swapchainFormat,
				DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
				imguiCpuHandle,
				imguiGpuHandle);
		}
	}

	void RaytracedDDGIRenderer::Start() const
	{
		m_queue->WaitQueueIdle();
		m_raytracingDataContainer->UploadSceneData();

		m_softwareRaytracedDDGI->UploadSceneData();

		m_hardwareRaytracedDDGI->UploadSceneData();
	}


	void RaytracedDDGIRenderer::Stop()
	{
		m_queue->WaitQueueIdle();

		m_softwareRaytracedDDGI = nullptr;
		m_hardwareRaytracedDDGI = nullptr;
		m_tonemapping = nullptr;
		m_queue = nullptr;

		{
			ImGui_ImplDX12_Shutdown();
		}
	}

	void RaytracedDDGIRenderer::RegisterSharedMaterial(SharedMaterial* sm)
	{
		m_sharedMaterials.insert(sm);
	}

	void RaytracedDDGIRenderer::UnregisterSharedMaterial(SharedMaterial* sm)
	{
		if (!m_sharedMaterials.contains(sm))
		{
			ASSERT(false);
		}
		m_sharedMaterials.erase(sm);
	}

	void RaytracedDDGIRenderer::RegisterCamera(Camera* camera)
	{
		m_currentCamera = camera;
		m_lightSystem->SetCamera(m_currentCamera);
	}

	void RaytracedDDGIRenderer::UnregisterCamera(Camera* camera)
	{
		ASSERT(m_currentCamera == camera);
		m_currentCamera = nullptr;
	}

	void RaytracedDDGIRenderer::PreUpdate()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void RaytracedDDGIRenderer::Update()
	{
		AbstractRaytracedDDGIController* raytracer;
		switch (g_currentRaytracer)
		{
		case CurrentDDGIRaytracer::Software:
			raytracer = m_softwareRaytracedDDGI.get();
			break;
		case CurrentDDGIRaytracer::Hardware:
			raytracer = m_hardwareRaytracedDDGI.get();
			break;
		default:
			raytracer = nullptr;
			ASSERT(false);
		}

		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);

		m_queue->ResetForFrame(m_currentFrameIndex);

		const auto commandList = m_queue->GetCommandList(m_currentFrameIndex);

		const auto swapchainResource = m_swapchainRenderTargets[m_currentFrameIndex]->GetImageResource().Get();
		const auto hdrRTVResource = m_mainColorRenderTarget->GetImageResource().Get();
		const auto swapchainRTVHandle = m_swapchainRenderTargets[m_currentFrameIndex]->GetRTV()->GetCPUHandle();
		const auto hdrRTVHandle = m_mainColorRenderTarget->GetRTV()->GetCPUHandle();

		ASSERT(m_currentCamera != nullptr);
		const jmath::mat4x4 mainCameraViewMatrix = m_currentCamera->GetViewMatrix();
		const jmath::mat4x4 mainCameraProjMatrix = m_currentCamera->GetProjMatrix();

		ViewProjectionMatrixData mainCameraMatrixVP = {
			.view = mainCameraViewMatrix,
			.proj = mainCameraProjMatrix
		};

		{
			const DynamicCpuBuffer<EngineData>* engineDataBuffer = EngineDataProvider::Get()->GetEngineDataBuffer();

			const auto data = static_cast<EngineData*>(engineDataBuffer->GetPtr(m_currentFrameIndex));
			data->cameraWorldPos = m_currentCamera->GetGameObject().GetTransform().GetPosition();
			data->time = Time::GetTime();
			data->cameraInvProj = jmath::inverse(mainCameraProjMatrix);
			data->cameraInvView = jmath::inverse(mainCameraViewMatrix);
			data->cameraNear = m_currentCamera->GetNear();
			data->cameraFar = m_currentCamera->GetFar();
			data->cameraFovRadians = m_currentCamera->GetFovRadians();
			data->screenWidth = m_width;
			data->screenHeight = m_height;
			data->cameraAspect = GetAspect();
		}

		m_lightSystem->Update(m_currentFrameIndex);

		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);

		// Drawing shadows
		{
			auto scopedEvent = ScopedGFXEvent(commandList, "Directional Light");

			m_lightSystem->RenderDirectionalShadows(
				commandList,
				m_currentFrameIndex,
				EngineDataProvider::Get()->GetStandardPhongSharedMaterial());
		}

		// Set main viewport-scissor rects
		GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

		//Drawing G-Buffer
		{
			auto scopedEvent = ScopedGFXEvent(commandList, "G-BUFFER rendering");

			m_gbuffer->BarrierColorToWrite(commandList);

			const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3] = {
				m_gbuffer->GetColorRTV()->GetCPUHandle(),
				m_gbuffer->GetNormalsRTV()->GetCPUHandle(),
				m_gbuffer->GetPositionRTV()->GetCPUHandle(),
			};
			const auto dsvHandle = m_gbuffer->GetDepthDSV()->GetCPUHandle();
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
			                             EngineDataProvider::Get()->GetStandardPhongSharedMaterial());

			m_gbuffer->BarrierColorToRead(commandList);
		}

		m_raytracingDataContainer->SetFrameData(m_currentFrameIndex, m_skybox->GetSkyboxTextureSrv());

		// Process raytracing
		{
			auto scopedEvent = ScopedGFXEvent(
				commandList,
				g_currentRaytracer == CurrentDDGIRaytracer::Software ? "Software Raytracing" : "Hardware Raytracing");

			raytracer->ProcessRaytracing(commandList, m_currentFrameIndex);


			const auto raytracedRTVHandle = raytracer->GetShadedRenderTexture()->GetRTV()->GetCPUHandle();

			GraphicsUtils::Barrier(commandList, raytracer->GetShadedRenderTexture()->GetImageResource().Get(),
			                       D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

			GraphicsUtils::SetViewportAndScissor(commandList, raytracer->GetRaytracedTextureWidth(), raytracer->GetRaytracedTextureHeight());

			commandList->OMSetRenderTargets(
				1,
				&raytracedRTVHandle,
				FALSE, nullptr);

			RenderDeferredShading(commandList, raytracer->GetGBuffer(), &mainCameraMatrixVP, raytracer);

			GraphicsUtils::Barrier(commandList, raytracer->GetShadedRenderTexture()->GetImageResource().Get(),
			                       D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			raytracer->GenerateProbeIrradiance(commandList, m_currentFrameIndex);
		}

		GraphicsUtils::SetViewportAndScissor(commandList, m_width, m_height);

		// Deferred shading 
		{
			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, nullptr);

			RenderDeferredShading(commandList, m_gbuffer.get(), &mainCameraMatrixVP, raytracer);
		}

		if (g_drawRaytracedImage)
		{
			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, nullptr);

			raytracer->DebugDrawRaytracedImage(commandList);
		}

		if (g_drawProbes)
		{
			auto scopedEvent = ScopedGFXEvent(commandList, "Draw probes");

			auto dsvHandle = m_gbuffer->GetDepthDSV()->GetCPUHandle();
			//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			commandList->OMSetRenderTargets(
				1,
				&hdrRTVHandle,
				FALSE, &dsvHandle);

			raytracer->DebugDrawProbes(commandList, m_currentFrameIndex, &mainCameraMatrixVP);
		}

		// HDR->LDR
		{
			auto scopedEvent = ScopedGFXEvent(commandList, "HDR->SDR transition + tonemapping");

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


			{
				auto scopedEvent1 = ScopedGFXEvent(commandList, "IMGUI drawings");

				DrawGui(commandList, &mainCameraMatrixVP);
			}


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

	void RaytracedDDGIRenderer::DrawGui(ID3D12GraphicsCommandList* commandList,
	                            const ViewProjectionMatrixData* viewProjectionData) const
	{
		// Draw axis gizmo
		{
			auto sm = EngineDataProvider::Get()->GetGizmoAxisDrawerSharedMaterial();

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

			GraphicsUtils::ProcessEngineBindings(
				commandList, sm->GetGraphicsPipeline(),
				m_currentFrameIndex,
				nullptr,
				viewProjectionData);

			commandList->DrawInstanced(
				6,
				1,
				0, 0);
		}
		float windowPosY = 0;
		float windowHeight = 150;
		ImGui::SetNextWindowPos({0, windowPosY});
		ImGui::SetNextWindowSize({300, windowHeight});
		{
			ImGui::Begin("Stats:");
			//ImGui::Text("Screen: %dx%d", m_width, m_height);
			ImGui::Text("Num triangles %d", m_trianglesCount);
			const jmath::vec3 camPos = m_currentCamera->GetGameObject().GetTransform().GetPosition();
			ImGui::Text("Camera: %.3f %.3f %.3f", camPos.x, camPos.y, camPos.z);
			ImGui::Checkbox("Debug draw raytraced image", &g_drawRaytracedImage);

			ImGui::RadioButton("Software raytracing", reinterpret_cast<int*>(&g_currentRaytracer), 0);
			ImGui::RadioButton("Hardware raytracing", reinterpret_cast<int*>(&g_currentRaytracer), 1);

			ImGui::End();
		}
		windowPosY += windowHeight;
		windowHeight = 75;
		ImGui::SetNextWindowPos({0, windowPosY});
		ImGui::SetNextWindowSize({300, windowHeight});
		{
			bool useDDGI = m_raytracingDataContainer->GetRaytracedProbesDataPtr()->useDDGI == 1;
			ImGui::Begin("DDGI:");
			ImGui::Checkbox("Draw probes", &g_drawProbes);
			ImGui::Checkbox("Use GI", &useDDGI);
			ImGui::End();
			m_raytracingDataContainer->GetRaytracedProbesDataPtr()->useDDGI = useDDGI ? 1 : 0;
		}
		windowPosY += windowHeight;
		windowHeight = 75;
		ImGui::SetNextWindowPos({0, windowPosY});
		ImGui::SetNextWindowSize({300, windowHeight});
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

	void RaytracedDDGIRenderer::RenderEntireSceneWithMaterials(
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
					GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), index, param.second);
				}

				GraphicsUtils::ProcessEngineBindings(
					commandList,
					sm->GetGraphicsPipeline(),
					m_currentFrameIndex,
					mr->GetGameObject().GetTransform().GetTransformIndexPtr(),
					viewProjectionData);

				commandList->DrawIndexedInstanced(
					mr->GetMesh()->GetIndexCount(),
					1,
					0, 0, 0);
				m_trianglesCount += mr->GetMesh()->GetIndexCount() / 3;
			}
		}
	}

	void RaytracedDDGIRenderer::RenderSceneForSharedMaterial(
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
				GraphicsUtils::AttachView(commandList, sharedMaterial->GetGraphicsPipeline(), index, param.second);
			}

			GraphicsUtils::ProcessEngineBindings(
				commandList,
				sharedMaterial->GetGraphicsPipeline(),
				m_currentFrameIndex,
				mr->GetGameObject().GetTransform().GetTransformIndexPtr(),
				viewProjectionData);

			commandList->DrawIndexedInstanced(
				mr->GetMesh()->GetIndexCount(),
				1,
				0, 0, 0);
			m_trianglesCount += mr->GetMesh()->GetIndexCount() / 3;
		}
	}

	void RaytracedDDGIRenderer::RenderDeferredShading(
		ID3D12GraphicsCommandList* commandList,
		const AbstractGBuffer* gBuffer,
		const ViewProjectionMatrixData* cameraVP,
		const AbstractRaytracedDDGIController* raytracer) const
	{
		const auto& sm = EngineDataProvider::Get()->GetDDGIDeferredShadingSharedMaterial();

		commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "colorTexture", gBuffer->GetColorSRV());
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "normalsTexture", gBuffer->GetNormalsSRV());
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "positionTexture", gBuffer->GetPositionSRV());
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "directionalLightData", m_lightSystem->GetDirectionalLightDataView(m_currentFrameIndex));
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "directionalLightShadowmap", m_lightSystem->GetDirectionalShadowmapView());
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "PCFSampler", EngineSamplersProvider::GetDepthPCFSampler());


		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "raytracedProbesData", m_raytracingDataContainer->GetProbesDataView(m_currentFrameIndex));
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "linearBlackBorderSampler", EngineSamplersProvider::GetLinearBlackBorderSampler());

		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "probeIrradianceTexture", raytracer->GetProbeIrradianceTexture()->GetSRV());
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "probeDepthTexture", raytracer->GetProbeDepthTexture()->GetSRV());

		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "clusteredEntryData", m_lightSystem->GetClusterEntryDataView(m_currentFrameIndex));
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "clusteredItemData", m_lightSystem->GetClusterItemDataView(m_currentFrameIndex));
		GraphicsUtils::AttachView(commandList, sm->GetGraphicsPipeline(), "lightData", m_lightSystem->GetLightDataView(m_currentFrameIndex));

		GraphicsUtils::ProcessEngineBindings(commandList, sm->GetGraphicsPipeline(), m_currentFrameIndex,
		                                     nullptr,
		                                     cameraVP);

		commandList->DrawIndexedInstanced(
			3,
			1,
			0, 0, 0);
	}

	void RaytracedDDGIRenderer::CopyRTVResource(
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

	float RaytracedDDGIRenderer::GetAspect() const noexcept
	{
		return static_cast<float>(m_width) / static_cast<float>(m_height);
	}

	float RaytracedDDGIRenderer::GetWidth_f() const noexcept
	{
		return static_cast<float>(m_width);
	}

	float RaytracedDDGIRenderer::GetHeight_f() const noexcept
	{
		return static_cast<float>(m_height);
	}

	uint32_t RaytracedDDGIRenderer::GetWidth() const noexcept
	{
		return m_width;
	}

	uint32_t RaytracedDDGIRenderer::GetHeight() const noexcept
	{
		return m_height;
	}
}