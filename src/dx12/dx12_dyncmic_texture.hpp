#pragma once
#include "dx12_defines.hpp"

namespace feng
{
    class Device;
    class DynamicTexture
    {
    public:
        DynamicTexture(Device& device, UINT64 width, UINT64 height, DXGI_FORMAT typeless_format, DXGI_FORMAT read_format, DXGI_FORMAT write_format);
    private:
        ComPtr<ID3D12Resource> buffer_;
        int dt_srv_heap_index_ = -10000;
        union 
        {
            int rtv_heap_index_ = -10000;
            int dsv_heap_index_ ;
        };
    };
}