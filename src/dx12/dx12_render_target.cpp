#include "dx12/dx12_render_target.hpp"
#include "window.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
namespace feng
{
    RenderWindow::RenderWindow(Device &device, const Window &window)
    {
        device_ = &device;
        DXGI_SWAP_CHAIN_DESC1 sd;
        sd.Width = window.GetWidth();
        sd.Height = window.GetHeight();
        sd.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
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

        auto &rtv_heap = device.GetRTVHeap();

        rtv_begin_index_ = device_->GetRTVAllocIndex();
        TRY(swap_chain_->GetBuffer(0, IID_PPV_ARGS(&rtv_[0])));
        device_->GetDevice()->CreateRenderTargetView(rtv_[0].Get(), nullptr, rtv_heap.GetCpuHandle(rtv_begin_index_));
        for (int i = 1; i < BACK_BUFFER_SIZE; i++)
        {
            TRY(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&rtv_[i])));
            device.GetDevice()->CreateRenderTargetView(rtv_[i].Get(), nullptr, rtv_heap.GetCpuHandle(device.GetRTVAllocIndex()));
        }
    }

    ID3D12Resource *RenderWindow::CurrentBackBuffer() const
    {
        return rtv_[frame_id_].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderWindow::CurrentBackBufferView() const
    {
        return device_->GetRTVHeap().GetCpuHandle(rtv_begin_index_ + frame_id_);
    }

    void RenderWindow::Swap()
    {
        swap_chain_->Present(0, 0);
        frame_id_ = (frame_id_ + 1) % BACK_BUFFER_SIZE;
    }
} // namespace feng