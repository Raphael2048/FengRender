#include "render/sky_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"

namespace feng
{
    SkyLightEffect::SkyLightEffect(std::shared_ptr<StaticTexture> texture, Renderer &renderer, ID3D12GraphicsCommandList* command_list)
        :cubemap_(texture)
    {
        auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\sky_light_sh.hlsl");
        sh_buffer_.reset(new UAVBuffer(renderer.GetDevice().GetDevice(), 9, sizeof(Vector3)));

        CD3DX12_ROOT_PARAMETER slotRootParameter[2];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
        slotRootParameter[1].InitAsUnorderedAccessView(0);

        auto samplers = renderer.GetStaticSamplers();
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &sh_sigature_));

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
        shader->FillPSO(psoDesc);
        psoDesc.pRootSignature = sh_sigature_.Get();
        renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&sh_pipeline));

        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        command_list->ResourceBarrier(1, &transition);
        command_list->SetPipelineState(sh_pipeline.Get());
        command_list->SetComputeRootSignature(sh_sigature_.Get());
        command_list->SetComputeRootDescriptorTable(0, cubemap_->GetGPUSRV());
        command_list->SetComputeRootUnorderedAccessView(1, sh_buffer_->GetGPUAddress());
        command_list->Dispatch(1, 1, 1);
    }
}