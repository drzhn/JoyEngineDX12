#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include "Common/Resource.h"
#include "Shader.h"
#include "Texture.h"
#include "Utils/GUID.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	class MeshRenderer;

	struct SharedMaterialArgs
	{
		GUID shader;
		bool hasVertexInput;
		bool hasMVP;
		bool depthTest;
		bool depthWrite;
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParams;
	};

	class SharedMaterial final : public Resource
	{
	public :
		SharedMaterial() = delete;

		explicit SharedMaterial(GUID);
		explicit SharedMaterial(GUID, SharedMaterialArgs);

		~SharedMaterial() final;

		[[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature; };

		[[nodiscard]] ComPtr<ID3D12PipelineState> GetPipelineObject() const noexcept { return m_pipelineState; };

		[[nodiscard]] bool IsLoaded() const noexcept override;

		[[nodiscard]] std::set<MeshRenderer*>& GetMeshRenderers();

		void RegisterMeshRenderer(MeshRenderer* meshRenderer);

		void UnregisterMeshRenderer(MeshRenderer* meshRenderer);

	private :
		std::set<MeshRenderer*> m_meshRenderers;
		ResourceHandle<Shader> m_shader;

		bool m_hasVertexInput = false;
		bool m_hasMVP = false;
		bool m_depthTest = false;
		bool m_depthWrite = false;

		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;

	private:
		void CreateRootSignature(const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParams);
		void CreateGraphicsPipeline();
	};
}

#endif //SHARED_MATERIAL_H
