#pragma once

#include "dx12/dx12_defines.hpp"
#include <array>

namespace feng
{
    class Device;
    class RenderTargetBase : public Uncopyable
    {
    public:
        virtual void SetupCommandList(ComPtr<ID3D12GraphicsCommandList> command){};

    protected:
        uint32_t width_;
        uint32_t height_;

        Device* device_;

        // std::vector<ID3D12Resource *> rt_buffers_;
        // ID3D12DescriptorHeap *rt_heap_;
        // uint32_t rt_desc_inc_size;

        // ID3D12Resource *ds_buffer_;
        // ID3D12DescriptorHeap *ds_heap_;
    };

    class RenderTarget
    {
    public:
        RenderTarget(int buffers);

    private:
    };

    class Window;
    class RenderWindow
    {
    public:
        RenderWindow(Device &device, const Window &window);
        ID3D12Resource *CurrentBackBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

        void Swap();

        uint8_t CurrentFrameIdx() const { return frame_id_; }

        void SetupCommandList(ComPtr<ID3D12GraphicsCommandList> command);

    private:
        uint32_t width_;
        uint32_t height_;

        Device* device_;
        std::array<ComPtr<ID3D12Resource>, BACK_BUFFER_SIZE> rtv_;
        size_t rtv_begin_index_;

        IDXGISwapChain4 *swap_chain_;
        uint8_t frame_id_;
        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;
    };

} // namespace feng