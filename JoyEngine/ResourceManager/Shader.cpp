#include "Shader.h"

#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	Shader::Shader(const char* shaderPath, ShaderTypeFlags shaderType):
		Resource(shaderPath),
		m_shaderType(shaderType)
	{
		const std::vector<char> shaderData = DataManager::Get()->GetData(shaderPath);
		std::wstring shaderName;
		DataManager::Get()->GetWFilename(shaderPath, shaderName);

		if (m_shaderType & JoyShaderTypeCompute)
		{
			CompileShader(
				JoyShaderTypeCompute,
				shaderName,
				shaderData,
				m_computeModule
			);
		}

		if (m_shaderType & JoyShaderTypeVertex)
		{
			CompileShader(
				JoyShaderTypeVertex,
				shaderName,
				shaderData,
				m_vertexModule
			);
		}

		if (m_shaderType & JoyShaderTypeGeometry)
		{
			CompileShader(
				JoyShaderTypeGeometry,
				shaderName,
				shaderData,
				m_geometryModule
			);
		}

		if (m_shaderType & JoyShaderTypePixel)
		{
			CompileShader(
				JoyShaderTypePixel,
				shaderName,
				shaderData,
				m_fragmentModule
			);
		}

		if (m_shaderType & JoyShaderTypeRaytracing)
		{
			CompileShader(
				JoyShaderTypeRaytracing,
				shaderName,
				shaderData,
				m_raytracingModule
			);
		}
	}

	void Shader::CompileShader(ShaderType type, const std::wstring& shaderName, const std::vector<char>& shaderData, ComPtr<ID3DBlob>& module)
	{
		ShaderCompiler::Compile(
			type,
			shaderName,
			shaderData,
			&module,
			m_globalInputMap,
			m_localInputMaps,
			m_typeFunctionNameMap);
	}
}
