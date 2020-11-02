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
        ConstantBuffer<T>(const ConstantBuffer<T>&) = delete;
        ConstantBuffer<T>(ConstantBuffer<T>&&) noexcept = default;
        ConstantBuffer<T>& operator = (const ConstantBuffer<T>&) = delete;
        ConstantBuffer<T>& operator = (ConstantBuffer<T>&&) noexcept = default;
        ConstantBuffer(const Device &device, uint32_t count)
        {
            size_ = sizeof(T);
            // constant buffer大小必须是256的倍数
            size_ = ConstantBufferSize(size_);

            device.GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                        D3D12_HEAP_FLAG_NONE,
                                                        &CD3DX12_RESOURCE_DESC::Buffer(size_ * count),
                                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                                        nullptr,
                                                        IID_PPV_ARGS(&resource_));
            resource_->Map(0, nullptr, reinterpret_cast<void**>(&map_));

            
            // D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            // cbvDesc.BufferLocation = resource_->GetGPUVirtualAddress();
            // cbvDesc.SizeInBytes = static_cast<UINT>(size_);
            // device.GetDevice()->CreateConstantBufferView(&cbvDesc, cbv_heap->GetCPUDescriptorHandleForHeapStart());
        }

        ID3D12Resource *GetResource() const
        {
            return resource_.Get();
        }

        size_t GetSize() {return size_; }

        void Write(int idx, const T &data)
        {
            memcpy(&map_[idx * size_], &data, sizeof(T));
        }

    private:
        ComPtr<ID3D12Resource> resource_ = nullptr;
        size_t size_ = 0;
        BYTE *map_ = nullptr;
    };

    template<typename T, int SIZE>
    class ConstantBufferGroup
    {
    public:
        ConstantBufferGroup(const Device& device, uint32_t count)
        {
            for (int i = 0; i < SIZE; i++)
            {
               buffers_.push_back(ConstantBuffer<T>(device, count));
            }
        }

        ConstantBuffer<T>& operator[](int idx)
        {
            return buffers_[idx];
        }
    private:
        std::vector<ConstantBuffer<T>> buffers_;
    };
} // namespace feng
