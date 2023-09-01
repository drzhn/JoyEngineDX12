#ifndef SHARED_MATERIAL_H
#define SHARED_MATERIAL_H

#include "Common/Resource.h"
#include "Shader.h"
#include "Texture.h"
#include "Utils/GUID.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
	class MeshRenderer;

	enum class EngineBindingType
	{
		ObjectIndexData,
		ModelMatrixData,
		ViewProjectionMatrixDataGraphics,
		ViewProjectionMatrixDataCompute,
		EngineDataGraphics,
		EngineDataCompute,
	};

	struct GraphicsPipelineArgs
	{
		GUID shader;
		ShaderTypeFlags shaderTypes;
		bool hasVertexInput;
		bool depthTest;
		bool depthWrite;
		D3D12_CULL_MODE cullMode;
		D3D12_COMPARISON_FUNC depthComparisonFunc;
		CD3DX12_BLEND_DESC blendDesc;
		DXGI_FORMAT renderTargetsFormats[8];
		uint32_t renderTargetsFormatsSize;
		DXGI_FORMAT depthFormat;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
	};

	struct ComputePipelineArgs
	{
		GUID computeShaderGuid;
		D3D_SHADER_MODEL shaderModel;
	};

	class AbstractPipelineObject
	{
	public:
		[[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature; }
		[[nodiscard]] ComPtr<ID3D12PipelineState> GetPipelineObject() const noexcept { return m_pipelineState; };
		[[nodiscard]] ShaderInput const* GetShaderInputByName(const std::string&) const;
		[[nodiscard]] uint32_t GetBindingIndexByName(const std::string&) const;
		[[nodiscard]] uint32_t GetBindingIndexByHash(const uint32_t hash) const;
		[[nodiscard]] std::map<uint32_t, EngineBindingType>& GetEngineBindings();

	protected:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ResourceHandle<Shader> m_shader;
		std::map<uint32_t, uint32_t> m_rootIndices;
		std::map<uint32_t, EngineBindingType> m_engineBindings;

	protected:
		void CreateShaderAndRootSignature(GUID shaderGuid, ShaderTypeFlags shaderTypes);
	private:
		void CreateRootSignature(const CD3DX12_ROOT_PARAMETER1* params, uint32_t paramsCount);
	};

	class ComputePipeline final : public AbstractPipelineObject
	{
	public:
		ComputePipeline() = delete;
		explicit ComputePipeline(ComputePipelineArgs);
	private:
		void CreateComputePipeline();
	};

	class GraphicsPipeline : public AbstractPipelineObject
	{
	public:
		explicit GraphicsPipeline(const GraphicsPipelineArgs&);
		[[nodiscard]] D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopology() const { return m_topology; }

	private:
		void CreateGraphicsPipeline(const GraphicsPipelineArgs& args);

		static std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topology;
		bool m_hasVertexInput = false;
		bool m_depthTest = false;
		bool m_depthWrite = false;
		D3D12_COMPARISON_FUNC m_depthComparisonFunc;
		D3D12_CULL_MODE m_cullMode;
	};


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
