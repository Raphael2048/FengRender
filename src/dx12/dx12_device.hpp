
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

        ID3D12Device5 *GetDevice() const { return device_; }

        IDXGIFactory6 *GetFactory() const { return factory_; }

        ID3D12CommandQueue *GetCommandQueue() const { return direct_queue_; }

        ID3D12GraphicsCommandList* BeginCommand(uint8_t index, ID3D12PipelineState* pso = nullptr);

        DirectX::DescriptorHeap& GetSTSRVHeap() { return *st_srv_heap_; }

        DirectX::DescriptorHeap& GetDTSRVHeap() { return *dt_srv_heap_; }

        size_t GetDTSRVAllocIndex() { return dt_srv_alloc_index_++; }

        DirectX::DescriptorHeap& GetRVTHeap() { return *rtv_heap_; }

        size_t GetRTVAllocIndex() { return rtv_alloc_index_++; }

        void EndCommand();

        void FlushCommand(uint8_t idx);

        void Wait(uint8_t idx);

        void Signal(uint8_t idx);

    private:
        // ID3D12Debug* debug;
        //IDXGIAdapter4 *_adapter;
        ID3D12Device5 *device_;
        IDXGIFactory6 *factory_;

        ID3D12Debug1 *debug_ = nullptr;
        ID3D12InfoQueue *_queue;

        ID3D12CommandQueue *direct_queue_;
        ID3D12CommandQueue *compute_queue_;
        ID3D12CommandQueue *copy_queue_;

        // Heap for Static Textures SRV
        std::unique_ptr<DirectX::DescriptorHeap> st_srv_heap_;

        // heap for dynamic textires' SRV
        std::unique_ptr<DirectX::DescriptorHeap> dt_srv_heap_;
        size_t dt_srv_alloc_index_ = 0;

        // heap for RTV
        std::unique_ptr<DirectX::DescriptorHeap> rtv_heap_;
        size_t rtv_alloc_index_ = 0;

        std::vector<ComPtr<ID3D12CommandAllocator>> command_allocators_;
        ComPtr<ID3D12GraphicsCommandList> command_list_;
        std::vector<Fence *> fences_;
    };
} // namespace feng