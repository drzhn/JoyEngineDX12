#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>

#include "JoyContext.h"
#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/JoyTypes.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	SharedMaterial::SharedMaterial(GUID guid) :
		Resource(guid) // UNUSED
	{
		ASSERT(false);
		//rapidjson::Document json = JoyContext::Data->GetSerializedData(m_guid, sharedMaterial);

		//m_shader = GUID::StringToGuid(json["shader"].GetString());

		//m_hasVertexInput = json["hasVertexInput"].GetBool();
		//m_hasMVP = json["hasMVP"].GetBool();
		//m_depthTest = json["depthTest"].GetBool();
		//m_depthWrite = json["depthWrite"].GetBool();

		//CreateGraphicsPipeline();
		//JoyContext::Render->RegisterSharedMaterial(this);
	}

	SharedMaterial::SharedMaterial(GUID guid, SharedMaterialArgs args) :
		Resource(guid),
		m_hasVertexInput(args.hasVertexInput),
		m_hasMVP(args.hasMVP), m_depthTest(args.depthTest), m_depthWrite(args.depthWrite)
	{
		m_shader = args.shader;

		CreateRootSignature();
		CreateGraphicsPipeline();
		JoyContext::Render->RegisterSharedMaterial(this);
	}

	void SharedMaterial::CreateRootSignature()
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(
			JoyContext::Graphics->GetDevice()->CheckFeatureSupport(
				D3D12_FEATURE_ROOT_SIGNATURE,
				&featureData,
				sizeof( featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER1 rootParameters[3];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[2].InitAsConstants(sizeof(MVP) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			_countof(rootParameters),
			rootParameters,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ASSERT_SUCC(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)));
	}

	void SharedMaterial::CreateGraphicsPipeline()
	{
		// Create the vertex input layout
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {
			m_rootSignature.Get(),
			CD3DX12_SHADER_BYTECODE(m_shader->GetVertexShadeModule().Get()),
			CD3DX12_SHADER_BYTECODE(m_shader->GetFragmentShadeModule().Get()),
			{},
			{},
			{},
			{},
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			UINT_MAX,
			CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
			{
				TRUE,
				D3D12_DEPTH_WRITE_MASK_ALL,
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				FALSE,
				0,
				0,
				D3D12_DEPTH_STENCILOP_DESC({}),
				D3D12_DEPTH_STENCILOP_DESC({})
			},
			{inputLayout, _countof(inputLayout)},
			{},
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_D32_FLOAT,
			{1, 0},
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};

		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&m_pipelineState)));
	}


	SharedMaterial::~SharedMaterial()
	{
		JoyContext::Render->UnregisterSharedMaterial(this);
	}


	bool SharedMaterial::IsLoaded() const noexcept
	{
		return m_shader->IsLoaded();
	}

	void SharedMaterial::RegisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		m_meshRenderers.insert(meshRenderer);
	}

	void SharedMaterial::UnregisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		if (m_meshRenderers.find(meshRenderer) == m_meshRenderers.end())
		{
			ASSERT(false);
		}
		m_meshRenderers.erase(meshRenderer);
	}

	std::set<MeshRenderer*>& SharedMaterial::GetMeshRenderers()
	{
		return m_meshRenderers;
	}
}
