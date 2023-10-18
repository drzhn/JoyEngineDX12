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

		if (m_shaderType & JoyShaderTypeCompute)
		{
			CompileShader(
				JoyShaderTypeCompute,
				shaderData,
				m_computeModule
			);
		}

		if (m_shaderType & JoyShaderTypeVertex)
		{
			CompileShader(
				JoyShaderTypeVertex,
				shaderData,
				m_vertexModule
			);
		}

		if (m_shaderType & JoyShaderTypeGeometry)
		{
			CompileShader(
				JoyShaderTypeGeometry,
				shaderData,
				m_geometryModule
			);
		}

		if (m_shaderType & JoyShaderTypePixel)
		{
			CompileShader(
				JoyShaderTypePixel,
				shaderData,
				m_fragmentModule
			);
		}

		if (m_shaderType & JoyShaderTypeRaytracing)
		{
			CompileShader(
				JoyShaderTypeRaytracing,
				shaderData,
				m_raytracingModule
			);
		}
	}

	void Shader::CompileShader(ShaderType type, const std::vector<char>& shaderData, ComPtr<ID3DBlob>& module)
	{
		ShaderCompiler::Compile(
			type,
			shaderData,
			&module,
			m_globalInputMap,
			m_localInputMaps,
			m_typeFunctionNameMap);
	}
}
