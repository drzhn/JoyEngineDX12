#include "ShaderInputContainer.h"

#include "CommonEngineStructs.h"
#include "d3dx12.h"
#include "Common/HashDefs.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"
#include "Utils/Log.h"

#define DESCRIPTOR_ARRAY_SIZE 32

namespace JoyEngine
{
	uint32_t ShaderInputContainer::GetBindingIndexByName(const std::string& name) const
	{
		ASSERT(m_rootIndices.contains(StrHash32(name.c_str())));
		return m_rootIndices.find(StrHash32(name.c_str()))->second;
	}

	uint32_t ShaderInputContainer::GetBindingIndexByHash(const uint32_t hash) const
	{
		if (!m_rootIndices.contains(hash))
		{
			Logger::LogFormat("Warning: pipeline doesn't contain hash %d", hash);
			return -1;
		}
		return m_rootIndices.find(hash)->second;
	}

	const std::map<uint32_t, EngineBindingType>& ShaderInputContainer::GetEngineBindings() const
	{
		return m_engineBindings;
	}

	void ShaderInputContainer::InitContainer(
		const ShaderInputMap& inputMap,
		D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[DESCRIPTOR_ARRAY_SIZE];
		uint32_t rangesIndex = 0;

		CD3DX12_ROOT_PARAMETER1 params[DESCRIPTOR_ARRAY_SIZE];
		uint32_t paramsIndex = 0;

		for (const auto& pair : inputMap)
		{
			const std::string& name = pair.first;
			const ShaderInput& input = pair.second;

			if (name == "objectIndex")
			{
				params[paramsIndex].InitAsConstants(
					sizeof(uint32_t) / 4, input.BindPoint, input.Space, input.Visibility);
				m_engineBindings.insert({paramsIndex, EngineBindingType::ObjectIndexData});
				paramsIndex++;
			}
			else if (name == "viewProjectionData")
			{
				params[paramsIndex].InitAsConstants(
					sizeof(ViewProjectionMatrixData) / 4, input.BindPoint, input.Space, input.Visibility);

				m_engineBindings.insert({paramsIndex, EngineBindingType::ViewProjectionMatrixData});
				paramsIndex++;
			}
			else if (name == "objectMatricesData")
			{
				ranges[rangesIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, input.BindPoint, input.Space,
				                         D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params[paramsIndex].InitAsDescriptorTable(1, &ranges[rangesIndex], input.Visibility);
				m_engineBindings.insert({paramsIndex, EngineBindingType::ModelMatrixData});
				rangesIndex++;
				paramsIndex++;
			}
			else if (name == "engineData")
			{
				ranges[rangesIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, input.BindPoint, input.Space,
				                         D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params[paramsIndex].InitAsDescriptorTable(1, &ranges[rangesIndex], input.Visibility);
				m_engineBindings.insert({paramsIndex, EngineBindingType::EngineData});
				rangesIndex++;
				paramsIndex++;
			}
			else if (name == "g_SceneAccelerationStructure")
			{
				params[paramsIndex].InitAsShaderResourceView(input.BindPoint, input.Space);
				m_rootIndices.insert({StrHash32(name.c_str()), paramsIndex});
				paramsIndex++;
			}
			else
			{
				D3D12_DESCRIPTOR_RANGE_TYPE type;
				switch (input.Type)
				{
				case D3D_SIT_CBUFFER:
					type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					break;

				case D3D_SIT_STRUCTURED:
				case D3D_SIT_TEXTURE:
					type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					break;
				case D3D_SIT_SAMPLER:
					type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					break;

				case D3D_SIT_UAV_RWTYPED:
				case D3D_SIT_UAV_RWSTRUCTURED:
				case D3D_SIT_UAV_RWBYTEADDRESS:
				case D3D_SIT_UAV_APPEND_STRUCTURED:
				case D3D_SIT_UAV_CONSUME_STRUCTURED:
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				case D3D_SIT_UAV_FEEDBACKTEXTURE:
					type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					break;

				case D3D_SIT_TBUFFER:
				case D3D_SIT_BYTEADDRESS:
				case D3D_SIT_RTACCELERATIONSTRUCTURE:
				default:
					ASSERT(false);
					throw;
				}
				ranges[rangesIndex].Init(
					type,
					input.BindCount == 0 ? UINT_MAX : input.BindCount,
					input.BindPoint,
					input.Space,
					input.BindCount == 0 ? D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE : D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params[paramsIndex].InitAsDescriptorTable(1, &ranges[rangesIndex], input.Visibility);
				m_rootIndices.insert({StrHash32(name.c_str()), paramsIndex});
				rangesIndex++;
				paramsIndex++;
			}
		}

		// TODO should I make compatibility with 1.0?
		ASSERT(GraphicsManager::Get()->GetHighestRootSignatureVersion() == D3D_ROOT_SIGNATURE_VERSION_1_1);
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			paramsIndex,
			params,
			0,
			nullptr,
			flags);

		ComPtr<ID3DBlob> signature;
		ID3D10Blob** errorPtr = nullptr;
#ifdef _DEBUG
		ComPtr<ID3DBlob> error;
		errorPtr = &error;
#endif

		const HRESULT result = D3DX12SerializeVersionedRootSignature(
			&rootSignatureDesc,
			GraphicsManager::Get()->GetHighestRootSignatureVersion(),
			&signature,
			errorPtr);

#ifdef _DEBUG
		ASSERT_DESC(result == S_OK, static_cast<const char*>(error->GetBufferPointer()));
#endif

		ASSERT_SUCC(GraphicsManager::Get()->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)));
	}
}
