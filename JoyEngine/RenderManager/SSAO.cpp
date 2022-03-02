#include "SSAO.h"

#include <glm/vec4.hpp>

#include <glm/geometric.hpp>

#include "ResourceManager/Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "ResourceManager/Texture.h"


namespace JoyEngine
{
	SSAO::SSAO(uint32_t width, uint32_t height, DXGI_FORMAT format):
		m_width(width),
		m_height(height)
	{
		m_randomColorTexture = GUID::StringToGuid("65c0d16a-9cf6-46e5-9a5e-a5026b350b8d"); // textures/ColorNoise.png
		m_ssaoRenderTarget = std::make_unique<RenderTexture>(
			m_width,
			m_height,
			format,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_HEAP_TYPE_DEFAULT);

		uint32_t bufferSize = ((sizeof(glm::vec4) * 14 - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
		m_offsetBuffer = std::make_unique<Buffer>(
			bufferSize,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD);
		m_offsetBufferView = std::make_unique<ResourceView>(
			D3D12_CONSTANT_BUFFER_VIEW_DESC{
				m_offsetBuffer->GetBuffer()->GetGPUVirtualAddress(),
				bufferSize
			}
		);

		const auto ptr = m_offsetBuffer->GetMappedPtr();
		glm::vec4* data = static_cast<glm::vec4*>(ptr->GetMappedPtr());

		data[0] = glm::vec4(+1.0f, +1.0f, +1.0f, 0.0f);
		data[1] = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
		data[2] = glm::vec4(-1.0f, +1.0f, +1.0f, 0.0f);
		data[3] = glm::vec4(+1.0f, -1.0f, -1.0f, 0.0f);
		data[4] = glm::vec4(+1.0f, +1.0f, -1.0f, 0.0f);
		data[5] = glm::vec4(-1.0f, -1.0f, +1.0f, 0.0f);
		data[6] = glm::vec4(-1.0f, +1.0f, -1.0f, 0.0f);
		data[7] = glm::vec4(+1.0f, -1.0f, +1.0f, 0.0f);
		// 6 centers of cube faces
		data[8] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
		data[9] = glm::vec4(+1.0f, 0.0f, 0.0f, 0.0f);
		data[10] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
		data[11] = glm::vec4(0.0f, +1.0f, 0.0f, 0.0f);
		data[12] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		data[13] = glm::vec4(0.0f, 0.0f, +1.0f, 0.0f);

		for (uint32_t i = 0; i < 14; ++i)
		{
			// Create random lengths in [0.25, 1.0].
			float s = 0.25f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (1 - 0.25f)));
			glm::vec4 norm = glm::normalize(data[i]);
			glm::vec4 v = norm * s;

			data[i] = v;
		}
	}
}
