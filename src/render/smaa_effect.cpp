#include "render/smaa_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{

    SMAAEffect::SMAAEffect(Renderer &renderer)
    {
        auto samplers = renderer.GetStaticSamplers();
        t_edge_.reset(new DynamicPlainTexture(renderer.GetDevice(), renderer.width_, renderer.height_, DXGI_FORMAT_R8G8_UNORM, true, true));
        t_weight_.reset(new DynamicPlainTexture(renderer.GetDevice(), renderer.width_, renderer.height_, DXGI_FORMAT_R8G8B8A8_UNORM, true, true));
       
        {
            CD3DX12_ROOT_PARAMETER slotRootParameter[2];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_edge_));

            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\smaa_edge_detection.hlsl", nullptr);
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_edge_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_edge_)));
        }

        {
            CD3DX12_ROOT_PARAMETER slotRootParameter[3];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            slotRootParameter[2].InitAsConstants(4, 0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_weight_));

            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\smaa_blending_weights.hlsl", nullptr);
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_weight_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_weight_)));
        }
     
        {
            CD3DX12_ROOT_PARAMETER slotRootParameter[4];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            CD3DX12_DESCRIPTOR_RANGE cbvTable3;
            cbvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable3);

            slotRootParameter[3].InitAsConstants(4, 0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_blend_));

            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\smaa_neighborhood_blending.hlsl", nullptr);
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_blend_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_blend_)));
        }
    }

    void SMAAEffect::Draw(Renderer &renderer, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_edge_.Get());
        command_list->SetComputeRootSignature(signature_edge_.Get());
        renderer.t_color_output_->TransitionState(command_list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        t_edge_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, renderer.t_color_output_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, t_edge_->GetGPUUAV());
        command_list->Dispatch(renderer.width_ / 16, renderer.height_ / 8, 1);

        command_list->SetPipelineState(pso_weight_.Get());
        command_list->SetComputeRootSignature(signature_weight_.Get());
        t_edge_->TransitionState(command_list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        t_weight_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, t_edge_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, t_weight_->GetGPUUAV());
        command_list->SetComputeRoot32BitConstants(2, 4, &renderer.screen_buffer_, 0);
        command_list->Dispatch(renderer.width_ / 8, renderer.height_ / 8, 1);

        command_list->SetPipelineState(pso_blend_.Get());
        command_list->SetComputeRootSignature(signature_blend_.Get());
        t_weight_->TransitionState(command_list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        renderer.t_color_output2_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, renderer.t_color_output_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, t_weight_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(2, renderer.t_color_output2_->GetGPUUAV());
        command_list->SetComputeRoot32BitConstants(3, 4, &renderer.screen_buffer_, 0);
        command_list->Dispatch(renderer.width_ / 8, renderer.height_ / 8, 1);
    }
} // namespace feng