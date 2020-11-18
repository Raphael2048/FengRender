#include "dx12/dx12_shader.hpp"
#include "d3dcompiler.h"
namespace feng
{
    ComPtr<ID3DBlob> Shader::CompileShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines, const std::string &entrypoint, const std::string &target)
    {
        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ComPtr<ID3DBlob> byteCode;
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

        if (errors != nullptr)
            OutputDebugStringA((char *)errors->GetBufferPointer());

        TRY(hr);
        return byteCode;
    }

    GraphicsShader::GraphicsShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines)
    {
        vs_bytes_ = CompileShader(filename, defines, "VS", "vs_5_1");
        ps_bytes_ = CompileShader(filename, defines, "PS", "ps_5_1");
    }

    void GraphicsShader::FillPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
    {
        desc.VS = {
            reinterpret_cast<BYTE *>(vs_bytes_->GetBufferPointer()),
            vs_bytes_->GetBufferSize(),
        };
        desc.PS = {
            reinterpret_cast<BYTE *>(ps_bytes_->GetBufferPointer()),
            ps_bytes_->GetBufferSize(),
        };
    }

    ComputeShader::ComputeShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines)
    {
        cs_bytes_ = CompileShader(filename, defines, "CS", "cs_5_1");
    }

    void ComputeShader::FillPSO(D3D12_COMPUTE_PIPELINE_STATE_DESC &desc)
    {
        desc.CS = {
            reinterpret_cast<BYTE *>(cs_bytes_->GetBufferPointer()),
            cs_bytes_->GetBufferSize(),
        };
    }

} // namespace feng