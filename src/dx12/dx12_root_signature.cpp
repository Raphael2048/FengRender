#include "dx12/dx12_root_signature.hpp"
namespace feng
{
    RootSignature::RootSignature(const Device& device, const CD3DX12_ROOT_SIGNATURE_DESC& desc)
    {
        ID3DBlob* serialize;
        ID3DBlob* error;
        HRESULT hr =  D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialize, &error);
        if (error != nullptr)
        {
            ::OutputDebugStringA((char*)error->GetBufferPointer());
        }
        TRY(hr);
        TRY(device.GetDevice()->CreateRootSignature(0, serialize->GetBufferPointer(), serialize->GetBufferSize(), IID_PPV_ARGS(&signature_)));
        SAFE_RELEASE(serialize);
        SAFE_RELEASE(error);
    }
}