#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include "Component.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	class ResourceView;
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

		[[nodiscard]] Buffer* GetBuffer() const noexcept { return m_buffer.get(); }
		[[nodiscard]] ResourceView* GetResourceView() const noexcept { return m_bufferView.get(); }
		[[nodiscard]] uint32_t GetSize() const noexcept { return m_size; }

	private:
		const uint32_t m_size = 128;


		std::unique_ptr<Buffer> m_buffer;
		std::unique_ptr<ResourceView> m_bufferView;
	};
}

#endif // PARTICLE_SYSTEM_H
