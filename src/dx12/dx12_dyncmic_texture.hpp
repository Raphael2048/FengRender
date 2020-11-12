#pragma once
#include "dx12_defines.hpp"

namespace feng
{
    class Device;
    class DynamicTexture : Uncopyable
    {
    public:
        DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format);
        DynamicTexture(Device &device, UINT64 width, UINT64 height, DXGI_FORMAT unified_format);
        void TransitionState(ID3D12GraphicsCommandList* command, D3D12_RESOURCE_STATES state);
        ID3D12Resource *GetResource() { return buffer_.Get(); }
        int GetSRVHeapIndex() {return srv_heap_index_; }
        int GetRTVHeapIndex() {return rtv_heap_index_; }
        int GetDSVHeapIndex() {return dsv_heap_index_; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSRV();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTV();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDSV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPURTV();
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDSV();
    private:
        Device* device_;
        D3D12_RESOURCE_STATES current_state_;
        ComPtr<ID3D12Resource> buffer_;
        int srv_heap_index_ = -10000;
        union
        {
            int rtv_heap_index_ = -10000;
            int dsv_heap_index_;
        };
    };
} // namespace feng