#include "Shader.h"

#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	Shader::Shader(GUID guid, ShaderTypeFlags shaderType) : Resource(guid), m_shaderType(shaderType)
	{
		const std::string shaderPath = DataManager::Get()->GetAbsolutePath(m_guid).string();

		const std::vector<char> shaderData = DataManager::Get()->GetData(m_guid);

		if (m_shaderType & JoyShaderTypeCompute)
		{
			CompileShader(
				JoyShaderTypeCompute,
				shaderPath.c_str(),
				shaderData,
				m_computeModule
			);
		}

		if (m_shaderType & JoyShaderTypeVertex)
		{
			CompileShader(
				JoyShaderTypeVertex,
				shaderPath.c_str(),
				shaderData,
				m_vertexModule
			);
		}

		if (m_shaderType & JoyShaderTypeGeometry)
		{
			CompileShader(
				JoyShaderTypeGeometry,
				shaderPath.c_str(),
				shaderData,
				m_geometryModule
			);
		}

		if (m_shaderType & JoyShaderTypePixel)
		{
			CompileShader(
				JoyShaderTypePixel,
				shaderPath.c_str(),
				shaderData,
				m_fragmentModule
			);
		}

		if (m_shaderType & JoyShaderTypeRaytracing)
		{
			CompileShader(
				JoyShaderTypeRaytracing,
				shaderPath.c_str(),
				shaderData,
				m_raytracingModule
			);
		}
	}

	void Shader::CompileShader(ShaderType type, const char* shaderPath, const std::vector<char>& shaderData, ComPtr<ID3DBlob>& module)
	{
		ShaderCompiler::Compile(
			type,
			shaderPath,
			shaderData,
			&module,
			m_globalInputMap,
			m_localInputMaps);
	}
}
