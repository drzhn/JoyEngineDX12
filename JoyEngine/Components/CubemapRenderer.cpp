#include "CubemapRenderer.h"

#include "JoyContext.h"
#include "Common/CameraUnit.h"
#include "RenderManager/RenderManager.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	CubemapRenderer::CubemapRenderer()
	{
		m_cubemap = std::make_unique<RenderTexture>(
			m_textureSize,
			m_textureSize,
			DXGI_FORMAT_R8G8B8A8_UNORM, // make, i don't know, getting that from graphics context manager
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT,
			6
		);
		m_depthTexture = std::make_unique<Texture>(
			m_textureSize,
			m_textureSize,
			DXGI_FORMAT_D32_FLOAT,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_HEAP_TYPE_DEFAULT,
			false,
			true);

		m_cameraUnit = CameraUnit(
			1,
			static_cast<float>(m_textureSize),
			static_cast<float>(m_textureSize),
			90,
			0.1f,
			1000
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
