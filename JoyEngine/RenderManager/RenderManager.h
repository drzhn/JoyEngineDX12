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

#include "Common/Singleton.h"
#include "ResourceManager/DynamicBuffer.h"

#include "Tonemapping.h"
#include "Raytracing.h"

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	enum EngineBindingType;

	struct JoyData;
	struct MVP;

	class CommandQueue;
	class Light;
	class Mesh;
	class Camera;
	class SharedMaterial;
	class Texture;
	class RenderTexture;
	class ResourceView;
	class DepthTexture;

	class RenderManager : public Singleton<RenderManager>
	{
	public:
		RenderManager() = default;

		~RenderManager() = default;

		void Init();

		void Start();

		void Stop();

		void Update();

		void DrawGui(ID3D12GraphicsCommandList* commandList);

		void RegisterSharedMaterial(SharedMaterial*);

		void UnregisterSharedMaterial(SharedMaterial*);

		void RegisterLight(Light*);

		void UnregisterLight(Light*);

		void RegisterDirectionLight(Light*);

		void UnregisterDirectionLight(Light*);

		void RegisterCamera(Camera* camera);

		void UnregisterCamera(Camera* camera);

		[[nodiscard]] float GetAspect() const noexcept;
		[[nodiscard]] float GetWidth() const noexcept;
		[[nodiscard]] float GetHeight() const noexcept;
		[[nodiscard]] static constexpr uint32_t GetFrameCount() noexcept { return frameCount; }
		[[nodiscard]] static constexpr DXGI_FORMAT GetMainColorFormat() noexcept { return hdrRTVFormat; }
		[[nodiscard]] static constexpr DXGI_FORMAT GetHdrRTVFormat() noexcept { return hdrRTVFormat; }
		[[nodiscard]] static constexpr DXGI_FORMAT GetLdrRTVFormat() noexcept { return ldrRTVFormat; }
		[[nodiscard]] static constexpr DXGI_FORMAT GetGBufferFormat() noexcept { return gBufferFormat; }
		[[nodiscard]] static constexpr DXGI_FORMAT GetDepthFormat() noexcept { return depthFormat; }
		[[nodiscard]] static constexpr DXGI_FORMAT GetSSAOFormat() noexcept { return ssaoFormat; }

	private:
		void RenderEntireScene(
			ID3D12GraphicsCommandList* commandList,
			glm::mat4 view,
			glm::mat4 proj
		) const;

		void RenderEntireSceneWithMaterials(
			ID3D12GraphicsCommandList* commandList,
			glm::mat4 view,
			glm::mat4 proj,
			bool isDrawingMainColor
		) const;

		void ProcessEngineBindings(
			ID3D12GraphicsCommandList* commandList,
			const std::map<uint32_t, EngineBindingType>& bindings,
			MVP* mvp,
			bool isDrawingMainColor
		) const;

		static void CopyRTVResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* rtvResource, ID3D12Resource* copyResource);

	private:
		static constexpr uint32_t frameCount = 3;


		static constexpr DXGI_FORMAT hdrRTVFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		static constexpr DXGI_FORMAT ldrRTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
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

		std::unique_ptr<DynamicBuffer<JoyData>> m_engineDataBuffer;

		//std::set<ParticleSystem*> m_particleSystems;
		std::set<SharedMaterial*> m_sharedMaterials;
		//std::set<Light*> m_lights;
		//Light* m_directionLight;
		//CubemapRenderer* m_cubemap; // TODO add collection of cubemaps
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
