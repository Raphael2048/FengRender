#include "render/tone_mapping.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"

namespace feng
{
    ToneMapping::ToneMapping(Renderer& renderer)
    {
        shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\tone_mapping.hlsl", nullptr);

        CD3DX12_ROOT_PARAMETER slotRootParameter[1];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

        auto samplers = renderer.GetStaticSamplers();
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                                                
        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &signature_));

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.InputLayout = renderer.pp_input_layout_;
        psoDesc.pRootSignature = signature_.Get();
        shader->FillPSO(psoDesc);

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

        // No depth test
        psoDesc.DepthStencilState.DepthEnable = false;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        // Final Output
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_));
    }

    void ToneMapping::Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list, uint8_t idx)
    {
        command_list->SetPipelineState(pso_.Get());
        command_list->RSSetViewports(1, &renderer.viewport_);
        command_list->RSSetScissorRects(1, &renderer.scissor_rect_);

        command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderer.GetRenderWindow().CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
        
        renderer.t_gbuffer_base_color_->TransitionState(command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        
        command_list->OMSetRenderTargets(1, &renderer.GetRenderWindow().CurrentBackBufferView(), FALSE, nullptr);

        command_list->SetGraphicsRootSignature(signature_.Get());

        command_list->SetGraphicsRootDescriptorTable(0, renderer.GetDevice().GetSRVHeap().GetGpuHandle(renderer.t_gbuffer_base_color_->GetSRVHeapIndex()));
        
        command_list->IASetVertexBuffers(0, 1, &renderer.pp_vertex_buffer_view_);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        command_list->DrawInstanced(3, 1, 0, 0);
    }
}