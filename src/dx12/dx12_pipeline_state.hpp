#pragma once
#include "dx12_defines.hpp"
#include "dx12_root_signature.hpp"
#include "dx12_shader.hpp"

namespace feng
{
    class Device;
    class PipelineState
    {
    public:
        PipelineState(const Device& device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
        ID3D12PipelineState* GetPipelineState() {return pso_;}
    private:
        ID3D12PipelineState* pso_;
    };
} // namespace feng