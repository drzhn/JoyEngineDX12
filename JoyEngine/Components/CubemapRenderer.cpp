#include "CubemapRenderer.h"

#include "JoyContext.h"
#include "Common/CameraUnit.h"
#include "RenderManager/RenderManager.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	CubemapRenderer::CubemapRenderer()
	{
		m_cubemap = std::make_unique<RenderTexture>(
			m_textureSize,
			m_textureSize,
			RenderManager::GetHdrRTVFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT,
			6
		);

		m_cubemapConvoluted = std::make_unique<RenderTexture>(
			m_convolutedTextureSize,
			m_convolutedTextureSize,
			RenderManager::GetHdrRTVFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT,
			6
		);

		m_depthTexture = std::make_unique<Texture>(
			m_textureSize,
			m_textureSize,
			RenderManager::GetDepthFormat(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_HEAP_TYPE_DEFAULT,
			false,
			true,
			false);

		m_cameraUnit = CameraUnit(
			1,
			static_cast<float>(m_textureSize),
			static_cast<float>(m_textureSize),
			90,
			0.1f,
			1000
		);

		uint32_t bufferSize = ((sizeof(CubemapConvolutionConstants) - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
		m_convolutionConstantsDataBuffer = std::make_unique<Buffer>(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		m_convolutionConstantsBufferView = std::make_unique<ResourceView>(
			D3D12_CONSTANT_BUFFER_VIEW_DESC{
				m_convolutionConstantsDataBuffer->GetBuffer()->GetGPUVirtualAddress(),
				bufferSize
			}
		);
	}

	void CubemapRenderer::Enable()
	{
		JoyContext::Render->RegisterCubemapRenderer(this);
	}

	void CubemapRenderer::Disable()
	{
		JoyContext::Render->UnregisterCubemapRenderer(this);
	}

	void CubemapRenderer::Update()
	{
		const auto ptr = m_convolutionConstantsDataBuffer->GetMappedPtr();
		const auto data = static_cast<CubemapConvolutionConstants*>(ptr->GetMappedPtr());

		data->view[0] = GetCubeViewMatrix(0);
		data->view[1] = GetCubeViewMatrix(1);
		data->view[2] = GetCubeViewMatrix(2);
		data->view[3] = GetCubeViewMatrix(3);
		data->view[4] = GetCubeViewMatrix(4);
		data->view[5] = GetCubeViewMatrix(5);
		data->projection = GetProjMatrix();
		data->model = m_transform->GetModelMatrix();
	}

	glm::mat4 CubemapRenderer::GetCubeViewMatrix(uint32_t index) const
	{
		const glm::vec3 eye = m_transform->GetPosition();
		return glm::lookAtLH(eye, eye + shadowTransformsForward[index], shadowTransformsUp[index]);
	}

	glm::mat4x4 CubemapRenderer::GetProjMatrix() const
	{
		return m_cameraUnit.GetProjMatrix();
	}
}
