#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "d3dx12.h"

#include "AbstractPipelineObject.h"
#include "ResourceManager/ShaderCompiler.h"

constexpr DXGI_FORMAT VERTEX_POSITION_FORMAT = DXGI_FORMAT_R16G16B16A16_FLOAT;
constexpr DXGI_FORMAT VERTEX_NORMAL_FORMAT = DXGI_FORMAT_R10G10B10A2_UNORM;
constexpr DXGI_FORMAT VERTEX_TANGENT_FORMAT = DXGI_FORMAT_R10G10B10A2_UNORM;
constexpr DXGI_FORMAT VERTEX_TEXCOORD_FORMAT = DXGI_FORMAT_R32G32_FLOAT;

constexpr DXGI_FORMAT INDEX_FORMAT = DXGI_FORMAT_R32_UINT;

namespace JoyEngine
{

	struct GraphicsPipelineArgs
	{
		const char* shaderPath;
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
