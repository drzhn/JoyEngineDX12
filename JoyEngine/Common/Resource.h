#ifndef RESOURCE_H
#define RESOURCE_H

#include "Utils/GUID.h"

namespace JoyEngine
{
	class Resource
	{
	public:
		Resource() = delete;

		Resource(GUID guid) : m_guid(guid)
		{
		}

		virtual ~Resource() = default;

		[[nodiscard]] int32_t GetRefCount() const { return m_refCount; }

		void AddRef() { m_refCount++; }

		void RemoveRef() { m_refCount--; }

		[[nodiscard]] virtual bool IsLoaded() const noexcept = 0;
		[[nodiscard]] GUID GetGuid() const noexcept { return m_guid; }

	private:
		int32_t m_refCount = 0;

	protected:
		const GUID m_guid;
	};
}

#endif //RESOURCE_H
