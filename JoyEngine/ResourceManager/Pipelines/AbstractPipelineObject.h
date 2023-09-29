#ifndef ABSTRACT_PIPELINE_OBJECT_H
#define ABSTRACT_PIPELINE_OBJECT_H

#include "ShaderInputContainer.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/Shader.h"
#include "ResourceManager/ShaderCompiler.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class AbstractPipelineObject
	{
	public:
		AbstractPipelineObject() = delete;
		explicit AbstractPipelineObject(GUID shaderGuid, ShaderTypeFlags shaderTypes, D3D12_ROOT_SIGNATURE_FLAGS flags);

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
}
#endif // ABSTRACT_PIPELINE_OBJECT_H
