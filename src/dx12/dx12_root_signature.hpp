#pragma once

#include "dx12/dx12_defines.hpp"
#include "dx12/dx12_device.hpp"
#include "d3dx12.h"
namespace feng
{
 
    class RootSignature
    {
    public:
        RootSignature(const Device& device,  const CD3DX12_ROOT_SIGNATURE_DESC& desc);
        ID3D12RootSignature* GetRootSignature() {return signature_; }
    private:
        ID3D12RootSignature* signature_;
    };


} // namespace feng