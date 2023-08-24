#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <functional>
#include <map>
#include <set>
#include <Utils/Assert.h>
#include <memory>

#include "ResourceHandle.h"
#include "Common/Resource.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	class ResourceManager: public Singleton<ResourceManager>
	{
	public:
		ResourceManager():
			m_unregisterResourceAction([this](GUID guid)
			{
				if (IsResourceLoaded(guid))
				{
					m_isResourceInUse.erase(guid);
				}
				else
				{
					ASSERT(false);
				}
			})
		{
		}

		[[nodiscard]] bool IsResourceLoaded(GUID guid) const
		{
			return m_isResourceInUse.contains(guid);
		}

		template <class T, typename... Args>
		ResourceHandle<T> LoadResource(GUID guid, Args&&... args)
		{
			if (!IsResourceLoaded(guid))
			{
				m_isResourceInUse.insert({guid, new T(guid, args...)});
			}
			m_isResourceInUse[guid]->AddRef();
			return ResourceHandle<T>(GetResource<T>(guid), &m_unregisterResourceAction);
		}

	private:
		template <class T>
		T* GetResource(GUID guid)
		{
			ASSERT(IsResourceLoaded(guid));
#ifdef _DEBUG
			T* ptr = dynamic_cast<T*>(m_isResourceInUse[guid]);
			ASSERT(ptr != nullptr);
#else
            T *ptr = reinterpret_cast<T *>(m_isResourceInUse[guid].get());
#endif //DEBUG
			return ptr;
		}

		std::map<GUID, Resource*> m_isResourceInUse;
		std::function<void(GUID)> m_unregisterResourceAction;
	};
}

#endif //RESOURCE_MANAGER_H
