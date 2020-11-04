#pragma once
#include "dx12_defines.hpp"
#include "DDSTextureLoader.h"
namespace feng
{
    class DirectX::ResourceUploadBatch;
    class Device;
    class StaticTexture
    {
    public:
        StaticTexture(const Device& device, DirectX::ResourceUploadBatch& uplaoder, const std::wstring path);
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpu_address_; };
    protected:
        DXGI_FORMAT format_;
        UINT width;
        UINT height;
        ComPtr<ID3D12Resource> buffer_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
    };
} // namespace feng
