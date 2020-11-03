#include "dx12/dx12_shader.hpp"
#include "d3dcompiler.h"
namespace feng
{
    GraphicsShader::GraphicsShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines)
    {
        vs_bytes_ = CompileShader(filename, defines, "VS", "vs_5_0");
        ps_bytes_ = CompileShader(filename, defines, "PS", "ps_5_0");
    }

    void GraphicsShader::FillPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
    {
        desc.VS =
            {
                reinterpret_cast<BYTE *>(vs_bytes_->GetBufferPointer()),
                vs_bytes_->GetBufferSize(),
            };

        desc.PS =
            {
                reinterpret_cast<BYTE *>(ps_bytes_->GetBufferPointer()),
                ps_bytes_->GetBufferSize(),
            };
    }
    ID3DBlob* GraphicsShader::CompileShader(const std::wstring &filename, const D3D_SHADER_MACRO *defines, const std::string &entrypoint, const std::string &target)
    {

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ID3DBlob *byteCode = nullptr;
        ID3DBlob *errors;
        HRESULT hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

        if (errors != nullptr)
            OutputDebugStringA((char *)errors->GetBufferPointer());

        TRY(hr);
        SAFE_RELEASE(errors);
        return byteCode;
    }
} // namespace feng