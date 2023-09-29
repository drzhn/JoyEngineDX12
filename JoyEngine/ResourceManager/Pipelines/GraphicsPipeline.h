#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "AbstractPipelineObject.h"
#include "d3dx12.h"
#include "ResourceManager/ShaderCompiler.h"
#include "Utils/GUID.h"

namespace JoyEngine
{

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

}
#endif // GRAPHICS_PIPELINE_H
