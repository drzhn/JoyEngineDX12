#ifndef SHADER_H
#define SHADER_H

#include <d3d12.h>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

#include "Common/Resource.h"

namespace JoyEngine {
    class Shader final: public Resource {
    public :

        Shader() = delete;

        explicit Shader(GUID);

        ~Shader() final;

        [[nodiscard]] ComPtr<ID3DBlob> GetVertexShadeModule() noexcept { return m_vertexModule; }
        [[nodiscard]] ComPtr<ID3DBlob> GetFragmentShadeModule() noexcept { return m_fragmentModule; }
        [[nodiscard]] bool IsLoaded() const noexcept override { return true; }

    private :
        ComPtr<ID3DBlob> m_vertexModule;
        ComPtr<ID3DBlob> m_fragmentModule;
    };
}

#endif //SHADER_H
