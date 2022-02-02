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

#include "Components/Light.h"
#include "ResourceManager/Mesh.h"
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

	private:
		static const UINT FrameCount = 3;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;

		std::array<std::unique_ptr<Texture>, FrameCount> m_renderTargets;

		std::unique_ptr<Texture> m_depthAttachment;

		std::unique_ptr<RenderTexture> m_positionAttachment;
		std::unique_ptr<RenderTexture> m_normalAttachment;
		std::unique_ptr<RenderTexture> m_lightingAttachment;

		ResourceHandle<Mesh> m_planeMesh;

		std::set<SharedMaterial*> m_sharedMaterials;
		std::set<Light*> m_lights;
		Light* m_directionLight;
		Camera* m_currentCamera;

		std::unique_ptr<CommandQueue> m_queue;
		uint32_t m_currentFrameIndex;

		uint32_t m_width;
		uint32_t m_height;
	};
}

#endif //RENDER_MANAGER_H
