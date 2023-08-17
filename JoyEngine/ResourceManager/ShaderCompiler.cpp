#include "ShaderCompiler.h"

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <dxcapi.h>

#include "DataManager/DataManager.h"

#define DXIL_FOURCC(ch0, ch1, ch2, ch3) (                            \
  (uint32_t)(uint8_t)(ch0)        | (uint32_t)(uint8_t)(ch1) << 8  | \
  (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24   \
  )

namespace JoyEngine
{
	EngineStructsInclude::EngineStructsInclude(IDxcLibrary* library) :
		m_commonEngineStructsPath(std::filesystem::absolute(R"(JoyEngine/CommonEngineStructs.h)").generic_string()),
		m_dxcLibrary(library)
	{
		m_commonEngineStructsData = ReadFile(m_commonEngineStructsPath, 0);
	}

	HRESULT EngineStructsInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		*ppData = m_commonEngineStructsData.data();
		*pBytes = static_cast<UINT>(m_commonEngineStructsData.size());
		return S_OK;
	}

	HRESULT EngineStructsInclude::Close(LPCVOID pData)
	{
		return S_OK;
	}

	HRESULT EngineStructsInclude::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		ComPtr<IDxcBlobEncoding> m_dataBlob;
		ASSERT_SUCC(m_dxcLibrary->CreateBlobWithEncodingFromPinned(m_commonEngineStructsData.data(), m_commonEngineStructsData.size(), 0, &m_dataBlob));
		*ppIncludeSource = m_dataBlob.Get();
		m_dataBlob.Detach();
		return S_OK;
	}

	HRESULT EngineStructsInclude::QueryInterface(const IID& riid, void** ppvObject)
	{
		return S_OK;
	}

	ComPtr<IDxcLibrary> ShaderCompiler::s_dxcLibrary = nullptr;
	ComPtr<IDxcCompiler> ShaderCompiler::s_dxcCompiler = nullptr;
	ComPtr<IDxcContainerReflection> ShaderCompiler::s_dxcReflection = nullptr;
	ComPtr<IDxcValidator> ShaderCompiler::s_validator = nullptr;
	std::unique_ptr<EngineStructsInclude> ShaderCompiler::m_commonEngineStructsInclude = nullptr;

	HMODULE dxil_module = nullptr;
	DxcCreateInstanceProc dxil_create_func = nullptr;

	HMODULE dxc_module = nullptr;
	DxcCreateInstanceProc dxc_create_func = nullptr;

	void ShaderCompiler::Compile(
		ShaderType type,
		const char* shaderPath,
		const std::vector<char>& shaderData,
		ID3DBlob** module,
		std::map<std::string, ShaderInput>& m_inputMap)
	{
		if (dxil_module == nullptr)
		{
			dxil_module = LoadLibrary(L"dxil.dll");
			ASSERT(dxil_module != nullptr);
			dxil_create_func = (DxcCreateInstanceProc)GetProcAddress(dxil_module, "DxcCreateInstance");
			ASSERT(dxil_create_func != nullptr);
		}

		if (dxc_module == nullptr)
		{
			dxc_module = LoadLibrary(L"dxcompiler.dll");
			ASSERT(dxc_module != nullptr);
			dxc_create_func = (DxcCreateInstanceProc)GetProcAddress(dxc_module, "DxcCreateInstance");
			ASSERT(dxc_create_func != nullptr);
		}

		if (s_dxcLibrary == nullptr)
		{
			(dxc_create_func(CLSID_DxcLibrary, IID_PPV_ARGS(&s_dxcLibrary)));
		}

		if (s_dxcCompiler == nullptr)
		{
			(dxc_create_func(CLSID_DxcCompiler, IID_PPV_ARGS(&s_dxcCompiler)));
		}

		if (s_dxcReflection == nullptr)
		{
			(dxc_create_func(CLSID_DxcContainerReflection, IID_PPV_ARGS(&s_dxcReflection)));
		}

		if (s_validator == nullptr)
		{
			(dxil_create_func(CLSID_DxcValidator, IID_PPV_ARGS(&s_validator)));
		}


		if (m_commonEngineStructsInclude == nullptr)
		{
			m_commonEngineStructsInclude = std::make_unique<EngineStructsInclude>(s_dxcLibrary.Get());
		}


		const char* entryPoint = nullptr;
		LPCWSTR entryPointL = nullptr;
		const char* target = nullptr;
		LPCWSTR targetL = nullptr;
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

		switch (type)
		{
		case JoyShaderTypeVertex:
			entryPointL = L"VSMain";
			targetL = L"vs_6_5";
			visibility = D3D12_SHADER_VISIBILITY_VERTEX;
			break;
		case JoyShaderTypeGeometry:
			entryPointL = L"GSMain";
			targetL = L"gs_6_5";
			visibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
			break;
		case JoyShaderTypePixel:
			entryPointL = L"PSMain";
			targetL = L"ps_6_5";
			visibility = D3D12_SHADER_VISIBILITY_PIXEL;
			break;
		case JoyShaderTypeCompute:
			entryPointL = L"CSMain";
			targetL = L"cs_6_5";
			visibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		default:
			ASSERT(false);
		}

		ComPtr<ID3D12ShaderReflection> reflection;


		DxcDefine Shader_Macros[] = { {L"SHADER", L"1"}, nullptr, nullptr };
		IDxcIncludeHandler* includeHandler = m_commonEngineStructsInclude.get();

		ComPtr<IDxcBlobEncoding> sourceBlob;
		ComPtr<IDxcOperationResult> dxcOperationResult;

		ASSERT_SUCC(s_dxcLibrary->CreateBlobWithEncodingFromPinned(
			shaderData.data(),
			shaderData.size(),
			0,
			&sourceBlob));

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		LPCWSTR arguments[] = {
			L"/Od", // D3DCOMPILE_SKIP_OPTIMIZATION 
			L"/Zi", // D3DCOMPILE_DEBUG
		};
		uint32_t argCount = 2;
#else
		LPCWSTR* arguments = nullptr;
		uint32_t argCount = 0;

#endif

		HRESULT res = s_dxcCompiler->Compile(
			sourceBlob.Get(), // pSource
			entryPointL, // pSourceName
			entryPointL, // pEntryPoint
			targetL, // pTargetProfile
			arguments, argCount, // pArguments, argCount
			Shader_Macros, 1, // pDefines, defineCount
			includeHandler, // pIncludeHandler
			&dxcOperationResult);
		if (SUCCEEDED(res))
		{
			dxcOperationResult->GetStatus(&res);
		}
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
		ASSERT_SUCC(res);

		ASSERT_SUCC(dxcOperationResult->GetResult(reinterpret_cast<IDxcBlob**>(module)));

		s_validator->Validate(reinterpret_cast<IDxcBlob*>(*module), 0, &dxcOperationResult);
		dxcOperationResult->GetStatus(&res);
		if (FAILED(res))
		{
			ComPtr<IDxcBlobEncoding> errorsBlob;
			res = dxcOperationResult->GetErrorBuffer(&errorsBlob);
			if (SUCCEEDED(res) && errorsBlob)
			{
				const char* errorMsg = static_cast<const char*>(errorsBlob->GetBufferPointer());
				OutputDebugStringA(errorMsg);
			}
		}

		ASSERT_SUCC(s_dxcReflection->Load(reinterpret_cast<IDxcBlob*>(*module)));
		uint32_t partCount;
		ASSERT_SUCC(s_dxcReflection->GetPartCount(&partCount));

		uint32_t shaderId;
		ASSERT_SUCC(s_dxcReflection->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &shaderId));
		ASSERT_SUCC(s_dxcReflection->GetPartReflection(shaderId, __uuidof(ID3D12ShaderReflection), (void**)&reflection));


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
