#include "render/depth_only.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
namespace feng
{
    void DepthOnly::Build(Renderer& renderer)
    {
        shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\depth_only.hlsl", nullptr);
        CD3DX12_ROOT_PARAMETER slotRootParameter[2];

        slotRootParameter[0].InitAsConstantBufferView(0);
        slotRootParameter[1].InitAsConstantBufferView(1);

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr,
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        signature_ = std::make_unique<RootSignature>(renderer.GetDevice(), rootSigDesc);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        auto input_layput = StaticMesh::InputLayout();
        input_layput.NumElements = 1;

        // Just need pos in depth only pass
        psoDesc.InputLayout = input_layput;
        psoDesc.pRootSignature = signature_->GetRootSignature();
        shader->FillPSO(psoDesc);

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

        // REVERSE-Z
        auto depth_desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        depth_desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
        psoDesc.DepthStencilState = depth_desc;

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // Only Depth
        psoDesc.NumRenderTargets = 0;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_));
    }

    void DepthOnly::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx)
    {

        command_list->SetPipelineState(pso_.Get());
        command_list->RSSetViewports(1, &renderer.viewport_);
        command_list->RSSetScissorRects(1, &renderer.scissor_rect_);

        renderer.t_depth_->TransitionState(command_list, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        auto depth_descriptor = renderer.GetDevice().GetDSVHeap().GetCpuHandle(renderer.t_depth_->GetDSVHeapIndex());
        command_list->ClearDepthStencilView( depth_descriptor
            , D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
        command_list->OMSetRenderTargets(0, nullptr, TRUE, &depth_descriptor);

        command_list->SetGraphicsRootSignature(signature_->GetRootSignature());

        command_list->SetGraphicsRootConstantBufferView(1, renderer.pass_constant_buffer_->operator[] (idx).GetResource()->GetGPUVirtualAddress());

        ConstantBuffer<ObjectConstantBuffer>& object_buffer = renderer.object_constant_buffer_->operator[](idx);
        D3D12_GPU_VIRTUAL_ADDRESS object_buffer_base_address = object_buffer.GetResource()->GetGPUVirtualAddress();

        for(auto it = scene.StaticMeshes.cbegin(); it != scene.StaticMeshes.cend(); it++)
        {
            auto dis = std::distance(scene.StaticMeshes.cbegin(), it);
            StaticMesh* static_mesh = it->get();

            command_list->IASetVertexBuffers(0, 1, &static_mesh->GetVertexBufferView());
            command_list->IASetIndexBuffer(&static_mesh->GetIndexBufferView());
            command_list->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            
            command_list->SetGraphicsRootConstantBufferView(0, object_buffer_base_address + dis * object_buffer.GetSize());

            command_list->DrawIndexedInstanced(static_mesh->mesh_->index_count_, 1, 0, 0, 0);
        }
    }
}