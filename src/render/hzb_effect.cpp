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

        CD3DX12_ROOT_PARAMETER slotRootParameter[2];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
        CD3DX12_DESCRIPTOR_RANGE cbvTable2;
        cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0);
        slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_));

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
        shader->FillPSO(psoDesc);
        psoDesc.pRootSignature = signature_.Get();
        renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_));
    }

    void HZBEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_.Get());
        command_list->RSSetViewports(1, &renderer.viewport_);
        command_list->RSSetScissorRects(1, &renderer.scissor_rect_);

        renderer.t_depth_->TransitionState(command_list, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        auto depth_descriptor = renderer.t_depth_->GetCPUDSV();

        command_list->OMSetRenderTargets(0, nullptr, TRUE, &depth_descriptor);
        command_list->ClearDepthStencilView(depth_descriptor, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

        command_list->SetGraphicsRootSignature(signature_.Get());

        command_list->SetGraphicsRootConstantBufferView(1, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());

        ConstantBuffer<ObjectConstantBuffer> &object_buffer = renderer.object_constant_buffer_->operator[](idx);
        D3D12_GPU_VIRTUAL_ADDRESS object_buffer_base_address = object_buffer.GetResource()->GetGPUVirtualAddress();

        for (auto it = scene.StaticMeshes.cbegin(); it != scene.StaticMeshes.cend(); it++)
        {
            auto dis = std::distance(scene.StaticMeshes.cbegin(), it);
            if (scene.StaticMeshesVisibity[dis])
            {
                command_list->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                command_list->SetGraphicsRootConstantBufferView(0, object_buffer_base_address + dis * object_buffer.GetSize());
                it->get()->DrawWithCommand(command_list);
            }
        }
    }
} // namespace feng