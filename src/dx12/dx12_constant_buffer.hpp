#pragma once
#include "util/defines.hpp"
#include "dx12/dx12_device.hpp"
#include <mutex>
#include "d3dx12.h"
namespace feng
{

    template <typename T>
    class ConstantBuffer
    {
    public:
        ConstantBuffer(const Device &device, uint32_t count, ID3D12DescriptorHeap* cbv_heap)
        {
            size_ = sizeof(T);
            size_ = ConstantBufferSize(size_);

            device.GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                        D3D12_HEAP_FLAG_NONE,
                                                        &CD3DX12_RESOURCE_DESC::Buffer(size_ * count),
                                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                                        nullptr,
                                                        IID_PPV_ARGS(&resource_));
            resource_->Map(0, nullptr, reinterpret_cast<void**>(&map_));

            
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = resource_->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = size_;
            device.GetDevice()->CreateConstantBufferView(&cbvDesc, cbv_heap->GetCPUDescriptorHandleForHeapStart());
        }

        ID3D12Resource *GetResource() const
        {
            return resource_.Get();
        }

        void Write(int idx, const T &data)
        {
            memcpy(&map_[idx * size_], &data, sizeof(T));
        }

    private:
        ComPtr<ID3D12Resource> resource_ = nullptr;
        size_t size_ = 0;
        BYTE *map_ = nullptr;


    };
} // namespace feng
