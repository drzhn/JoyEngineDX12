#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>
#include <memory>

#include "Common/Resource.h"
#include "SharedMaterial.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/ResourceHandle.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	struct MaterialArgs
	{
		GUID sharedMaterial;
		std::map<uint32_t, ID3D12DescriptorHeap*> rootParams;
	};

	class Material final : public Resource
	{
	public :
		Material() = delete;

		explicit Material(GUID);
		explicit Material(GUID, MaterialArgs);

		~Material() final;

		[[nodiscard]] SharedMaterial* GetSharedMaterial() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;
		std::map<uint32_t, ID3D12DescriptorHeap*>& GetRootParams() { return m_rootParams; }
		std::vector<ID3D12DescriptorHeap*>& GetHeaps() { return m_heaps; }
	private :
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::map<uint32_t, ID3D12DescriptorHeap*> m_rootParams;
		std::vector<ID3D12DescriptorHeap*> m_heaps;
	};
}

#endif //MATERIAL_H
