#pragma once

#include "dx12/dx12_defines.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
namespace feng
{

    class Shader
    {
    public:
        static ComPtr<ID3DBlob> CompileShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines, const std::string &entrypoint, const std::string &target);
    };
    class GraphicsShader : public Shader
    {
    public:
        GraphicsShader(const std::wstring &filename, const D3D_SHADER_MACRO* defines = nullptr);
        void FillPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
    private:
        ComPtr<ID3DBlob> vs_bytes_;
        ComPtr<ID3DBlob> ps_bytes_;
    };

    class ComputeShader : public Shader
    {
    public:
        ComputeShader(const std::wstring &filename, const D3D_SHADER_MACRO* defines = nullptr);
        void FillPSO(D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);
    private:
        ComPtr<ID3DBlob> cs_bytes_;
    };

} // namespace feng