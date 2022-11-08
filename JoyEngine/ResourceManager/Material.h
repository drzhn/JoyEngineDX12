#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "Common/Resource.h"
#include "ResourceManager/Buffer.h"

#include "ResourceManager/ResourceManager.h"
#include "Utils/GUID.h"

#include <d3d12.h>
#include "RenderManager/RenderManager.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class Texture;
	//class Buffer;
	class SharedMaterial;

	class Material final : public Resource
	{
	public :
		Material() = delete;
		explicit Material(GUID);
		explicit Material(
			GUID guid, 
			const std::map<std::string, std::string>& bindings, 
			bool bindingsArePaths);

		~Material() override = default;

		[[nodiscard]] SharedMaterial* GetSharedMaterial() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;
		[[nodiscard]] const std::map<uint32_t, ResourceView*>& GetRootParams() { return m_rootParams; }
		[[nodiscard]] uint32_t GetMaterialIndex() const noexcept { return m_materialIndex; }

	private:
		void InitMaterial(const std::map<std::string, std::string>& bindings, bool bindingsArePaths);

	private :
		uint32_t m_materialIndex;
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::map<uint32_t, ResourceView*> m_rootParams;

		std::vector<ResourceHandle<Texture>> m_textures;
		std::vector<std::unique_ptr<Buffer>> m_buffers;

		static uint32_t s_currentMaterialIndex;
	};
}

#endif //MATERIAL_H
