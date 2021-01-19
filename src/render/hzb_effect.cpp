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
        slotRootParameter[2].InitAsConstants(36, 0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_));

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
        shader->FillPSO(psoDesc);
        psoDesc.pRootSignature = signature_.Get();
        TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));

        uint32_t posx = 0, posy = 0;
        uint32_t sizex = renderer.width_;
        uint32_t sizey = renderer.height_;
        bool dir_v = true;
        renderer.hzb_size_pos_.clear();
        for(int i = 0; i < 9; i++)
        {
            renderer.hzb_size_pos_.emplace_back(posx);
            renderer.hzb_size_pos_.emplace_back(posy);
            renderer.hzb_size_pos_.emplace_back(sizex);
            renderer.hzb_size_pos_.emplace_back(sizey);
            if (dir_v) posy += sizey + 2;
            else posx += sizex + 2;
            dir_v = !dir_v;
            sizex = (sizex + 1) >> 1;
            sizey = (sizey + 1) >> 1;
        }
    }

    void HZBEffect::Draw(Renderer &renderer, ID3D12GraphicsCommandList *command_list)
    {
        command_list->SetPipelineState(pso_.Get());
        command_list->SetComputeRootSignature(signature_.Get());

        // 生成 Mips 0~8
        renderer.t_hzb_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, renderer.t_depth_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, renderer.t_hzb_->GetGPUUAV());
        command_list->SetComputeRoot32BitConstants(2, 36, renderer.hzb_size_pos_.data(), 0);
        command_list->Dispatch(80, 45, 1);

        renderer.t_hzb_->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
} // namespace feng