#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>
#include <Utils/Assert.h>
#include <memory>

#include "Common/Resource.h"

namespace JoyEngine {

    class ResourceManager {
    public:

        ResourceManager() = default;

        void Init() {}

        void Start() {}

        void Stop() {}

        bool IsResourceLoaded(GUID guid) {
            return m_isResourceInUse.find(guid) != m_isResourceInUse.end();
        }

        template<class T>
        T* LoadResource(GUID guid) {
            if (!IsResourceLoaded(guid)) {
                m_isResourceInUse.insert({ guid, std::make_unique<T>(guid) });
            }
            m_isResourceInUse[guid]->IncreaseRefCount();
            return GetResource<T>(guid);
        }

        template<class T, typename... Args>
        T* LoadResource(GUID guid, Args... args) {
            if (!IsResourceLoaded(guid)) {
                m_isResourceInUse.insert({ guid, std::make_unique<T>(guid, args...) });
            }
            m_isResourceInUse[guid]->IncreaseRefCount();
            return GetResource<T>(guid);
        }

        void UnloadResource(GUID guid) {
            if (IsResourceLoaded(guid)) {
                m_isResourceInUse[guid]->DecreaseRefCount();
            } else {
                ASSERT(false);
            }
            if (m_isResourceInUse[guid]->GetRefCount() == 0) {
                m_isResourceInUse.erase(guid);
            }
        }

        template<class T>
        T *GetResource(GUID guid) {
            ASSERT(IsResourceLoaded(guid));
#ifdef DEBUG
            T *ptr = dynamic_cast<T *>(m_isResourceInUse[guid].get());
            ASSERT(ptr != nullptr);
#else
            T *ptr = reinterpret_cast<T *>(m_isResourceInUse[guid].get());
#endif //DEBUG
            return ptr;
        }

    private:
        std::map<GUID, std::unique_ptr<Resource>> m_isResourceInUse;
    };
}

#endif //RESOURCE_MANAGER_H