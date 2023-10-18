#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <functional>
#include <map>
#include <set>
#include <Utils/Assert.h>
#include <memory>

#include "ResourceHandle.h"
#include "Common/HashDefs.h"
#include "Common/Resource.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	class ResourceManager : public Singleton<ResourceManager>
	{
	public:
		ResourceManager():
			m_unregisterResourceAction([this](uint64_t id)
			{
				if (IsResourceLoaded(id))
				{
					m_isResourceInUse.erase(id);
				}
				else
				{
					ASSERT(false);
				}
			})
		{
		}

		[[nodiscard]] bool IsResourceLoaded(uint64_t id) const
		{
			return m_isResourceInUse.contains(id);
		}

		template <class T, typename... Args>
		ResourceHandle<T> LoadResource(const char* path, Args&&... args)
		{
			uint64_t id = StrHash64(path);
			if (!IsResourceLoaded(id))
			{
				m_isResourceInUse.insert({id, new T(path, std::forward<Args>(args)...)});
			}
			m_isResourceInUse[id]->AddRef();
			return ResourceHandle<T>(GetResource<T>(id), &m_unregisterResourceAction);
		}

		template <class T, typename... Args>
		ResourceHandle<T> LoadResource(uint64_t id, Args&&... args)
		{
			if (!IsResourceLoaded(id))
			{
				m_isResourceInUse.insert({id, new T(id, std::forward<Args>(args)...)});
			}
			m_isResourceInUse[id]->AddRef();
			return ResourceHandle<T>(GetResource<T>(id), &m_unregisterResourceAction);
		}

		template <class T>
		ResourceHandle<T> RegisterResource(T* resource)
		{
			Resource* res = static_cast<Resource*>(resource);
			ASSERT(!IsResourceLoaded(res->GetResourceId()));
			m_isResourceInUse.insert({res->GetResourceId(), res});
			res->AddRef();
			return ResourceHandle<T>(resource, &m_unregisterResourceAction);
		}

	private:
		template <class T>
		T* GetResource(uint64_t id)
		{
			ASSERT(IsResourceLoaded(id));
#ifdef _DEBUG
			T* ptr = dynamic_cast<T*>(m_isResourceInUse[id]);
			ASSERT(ptr != nullptr);
#else
            T *ptr = reinterpret_cast<T *>(m_isResourceInUse[id]);
#endif //DEBUG
			return ptr;
		}

		std::map<uint64_t, Resource*> m_isResourceInUse;
		std::function<void(uint64_t)> m_unregisterResourceAction;
	};
}

#endif //RESOURCE_MANAGER_H
