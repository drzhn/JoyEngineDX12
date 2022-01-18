#ifndef DUMMY_MATERIAL_PROVIDER_H
#define DUMMY_MATERIAL_PROVIDER_H

#include "ResourceManager/Material.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class DummyMaterialProvider
	{
	public:
		DummyMaterialProvider() = default;
		~DummyMaterialProvider() = default;
		void Init();

		[[nodiscard]] GUID GetMaterialGuid() const noexcept { return m_materialGuid; }
	private:
		GUID m_materialGuid;
	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
