#ifndef RESOURCE_HANDLE_H
#define RESOURCE_HANDLE_H


#include "Common/Resource.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	class IResourceManager
	{
	public:
		virtual ~IResourceManager() = default;
		virtual void IncreaseRefCounter(GUID guid) = 0;
		virtual void UnloadResource(GUID guid) = 0;
	};

	template <typename T>
	class ResourceHandle
	{
	public:
		ResourceHandle() = default;

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

		[[nodiscard]] bool Empty() const { return m_ptr == nullptr; }

	private:
		GUID m_guid;
		T* m_ptr = nullptr;
		IResourceManager* m_manager = nullptr;

	public:
		explicit ResourceHandle(T* ptr, IResourceManager* manager)
		{
			const Resource* resource = reinterpret_cast<Resource*>(ptr);
			m_ptr = ptr;
			m_guid = resource->GetGuid();
			m_manager = manager;
		}


		void Copy(const ResourceHandle<T>& other)
		{
			m_guid = other.m_guid;
			m_ptr = other.m_ptr;
			m_manager = other.m_manager;

			m_manager->IncreaseRefCounter(m_guid);
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
				m_manager->UnloadResource(m_guid);
				m_ptr = nullptr;
			}
		}
	};
}

#endif // RESOURCE_HANDLE_H
