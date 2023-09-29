#include "AbstractPipelineObject.h"

#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
	AbstractPipelineObject::AbstractPipelineObject(
		GUID shaderGuid,
		ShaderTypeFlags shaderTypes,
		D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		m_shader = ResourceManager::Get()->LoadResource<Shader>(shaderGuid, shaderTypes);
		m_inputContainer.InitContainer(m_shader.Get()->GetInputMap(), flags);
	}

	ShaderInput const* AbstractPipelineObject::GetShaderInputByName(const std::string& name) const
	{
		if (m_shader->GetInputMap().contains(name))
		{
			return &m_shader->GetInputMap().find(name)->second;
		}
		return nullptr;
	}
}
