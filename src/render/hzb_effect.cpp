#include "render/hzb_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{
    HZBEffect::HZBEffect(Renderer &renderer)
    {
        auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\hzb_generation.hlsl", nullptr);
        auto samplers = renderer.GetStaticSamplers();

        CD3DX12_ROOT_PARAMETER slotRootParameter[3];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

        CD3DX12_DESCRIPTOR_RANGE cbvTable2;
        cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0);
        slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);
        slotRootParameter[2].InitAsConstants(2, 0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_));

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
        shader->FillPSO(psoDesc);
        psoDesc.pRootSignature = signature_.Get();
        TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
        // 预先分配好连续地址的UAV
        for (UINT i = 0; i < 10; ++i)
        {
            renderer.t_hzb_->GetGPUUAVAt(i);
        }
    }

    void HZBEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_.Get());
        command_list->SetComputeRootSignature(signature_.Get());

        // 生成 Mips 0~4
        renderer.t_hzb_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, renderer.t_depth_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, renderer.t_hzb_->GetGPUUAVAt(0));
        Vector2 convert = {1.0f / 1024, 1.0f / 512};
        command_list->SetComputeRoot32BitConstants(2, 2, &convert, 0);
        command_list->Dispatch(64, 32, 1);

        // 生成 Mips 5~9
        // 将Mip4转换为SRV
        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
            renderer.t_hzb_->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 4);
        command_list->ResourceBarrier(1, &transition);
        command_list->SetComputeRootDescriptorTable(0, renderer.t_hzb_->GetGPUSRVAt(4));
        command_list->SetComputeRootDescriptorTable(1, renderer.t_hzb_->GetGPUUAVAt(5));
        convert = Vector2{1.0f / 32, 1.0f / 16};
        command_list->SetComputeRoot32BitConstants(2, 2, &convert, 0);
        command_list->Dispatch(2, 1, 1);
        auto transition2 = CD3DX12_RESOURCE_BARRIER::Transition(
            renderer.t_hzb_->GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 4);
        command_list->ResourceBarrier(1, &transition2);
    }
} // namespace feng