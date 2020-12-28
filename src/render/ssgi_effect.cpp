#include "render/ssgi_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{

    SSGIEffect::SSGIEffect(Renderer &renderer)
    {
        {
            auto samplers = renderer.GetStaticSamplers();

            CD3DX12_ROOT_PARAMETER slotRootParameter[6];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            CD3DX12_DESCRIPTOR_RANGE cbvTable3;
            cbvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
            slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable3);

            CD3DX12_DESCRIPTOR_RANGE cbvTable4;
            cbvTable4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            slotRootParameter[3].InitAsDescriptorTable(1, &cbvTable4);
            
            slotRootParameter[4].InitAsConstantBufferView(0);

            slotRootParameter[5].InitAsConstants(8, 1);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_));

            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\ssr.hlsl", nullptr);
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
        }
        t_ssr_.reset(new DynamicPlainTexture(renderer.GetDevice(), 1024, 512, DXGI_FORMAT_R32G32B32A32_FLOAT, false, true));
    }

    void SSGIEffect::Draw(Renderer &renderer, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_.Get());
        command_list->SetComputeRootSignature(signature_.Get());

        command_list->SetComputeRootDescriptorTable(0, renderer.t_gbuffer_base_color_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, renderer.t_hzb_->GetGPUSRV());
        renderer.t_color_output_->TransitionState(command_list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        command_list->SetComputeRootDescriptorTable(2, renderer.t_color_output_->GetGPUSRV());
        t_ssr_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(3, t_ssr_->GetGPUUAV());
        command_list->SetComputeRootConstantBufferView(4, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        // command_list->SetComputeRoot32BitConstants(5, 8, &info_, 0);
        command_list->Dispatch(1024 / 8, 512 / 8, 1);

        renderer.blit_->AccumulateTo(renderer, command_list, t_ssr_.get(), renderer.t_color_output_.get());
    }
} // namespace feng