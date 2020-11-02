
#pragma once
#include "dx12_defines.hpp"
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

        ID3D12DescriptorHeap* GetCBVHeap() { return cbv_heap_.Get(); }

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

        ID3D12DescriptorHeap *_rtv_heap;
        UINT _rtv_desc_size;
        ID3D12DescriptorHeap *_dsv_heap;
        UINT _dsv_desc_size;

        // CBV_SRV_UAV
        ComPtr<ID3D12DescriptorHeap> cbv_heap_;
        UINT cbv_heap_size_;

        std::vector<ComPtr<ID3D12CommandAllocator>> command_allocators_;
        ComPtr<ID3D12GraphicsCommandList> command_list_;
        std::vector<Fence *> fences_;
    };
} // namespace feng