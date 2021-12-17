#ifndef RESOURCE_H
#define RESOURCE_H

#include "Utils/GUID.h"

namespace JoyEngine {
    class Resource {
    public:
        Resource() = default;

        Resource(GUID guid) :m_guid(guid) {}

        virtual ~Resource() = default;

        [[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }

        void IncreaseRefCount() { m_refCount++; }

        void DecreaseRefCount() { m_refCount--; }

        [[nodiscard]] virtual bool IsLoaded() const noexcept = 0;
        [[nodiscard]] GUID GetGuid() const noexcept { return m_guid; }

    private:
        uint32_t m_refCount = 0;
    protected:
        const GUID m_guid;
    };
}

#endif //RESOURCE_H
