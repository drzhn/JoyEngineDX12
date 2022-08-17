#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>
#include <Utils/Assert.h>
#include <memory>

#include "Common/Resource.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	class ResourceManager;

	class IResourceManager
	{
	public:
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

		[[nodiscard]] bool Empty() const { return m_ptr == nullptr; }

	private:
		GUID m_guid;
		T* m_ptr = nullptr;
		IResourceManager* m_manager = nullptr;

	private:
		explicit ResourceHandle(T* ptr, IResourceManager* manager)
		{
			Resource* resource = dynamic_cast<Resource*>(ptr);
			ASSERT(resource != nullptr);
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

		friend class ResourceManager;
	};

	class ResourceManager : IResourceManager, public Singleton<ResourceManager>
	{
	public:
		ResourceManager() = default;

		void Init()
		{
		}

		void Start()
		{
		}

		void Stop()
		{
		}

		bool IsResourceLoaded(GUID guid)
		{
			return m_isResourceInUse.find(guid) != m_isResourceInUse.end();
		}

		template <class T>
		ResourceHandle<T> LoadResource(GUID guid)
		{
			if (!IsResourceLoaded(guid))
			{
				m_isResourceInUse.insert({guid, std::make_unique<T>(guid)});
			}
			m_isResourceInUse[guid]->IncreaseRefCount();
			return ResourceHandle<T>(GetResource<T>(guid), this);
		}

		template <class T, typename... Args>
		ResourceHandle<T> LoadResource(GUID guid, Args&& ... args)
		{
			if (!IsResourceLoaded(guid))
			{
				m_isResourceInUse.insert({guid, std::make_unique<T>(guid, args...)});
			}
			m_isResourceInUse[guid]->IncreaseRefCount();
			return ResourceHandle<T>(GetResource<T>(guid), this);
		}

	private:
		template <class T>
		T* GetResource(GUID guid)
		{
			ASSERT(IsResourceLoaded(guid));
#ifdef _DEBUG
			T* ptr = dynamic_cast<T*>(m_isResourceInUse[guid].get());
			ASSERT(ptr != nullptr);
#else
            T *ptr = reinterpret_cast<T *>(m_isResourceInUse[guid].get());
#endif //DEBUG
			return ptr;
		}

		void IncreaseRefCounter(GUID guid) override
		{
			if (IsResourceLoaded(guid))
			{
				m_isResourceInUse[guid]->IncreaseRefCount();
			}
			else
			{
				ASSERT(false);
			}
		}

		void UnloadResource(GUID guid) override
		{
			if (IsResourceLoaded(guid))
			{
				m_isResourceInUse[guid]->DecreaseRefCount();
			}
			else
			{
				ASSERT(false);
			}
			if (m_isResourceInUse[guid]->GetRefCount() == 0)
			{
				m_isResourceInUse.erase(guid);
			}
		}

		std::map<GUID, std::unique_ptr<Resource>> m_isResourceInUse;
	};
}

#endif //RESOURCE_MANAGER_H
