#pragma once
#include "dx12_defines.hpp"
namespace feng
{
    class Device;
    class CommandList;
    class Buffer
    {
    public:
        Buffer(const Device& device, void* data, UINT size, ID3D12GraphicsCommandList* command);
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpu_address_; };
        UINT GetSize() { return size_; }
    protected:
        ComPtr<ID3D12Resource> buffer_;
        ComPtr<ID3D12Resource> staging_;
        UINT size_;
        // uint64_t stride_;
        //D3D12_RESOURCE_STATES state_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
        //std::uint8_t * cpu_address_;
        //bool staged_;
    };
} // namespace feng
