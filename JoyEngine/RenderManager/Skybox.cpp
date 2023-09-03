#include "Skybox.h"

#include "RenderManager.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GraphicsUtils.h"

namespace JoyEngine
{
	// we cannot use depth testing for paint skybox pixels because we cannot use UAVgbuffer.m_depthTexture as a depth target
	// so we paint all pixels where color.a == 0;
	Skybox::Skybox()
	{
		m_skyboxTexture = ResourceManager::Get()->LoadResource<Texture>(
			GUID::StringToGuid("17663088-100d-4e78-8305-17b5818256db"));
		m_skyboxMesh = ResourceManager::Get()->LoadResource<Mesh>(
			GUID::StringToGuid("b7d27f1a-006b-41fa-b10b-01b212ebfebe")); // DefaultSphere

		m_skyboxTextureIndexData.SetData(TextureIndexData{.data = m_skyboxTexture->GetSRV()->GetDescriptorIndex()});

		const GUID skyboxShaderGuid = GUID::StringToGuid("7e43e76d-9d5f-4fc8-a8f1-c8ec0dce95ef"); //shaders/skybox.hlsl

		//const D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {
		//	true,
		//	FALSE,
		//	D3D12_BLEND_SRC_ALPHA,
		//	D3D12_BLEND_INV_SRC_ALPHA,
		//	D3D12_BLEND_OP_ADD,
		//	D3D12_BLEND_ONE,
		//	D3D12_BLEND_ONE,
		//	D3D12_BLEND_OP_ADD,
		//	D3D12_LOGIC_OP_NOOP,
		//	D3D12_COLOR_WRITE_ENABLE_ALL
		//};

		//D3D12_BLEND_DESC blend = {
		//	.AlphaToCoverageEnable = false,
		//	.IndependentBlendEnable = false,
		//};
		//blend.RenderTarget[0] = blendDesc;

		m_skyboxPipeline = std::make_unique<GraphicsPipeline>(GraphicsPipelineArgs
			{
				skyboxShaderGuid,
				JoyShaderTypeVertex | JoyShaderTypePixel,
				true,
				false,
				false,
				D3D12_CULL_MODE_FRONT,
				D3D12_COMPARISON_FUNC_GREATER_EQUAL,
				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
				{
					RenderManager::GetMainColorFormat(),
				},
				1,
				RenderManager::GetDepthFormat(),
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			});
	}

	void Skybox::DrawSky(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionData) const
	{
		commandList->SetPipelineState(m_skyboxPipeline->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(m_skyboxPipeline->GetRootSignature().Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, m_skyboxMesh->GetVertexBufferView());
		commandList->IASetIndexBuffer(m_skyboxMesh->GetIndexBufferView());


		GraphicsUtils::ProcessEngineBindings(
			commandList,
			m_skyboxPipeline.get(),
			frameIndex,
			nullptr,
			viewProjectionData);

		GraphicsUtils::AttachView(commandList, m_skyboxPipeline.get(), "skyboxTexture", m_skyboxTexture->GetSRV());
		GraphicsUtils::AttachView(commandList, m_skyboxPipeline.get(), "textureSampler", EngineSamplersProvider::GetLinearClampSampler());

		commandList->DrawIndexedInstanced(
			m_skyboxMesh->GetIndexCount(),
			1,
			0, 0, 0);
	}
}
