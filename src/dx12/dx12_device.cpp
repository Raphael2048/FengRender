#include "dx12/dx12_device.hpp"
#include "dx12/dx12_fence.hpp"
namespace feng
{
    Device::Device(bool debug)
    {
        if (debug)
        {
            ID3D12Debug *t;
            TRY(D3D12GetDebugInterface(IID_PPV_ARGS(&t)));
            TRY(t->QueryInterface(IID_PPV_ARGS(&debug_)));

            debug_->SetEnableSynchronizedCommandQueueValidation(true);
            debug_->SetEnableGPUBasedValidation(true);
            debug_->EnableDebugLayer();
        }

        TRY(CreateDXGIFactory2(debug ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&factory_)));

        TRY(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)));
        device_->SetName(L"DX12 Device");

        // COMMAND QUEUE CREATION
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        TRY(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&direct_queue_)));
        direct_queue_->SetName(L"Direct Queue");

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        TRY(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&compute_queue_)));
        compute_queue_->SetName(L"Compute Queue");

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        TRY(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&copy_queue_)));
        copy_queue_->SetName(L"Copy Queue");

        // TRY(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_)));

        command_allocators_.resize(BACK_BUFFER_SIZE);
        for (auto &allocator : command_allocators_)
        {
            TRY(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(allocator.GetAddressOf())));
            NAME_D3D12RESOURCE(allocator);
        }
        //HRESULT hr = device.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators_[0], nullptr, IID_PPV_ARGS(&command_list_));
        TRY(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators_[0].Get(), nullptr, IID_PPV_ARGS(command_list_.GetAddressOf())));
        NAME_D3D12RESOURCE(command_list_);
        command_list_->Close();
        //

        st_srv_heap_.reset( new DirectX::DescriptorHeap(device_.Get(), 10));

        dt_srv_heap_.reset( new DirectX::DescriptorHeap(device_.Get(), 10));

        rtv_heap_.reset( new DirectX::DescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 10));

        dsv_heap_.reset( new DirectX::DescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 10));


        fences_.resize(BACK_BUFFER_SIZE);
        for (int i = 0; i < BACK_BUFFER_SIZE; i++)
        {
            fences_[i] = new Fence(*this);
            fences_[i]->SetName(L"Fence" + std::to_wstring(i));
        }
    }

    ID3D12GraphicsCommandList *Device::BeginCommand(uint8_t idx, ID3D12PipelineState *pso)
    {
        TRY(command_allocators_[idx]->Reset());
        TRY(command_list_->Reset(command_allocators_[idx].Get(), pso));
        return command_list_.Get();
    }

    void Device::EndCommand()
    {
        command_list_->Close();
        ID3D12CommandList *cmds[] = {command_list_.Get()};
        direct_queue_->ExecuteCommandLists(_countof(cmds), cmds);
    }

    void Device::FlushCommand(uint8_t idx)
    {
        fences_[idx]->Signal(direct_queue_);
        fences_[idx]->Wait();
    }

    void Device::Signal(uint8_t idx)
    {
        fences_[idx]->Signal(direct_queue_);
    }

    void Device::Wait(uint8_t idx)
    {
        fences_[idx]->Wait();
    }
} // namespace feng