#ifndef RESOURCE_H
#define RESOURCE_H
#include "HashDefs.h"

namespace JoyEngine
{
	class Resource
	{
	public:
		Resource() = delete;

		Resource(uint64_t resourceId) : m_resourceId(resourceId)
		{
		}

		Resource(const char* resourcePath) : m_resourceId(StrHash64(resourcePath))
		{
		}

		virtual ~Resource() = default;

		[[nodiscard]] int32_t GetRefCount() const { return m_refCount; }

		void AddRef() { m_refCount++; }

		void RemoveRef() { m_refCount--; }

		[[nodiscard]] virtual bool IsLoaded() const noexcept = 0;
		[[nodiscard]] uint64_t GetResourceId() const noexcept { return m_resourceId; }

	private:
		int32_t m_refCount = 0;

	protected:
		const uint64_t m_resourceId;
	};
}

#endif //RESOURCE_H
