#ifndef SHADER_H
#define SHADER_H

#include <d3d12.h>
#include <map>

#include "ShaderCompiler.h"

using Microsoft::WRL::ComPtr;

#include "Common/Resource.h"

namespace JoyEngine
{
	class Shader final : public Resource
	{
	public :
		Shader() = delete;

		explicit Shader(GUID, ShaderTypeFlags shaderType);

		~Shader() final = default;

		[[nodiscard]] ShaderTypeFlags GetShaderType() const noexcept { return m_shaderType; }
		[[nodiscard]] const ShaderInputMap& GetInputMap() { return m_globalInputMap; }

		[[nodiscard]] const std::map<ShaderTableType, ShaderInputMap>& GetLocalInputMaps() const
		{
			ASSERT((m_shaderType & JoyShaderTypeRaytracing) != 0);
			return m_localInputMaps;
		}

		[[nodiscard]] const std::map<D3D12_SHADER_VERSION_TYPE, std::wstring>& GetTypeFunctionMap() const
		{
			ASSERT((m_shaderType & JoyShaderTypeRaytracing) != 0);
			return m_typeFunctionNameMap;
		}

		[[nodiscard]] ComPtr<ID3DBlob> GetVertexShadeModule() const noexcept { return m_vertexModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetFragmentShadeModule() const noexcept { return m_fragmentModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetGeometryShadeModule() const noexcept { return m_geometryModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetComputeShadeModule() const noexcept { return m_computeModule; }
		[[nodiscard]] ComPtr<ID3DBlob> GetRaytracingShadeModule() const noexcept { return m_raytracingModule; }
		// TODO other types
		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

		[[nodiscard]] LPCWSTR GetFunctionNameByType(D3D12_SHADER_VERSION_TYPE type) const
		{
			// other shaders doesn't support libraries yet
			ASSERT((m_shaderType & JoyShaderTypeRaytracing) != 0);
			if (!m_typeFunctionNameMap.contains(type))
			{
				return nullptr;
			}
			return m_typeFunctionNameMap.at(type).c_str();
		}

	private :
		ShaderTypeFlags m_shaderType = 0;

		ComPtr<ID3DBlob> m_vertexModule;
		ComPtr<ID3DBlob> m_fragmentModule;
		ComPtr<ID3DBlob> m_geometryModule;
		ComPtr<ID3DBlob> m_computeModule;
		ComPtr<ID3DBlob> m_raytracingModule;

		ShaderInputMap m_globalInputMap;


		std::map<ShaderTableType, ShaderInputMap> m_localInputMaps;
		std::map<D3D12_SHADER_VERSION_TYPE, std::wstring> m_typeFunctionNameMap;

	private:
		void CompileShader(ShaderType type, const char* shaderPath, const std::vector<char>& shaderData, ComPtr<ID3DBlob>& module);
	};
}

#endif //SHADER_H
