#ifndef SHADER_H
#define SHADER_H

#include <d3d12.h>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

#include "Common/Resource.h"

namespace JoyEngine
{
	typedef
	enum ShaderType
	{
		JoyShaderTypeVertex = 1 << 0,
		JoyShaderTypeHull = 1 << 1,
		JoyShaderTypeDomain = 1 << 2,
		JoyShaderTypeGeometry = 1 << 3,
		JoyShaderTypePixel = 1 << 4,
		JoyShaderTypeAmplification = 1 << 5,
		JoyShaderTypeMesh = 1 << 6
	} ShaderType;

	typedef uint32_t ShaderTypeFlags;

	class Shader final : public Resource
	{
	public :
		Shader() = delete;

		explicit Shader(GUID);
		explicit Shader(GUID, ShaderTypeFlags shaderType);
		void InitShader();

		~Shader() final = default;

		[[nodiscard]] ShaderTypeFlags GetShaderType() const noexcept { return m_shaderType; }
		[[nodiscard]] ComPtr<ID3DBlob> GetVertexShadeModule() const noexcept { return m_vertexModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetFragmentShadeModule() const noexcept { return m_fragmentModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetGeometryShadeModule() const noexcept { return m_geometryModule; }
		// TODO other types
		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private :
		ShaderTypeFlags m_shaderType = JoyShaderTypeVertex | JoyShaderTypePixel;

		ComPtr<ID3DBlob> m_vertexModule;
		ComPtr<ID3DBlob> m_fragmentModule;
		ComPtr<ID3DBlob> m_geometryModule;
	};
}

#endif //SHADER_H
