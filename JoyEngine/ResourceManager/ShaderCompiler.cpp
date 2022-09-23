#include "ShaderCompiler.h"

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <dxcapi.h>

#include "DataManager/DataManager.h"

namespace JoyEngine
{
	EngineStructsInclude::EngineStructsInclude() :
		m_commonEngineStructsPath(std::filesystem::absolute(R"(JoyEngine/CommonEngineStructs.h)").generic_string())
	{
		m_data = ReadFile(m_commonEngineStructsPath, 0);

		uint32_t codePage = CP_UTF8;

		ASSERT_SUCC(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_dxcLibrary)));
		m_dxcLibrary->CreateBlobWithEncodingFromPinned(
			m_data.data(), m_data.size(), codePage, &m_dataBlob);
	}

	HRESULT EngineStructsInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		*ppData = m_data.data();
		*pBytes = static_cast<UINT>(m_data.size());
		return S_OK;
	}

	HRESULT EngineStructsInclude::Close(LPCVOID pData)
	{
		return S_OK;
	}

	HRESULT EngineStructsInclude::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		*ppIncludeSource = m_dataBlob.Get();
		return S_OK;
	}

	HRESULT EngineStructsInclude::QueryInterface(const IID& riid, void** ppvObject)
	{
		return S_OK;
	}

	ComPtr<IDxcLibrary> ShaderCompiler::s_dxcLibrary = nullptr;
	ComPtr<IDxcCompiler> ShaderCompiler::s_dxcCompiler = nullptr;
	std::unique_ptr<EngineStructsInclude> ShaderCompiler::m_commonEngineStructsInclude = nullptr;

	void ShaderCompiler::Compile(
		ShaderType type,
		const char* shaderPath,
		const std::vector<char>& shaderData,
		ComPtr<ID3DBlob>& module,
		std::map<std::string, ShaderInput>& m_inputMap)
	{
		if (s_dxcLibrary == nullptr)
		{
			ASSERT_SUCC(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&s_dxcLibrary)));
		}

		if (s_dxcCompiler == nullptr)
		{
			ASSERT_SUCC(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_dxcCompiler)));
		}

		if (m_commonEngineStructsInclude == nullptr)
		{
			m_commonEngineStructsInclude = std::make_unique<EngineStructsInclude>();
		}


		const char* entryPoint = nullptr;
		LPCWSTR entryPointL = nullptr;
		const char* target = nullptr;
		LPCWSTR targetL = nullptr;
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

		switch (type)
		{
		case JoyShaderTypeVertex:
			entryPoint = "VSMain";
			target = "vs_5_1";
			visibility = D3D12_SHADER_VISIBILITY_VERTEX;
			break;
		case JoyShaderTypeGeometry:
			entryPoint = "GSMain";
			target = "gs_5_1";
			visibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
			break;
		case JoyShaderTypePixel:
			entryPoint = "PSMain";
			target = "ps_5_1";
			visibility = D3D12_SHADER_VISIBILITY_PIXEL;
			break;
		case JoyShaderTypeCompute:
			entryPoint = "CSMain";
			target = "cs_5_1";
			visibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		case JoyShaderTypeCompute6_5:
			entryPointL = L"CSMain";
			targetL = L"cs_6_5";
			visibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		default:
			ASSERT(false);
		}


		if (type == JoyShaderTypeCompute6_5)
		{
			DxcDefine Shader_Macros[] = { {L"SHADER", L"0"}, nullptr, nullptr };
			IDxcIncludeHandler* includeHandler = m_commonEngineStructsInclude.get();

			uint32_t codePage = CP_UTF8;
			ComPtr<IDxcBlobEncoding> sourceBlob;
			ComPtr<IDxcOperationResult> dxcOperationResult;

			ASSERT_SUCC(s_dxcLibrary->CreateBlobWithEncodingFromPinned(
				shaderData.data(),
				shaderData.size(),
				codePage,
				&sourceBlob));

			HRESULT res = s_dxcCompiler->Compile(
				sourceBlob.Get(), // pSource
				entryPointL, // pSourceName
				entryPointL, // pEntryPoint
				targetL, // pTargetProfile
				NULL, 0, // pArguments, argCount
				Shader_Macros, 1, // pDefines, defineCount
				includeHandler, // pIncludeHandler
				&dxcOperationResult);
			if (SUCCEEDED(res))
				dxcOperationResult->GetStatus(&res);
			if (FAILED(res))
			{
				if (dxcOperationResult)
				{
					ComPtr<IDxcBlobEncoding> errorsBlob;
					res = dxcOperationResult->GetErrorBuffer(&errorsBlob);
					if (SUCCEEDED(res) && errorsBlob)
					{
						const char* errorMsg = static_cast<const char*>(errorsBlob->GetBufferPointer());
						OutputDebugStringA(errorMsg);
					}
				}
				// Handle compilation error...
			}
			ComPtr<IDxcBlob> code;
			dxcOperationResult->GetResult(&code);
		}
		else
		{
			D3D_SHADER_MACRO Shader_Macros[] = {{"SHADER", "1"}, nullptr, nullptr};
			ID3DInclude* pInclude = m_commonEngineStructsInclude.get();

#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			constexpr UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			constexpr UINT compileFlags = 0;
#endif

			ID3DBlob* errorMessages = nullptr;


			HRESULT hr;

			hr = (D3DCompile(
				shaderData.data(),
				shaderData.size(),
				shaderPath,
				Shader_Macros,
				pInclude,
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

				std::string name = inputBindDesc.Name;
				if (m_inputMap.find(name) == m_inputMap.end())
				{
					m_inputMap.insert({
						inputBindDesc.Name,
						{
							inputBindDesc.Type,
							inputBindDesc.BindPoint,
							inputBindDesc.BindCount,
							inputBindDesc.Space,
							visibility
						}
					});
				}
				else
				{
					m_inputMap[name].Visibility = D3D12_SHADER_VISIBILITY_ALL;
				}
			}
		}
	}
}
