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

                                                
        signature_ = Shader::CreateRootSignature(renderer.GetDevice().GetDevice(), rootSigDesc);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        psoDesc.InputLayout = StaticMesh::InputLayout();
        psoDesc.pRootSignature = signature_.Get();
        shader->FillPSO(psoDesc);

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

        auto depthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

        psoDesc.DepthStencilState = depthDesc;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 3;
        // Base Color
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        // World Normal
        psoDesc.RTVFormats[1] = DXGI_FORMAT_R10G10B10A2_UNORM;
        // Roughness & Metallic
        psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8_UNORM;

        // Depth
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_));
    }

    void ToneMapping::Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list, uint8_t idx)
    {

    }
}