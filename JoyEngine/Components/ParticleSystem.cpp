#include "ParticleSystem.h"

#include "RenderManager/RenderManager.h"
#include "ResourceManager/Buffer.h"


namespace JoyEngine
{
	void ParticleSystem::Enable()
	{
		JoyContext::Render->RegisterParticleSystem(this);

		m_buffer = std::make_unique<Buffer>(1024 * 1024 * sizeof(glm::vec3), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_DEFAULT);
	}

	void ParticleSystem::Disable()
	{
		JoyContext::Render->UnregisterParticleSystem(this);
	}

	void ParticleSystem::Update()
	{
	}
}
