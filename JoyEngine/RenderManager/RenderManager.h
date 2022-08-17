#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <array>
#include <set>
#include <chrono>
#include <memory>

#include <glm/glm.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "JoyTypes.h"
#include "Common/Singleton.h"
#include "ResourceManager/SharedMaterial.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class Buffer;
	class CubemapRenderer;
	class ParticleSystem;
	class CommandQueue;
	class Light;
	class Mesh;
	class Camera;
	class SharedMaterial;
	class Texture;
	class RenderTexture;
	class ResourceView;

	class RenderManager : public Singleton<RenderManager>
	{
	public:
		RenderManager() = default;

		~RenderManager() = default;

		void Init();

		void Start();

		void Stop();

		void Update();

		void RegisterSharedMaterial(SharedMaterial*);

		void UnregisterSharedMaterial(SharedMaterial*);

		void RegisterLight(Light*);

		void UnregisterLight(Light*);

		void RegisterDirectionLight(Light*);

		void UnregisterDirectionLight(Light*);

		void RegisterCamera(Camera* camera);

		void UnregisterCamera(Camera* camera);

		void RegisterParticleSystem(ParticleSystem* ps);

		void UnregisterParticleSystem(ParticleSystem* ps);

		void RegisterCubemapRenderer(CubemapRenderer* cr);

		void UnregisterCubemapRenderer(CubemapRenderer* cr);

		[[nodiscard]] float GetAspect() const noexcept;
		[[nodiscard]] float GetWidth() const noexcept;
		[[nodiscard]] float GetHeight() const noexcept;
		[[nodiscard]] static DXGI_FORMAT GetMainColorFormat() noexcept { return ldrRTVFormat; }
		[[nodiscard]] static DXGI_FORMAT GetHdrRTVFormat() noexcept { return hdrRTVFormat; }
		[[nodiscard]] static DXGI_FORMAT GetLdrRTVFormat() noexcept { return ldrRTVFormat; }
		[[nodiscard]] static DXGI_FORMAT GetGBufferFormat() noexcept { return gBufferFormat; }
		[[nodiscard]] static DXGI_FORMAT GetDepthFormat() noexcept { return depthFormat; }
		[[nodiscard]] static DXGI_FORMAT GetSSAOFormat() noexcept { return ssaoFormat; }

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

		static void SetViewportAndScissor(ID3D12GraphicsCommandList* commandList, uint32_t width, uint32_t height);
		static void AttachViewToGraphics(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex, const ResourceView* view);
		static void AttachViewToCompute(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex, const ResourceView* view);
		static void Barrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		static void CopyRTVResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* rtvResource, ID3D12Resource* copyResource);

	private:
		static constexpr DXGI_FORMAT hdrRTVFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		static constexpr DXGI_FORMAT ldrRTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT gBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		static constexpr DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT ssaoFormat = DXGI_FORMAT_R16_FLOAT;

		static constexpr UINT FrameCount = 3;

		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<RenderTexture>, FrameCount> m_swapchainRenderTargets;

		std::unique_ptr<DepthTexture> m_depthAttachment;
		//std::unique_ptr<RenderTexture> m_hdrRenderTarget;
		//std::unique_ptr<Texture> m_renderTargetCopyAttachment;

		//std::unique_ptr<UAVTexture> m_hrdDownScaledTexture;

		//std::unique_ptr<UAVTexture> m_bloomFirstTexture;
		//std::unique_ptr<UAVTexture> m_bloomSecondTexture;

		//std::unique_ptr<Buffer> m_hdrLuminationBuffer;
		//std::unique_ptr<ResourceView> m_hdrLuminationBufferUAVView;
		//std::unique_ptr<ResourceView> m_hdrLuminationBufferSRVView;
		//std::unique_ptr<Buffer> m_hdrPrevLuminationBuffer;
		//std::unique_ptr<ResourceView> m_hdrPrevLuminationBufferUAVView;


		//std::unique_ptr<RenderTexture> m_positionAttachment;
		//std::unique_ptr<RenderTexture> m_worldNormalAttachment;
		//std::unique_ptr<RenderTexture> m_viewNormalAttachment;
		//std::unique_ptr<RenderTexture> m_lightingAttachment;

		//std::unique_ptr<SSAO> m_ssaoEffect;

		//ResourceHandle<Mesh> m_planeMesh;
		//ResourceHandle<Mesh> m_cubeMesh;

		//std::unique_ptr<Buffer> m_engineDataBuffer;
		//std::unique_ptr<ResourceView> m_engineDataBufferView;

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
	};
}

#endif //RENDER_MANAGER_H
