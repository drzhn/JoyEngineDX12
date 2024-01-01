#include "Skybox.h"

#include "IRenderer.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GraphicsUtils.h"

namespace JoyEngine
{
	// we cannot use depth testing for paint skybox pixels because we cannot use UAVgbuffer.m_depthTexture as a depth target
	// so we paint all pixels where color.a == 0;
	Skybox::Skybox(const char* texturePath)
	{
		m_skyboxTexture = ResourceManager::Get()->LoadResource<Texture>(texturePath);
		m_skyboxMesh = ResourceManager::Get()->LoadResource<Mesh>(
			"models/DefaultSphere.obj:RootNode/DefaultSphere_root/pSphere1"); // DefaultSphere

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
				"shaders/skybox.hlsl",
				JoyShaderTypeVertex | JoyShaderTypePixel,
				true,
				false,
				false,
				D3D12_CULL_MODE_FRONT,
				D3D12_COMPARISON_FUNC_GREATER_EQUAL,
				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
				{
					IRenderer::GetHDRRenderTextureFormat(),
				},
				1,
				IRenderer::GetDepthFormat(),
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
		GraphicsUtils::AttachView(commandList, m_skyboxPipeline.get(), "TextureSampler", EngineSamplersProvider::GetLinearClampSampler());

		commandList->DrawIndexedInstanced(
			m_skyboxMesh->GetIndexCount(),
			1,
			0, 0, 0);
	}
}
