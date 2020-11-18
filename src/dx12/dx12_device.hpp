
#pragma once
#include "dx12_defines.hpp"
#include "DescriptorHeap.h"
namespace feng
{
    class Fence;
    class Device
    {
    public:
        Device(bool debug = true);

        ID3D12Device *GetDevice() const { return device_.Get(); }

        IDXGIFactory6 *GetFactory() const { return factory_.Get(); }

        ID3D12CommandQueue *GetCommandQueue() const { return direct_queue_.Get(); }

        ID3D12GraphicsCommandList* BeginCommand(uint8_t index, ID3D12PipelineState* pso = nullptr);

        DirectX::DescriptorHeap& GetSRVHeap() const { return *srv_heap_; }

        size_t GetSRVAllocIndex() { return srv_alloc_index++; }

        DirectX::DescriptorHeap& GetRTVHeap() const { return *rtv_heap_; } 

        size_t GetRTVAllocIndex() { return rtv_alloc_index_++; }

        DirectX::DescriptorHeap& GetDSVHeap() const { return *dsv_heap_; }

        size_t GetDSVAllocIndex() {return dsv_alloc_index_++;}

        void EndCommand();

        void FlushCommand(uint8_t idx);

        void Wait(uint8_t idx);

        void Signal(uint8_t idx);

    private:
        ComPtr<ID3D12Device> device_;
        ComPtr<IDXGIFactory6> factory_;

        ComPtr<ID3D12Debug1> debug_;
        ComPtr<ID3D12InfoQueue> _queue;

        ComPtr<ID3D12CommandQueue> direct_queue_;

        // Heap for SRV
        std::unique_ptr<DirectX::DescriptorHeap> srv_heap_;
        size_t srv_alloc_index = 0;

        // heap for RTV
        std::unique_ptr<DirectX::DescriptorHeap> rtv_heap_;
        size_t rtv_alloc_index_ = 0;

        std::unique_ptr<DirectX::DescriptorHeap> dsv_heap_;
        size_t dsv_alloc_index_ = 0;

        std::vector<ComPtr<ID3D12CommandAllocator>> command_allocators_;
        ComPtr<ID3D12GraphicsCommandList4> command_list_;
        std::vector<Fence *> fences_;
    };
} // namespace feng