#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <array>
#include <set>
#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "CommonEngineStructs.h"
#include "RenderManager/GBuffer.h"
#include "RenderManager/IRenderer.h"
#include "RenderManager/Skybox.h"

#include "RenderManager/Tonemapping.h"
#include "RenderManager/LightSystems/ClusteredLightSystem.h"
#include "RenderManager/RaytracedDDGIRenderer/AbstractRaytracedDDGIController.h"
#include "RenderManager/RaytracedDDGIRenderer/RaytracedDDGIDataContainer.h"


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

	class RaytracedDDGIRenderer final : public IRenderer
	{
	public:
		RaytracedDDGIRenderer() = delete;

		explicit RaytracedDDGIRenderer(HWND windowHandle);

		~RaytracedDDGIRenderer() override = default;

		void Init(Skybox* skybox) override;

		void Start() const override;

		void Stop() override;

		void PreUpdate() override;

		void Update() override;

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
			const AbstractGBuffer* gBuffer, const ViewProjectionMatrixData* cameraVP, const AbstractRaytracedDDGIController* raytracer
		) const;

		static void CopyRTVResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* rtvResource,
		                            ID3D12Resource* copyResource);

	private:
		static constexpr uint32_t FRAME_COUNT = 3;
		Skybox* m_skybox;

		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<RenderTexture>, FRAME_COUNT> m_swapchainRenderTargets;

		std::unique_ptr<RTVGbuffer> m_gbuffer;
		std::unique_ptr<RenderTexture> m_mainColorRenderTarget;

		std::unique_ptr<Tonemapping> m_tonemapping;
		std::unique_ptr<RaytracedDDGIDataContainer> m_raytracingDataContainer;
		std::unique_ptr<AbstractRaytracedDDGIController> m_softwareRaytracedDDGI;
		std::unique_ptr<AbstractRaytracedDDGIController> m_hardwareRaytracedDDGI;
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