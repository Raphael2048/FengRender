#include "render/ssgi_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{

    SSGIEffect::SSGIEffect(Renderer &renderer)
    {
        auto samplers = renderer.GetStaticSamplers();
        {
            CD3DX12_ROOT_PARAMETER slotRootParameter[3];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            slotRootParameter[2].InitAsConstants(3, 0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_reduction_));

            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\ssgi_reduction.hlsl", nullptr);
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_reduction_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_reduction_)));
        }
        t_reduction_.reset(new DynamicPlainTextureMips(renderer.GetDevice(), renderer.width_ / 2, renderer.height_ / 2, 4, DXGI_FORMAT_R32G32B32A32_FLOAT, false, true));
        for(int i = 0; i < 4; i++)
        {
            t_reduction_->GetGPUUAVAt(i);
        }
    }

    void SSGIEffect::Draw(Renderer &renderer, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_reduction_.Get());
        command_list->SetComputeRootSignature(signature_reduction_.Get());

        t_reduction_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        renderer.t_color_output_->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);

        command_list->SetComputeRootDescriptorTable(0, renderer.t_color_output_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, t_reduction_->GetGPUUAVAt(0));

        command_list->Dispatch(renderer.width_ / 16, renderer.height_ / 16, 1);
    }
} // namespace feng