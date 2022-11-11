#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <array>
#include <set>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "CommonEngineStructs.h"
#include "IRenderManager.h"
#include "Common/Singleton.h"
#include "ResourceManager/DynamicCpuBuffer.h"

#include "Tonemapping.h"
#include "Raytracing/Raytracing.h"

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	enum class EngineBindingType;

	class CommandQueue;
	class Light;
	class Mesh;
	class Camera;
	class SharedMaterial;
	class Texture;
	class RenderTexture;
	class ResourceView;
	class DepthTexture;

	class RenderManager final : public Singleton<RenderManager>, public IRenderManager
	{
	public:
		RenderManager() = default;

		~RenderManager() override = default;

		void Init();

		void Start() const;

		void Stop();

		void Update();

		void DrawGui(ID3D12GraphicsCommandList* commandList) const;

		void RegisterSharedMaterial(SharedMaterial*) override;

		void UnregisterSharedMaterial(SharedMaterial*) override;

		void RegisterLight(Light*) override;

		void UnregisterLight(Light*) override;

		void RegisterDirectionLight(Light*) override;

		void UnregisterDirectionLight(Light*) override;

		void RegisterCamera(Camera* camera) override;

		void UnregisterCamera(Camera* camera) override;

		float GetAspect() const noexcept override;
		float GetWidth() const noexcept override;
		float GetHeight() const noexcept override;
		[[nodiscard]] uint32_t GetFrameCount() noexcept { return frameCount; }
		[[nodiscard]] DXGI_FORMAT GetMainColorFormat() noexcept { return hdrRTVFormat; };
		[[nodiscard]] DXGI_FORMAT GetHdrRTVFormat() noexcept { return hdrRTVFormat; }
		[[nodiscard]] DXGI_FORMAT GetSwapchainFormat() noexcept { return swapchainFormat; }
		[[nodiscard]] DXGI_FORMAT GetGBufferFormat() noexcept { return gBufferFormat; }
		[[nodiscard]] DXGI_FORMAT GetDepthFormat() noexcept { return depthFormat; }
		[[nodiscard]] DXGI_FORMAT GetSSAOFormat() noexcept { return ssaoFormat; }

	private:
		//void RenderEntireScene(
		//	ID3D12GraphicsCommandList* commandList,
		//	glm::mat4 view,
		//	glm::mat4 proj
		//) const;

		void RenderEntireSceneWithMaterials(
			ID3D12GraphicsCommandList* commandList,
			const ViewProjectionMatrixData* viewProjectionData
		) const;

		void ProcessEngineBindings(
			ID3D12GraphicsCommandList* commandList,
			const std::map<uint32_t, EngineBindingType>& bindings,
			const uint32_t* modelIndex,
			const ViewProjectionMatrixData* viewProjectionMatrix
		) const;
		void UpdateObjectMatrices() const;

		static void CopyRTVResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* rtvResource, ID3D12Resource* copyResource);

	private:
		static constexpr uint32_t frameCount = 3;


		static constexpr DXGI_FORMAT hdrRTVFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		static constexpr DXGI_FORMAT swapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT gBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		static constexpr DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT ssaoFormat = DXGI_FORMAT_R16_FLOAT;


		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<RenderTexture>, frameCount> m_swapchainRenderTargets;

		std::unique_ptr<DepthTexture> m_depthAttachment;
		std::unique_ptr<RenderTexture> m_hdrRenderTarget;


		std::unique_ptr<Tonemapping> m_tonemapping;
		std::unique_ptr<Raytracing> m_raytracing;
		//std::unique_ptr<Texture> m_renderTargetCopyAttachment;


		//std::unique_ptr<RenderTexture> m_positionAttachment;
		//std::unique_ptr<RenderTexture> m_worldNormalAttachment;
		//std::unique_ptr<RenderTexture> m_viewNormalAttachment;
		//std::unique_ptr<RenderTexture> m_lightingAttachment;

		std::unique_ptr<DynamicCpuBuffer<EngineData>> m_engineDataBuffer;

		//std::set<ParticleSystem*> m_particleSystems;
		std::set<SharedMaterial*> m_sharedMaterials;
		//std::set<Light*> m_lights;
		//Light* m_directionLight;
		Camera* m_currentCamera;

		std::unique_ptr<CommandQueue> m_queue;
		uint32_t m_currentFrameIndex;

		uint32_t m_width;
		uint32_t m_height;

		uint32_t m_imguiDescriptorIndex;
		mutable  uint32_t m_trianglesCount = 0;
	};
}

#endif //RENDER_MANAGER_H
