﻿#ifndef SHADER_COMPILER_H
#define SHADER_COMPILER_H
#include <d3d12.h>
#include <d3dcommon.h>
#include <dxcapi.h>
#include <map>
#include <memory>
#include <vector>
#include <xstring>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


namespace JoyEngine
{
	struct EngineStructsInclude final : ID3DInclude, IDxcIncludeHandler
	{
		EngineStructsInclude() = delete;

		EngineStructsInclude(IDxcLibrary* library);

		~EngineStructsInclude() = default;

		HRESULT __stdcall Open(
			D3D_INCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID* ppData,
			UINT* pBytes) override;

		HRESULT __stdcall Close(LPCVOID pData) override;

		HRESULT LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override;

		HRESULT QueryInterface(const IID& riid, void** ppvObject) override;
		ULONG AddRef() override { return 0; }
		ULONG Release() override { return 0; }

	private:
		const std::string m_commonEngineStructsPath;
		std::vector<char> m_commonEngineStructsData;

		const std::string m_shadersFolderPath;
		//ComPtr<IDxcBlobEncoding> m_dataBlob;
		IDxcLibrary* m_dxcLibrary;
	};


	typedef
	enum ShaderType
	{
		JoyShaderTypeVertex = 1 << 0,
		JoyShaderTypeHull = 1 << 1,
		JoyShaderTypeDomain = 1 << 2,
		JoyShaderTypeGeometry = 1 << 3,
		JoyShaderTypePixel = 1 << 4,
		JoyShaderTypeAmplification = 1 << 5,
		JoyShaderTypeMesh = 1 << 6,
		JoyShaderTypeCompute = 1 << 7,
		JoyShaderTypeCompute6_5 = 1 << 8
	} ShaderType;

	struct ShaderInput
	{
		D3D_SHADER_INPUT_TYPE Type; // Type of resource (e.g. texture, cbuffer, etc.)
		uint32_t BindPoint; // Starting bind point
		uint32_t BindCount; // Number of contiguous bind points (for arrays)
		uint32_t Space; // Register space
		D3D12_SHADER_VISIBILITY Visibility;
	};

	typedef uint32_t ShaderTypeFlags;


	class ShaderCompiler
	{
	public:
		static void Compile(
			ShaderType type,
			const char* shaderPath,
			const std::vector<char>& shaderData,
			ID3DBlob** module,
			std::map<std::string, ShaderInput>& m_inputMap
		);


		static ComPtr<IDxcLibrary> s_dxcLibrary;
		static ComPtr<IDxcCompiler> s_dxcCompiler;
		static ComPtr<IDxcContainerReflection> s_dxcReflection;
		static ComPtr<IDxcValidator> s_validator;
		static std::unique_ptr<EngineStructsInclude> m_commonEngineStructsInclude;
	};
}
#endif // SHADER_COMPILER_H
