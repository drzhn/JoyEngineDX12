#include "ComputePipeline.h"

#include "d3dx12.h"
#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	ComputePipeline::ComputePipeline(const ComputePipelineArgs& args) :
		AbstractPipelineObject(
			args.shaderPath,
			JoyShaderTypeCompute,
			D3D12_ROOT_SIGNATURE_FLAG_NONE)
	{
		const D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {
			m_inputContainer.GetRootSignature().Get(),
			CD3DX12_SHADER_BYTECODE(m_shader->GetComputeShadeModule().Get()),
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};
		ASSERT_SUCC(
			GraphicsManager::Get()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&
				m_pipelineState)));
	}
}
