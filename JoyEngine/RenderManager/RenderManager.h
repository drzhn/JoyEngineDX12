#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "windows.h"

#include <vector>
#include <array>
#include <set>
#include <chrono>
#include <map>
#include <memory>

//#include "ResourceManager/Texture.h"

//#include "Components/MeshRenderer.h"
//#include "Components/Camera.h"

#include "Common/CommandQueue.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;


#include "Components/Camera.h"
#include "ResourceManager/SharedMaterial.h"

namespace JoyEngine
{
	class RenderObject;

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

		void DrawFrame();

		void RegisterSharedMaterial(SharedMaterial* sharedMaterial);

		void UnregisterSharedMaterial(SharedMaterial* sharedMaterial);

		void RegisterCamera(Camera* camera);

		void UnregisterCamera(Camera* camera);

		//[[nodiscard]] Swapchain* GetSwapchain() const noexcept;

		[[nodiscard]] float GetAspect() const noexcept;
		[[nodiscard]] float GetWidth() const noexcept;
		[[nodiscard]] float GetHeight() const noexcept;
		//[[nodiscard]] Texture* GetGBufferPositionTexture() const noexcept;
		//[[nodiscard]] Texture* GetGBufferNormalTexture() const noexcept;

	private:
		void CreateRenderPass();

		void CreateFramebuffers();

		void CreateCommandBuffers();

		void WriteCommandBuffers(uint32_t imageIndex) const;

		void ResetCommandBuffers(uint32_t imageIndex) const;

		void CreateSyncObjects();

	private:
		static const UINT FrameCount = 3;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<Texture>, FrameCount> m_renderTargets;

		std::unique_ptr<Texture> m_depthAttachment;

		std::unique_ptr<Texture> m_positionAttachment;
		std::unique_ptr<Texture> m_normalAttachment;


		std::set<SharedMaterial*> m_sharedMaterials;
		Camera* m_currentCamera;

		std::unique_ptr<CommandQueue> m_queue;
		uint32_t m_currentFrameIndex;

		uint32_t m_width;
		uint32_t m_height;
	};
}

#endif //RENDER_MANAGER_H
