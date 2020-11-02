#include "dx12/dx12_fence.hpp"
#include "dx12/dx12_device.hpp"

namespace feng
{
    Fence::Fence(const Device& device)
    {
        TRY(device.GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
        fence_->SetName(L"Fence");
        event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (event_ == 0)
        {
            FMSG("Error");
        }
    }

    Fence::~Fence()
    {
        SAFE_RELEASE(fence_);
        CloseHandle(event_);
    }
    void Fence::SetName(std::wstring name)
    {
        fence_->SetName(name.c_str());
    }
    void Fence::Wait()
    {
        if (fence_->GetCompletedValue() < value_)
        {
            TRY(fence_->SetEventOnCompletion(value_, event_));
            WaitForSingleObject(event_, INFINITE);
        }
    }
    void Fence::Signal(ID3D12CommandQueue* queue)
    {
        ++ value_;
        queue->Signal(fence_, value_);
    }
} // namespace feng
