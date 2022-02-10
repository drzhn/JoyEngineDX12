#include "Shader.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#include "JoyContext.h"

#include "Utils/Assert.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"

namespace JoyEngine
{
	Shader::Shader(GUID guid) : Resource(guid)
	{
		InitShader();
	}

	Shader::Shader(GUID guid, ShaderTypeFlags shaderType) : Resource(guid), m_shaderType(shaderType)
	{
		InitShader();
	}

	void Shader::InitShader()
	{
		const std::wstring shaderPath = JoyContext::Data->GetAbsolutePath(m_guid).wstring();

		const std::vector<char> shaderData = JoyContext::Data->GetData(m_guid);

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		constexpr UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ID3DBlob* errorMessages = nullptr;
		HRESULT hr;

		if (m_shaderType & JoyShaderTypeVertex)
		{
			hr = (D3DCompile(
				shaderData.data(),
				shaderData.size(),
				"shader", nullptr,
				nullptr,
				"VSMain", "vs_5_1", compileFlags, 0, &m_vertexModule, &errorMessages));

			if (FAILED(hr) && errorMessages)
			{
				const char* errorMsg = static_cast<const char*>(errorMessages->GetBufferPointer());
				OutputDebugStringA(errorMsg);
			}

			errorMessages = nullptr;
		}

		if (m_shaderType & JoyShaderTypeGeometry)
		{
			hr = (D3DCompile(
				shaderData.data(),
				shaderData.size(),
				"shader",
				nullptr,
				nullptr,
				"GSMain",
				"gs_5_1", compileFlags, 0, &m_geometryModule, &errorMessages));

			if (FAILED(hr) && errorMessages)
			{
				const char* errorMsg = static_cast<const char*>(errorMessages->GetBufferPointer());
				OutputDebugStringA(errorMsg);
			}

			errorMessages = nullptr;
		}

		if (m_shaderType & JoyShaderTypePixel)
		{
			hr = (D3DCompile(
				shaderData.data(),
				shaderData.size(),
				"shader",
				nullptr,
				nullptr,
				"PSMain",
				"ps_5_1", compileFlags, 0, &m_fragmentModule, &errorMessages));

			if (FAILED(hr) && errorMessages)
			{
				const char* errorMsg = static_cast<const char*>(errorMessages->GetBufferPointer());
				OutputDebugStringA(errorMsg);
			}
		}
	}
}
