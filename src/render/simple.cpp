#include "render/simple.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
namespace feng
{

    void Simple::Build(Renderer &renderer)
    {
        shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\simple.hlsl", nullptr);
        CD3DX12_ROOT_PARAMETER slotRootParameter[3];

        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

        slotRootParameter[1].InitAsConstantBufferView(0);
        slotRootParameter[2].InitAsConstantBufferView(1);

        auto samplers = renderer.GetStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 1, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        signature_ = Shader::CreateRootSignature(renderer.GetDevice().GetDevice(), rootSigDesc);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.InputLayout = StaticMesh::InputLayout();
        psoDesc.pRootSignature = signature_.Get();
        shader->FillPSO(psoDesc);

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        // psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
        // psoDesc
        // psoDesc.RTVFormats[0] = mBackBufferFormat;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        // psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_));
    }

    void Simple::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx)
    {

        command_list->SetPipelineState(pso_.Get());
        RenderWindow &render_window = renderer.GetRenderWindow();
        render_window.SetupCommandList(command_list);

        command_list->SetGraphicsRootSignature(signature_.Get());

        command_list->SetGraphicsRootConstantBufferView(2, renderer.pass_constant_buffer_->operator[] (idx).GetResource()->GetGPUVirtualAddress());

        ConstantBuffer<ObjectConstantBuffer>& object_buffer = renderer.object_constant_buffer_->operator[](idx);
        D3D12_GPU_VIRTUAL_ADDRESS object_buffer_base_address = object_buffer.GetResource()->GetGPUVirtualAddress();

        for(auto it = scene.StaticMeshes.cbegin(); it != scene.StaticMeshes.cend(); it++)
        {
            auto dis = std::distance(scene.StaticMeshes.cbegin(), it);
            StaticMesh* static_mesh = it->get();

            command_list->IASetVertexBuffers(0, 1, &static_mesh->GetVertexBufferView());
            command_list->IASetIndexBuffer(&static_mesh->GetIndexBufferView());
            command_list->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            
            command_list->SetGraphicsRootConstantBufferView(1, object_buffer_base_address + dis * object_buffer.GetSize());

            command_list->SetGraphicsRootDescriptorTable(0, renderer.GetDevice().GetSRVHeap().GetGpuHandle(static_mesh->material_->first_index_));

            command_list->DrawIndexedInstanced(static_mesh->mesh_->index_count_, 1, 0, 0, 0);
        }

       
        
        // renderer.GetDevice().FlushCommand(0);
    }
} // namespace feng