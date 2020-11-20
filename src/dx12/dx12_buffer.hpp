#pragma once
#include "dx12_defines.hpp"
#include "ResourceUploadBatch.h"
#include "BufferHelpers.h"
#include "d3dx12.h"
#include "dx12/dx12_device.hpp"
namespace feng
{
    class Device;
    class CommandList;
    class StaticBuffer : public Uncopyable
    {
    public:
        StaticBuffer(ID3D12Device *device, DirectX::ResourceUploadBatch &uploader, void *data, size_t count, size_t stride)
            : size_(count * stride)
        {
            DirectX::CreateStaticBuffer(
                device, uploader, data, count, stride, D3D12_RESOURCE_STATE_GENERIC_READ, buffer_.GetAddressOf());

            gpu_address_ = buffer_->GetGPUVirtualAddress();
        }
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpu_address_; };
        UINT GetSize() { return size_; }

    private:
        ComPtr<ID3D12Resource> buffer_;
        UINT size_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
    };

    class UAVBuffer : public Uncopyable
    {
    public:
        UAVBuffer(ID3D12Device *device, size_t count, size_t stride)
        {
            size_ = count * stride;
            auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto desc_desc = CD3DX12_RESOURCE_DESC::Buffer(
                count * stride,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            device->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &desc_desc,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                nullptr,
                IID_PPV_ARGS(&resource_));
            gpu_address_ = resource_->GetGPUVirtualAddress();
        }
        ID3D12Resource *GetResource()
        {
            return resource_.Get();
        }
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress()
        {
            return gpu_address_;
        }
        void AsSRV(ID3D12GraphicsCommandList *command)
        {
            auto transition = CD3DX12_RESOURCE_BARRIER::Transition(resource_.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            command->ResourceBarrier(1, &transition);
        }
        void AsUAV(ID3D12GraphicsCommandList *command)
        {
            auto transition = CD3DX12_RESOURCE_BARRIER::Transition(resource_.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            command->ResourceBarrier(1, &transition);
        }

    private:
        ComPtr<ID3D12Resource> resource_;
        UINT size_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
    };

    template <typename T>
    class ConstantBuffer
    {
    public:
        ConstantBuffer<T>(const ConstantBuffer<T> &) = delete;
        ConstantBuffer<T>(ConstantBuffer<T> &&) noexcept = default;
        ConstantBuffer<T> &operator=(const ConstantBuffer<T> &) = delete;
        ConstantBuffer<T> &operator=(ConstantBuffer<T> &&) noexcept = default;
        ConstantBuffer(const Device &device, uint32_t count)
        {
            size_ = sizeof(T);
            // constant buffer大小必须是256的倍数
            size_ = ConstantBufferSize(size_);

            auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto desc_desc = CD3DX12_RESOURCE_DESC::Buffer(size_ * count);
            device.GetDevice()->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &desc_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&resource_));
            resource_->Map(0, nullptr, reinterpret_cast<void **>(&map_));
        }

        ID3D12Resource *GetResource() const
        {
            return resource_.Get();
        }

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddressOf(int at) const
        {
            return resource_->GetGPUVirtualAddress() + at * size_;
        }

        size_t GetSize() { return size_; }

        void Write(int idx, const T &data)
        {
            memcpy(&map_[idx * size_], &data, sizeof(T));
        }

    private:
        ComPtr<ID3D12Resource> resource_ = nullptr;
        size_t size_ = 0;
        BYTE *map_ = nullptr;
    };

    template <typename T, int SIZE>
    class ConstantBufferGroup
    {
    public:
        ConstantBufferGroup(const Device &device, uint32_t count)
        {
            buffers_.reserve(SIZE);
            for (int i = 0; i < SIZE; i++)
            {
                buffers_.emplace_back(device, count);
            }
        }

        ConstantBuffer<T> &operator[](int idx)
        {
            return buffers_[idx];
        }

    private:
        std::vector<ConstantBuffer<T>> buffers_;
    };
} // namespace feng
