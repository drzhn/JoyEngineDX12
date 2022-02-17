#include "ParticleSystem.h"

#include "RenderManager/JoyTypes.h"
#include "RenderManager/RenderManager.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/DummyMaterialProvider.h"


namespace JoyEngine
{
	void ParticleSystem::Enable()
	{
		JoyContext::Render->RegisterParticleSystem(this);

		m_buffer = std::make_unique<Buffer>(
			m_size * m_size * m_size * sizeof(glm::vec3), 
			D3D12_RESOURCE_STATE_GENERIC_READ, 
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer = {
			0,
			m_size * m_size * m_size,
			sizeof(glm::vec3),
			0
		};
		m_bufferView = std::make_unique<ResourceView>(desc, m_buffer->GetBuffer().Get());
	}

	void ParticleSystem::Disable()
	{
		JoyContext::Render->UnregisterParticleSystem(this);
	}

	void ParticleSystem::Update()
	{
	}
}
