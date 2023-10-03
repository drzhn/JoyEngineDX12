#ifndef SHADER_COMPILER_H
#define SHADER_COMPILER_H
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcommon.h>
#include <dxc/dxcapi.h>
#include <map>
#include <memory>
#include <vector>
#include <xstring>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	struct ShaderSystemIncludeHandler final : IDxcIncludeHandler
	{
		ShaderSystemIncludeHandler();

		~ShaderSystemIncludeHandler() = default;

		HRESULT LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override;

		HRESULT QueryInterface(const IID& riid, void** ppvObject) override { return S_OK; };
		ULONG AddRef() override { return 0; }
		ULONG Release() override { return 0; }

	private:
		const std::string m_shadersFolderPath;
		ComPtr<IDxcBlobEncoding> m_commonEngineStructsDataBlob;
	};

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
		JoyShaderTypeRaytracing = 1 << 8
	};

	enum ShaderTableType
	{
		ShaderTableRaygen = 0,
		ShaderTableMiss,
		ShaderTableHitGroup,
		ShaderTableCallable,
	};

	struct ShaderInput
	{
		D3D_SHADER_INPUT_TYPE Type; // Type of resource (e.g. texture, cbuffer, etc.)
		uint32_t BindPoint; // Starting bind point
		uint32_t BindCount; // Number of contiguous bind points (for arrays)
		uint32_t Space; // Register space
		D3D12_SHADER_VISIBILITY Visibility;
	};

	typedef uint32_t ShaderTypeFlags;
	typedef std::map<std::string, ShaderInput> ShaderInputMap;

	ShaderTableType ShaderKindToShaderTableType(D3D12_SHADER_VERSION_TYPE kind);

	class ShaderCompiler
	{
	public:
		static void Compile(
			ShaderType type,
			const std::vector<char>& shaderData,
			ID3DBlob** module,
			ShaderInputMap& globalInputMap,
			std::map<ShaderTableType, ShaderInputMap>& localInputMaps,
			std::map<D3D12_SHADER_VERSION_TYPE, std::wstring>& typeFunctionNameMap
		);


		inline static ComPtr<IDxcUtils> s_dxcUtils;
		inline static ComPtr<IDxcCompiler> s_dxcCompiler;
		inline static ComPtr<IDxcContainerReflection> s_dxcReflection;
		inline static ComPtr<IDxcValidator> s_validator;
		inline static std::unique_ptr<ShaderSystemIncludeHandler> m_includeHandler;
	};
}
#endif // SHADER_COMPILER_H
