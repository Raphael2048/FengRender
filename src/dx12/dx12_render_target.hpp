#pragma once

#include "dx12/dx12_defines.hpp"

namespace feng
{

    class RenderTargetBase : public Uncopyable
    {
    public:
        virtual void SetupCommandList(ComPtr<ID3D12GraphicsCommandList> command){};
    protected:
        uint32_t width_;
        uint32_t height_;

        std::vector<ID3D12Resource *> rt_buffers_;
        ID3D12DescriptorHeap *rt_heap_;
        uint32_t rt_desc_inc_size;

        ID3D12Resource *ds_buffer_;
        ID3D12DescriptorHeap *ds_heap_;
    };

    class RenderTarget : public RenderTargetBase
    {
    public:
        RenderTarget(int buffers);

    private:
    };

    class Window;
    class Device;
    class RenderWindow : public RenderTargetBase
    {
    public:
        RenderWindow(const Device &device, const Window &window);
        ID3D12Resource *CurrentBackBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
        D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

        void Swap();

        uint8_t CurrentFrameIdx() const { return frame_id_; }

        virtual void SetupCommandList(ComPtr<ID3D12GraphicsCommandList> command) override;
    private:
        IDXGISwapChain4 *swap_chain_;
        uint8_t frame_id_;
        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;
    };

} // namespace feng