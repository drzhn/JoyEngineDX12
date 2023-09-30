#ifndef SHADER_INPUT_CONTAINER_H
#define SHADER_INPUT_CONTAINER_H

#include "ResourceManager/ShaderCompiler.h"

namespace JoyEngine
{
	enum class EngineBindingType
	{
		ObjectIndexData,
		ModelMatrixData,
		ViewProjectionMatrixData,
		EngineData,
	};

	class ShaderInputContainer
	{
	public:
		ShaderInputContainer() = default;

		ShaderInputContainer(const ShaderInputMap& inputMap, D3D12_ROOT_SIGNATURE_FLAGS flags)
		{
			InitContainer(inputMap, flags);
		}

		void InitContainer(const ShaderInputMap& inputMap, D3D12_ROOT_SIGNATURE_FLAGS flags);

		[[nodiscard]] ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature; }
		[[nodiscard]] uint32_t GetBindingIndexByName(const std::string&) const;
		[[nodiscard]] uint32_t GetBindingIndexByHash(const uint32_t hash) const;
		[[nodiscard]] const std::map<uint32_t, EngineBindingType>& GetEngineBindings() const;

	private:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		std::map<uint32_t, uint32_t> m_rootIndices; // hash(paramName) -> rootIndex
		std::map<uint32_t, EngineBindingType> m_engineBindings; // hash(paramName) -> engine binding type
	};
}
#endif // SHADER_INPUT_CONTAINER_H
