#include "GraphicsPipeline.h"

#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> GraphicsPipeline::m_inputLayout = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineArgs& args) :
		AbstractPipelineObject(
			args.shaderPath,
			args.shaderTypes,
			args.hasVertexInput ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE),
		m_topology(args.topology),
		m_hasVertexInput(args.hasVertexInput),
		m_depthTest(args.depthTest),
		m_depthWrite(args.depthWrite),
		m_depthComparisonFunc(args.depthComparisonFunc),
		m_cullMode(args.cullMode)
	{
		// Create the vertex input layout

		CD3DX12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterizerDesc.CullMode = m_cullMode;

		constexpr CD3DX12_SHADER_BYTECODE emptyBytecode = {};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {
			m_inputContainer.GetRootSignature().Get(),
			m_shader->GetShaderType() & JoyShaderTypeVertex
				? CD3DX12_SHADER_BYTECODE(m_shader->GetVertexShadeModule().Get())
				: emptyBytecode,
			m_shader->GetShaderType() & JoyShaderTypeVertex
				? CD3DX12_SHADER_BYTECODE(m_shader->GetFragmentShadeModule().Get())
				: emptyBytecode,
			{},
			{},
			m_shader->GetShaderType() & JoyShaderTypeGeometry
				? CD3DX12_SHADER_BYTECODE(m_shader->GetGeometryShadeModule().Get())
				: emptyBytecode,
			{},
			args.blendDesc,
			UINT_MAX,
			rasterizerDesc,
			{
				m_depthTest ? TRUE : FALSE,
				m_depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
				m_depthComparisonFunc,
				FALSE,
				0,
				0,
				D3D12_DEPTH_STENCILOP_DESC({}),
				D3D12_DEPTH_STENCILOP_DESC({})
			},
			{
				m_hasVertexInput ? m_inputLayout.data() : nullptr,
				m_hasVertexInput ? static_cast<uint32_t>(m_inputLayout.size()) : 0
			},
			{},
			args.topology,
			args.renderTargetsFormatsSize,
			{
				args.renderTargetsFormatsSize > 0 ? args.renderTargetsFormats[0] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 1 ? args.renderTargetsFormats[1] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 2 ? args.renderTargetsFormats[2] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 3 ? args.renderTargetsFormats[3] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 4 ? args.renderTargetsFormats[4] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 5 ? args.renderTargetsFormats[5] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 6 ? args.renderTargetsFormats[6] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 7 ? args.renderTargetsFormats[7] : DXGI_FORMAT_UNKNOWN
			},
			args.depthFormat,
			{1, 0},
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};

		ASSERT_SUCC(
			GraphicsManager::Get()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&
				m_pipelineState)));
	}
}
