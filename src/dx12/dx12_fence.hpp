#pragma once
#include "dx12_defines.hpp"
namespace feng
{
    class Device;
    class Fence
    {
    public:
        Fence(const Device& device);
        ~Fence();
        void SetName(std::wstring name);
        void Wait();
        void Signal(ID3D12CommandQueue* queue);
    private:
        ID3D12Fence1* fence_;
        HANDLE event_ = nullptr;
        UINT64 value_ = 1u;
    };
} // namespace feng
