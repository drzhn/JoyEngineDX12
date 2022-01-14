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
	struct MaterialData
	{
		GUID sharedMaterial;
		std::map<uint32_t, D3D12_CPU_DESCRIPTOR_HANDLE> rootParams;
	};

	class Material final : public Resource
	{
	public :
		Material() = delete;

		explicit Material(GUID);
		explicit Material(GUID, MaterialData);

		~Material() final;

		[[nodiscard]] SharedMaterial* GetSharedMaterial() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;
	private :
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::map<uint32_t, D3D12_CPU_DESCRIPTOR_HANDLE> m_rootParams;
	};
}

#endif //MATERIAL_H
