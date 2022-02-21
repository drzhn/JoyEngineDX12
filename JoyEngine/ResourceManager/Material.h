#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "Common/Resource.h"

#include "ResourceManager/ResourceHandle.h"
#include "Utils/GUID.h"

#include <d3d12.h>
#include "RenderManager/RenderManager.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class Texture;
	class Buffer;
	class SharedMaterial;

	struct MaterialArgs
	{
		GUID sharedMaterial;
		std::map<uint32_t, ID3D12DescriptorHeap*> rootParams;

		// material is responsible for storing references to resources
		// TODO combine texture and buffer to some AbstractResource
		std::vector<ResourceHandle<Texture>> textures;
		std::vector<std::shared_ptr<Buffer*>> buffers;
	};

	class Material final : public Resource
	{
	public :
		Material() = delete;

		explicit Material(GUID);
		explicit Material(GUID, MaterialArgs);

		~Material() override = default;

		[[nodiscard]] SharedMaterial* GetSharedMaterial() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;
		std::map<uint32_t, ID3D12DescriptorHeap*>& GetRootParams() { return m_rootParams; }
		//std::vector<ID3D12DescriptorHeap*>& GetHeaps() { return m_heaps; }
	private :
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::map<uint32_t, ID3D12DescriptorHeap*> m_rootParams;
		//std::vector<ID3D12DescriptorHeap*> m_heaps;

		std::vector<ResourceHandle<Texture>> m_textures;
		std::vector<std::shared_ptr<Buffer*>> m_buffers;
	};
}

#endif //MATERIAL_H
