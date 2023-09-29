#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include "Common/Resource.h"
#include "Texture.h"
#include "Pipelines/GraphicsPipeline.h"
#include "Utils/GUID.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
	class MeshRenderer;

	class SharedMaterial final : public Resource
	{
	public :
		SharedMaterial() = delete;

		explicit SharedMaterial(GUID);
		explicit SharedMaterial(GUID, GraphicsPipelineArgs);

		~SharedMaterial() final;

		[[nodiscard]] bool IsLoaded() const noexcept override;

		[[nodiscard]] std::set<MeshRenderer*>& GetMeshRenderers();
		[[nodiscard]] GraphicsPipeline* GetGraphicsPipeline() const;
		[[nodiscard]] uint32_t GetBindingIndexByHash(uint32_t hash) const;

		void RegisterMeshRenderer(MeshRenderer* meshRenderer);

		void UnregisterMeshRenderer(MeshRenderer* meshRenderer);

	private :
		std::set<MeshRenderer*> m_meshRenderers;
		std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
	};
}

#endif //SHARED_MATERIAL_H
