#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <array>
#include <set>
#include <chrono>
#include <memory>

#include "ResourceManager/ResourceHandle.h"

#include <glm/glm.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "JoyTypes.h"
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

	class RenderManager
	{
	public:
		RenderManager() = default;

		~RenderManager() = default;

		void Init();

		void Start()
		{
		}

		void Stop();

		void Update();
		void ProcessEngineBindings(ID3D12GraphicsCommandList* commandList, const std::map<uint32_t, EngineBindingType>& bindings, MVP mvp, bool isDrawingMainColor) const;


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

	private:
		void RenderEntireScene(
			ID3D12GraphicsCommandList* commandList,
			glm::mat4 view,
			glm::mat4 proj) const;

		void RenderEntireSceneWithMaterials(
			::ID3D12GraphicsCommandList* commandList,
			glm::mat4 view,
			glm::mat4 proj, bool isDrawingMainColor
		) const;

		static void SetViewportAndScissor(ID3D12GraphicsCommandList* commandList, uint32_t width, uint32_t height);
		static void AttachViewToGraphics(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex, const ResourceView* view);
		static void AttachViewToCompute(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex, const ResourceView* view);

	private:
		static const UINT FrameCount = 3;

		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<Texture>, FrameCount> m_renderTargets;

		std::unique_ptr<DepthTexture> m_depthAttachment;
		std::unique_ptr<Texture> m_renderTargetCopyAttachment;

		std::unique_ptr<RenderTexture> m_positionAttachment;
		std::unique_ptr<RenderTexture> m_normalAttachment;
		std::unique_ptr<RenderTexture> m_lightingAttachment;

		ResourceHandle<Mesh> m_planeMesh;

		std::unique_ptr<Buffer> m_engineDataBuffer;
		std::unique_ptr<ResourceView> m_engineDataBufferView;

		std::set<ParticleSystem*> m_particleSystems;
		std::set<SharedMaterial*> m_sharedMaterials;
		std::set<Light*> m_lights;
		Light* m_directionLight;
		CubemapRenderer* m_cubemap; // TODO add collection of cubemaps
		Camera* m_currentCamera;

		std::unique_ptr<CommandQueue> m_queue;
		uint32_t m_currentFrameIndex;

		uint32_t m_width;
		uint32_t m_height;
	};
}

#endif //RENDER_MANAGER_H
