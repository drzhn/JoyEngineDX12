#ifndef RESOURCE_HANDLE_H
#define RESOURCE_HANDLE_H

#include "JoyContext.h"

#include "ResourceManager/ResourceManager.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	template <typename T>
	class ResourceHandle
	{
	public:
		ResourceHandle() = default;
		ResourceHandle(ResourceHandle&& other) = delete;
		ResourceHandle(const ResourceHandle& other) = delete;
		ResourceHandle<T>& operator=(const ResourceHandle<T>& other) = delete;
		ResourceHandle<T>& operator=(const ResourceHandle<T>&& other) = delete;

		explicit ResourceHandle(GUID guid): m_guid(guid)
		{
			m_ptr = JoyContext::Resource->LoadResource<T>(m_guid);
		}

		~ResourceHandle()
		{
			if (m_ptr != nullptr)
			{
				JoyContext::Resource->UnloadResource(m_guid);
				m_ptr = nullptr;
			}
		}

		void Clear()
		{
			if (m_ptr != nullptr)
			{
				JoyContext::Resource->UnloadResource(m_guid);
				m_ptr = nullptr;
			}
		}

		ResourceHandle<T>& operator=(GUID guid)
		{
			if (m_ptr != nullptr)
			{
				JoyContext::Resource->UnloadResource(m_guid);
			}
			m_guid = guid;
			m_ptr = JoyContext::Resource->LoadResource<T>(m_guid);
			return *this;
		}

		bool operator ==(const ResourceHandle<T>& other)
		{
			return m_ptr == other.m_ptr;
		}

		operator T*() const
		{
			return m_ptr;
		}

		T* operator ->() const
		{
			return m_ptr;
		}

	private:
		GUID m_guid;
		T* m_ptr = nullptr;
	};
}

#endif // RESOURCE_HANDLE_H
