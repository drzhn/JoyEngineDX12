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

	enum EngineBindingType
	{
		ModelViewProjection,
		LightAttachment,
		EnvironmentCubemap,
		EnvironmentConvolutedCubemap,
		EngineData
	};

	struct SharedMaterialArgs
	{
		GUID shader;
		ShaderTypeFlags shaderTypes;
		bool hasVertexInput;
		bool depthTest;
		bool depthWrite;
		D3D12_CULL_MODE cullMode;
		D3D12_COMPARISON_FUNC depthComparisonFunc;
		CD3DX12_BLEND_DESC blendDesc;
		std::vector<DXGI_FORMAT> renderTargetsFormats;
		DXGI_FORMAT depthFormat;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
	};

	struct ComputePipelineArgs
	{
		GUID computeShaderGuid;
	};

	class AbstractPipelineObject
	{
	public:
		[[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature; }
		[[nodiscard]] ComPtr<ID3D12PipelineState> GetPipelineObject() const noexcept { return m_pipelineState; };
		[[nodiscard]] const ShaderInput& GetShaderInputByName(const std::string&) const;
		[[nodiscard]] uint32_t GetRootIndexByName(const std::string&) const;

	protected:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ResourceHandle<Shader> m_shader;
		std::map<std::string, uint32_t> m_rootIndices;
		std::map<uint32_t, EngineBindingType> m_engineBindings;

	protected:
		void CreateShaderAndRootSignature(GUID shaderGuid, ShaderTypeFlags shaderTypes);
	private:
		void CreateRootSignature(CD3DX12_ROOT_PARAMETER1* params, uint32_t paramsCount);
	};

	class ComputePipeline final : public Resource, public AbstractPipelineObject
	{
	public:
		ComputePipeline() = delete;
		explicit ComputePipeline(GUID, ComputePipelineArgs);
		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }
	private:
		void CreateComputePipeline();
	};


	// TODO: make AbstractPipelineObject -> GraphicsPipeline -> SharedMaterial
	class SharedMaterial final : public Resource, public AbstractPipelineObject 
	{
	public :
		SharedMaterial() = delete;

		explicit SharedMaterial(GUID);
		explicit SharedMaterial(GUID, SharedMaterialArgs);

		~SharedMaterial() final;

		[[nodiscard]] bool IsLoaded() const noexcept override;

		[[nodiscard]] std::set<MeshRenderer*>& GetMeshRenderers();
		[[nodiscard]] std::map<uint32_t, EngineBindingType>& GetEngineBindings();

		void RegisterMeshRenderer(MeshRenderer* meshRenderer);

		void UnregisterMeshRenderer(MeshRenderer* meshRenderer);

	private :
		std::set<MeshRenderer*> m_meshRenderers;

		bool m_hasVertexInput = false;
		bool m_depthTest = false;
		bool m_depthWrite = false;
		D3D12_COMPARISON_FUNC m_depthComparisonFunc;
		D3D12_CULL_MODE m_cullMode;

	private:
		void CreateGraphicsPipeline(const std::vector<DXGI_FORMAT>& renderTargetsFormats, CD3DX12_BLEND_DESC blendDesc, DXGI_FORMAT depthFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);
		static std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	};
}

#endif //SHARED_MATERIAL_H
