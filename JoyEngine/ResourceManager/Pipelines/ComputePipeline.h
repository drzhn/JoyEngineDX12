#ifndef COMPUTE_PIPELINE_H
#define COMPUTE_PIPELINE_H

#include "AbstractPipelineObject.h"

namespace JoyEngine
{
	struct ComputePipelineArgs
	{
		const char* shaderPath;
		D3D_SHADER_MODEL shaderModel;
	};

	class ComputePipeline final : public AbstractPipelineObject
	{
	public:
		ComputePipeline() = delete;
		explicit ComputePipeline(const ComputePipelineArgs&);
	};
}
#endif // COMPUTE_PIPELINE_H
