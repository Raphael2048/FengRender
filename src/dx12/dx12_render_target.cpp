#include "dx12/dx12_render_target.hpp"
#include "window.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
namespace feng
{
    RenderTarget::RenderTarget(int buffers)
    {
        // rt_buffers_.resize(buffers);
    }

    RenderWindow::RenderWindow(Device &device, const Window &window)
    {
        device_ = &device;
        DXGI_SWAP_CHAIN_DESC1 sd;
        sd.Width = window.GetWidth();
        sd.Height = window.GetHeight();
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        // sd.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferCount = BACK_BUFFER_SIZE;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.Scaling = DXGI_SCALING_STRETCH;

        height_ = window.GetHeight();
        width_ = window.GetWidth();

        auto *f = device.GetFactory();
        IDXGISwapChain1 *temp_chain;
        f->CreateSwapChainForHwnd(device.GetCommandQueue(), window.GetWindowHandle(), &sd, NULL, NULL, &temp_chain);
        swap_chain_ = static_cast<IDXGISwapChain4 *>(temp_chain);
        frame_id_ = swap_chain_->GetCurrentBackBufferIndex();

        // Init Render Target Views
        // rt_buffers_.resize(BACK_BUFFER_SIZE);

        auto& rtv_heap = device.GetRTVHeap();

        rtv_begin_index_ = device_->GetRTVAllocIndex();
        TRY(swap_chain_->GetBuffer(0, IID_PPV_ARGS(&rtv_[0])));
        device_->GetDevice()->CreateRenderTargetView(rtv_[0].Get(), nullptr, rtv_heap.GetCpuHandle(rtv_begin_index_));
        for (int i = 1; i < BACK_BUFFER_SIZE; i++)
        {
            TRY(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&rtv_[i])));
            device.GetDevice()->CreateRenderTargetView(rtv_[i].Get(), nullptr, rtv_heap.GetCpuHandle(device.GetRTVAllocIndex()));
        }

        // Init Depth Stencil View
        // D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        // dsvHeapDesc.NumDescriptors = 1;
        // dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        // dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        // TRY(device.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&ds_heap_)));

        // NAME_D3D12RESOURCE(ds_heap_);

        // D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        // depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        // depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        // depthOptimizedClearValue.DepthStencil.Stencil = 0;

        // CD3DX12_HEAP_PROPERTIES heap_properties_default = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        // CD3DX12_RESOURCE_DESC tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, width_, height_, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        // TRY(device.GetDevice()->CreateCommittedResource(&heap_properties_default,
        //                                                 D3D12_HEAP_FLAG_NONE,
        //                                                 &tex_desc,
        //                                                 D3D12_RESOURCE_STATE_DEPTH_WRITE,
        //                                                 &depthOptimizedClearValue,
        //                                                 IID_PPV_ARGS(&ds_buffer_)));
        // NAME_D3D12RESOURCE(ds_buffer_);

        // D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        // depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        // depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        // depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
        // device.GetDevice()->CreateDepthStencilView(ds_buffer_, &depthStencilDesc, DepthStencilView());

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = static_cast<float>(width_);
        viewport_.Height = static_cast<float>(height_);
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
    }

    ID3D12Resource *RenderWindow::CurrentBackBuffer() const
    {
        // return rt_buffers_[frame_id_];
        return rtv_[frame_id_].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderWindow::CurrentBackBufferView() const
    {
        return device_->GetRTVHeap().GetCpuHandle(rtv_begin_index_ + frame_id_);
    }
    // D3D12_CPU_DESCRIPTOR_HANDLE RenderWindow::DepthStencilView() const
    // {
    //     return ds_heap_->GetCPUDescriptorHandleForHeapStart();
    // }

    void RenderWindow::Swap()
    {
        swap_chain_->Present(0, 0);
        frame_id_ = (frame_id_ + 1) % BACK_BUFFER_SIZE;
    }

    void RenderWindow::SetupCommandList(ComPtr<ID3D12GraphicsCommandList> command)
    {
        command->RSSetViewports(1, &viewport_);
        command->RSSetScissorRects(1, &scissor_rect_);
        command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
        command->ClearRenderTargetView(CurrentBackBufferView(), Color(0.1, 0.1, 0.1, 1), 0, nullptr);
        // command->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        // command->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
        command->OMSetRenderTargets(1, &CurrentBackBufferView(), FALSE, nullptr);
    }
} // namespace feng