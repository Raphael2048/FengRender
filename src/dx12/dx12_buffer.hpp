#pragma once
#include "dx12_defines.hpp"
#include "ResourceUploadBatch.h"
namespace feng
{
    class Device;
    class CommandList;
    class Buffer
    {
    public:
        Buffer(ID3D12Device* device, DirectX::ResourceUploadBatch& uploader, void* data, size_t count, size_t stride);
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpu_address_; };
        UINT GetSize() { return size_;}
    protected:
        ComPtr<ID3D12Resource> buffer_;
        UINT size_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
    };
} // namespace feng
