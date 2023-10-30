#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <memory>
#include <vector>
#include <rapidjson/document.h>

#include "ResourceHandle.h"
#include "Common/Resource.h"
#include "ResourceView.h"
#include "Buffers/Buffer.h"


namespace JoyEngine
{
	class Texture;
	//class Buffer;
	class SharedMaterial;

	class Material final : public Resource
	{
		DECLARE_JOY_OBJECT(Material, Resource);

	public :
		Material() = delete;
		explicit Material(const char* materialPath);
		explicit Material(
			uint64_t id,
			rapidjson::Value& materialJson);

		~Material() override = default;

		[[nodiscard]] SharedMaterial* GetSharedMaterial() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override;
		[[nodiscard]] const std::map<uint32_t, ResourceView*>& GetRootParams() { return m_rootParams; }
		[[nodiscard]] uint32_t GetMaterialIndex() const noexcept { return m_materialIndex; }

	private:
		void InitMaterial(rapidjson::Value& materialJson);

	private :
		uint32_t m_materialIndex;
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::map<uint32_t, ResourceView*> m_rootParams;

		std::vector<ResourceHandle<Texture>> m_textures;
		std::vector<std::unique_ptr<Buffer>> m_buffers;

		static uint32_t s_currentMaterialIndex;
		static std::map<std::string, size_t> s_fieldOffsets;
	};
}

#endif //MATERIAL_H
