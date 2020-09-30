#include "dx12/dx12_pipeline_state.hpp"
#include "dx12/dx12_defines.hpp"
namespace feng
{
    PipelineState::PipelineState(const Device& device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
    {
        device.GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso_));
    }

}