#include "render/point_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"

namespace feng
{
    PointLightEffect::PointLightEffect(Renderer &renderer, const Scene &scene)
    {
        const int SHADOWMAP_WIDTH = 256;
        auto samplers = renderer.GetStaticSamplers();
        {
            auto shader = std::make_unique<GraphicsShaderWithGS>(L"resources\\shaders\\point_light_depth.hlsl");
            CD3DX12_ROOT_PARAMETER slotRootParameter[2];
            slotRootParameter[0].InitAsConstantBufferView(0);
            slotRootParameter[1].InitAsConstantBufferView(1);
            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &shadow_pass_signature_));

            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

            auto input_layput = StaticMesh::InputLayout();
            input_layput.NumElements = 1;
            psoDesc.InputLayout = input_layput;
            psoDesc.pRootSignature = shadow_pass_signature_.Get();
            shader->FillPSO(psoDesc);
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

            auto depth_desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
            depth_desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.DepthStencilState = depth_desc;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.NumRenderTargets = 0;
            psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            psoDesc.SampleDesc.Count = 1;
            psoDesc.SampleDesc.Quality = 0;
            renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&shadow_pass_pso_));
        }
        for (int i = 0; i < 6; ++i)
        {
            t_shadowmaps[i].reset(new DynamicDepthTexture(renderer.GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_WIDTH, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));
        }

        plight_pass_constant_buffer_.reset(
            new ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>(renderer.GetDevice(), 6));

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = SHADOWMAP_WIDTH;
        viewport_.Height = SHADOWMAP_WIDTH;
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, SHADOWMAP_WIDTH, SHADOWMAP_WIDTH};

        light_info_buffer_ = std::make_unique<ConstantBufferGroup<PointLightBuffer, BACK_BUFFER_SIZE>>(renderer.GetDevice(), scene.SpotLights.size());
    }

    void PointLightEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx)
    {
        
    }
    
} // namespace feng