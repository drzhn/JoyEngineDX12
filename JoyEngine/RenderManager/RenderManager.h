#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <array>
#include <set>
#include <map>
#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "CommonEngineStructs.h"
#include "GBuffer.h"
#include "IRenderManager.h"
#include "Skybox.h"

#include "Tonemapping.h"
#include "LightSystems/ClusteredLightSystem.h"
#include "Raytracing/AbstractRaytracedDDGI.h"
#include "Raytracing/RaytracedDDGIDataContainer.h"


using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	enum class EngineBindingType;

	class CommandQueue;
	class Mesh;
	class Camera;
	class SharedMaterial;
	class Texture;
	class RenderTexture;
	class ResourceView;
	class DepthTexture;

	class RenderManager final : public IRenderManager
	{
	public:
		RenderManager() = delete;

		explicit RenderManager(HWND windowHandle);

		~RenderManager() override = default;

		void Init();

		void Start() const;

		void Stop();

		void PreUpdate();

		void Update();

		void DrawGui(ID3D12GraphicsCommandList* commandList, const ViewProjectionMatrixData* viewProjectionData) const;

		void RegisterSharedMaterial(SharedMaterial*) override;

		void UnregisterSharedMaterial(SharedMaterial*) override;

		void RegisterCamera(Camera* camera) override;

		void UnregisterCamera(Camera* camera) override;

		[[nodiscard]] float GetAspect() const noexcept override;

		[[nodiscard]] float GetWidth_f() const noexcept override;
		[[nodiscard]] float GetHeight_f() const noexcept override;

		[[nodiscard]] uint32_t GetWidth() const noexcept override;
		[[nodiscard]] uint32_t GetHeight() const noexcept override;

		[[nodiscard]] const uint32_t GetFrameCount() const noexcept override { return FRAME_COUNT; }
		[[nodiscard]] const uint32_t GetCurrentFrameIndex() const noexcept override { return m_currentFrameIndex; }

		[[nodiscard]] ILightSystem& GetLightSystem() const noexcept override { return *m_lightSystem; }

	private:
		void RenderEntireSceneWithMaterials(
			ID3D12GraphicsCommandList* commandList,
			const ViewProjectionMatrixData* viewProjectionData
		) const;

		void RenderSceneForSharedMaterial(
			ID3D12GraphicsCommandList* commandList,
			const ViewProjectionMatrixData* viewProjectionData,
			SharedMaterial* sharedMaterial
		) const;

		void RenderDeferredShading(
			ID3D12GraphicsCommandList* commandList,
			const AbstractGBuffer* gBuffer, const ViewProjectionMatrixData* cameraVP, const AbstractRaytracedDDGI* raytracer
		) const;

		static void CopyRTVResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* rtvResource,
		                            ID3D12Resource* copyResource);

	private:
		static constexpr uint32_t FRAME_COUNT = 3;

		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<RenderTexture>, FRAME_COUNT> m_swapchainRenderTargets;

		std::unique_ptr<RTVGbuffer> m_gbuffer;
		std::unique_ptr<Skybox> m_skybox;
		std::unique_ptr<RenderTexture> m_mainColorRenderTarget;


		std::unique_ptr<Tonemapping> m_tonemapping;
		std::unique_ptr<RaytracedDDGIDataContainer> m_raytracingDataContainer;
		std::unique_ptr<AbstractRaytracedDDGI> m_softwareRaytracedDDGI;
		std::unique_ptr<AbstractRaytracedDDGI> m_hardwareRaytracedDDGI;
		std::unique_ptr<ClusteredLightSystem> m_lightSystem;
		std::set<SharedMaterial*> m_sharedMaterials;

		Camera* m_currentCamera;

		std::unique_ptr<CommandQueue> m_queue;
		uint32_t m_currentFrameIndex;

		uint32_t m_width;
		uint32_t m_height;

		uint32_t m_imguiDescriptorIndex;
		mutable uint32_t m_trianglesCount = 0;
	};
}

#endif //RENDER_MANAGER_H
