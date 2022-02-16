#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include "Component.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	class Buffer;
	class SharedMaterial;
	class ComputePipeline;

	class ParticleSystem final : public Component
	{
	public:
		ParticleSystem() = default;
		~ParticleSystem() override = default;
		void Enable() override;
		void Disable() override;
		void Update() override;
	private:
		ResourceHandle<ComputePipeline> m_computePipeline;
		ResourceHandle<SharedMaterial> m_sharedMaterial;
		std::unique_ptr<Buffer> m_buffer;
	};
}

#endif // PARTICLE_SYSTEM_H
