#pragma once

#include "dx12/dx12_texture.hpp"
#include "dx12/dx12_buffer.hpp"
#include "render/depth_only.hpp"
#include "render/spot_light_effect.hpp"
#include "ResourceUploadBatch.h"
#include "DirectXHelpers.h"

namespace feng
{
    class VXGIEffect : public Uncopyable
    {
    private:
        ComPtr<ID3D12RootSignature> signature_density_, signature_accumulate_, signature_integration_;
        ComPtr<ID3D12PipelineState> pso_density_, pso_accumulate_, pso_integration_;

    };
}