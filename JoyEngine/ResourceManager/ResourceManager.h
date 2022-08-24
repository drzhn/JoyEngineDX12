#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>
#include <Utils/Assert.h>
#include <memory>

#include "ResourceHandle.h"
#include "Common/Resource.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	class ResourceManager : IResourceManager, public Singleton<ResourceManager>
	{
	public:
		ResourceManager() = default;

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
