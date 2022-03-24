#ifndef SHADER_H
#define SHADER_H

#include <d3d12.h>
#include <map>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

#include "Common/Resource.h"

namespace JoyEngine
{
	struct ShaderInput
	{
		D3D_SHADER_INPUT_TYPE Type; // Type of resource (e.g. texture, cbuffer, etc.)
		uint32_t BindPoint; // Starting bind point
		uint32_t BindCount; // Number of contiguous bind points (for arrays)
		uint32_t Space; // Register space
		D3D12_SHADER_VISIBILITY Visibility;
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
		JoyShaderTypeCompute = 1 << 7
	} ShaderType;

	typedef uint32_t ShaderTypeFlags;

	class Shader final : public Resource
	{
	public :
		Shader() = delete;

		explicit Shader(GUID);
		explicit Shader(GUID, ShaderTypeFlags shaderType);
		void InitShader();
		void CompileShader(ShaderType type, const char* shaderPath, const std::vector<char>& shaderData, ComPtr<ID3DBlob>& module);

		~Shader() final = default;

		[[nodiscard]] ShaderTypeFlags GetShaderType() const noexcept { return m_shaderType; }
		[[nodiscard]] const std::map<std::string, ShaderInput>& GetInputMap() { return m_inputMap; }

		[[nodiscard]] ComPtr<ID3DBlob> GetVertexShadeModule() const noexcept { return m_vertexModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetFragmentShadeModule() const noexcept { return m_fragmentModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetGeometryShadeModule() const noexcept { return m_geometryModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetComputeShadeModule() const noexcept { return m_computeModule; }
		// TODO other types
		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private :
		ShaderTypeFlags m_shaderType = JoyShaderTypeVertex | JoyShaderTypePixel;

		ComPtr<ID3DBlob> m_vertexModule;
		ComPtr<ID3DBlob> m_fragmentModule;
		ComPtr<ID3DBlob> m_geometryModule;
		ComPtr<ID3DBlob> m_computeModule;

		std::map<std::string, ShaderInput> m_inputMap;
	};
}

#endif //SHADER_H
