#ifndef RESOURCE_HANDLE_H
#define RESOURCE_HANDLE_H

#include <functional>

#include "Common/Resource.h"

namespace JoyEngine
{
	template <typename T>
	class ResourceHandle
	{
	public:
		ResourceHandle() = default;

		explicit ResourceHandle(T* ptr, std::function<void(uint64_t)>* unregisterResourceAction)
			: m_unregisterResourceAction(unregisterResourceAction)
		{
			const Resource* resource = static_cast<Resource*>(ptr);
			m_ptr = ptr;
			m_guid = resource->GetResourceId();
		}

		// copy
		ResourceHandle(const ResourceHandle<T>& other)
		{
			Copy(other);
		}

		// move
		ResourceHandle(ResourceHandle<T>&& other) noexcept
		{
			Move(other);
		}

		// copy assignment
		ResourceHandle<T>& operator=(const ResourceHandle<T>& other) noexcept
		{
			Copy(other);
			return *this;
		}

		// move assignment
		ResourceHandle<T>& operator=(ResourceHandle<T>&& other) noexcept
		{
			Move(other);
			return *this;
		}

		~ResourceHandle()
		{
			Release();
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

		T* Get() const
		{
			return m_ptr;
		}

	private:
		uint64_t m_guid;
		T* m_ptr = nullptr;
		std::function<void(uint64_t)>* m_unregisterResourceAction;


		void Copy(const ResourceHandle<T>& other)
		{
			m_guid = other.m_guid;
			m_unregisterResourceAction = other.m_unregisterResourceAction;

			if (other.m_ptr != nullptr)
			{
				m_ptr = other.m_ptr;
				Resource* resource = static_cast<Resource*>(m_ptr);
				resource->AddRef();
			}
		}

		void Move(ResourceHandle<T>& other)
		{
			Copy(other);

			other.Release();
		}

		void Release()
		{
			if (m_ptr != nullptr)
			{
				Resource* resource = static_cast<Resource*>(m_ptr);
				resource->RemoveRef();

				if (resource->GetRefCount() == 0)
				{
					delete m_ptr;
					m_unregisterResourceAction->operator()(m_guid);
				}

				m_ptr = nullptr;
			}
		}
	};
}

#endif // RESOURCE_HANDLE_H
