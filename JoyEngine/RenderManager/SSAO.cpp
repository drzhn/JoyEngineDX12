#include "SSAO.h"

#include <glm/vec4.hpp>

#include <glm/geometric.hpp>

#include "JoyContext.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "ResourceManager/Texture.h"
#include "JoyTypes.h"


namespace JoyEngine
{
	SSAO::SSAO(uint32_t width, uint32_t height, DXGI_FORMAT format):
		m_width(width),
		m_height(height)
	{
		//m_randomColorTexture = JoyContext::Resource->LoadResource<Texture>(
		//	GUID::StringToGuid("65c0d16a-9cf6-46e5-9a5e-a5026b350b8d")
		//); // textures/ColorNoise.png
		//m_ssaoRenderTarget = std::make_unique<RenderTexture>(
		//	m_width,
		//	m_height,
		//	format,
		//	D3D12_RESOURCE_STATE_RENDER_TARGET,
		//	D3D12_HEAP_TYPE_DEFAULT);

		//m_ssaoCopyResource = std::make_unique<Texture>(
		//	m_width,
		//	m_height,
		//	format,
		//	D3D12_RESOURCE_STATE_COPY_DEST,
		//	D3D12_HEAP_TYPE_DEFAULT,
		//	false,
		//	false,
		//	false,
		//	1
		//);

		uint32_t bufferSize = ((sizeof(SSAOData) - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
		m_ssaoDataBuffer = std::make_unique<Buffer>(
			bufferSize,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD);
		m_ssaoDataBufferView = std::make_unique<ResourceView>(
			D3D12_CONSTANT_BUFFER_VIEW_DESC{
				m_ssaoDataBuffer->GetBuffer()->GetGPUVirtualAddress(),
				bufferSize
			}
		);

		const auto ptr = m_ssaoDataBuffer->GetMappedPtr();
		SSAOData* data = static_cast<SSAOData*>(ptr->GetMappedPtr());

		data->offsetVectors[0] = glm::vec4(+1.0f, +1.0f, +1.0f, 0.0f);
		data->offsetVectors[1] = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
		data->offsetVectors[2] = glm::vec4(-1.0f, +1.0f, +1.0f, 0.0f);
		data->offsetVectors[3] = glm::vec4(+1.0f, -1.0f, -1.0f, 0.0f);
		data->offsetVectors[4] = glm::vec4(+1.0f, +1.0f, -1.0f, 0.0f);
		data->offsetVectors[5] = glm::vec4(-1.0f, -1.0f, +1.0f, 0.0f);
		data->offsetVectors[6] = glm::vec4(-1.0f, +1.0f, -1.0f, 0.0f);
		data->offsetVectors[7] = glm::vec4(+1.0f, -1.0f, +1.0f, 0.0f);
		// 6 centers of cube faces
		data->offsetVectors[8] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
		data->offsetVectors[9] = glm::vec4(+1.0f, 0.0f, 0.0f, 0.0f);
		data->offsetVectors[10] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
		data->offsetVectors[11] = glm::vec4(0.0f, +1.0f, 0.0f, 0.0f);
		data->offsetVectors[12] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		data->offsetVectors[13] = glm::vec4(0.0f, 0.0f, +1.0f, 0.0f);

		for (uint32_t i = 0; i < 14; ++i)
		{
			// Create random lengths in [0.25, 1.0].
			float s = 0.25f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (1 - 0.25f)));
			glm::vec4 norm = glm::normalize(data->offsetVectors[i]);
			glm::vec4 v = norm * s;

			data->offsetVectors[i] = v;
		}

		data->invRenderTargetSize = glm::vec2(1.0f / static_cast<float>(m_width), 1.0f / static_cast<float>(m_height));

		float* weightSumArr = reinterpret_cast<float*>(&data->blurWeights[0]);

		float sigma = 2.5f;

		float twoSigma2 = 2.0f * sigma * sigma;

		// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
		// For example, for sigma = 3, the width of the bell curve is 
		int blurRadius = static_cast<int>(ceil(2.0f * sigma));

		float weightSum = 0.0f;

		for (int i = -blurRadius; i <= blurRadius; ++i)
		{
			float x = (float)i;

			weightSumArr[i + blurRadius] = expf(-x * x / twoSigma2);

			weightSum += weightSumArr[i + blurRadius];
		}

		// Divide by the sum so all the weights add up to 1.0.
		for (int i = 0; i < 11; ++i)
		{
			weightSumArr[i] /= weightSum;
		}
		weightSumArr[11] = 0;
	}

	void SSAO::SetDirection(bool isHorizontal) const
	{
		const auto ptr = m_ssaoDataBuffer->GetMappedPtr();
		SSAOData* data = static_cast<SSAOData*>(ptr->GetMappedPtr());
		data->isHorizontal = isHorizontal ? 1 : 0;
	}
}
