#include "render/volume_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"

namespace feng
{
    VolumeEffect::VolumeEffect(Renderer& renderer)
    {
        texture_density_.reset(new DynamicPlain3DTexture(renderer.GetDevice(), 160, 96, 64, DXGI_FORMAT_R16G16B16A16_FLOAT));
        texture_accumulate_.reset(new DynamicPlain3DTexture(renderer.GetDevice(), 160, 96, 64, DXGI_FORMAT_R16G16B16A16_FLOAT));
        inv_size_ = Vector3(1.0f / 160, 1.0f / 96, 1.0f / 64);
        auto samplers = renderer.GetStaticSamplers();
        {
            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\volume_density.hlsl");

            CD3DX12_ROOT_PARAMETER slotRootParameter[6];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            slotRootParameter[2].InitAsConstantBufferView(0);

            slotRootParameter[3].InitAsConstantBufferView(1);

            slotRootParameter[4].InitAsConstants(4, 2);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter, 2, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_density_));
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_density_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_density_)));
        }
        {
            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\volume_accumulate.hlsl");
            CD3DX12_ROOT_PARAMETER slotRootParameter[6];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            slotRootParameter[2].InitAsConstantBufferView(0);

            slotRootParameter[3].InitAsConstants(3, 1);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_accumulate_));
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = signature_accumulate_.Get();
            TRY(renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso_accumulate_)));
        }

        {
            auto shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\volume_integration.hlsl");
            CD3DX12_ROOT_PARAMETER slotRootParameter[3];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

            CD3DX12_DESCRIPTOR_RANGE cbvTable2;
            cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
            slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2);

            slotRootParameter[2].InitAsConstants(3, 0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_integration_));
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
            psoDesc.InputLayout = renderer.pp_input_layout_;
            psoDesc.pRootSignature = signature_integration_.Get();
            shader->FillPSO(psoDesc);
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
            psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_ALPHA;
            psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
            psoDesc.DepthStencilState.DepthEnable = false;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            psoDesc.SampleDesc.Count = 1;
            psoDesc.SampleDesc.Quality = 0;
            renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_integration_));
        }
    }

    void VolumeEffect::Draw(Renderer &renderer, DirectionalLightEffect& dlight, ID3D12GraphicsCommandList* command_list, uint8_t idx)
    {
        texture_density_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetPipelineState(pso_density_.Get());
        command_list->SetComputeRootSignature(signature_density_.Get());
        command_list->SetComputeRootDescriptorTable(0, dlight.t_shadow_split[0]->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, texture_density_->GetGPUUAV());
        command_list->SetComputeRootConstantBufferView(2, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        command_list->SetComputeRootConstantBufferView(3, dlight.light_info_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        command_list->SetComputeRoot32BitConstants(4, 3, &inv_size_, 0);
        static float time = 0;
        time += 0.1f;

        command_list->SetComputeRoot32BitConstants(4, 1, &time, 3);
        command_list->Dispatch(160 / 8, 96 / 8, 64);

        command_list->SetPipelineState(pso_accumulate_.Get());
        command_list->SetComputeRootSignature(signature_accumulate_.Get());
        texture_density_->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);
        texture_accumulate_->TransitionState(command_list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        command_list->SetComputeRootDescriptorTable(0, texture_density_->GetGPUSRV());
        command_list->SetComputeRootDescriptorTable(1, texture_accumulate_->GetGPUUAV());
        command_list->SetComputeRootConstantBufferView(2, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        command_list->SetComputeRoot32BitConstants(3, 3, &inv_size_, 0);
        command_list->Dispatch(160 / 8, 96 / 8, 1);

        command_list->SetPipelineState(pso_integration_.Get());
        command_list->SetGraphicsRootSignature(signature_integration_.Get());
        texture_accumulate_->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);
        command_list->SetGraphicsRootDescriptorTable(0, texture_accumulate_->GetGPUSRV());
        command_list->SetGraphicsRootDescriptorTable(1, renderer.t_depth_->GetGPUSRV());
        command_list->SetGraphicsRoot32BitConstants(2, 3, &inv_size_, 0);
        command_list->RSSetViewports(1, &renderer.viewport_);
        command_list->RSSetScissorRects(1, &renderer.scissor_rect_);
        auto color_rtv = renderer.t_color_output_->GetCPURTV();
        command_list->OMSetRenderTargets(1, &color_rtv, FALSE, nullptr);
        command_list->IASetVertexBuffers(0, 1, &renderer.pp_vertex_buffer_view_);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->DrawInstanced(3, 1, 0, 0);

    }
}