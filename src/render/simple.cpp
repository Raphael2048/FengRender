#include "render/simple.hpp"
#include "renderer.hpp"
namespace feng
{

    void Simple::Build(Renderer &renderer)
    {
        shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\simple.hlsl", nullptr);
        CD3DX12_ROOT_PARAMETER slotRootParameter[1];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        signature_ = std::make_unique<RootSignature>(renderer.GetDevice(), rootSigDesc);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.InputLayout = StaticMesh::InputLayout();
        psoDesc.pRootSignature = signature_->GetRootSignature();
        shader->FillPSO(psoDesc);

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        // psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        // psoDesc
        // psoDesc.RTVFormats[0] = mBackBufferFormat;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        // psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        pso_ = std::make_unique<PipelineState>(renderer.GetDevice(), psoDesc);
    }

    void Simple::Draw(Renderer &renderer, const Scene &scene)
    {
        auto command_list = renderer.GetDevice().BeginCommand(0, pso_->GetPipelineState());
        RenderWindow &render_window = renderer.GetRenderWindow();
        render_window.SetupCommandList(command_list);

        ID3D12DescriptorHeap *heaps[] = {renderer.GetDevice().GetCBVHeap()};
        command_list->SetDescriptorHeaps(1, heaps);

        command_list->SetGraphicsRootSignature(signature_->GetRootSignature());

        for(auto& static_mesh:scene.StaticMeshes)
        {
            command_list->IASetVertexBuffers(0, 1, &static_mesh->GetVertexBufferView());
            command_list->IASetIndexBuffer(&static_mesh->GetIndexBufferView());
            command_list->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            command_list->SetGraphicsRootDescriptorTable(0, renderer.GetDevice().GetCBVHeap()->GetGPUDescriptorHandleForHeapStart());

            command_list->DrawIndexedInstanced(static_mesh->index_count_, 1, 0, 0, 0);
            // ID3D12DescriptorHeap* descriptorHeaps[] = { cbvHeap}
        }

        command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_window.CurrentBackBuffer(),
                                                                               D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
        renderer.GetDevice().EndCommand();
        render_window.Swap();
        renderer.GetDevice().FlushCommand(0);
        // command_list->RSSetViewports(1, )
    }
} // namespace feng