#pragma once

#include "dx12/dx12_defines.hpp"
#include <array>

namespace feng
{
    class Device;
    class Window;
    class RenderWindow
    {
    public:
        RenderWindow(Device &device, const Window &window);
        ID3D12Resource *CurrentBackBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

        void Swap();

        uint8_t CurrentFrameIdx() const { return frame_id_; }

    private:
        uint32_t width_;
        uint32_t height_;

        Device* device_;
        std::array<ComPtr<ID3D12Resource>, BACK_BUFFER_SIZE> rtv_;
        size_t rtv_begin_index_;

        IDXGISwapChain4 *swap_chain_;
        uint8_t frame_id_;
    };

} // namespace feng