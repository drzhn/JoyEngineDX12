#include "Shader.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#include "JoyContext.h"

#include "Utils/Assert.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

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
		const std::string shaderPath = JoyContext::Data->GetAbsolutePath(m_guid).string();

		const std::vector<char> shaderData = JoyContext::Data->GetData(m_guid);

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
	}

	void Shader::CompileShader(ShaderType type, const char* shaderPath, const std::vector<char>& shaderData, ComPtr<ID3DBlob>& module)
	{
		const char* entryPoint = nullptr;
		const char* target = nullptr;

		switch (type)
		{
		case JoyShaderTypeVertex:
			entryPoint = "VSMain";
			target = "vs_5_1";
			break;
		case JoyShaderTypeGeometry:
			entryPoint = "GSMain";
			target = "gs_5_1";
			break;
		case JoyShaderTypePixel:
			entryPoint = "PSMain";
			target = "ps_5_1";
			break;
		case JoyShaderTypeCompute:
			entryPoint = "CSMain";
			target = "cs_5_1";
			break;
		default:
			ASSERT(false);
		}

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		constexpr UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ID3DBlob* errorMessages = nullptr;
		HRESULT hr;

		hr = (D3DCompile(
			shaderData.data(),
			shaderData.size(),
			shaderPath,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint,
			target, compileFlags, 0, &module, &errorMessages));

		if (FAILED(hr) && errorMessages)
		{
			const char* errorMsg = static_cast<const char*>(errorMessages->GetBufferPointer());
			OutputDebugStringA(errorMsg);
			ASSERT(false);
		}

		ComPtr<ID3D12ShaderReflection> reflection;
		hr = D3DReflect(module->GetBufferPointer(),
		                module->GetBufferSize(),
		                IID_PPV_ARGS(&reflection)
		);

		ASSERT(!FAILED(hr));

		D3D12_SHADER_DESC desc;
		reflection->GetDesc(&desc);

		for (uint32_t i = 0; i < desc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC inputBindDesc;
			reflection->GetResourceBindingDesc(i, &inputBindDesc);
			m_inputMap.insert({
				inputBindDesc.Name,
				{
					inputBindDesc.Type,
					inputBindDesc.BindPoint,
					inputBindDesc.BindCount,
					inputBindDesc.Space
				}
			});
		}
	}
}
