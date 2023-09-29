#ifndef RAYTRACING_PIPELINE_H
#define RAYTRACING_PIPELINE_H

#include "ShaderInputContainer.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/Shader.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	struct RaytracingPipelineArgs
	{
		GUID raytracingShaderGuid;
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
	};
}
#endif // RAYTRACING_PIPELINE_H
