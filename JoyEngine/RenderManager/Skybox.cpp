#include "Skybox.h"

#include "RenderManager.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GraphicsUtils.h"

namespace JoyEngine
{
	// we cannot use depth testing for paint skybox pixels because we cannot use UAVgbuffer.m_depthTexture as a depth target
	// so we paint all pixels where color.a == 0;
	Skybox::Skybox(ResourceView* colorTextureSRV):
		m_colorTextureSRV(colorTextureSRV)
	{
		m_skyboxTexture = ResourceManager::Get()->LoadResource<Texture>(GUID::StringToGuid("17663088-100d-4e78-8305-17b5818256db"));
		m_skyboxMesh = ResourceManager::Get()->LoadResource<Mesh>(GUID::StringToGuid("b7d27f1a-006b-41fa-b10b-01b212ebfebe")); // DefaultSphere

		const GUID skyboxShaderGuid = GUID::StringToGuid("7e43e76d-9d5f-4fc8-a8f1-c8ec0dce95ef"); //shaders/skybox.hlsl
		const GUID skyboxSharedMaterialGuid = GUID::Random();

		m_skyboxPipeline = ResourceManager::Get()->LoadResource<GraphicsPipeline, GraphicsPipelineArgs>(
			skyboxSharedMaterialGuid,
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
			frameIndex,
			m_skyboxPipeline ->GetEngineBindings(),
			nullptr,
			viewProjectionData);

		GraphicsUtils::AttachViewToGraphics(commandList, m_skyboxPipeline, "skyboxTexture", m_skyboxTexture->GetSRV());
		GraphicsUtils::AttachViewToGraphics(commandList, m_skyboxPipeline, "gBufferColorTexture", m_colorTextureSRV);
		GraphicsUtils::AttachViewToGraphics(commandList, m_skyboxPipeline, "textureSampler", EngineSamplersProvider::GetLinearClampSampler());

		commandList->DrawIndexedInstanced(
			m_skyboxMesh->GetIndexCount(),
			1,
			0, 0, 0);
	}
}
