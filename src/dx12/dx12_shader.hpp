#pragma once

#include "dx12/dx12_defines.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
namespace feng
{

    class Shader
    {
    public:
        // virtual void FillPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);
    };
    class GraphicsShader : public Shader
    {
    public:
        GraphicsShader(const std::wstring &filename, const D3D_SHADER_MACRO* defines);
        void FillPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
    private:
        static ID3DBlob* CompileShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines, const std::string &entrypoint, const std::string &target);
        ID3DBlob *vs_bytes_ = nullptr;
        ID3DBlob *ps_bytes_ = nullptr;
    };

    // class ComputeShader : public Shader
    // {
    // };

} // namespace feng