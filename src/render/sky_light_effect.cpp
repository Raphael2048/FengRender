#include "render/sky_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"

namespace feng
{
    SkyLightEffect::SkyLightEffect(std::shared_ptr<StaticTexture> texture, Renderer &renderer, ID3D12GraphicsCommandList *command_list)
        : cubemap_(texture)
    {
        auto samplers = renderer.GetStaticSamplers();
        {
            auto shader = std::make_unique<ComputeShader>(L"resources\\shaders\\sky_light_sh.hlsl");
            sh_buffer_.reset(new UAVBuffer(renderer.GetDevice().GetDevice(), 3, sizeof(Vector4) * 3));
            CD3DX12_ROOT_PARAMETER slotRootParameter[2];
            CD3DX12_DESCRIPTOR_RANGE cbvTable;
            cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
            slotRootParameter[1].InitAsUnorderedAccessView(0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 1, samplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &sh_sigature_));

            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
            shader->FillPSO(psoDesc);
            psoDesc.pRootSignature = sh_sigature_.Get();
            renderer.GetDevice().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&sh_pipeline));

            auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            command_list->ResourceBarrier(1, &transition);
            command_list->SetPipelineState(sh_pipeline.Get());
            command_list->SetComputeRootSignature(sh_sigature_.Get());
            command_list->SetComputeRootDescriptorTable(0, cubemap_->GetGPUSRV());
            command_list->SetComputeRootUnorderedAccessView(1, sh_buffer_->GetGPUAddress());
            command_list->Dispatch(1, 1, 1);
            //把UAV转成 SRV了
            auto transition2 = CD3DX12_RESOURCE_BARRIER::Transition(sh_buffer_->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            command_list->ResourceBarrier(1, &transition2);
        }

        {
            auto light_shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\sky_light_specular.hlsl");
            CD3DX12_ROOT_PARAMETER slotRootParameter[3];
            CD3DX12_DESCRIPTOR_RANGE lightTable;
            lightTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
            slotRootParameter[0].InitAsConstants(16, 0); //逆矩阵
            slotRootParameter[1].InitAsConstants(2, 1); // Roughness
            slotRootParameter[2].InitAsDescriptorTable(1, &lightTable); //CUBEMAP
            CD3DX12_ROOT_SIGNATURE_DESC light_root_desc(
                3, slotRootParameter, 1, samplers.data(),
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &light_root_desc, &specular_sigature_));
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
            psoDesc.InputLayout = renderer.pp_input_layout_;
            psoDesc.pRootSignature = specular_sigature_.Get();
            light_shader->FillPSO(psoDesc);
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.DepthStencilState.DepthEnable = false;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            psoDesc.SampleDesc.Count = 1;
            psoDesc.SampleDesc.Quality = 0;
            renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&specular_pipeline_));

            UINT64 texture_size = 128;
            D3D12_RESOURCE_DESC texDesc;
            ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
            texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            texDesc.Alignment = 0;
            texDesc.Width = texture_size;
            texDesc.Height = texture_size;
            texDesc.DepthOrArraySize = 6;
            texDesc.MipLevels = 5; // 128 >> 1
            texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            TRY(renderer.GetDevice().GetDevice()->CreateCommittedResource(
                &HeapProp,
                D3D12_HEAP_FLAG_NONE,
                &texDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&specular_resource_)));

            auto srv_handle = texture->GetGPUSRV();
            for (int mip = 0; mip < 5; ++mip)
            {
                for(int face = 0; face < 6; ++face)
                {

                }
            }
        }

        {
            auto light_shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\sky_light_evaluate.hlsl");
            CD3DX12_ROOT_PARAMETER slotRootParameter[4];
            CD3DX12_DESCRIPTOR_RANGE lightTable;
            lightTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
            slotRootParameter[0].InitAsDescriptorTable(1, &lightTable);
            slotRootParameter[1].InitAsConstants(1, 0);
            slotRootParameter[2].InitAsShaderResourceView(4, 0);
            slotRootParameter[3].InitAsConstantBufferView(1);
            CD3DX12_ROOT_SIGNATURE_DESC light_root_desc(
                4, slotRootParameter, 1, samplers.data(),
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &light_root_desc, &light_signature_));
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
            ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
            psoDesc.InputLayout = renderer.pp_input_layout_;
            psoDesc.pRootSignature = light_signature_.Get();
            light_shader->FillPSO(psoDesc);
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
            psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            psoDesc.DepthStencilState.DepthEnable = false;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            psoDesc.SampleDesc.Count = 1;
            psoDesc.SampleDesc.Quality = 0;
            renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&light_pipeline_));
        }
    }

    void SkyLightEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(light_pipeline_.Get());
        command_list->SetGraphicsRootSignature(light_signature_.Get());
        command_list->RSSetViewports(1, &renderer.viewport_);
        command_list->RSSetScissorRects(1, &renderer.scissor_rect_);
        auto color_rtv = renderer.t_color_output_->GetCPURTV();
        command_list->OMSetRenderTargets(1, &color_rtv, FALSE, nullptr);
        command_list->SetGraphicsRootDescriptorTable(0, renderer.t_gbuffer_base_color_->GetGPUSRV());
        command_list->SetGraphicsRoot32BitConstants(1, 1, &scene.SkyLight->intensity_, 0);
        command_list->SetGraphicsRootShaderResourceView(2, sh_buffer_->GetGPUAddress());
        command_list->SetGraphicsRootConstantBufferView(3, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        command_list->IASetVertexBuffers(0, 1, &renderer.pp_vertex_buffer_view_);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->DrawInstanced(3, 1, 0, 0);
    }
} // namespace feng