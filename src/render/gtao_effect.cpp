#include "render/gtao_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{
    struct GTAOBuffer
    {
        UINT width;
        UINT height;
        Vector2 InvSize;
        float InvRadiusSq;
    };
    GTAOEffect::GTAOEffect(Renderer &renderer)
    {
        auto samplers = renderer.GetStaticSamplers();
        CD3DX12_ROOT_PARAMETER slotRootParameter[5];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

        CD3DX12_DESCRIPTOR_RANGE cbvTable2;
        cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

        slotRootParameter[2].InitAsConstants(5, 1);
        slotRootParameter[3].InitAsConstantBufferView(0);

        CD3DX12_DESCRIPTOR_RANGE cbvTable3;
        cbvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        slotRootParameter[4].InitAsDescriptorTable(1, &cbvTable3);
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_));

        {
            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\gtao.hlsl");
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
        }
        {
            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\gtao_spatial_filter.hlsl");
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso2_)));
        }

        ao_filter_.reset(new DynamicPlainTexture(renderer.GetDevice(), renderer.width_, renderer.height_, DXGI_FORMAT_R8_UNORM, false, true));
    }

    void GTAOEffect::Draw(Renderer &renderer, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_.Get());
        command_list->SetComputeRootSignature(signature_.Get());
        ao_filter_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, renderer.t_depth_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, renderer.t_gbuffer_normal->GetGPUSRV());
        GTAOBuffer buffer = {
            renderer.width_ ,
            renderer.height_ ,
            Vector2(1.0f / (renderer.width_), 1.0f / (renderer.height_)),
            1.0f / (10 * 10)};
        command_list->SetComputeRoot32BitConstants(2, 5, &buffer, 0);
        command_list->SetComputeRootConstantBufferView(3, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        command_list->SetComputeRootDescriptorTable(4, ao_filter_->GetGPUUAV());
        command_list->Dispatch((renderer.width_ + 7) / 8, (renderer.height_ + 7) / 8, 1);
        
        command_list->SetPipelineState(pso2_.Get());
        ao_filter_->TransitionState(command_list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        renderer.t_ao_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(1, ao_filter_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(4, renderer.t_ao_->GetGPUUAV());
        command_list->Dispatch((renderer.width_ + 15) / 16, (renderer.height_ + 7) / 8, 1);

        renderer.t_ao_->TransitionState(command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);


    }
} // namespace feng