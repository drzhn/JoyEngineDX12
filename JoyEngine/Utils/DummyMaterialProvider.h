#ifndef DUMMY_MATERIAL_PROVIDER_H
#define DUMMY_MATERIAL_PROVIDER_H
#include "ResourceManager/Material.h"

namespace JoyEngine
{
	class DummyMaterialProvider
	{
	public:
		DummyMaterialProvider() = default;
		~DummyMaterialProvider() = default;
		void Init();

		[[nodiscard]] Material* GetMaterial() const noexcept { return m_material; }
	private:
		Material* m_material;
	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
