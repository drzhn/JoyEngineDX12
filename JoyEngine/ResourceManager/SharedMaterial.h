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
		ViewProjectionMatrixData,
		EngineData,
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

	struct RaytracingPipelineArgs
	{
		GUID raytracingShaderGuid;
	};

	class ShaderInputContainer
	{
	public :
		ShaderInputContainer() = default;
		void InitContainer(const ShaderInputMap& inputMap);

		[[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature; }
		[[nodiscard]] uint32_t GetBindingIndexByName(const std::string&) const;
		[[nodiscard]] uint32_t GetBindingIndexByHash(const uint32_t hash) const;
		[[nodiscard]] const std::map<uint32_t, EngineBindingType>& GetEngineBindings() const;

	private:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		std::map<uint32_t, uint32_t> m_rootIndices;
		std::map<uint32_t, EngineBindingType> m_engineBindings;
		void CreateRootSignature(const CD3DX12_ROOT_PARAMETER1* params, uint32_t paramsCount);
	};


	class RaytracingPipeline
	{
	public:
		RaytracingPipeline() = delete;
		explicit RaytracingPipeline(const RaytracingPipelineArgs&);

	private:
		ResourceHandle<Shader> m_raytracingShader;
		ComPtr<ID3D12StateObject> m_stateObject;

		ShaderInputContainer m_globalInputContainer;
		std::map<std::wstring, ShaderInputContainer> m_localInputContainers;

		//// Root signatures
		//ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
		//ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;
	};

	class AbstractPipelineObject
	{
	public:
		AbstractPipelineObject() = delete;
		explicit AbstractPipelineObject(GUID shaderGuid, ShaderTypeFlags shaderTypes);

		[[nodiscard]] ComPtr<ID3D12PipelineState> GetPipelineObject() const noexcept { return m_pipelineState; }
		[[nodiscard]] ShaderInput const* GetShaderInputByName(const std::string&) const;


		[[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept
		{
			return m_inputContainer.GetRootSignature();
		}

		[[nodiscard]] uint32_t GetBindingIndexByName(const std::string& name) const
		{
			return m_inputContainer.GetBindingIndexByName(name);
		}

		[[nodiscard]] uint32_t GetBindingIndexByHash(const uint32_t hash) const
		{
			return m_inputContainer.GetBindingIndexByHash(hash);
		}

		[[nodiscard]] const std::map<uint32_t, EngineBindingType>& GetEngineBindings() const
		{
			return m_inputContainer.GetEngineBindings();
		}

	protected:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ResourceHandle<Shader> m_shader;
		ShaderInputContainer m_inputContainer;
	};

	class ComputePipeline final : public AbstractPipelineObject
	{
	public:
		ComputePipeline() = delete;
		explicit ComputePipeline(const ComputePipelineArgs&);
	};

	class GraphicsPipeline : public AbstractPipelineObject
	{
	public:
		GraphicsPipeline() = delete;
		explicit GraphicsPipeline(const GraphicsPipelineArgs&);
		[[nodiscard]] D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopology() const { return m_topology; }

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
