#include "render/spot_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{
    SpotLightEffect::SpotLightEffect(DepthOnly &depthEffect, Renderer &renderer, const Scene &scene)
    {
        // For Shadow
        shadow_pass_signature_ = depthEffect.signature_;
        shadow_pass_pso_ = depthEffect.pso_;

        for (ptrdiff_t i = 0; i < scene.SpotLights.size(); i++)
        {
            t_shadowmaps.emplace_back(
                new DynamicDepthTexture(renderer.GetDevice(), 512, 512, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));
        }

        slight_pass_constant_buffer_ = std::make_unique<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>>(renderer.GetDevice(), scene.SpotLights.size());

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = 512;
        viewport_.Height = 512;
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, 512, 512};

        // For Lighting
        light_info_buffer_ = std::make_unique<ConstantBufferGroup<SpotLightBuffer, BACK_BUFFER_SIZE>>(renderer.GetDevice(), scene.SpotLights.size());

        auto shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\spot_light.hlsl", nullptr);
        CD3DX12_ROOT_PARAMETER slotRootParameter[4];

        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable, D3D12_SHADER_VISIBILITY_PIXEL);
        CD3DX12_DESCRIPTOR_RANGE cbvTable2;
        cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
        slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable2, D3D12_SHADER_VISIBILITY_PIXEL);
        slotRootParameter[2].InitAsConstantBufferView(0);
        slotRootParameter[3].InitAsConstantBufferView(1);

        auto samplers = renderer.GetStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, 2, samplers.data(),
                                                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        TRY(DirectX::CreateRootSignature(renderer.GetDevice().GetDevice(), &rootSigDesc, &light_pass_signature_));

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
        psoDesc.InputLayout = renderer.pp_input_layout_;
        psoDesc.pRootSignature = light_pass_signature_.Get();
        shader->FillPSO(psoDesc);
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
        renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&light_pass_pso_));
    }

    void SpotLightEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        auto &CameraFrutum = scene.Camera->GetBoundingFrustrum();
        for (auto it = scene.SpotLights.begin(); it != scene.SpotLights.end(); ++it)
        {
            auto &spotLight = *(*it);
            if (CameraFrutum.Intersects((*it)->GetBoundingFrustum()))
            {
                auto index = std::distance(scene.SpotLights.begin(), it);

                command_list->SetPipelineState(shadow_pass_pso_.Get());
                command_list->RSSetViewports(1, &viewport_);
                command_list->RSSetScissorRects(1, &scissor_rect_);
                
                std::vector<bool> visibile;
                visibile.resize(scene.StaticMeshes.size());
                
                t_shadowmaps[index]->TransitionState(command_list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
                auto depth_descriptor = t_shadowmaps[index]->GetCPUDSV();
                command_list->OMSetRenderTargets(0, nullptr, TRUE, &depth_descriptor);
                command_list->ClearDepthStencilView(depth_descriptor, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
                auto &WorldSpaceBoundingBox = spotLight.GetBoundingBox();
                auto &WorldSpaceBoundingFrustum = spotLight.GetBoundingFrustum();
                if (WorldSpaceBoundingBox.Intersects(scene.StaticMeshesOctree->GetRootBounds()))
                {
                    for (Octree<Scene::StaticMeshProxy>::NodeIterator NodeIt(*(scene.StaticMeshesOctree)); NodeIt.HasPendingNodes(); NodeIt.Advance())
                    {
                        auto &n = NodeIt.GetCurrentNode();
                        auto &context = NodeIt.GetCurrentContext();
                        for (const auto &ele : n.GetElements())
                        {
                            if (ele.pointer->GetBoundingBox().Intersects(WorldSpaceBoundingBox) 
                                && WorldSpaceBoundingFrustum.Intersects(ele.pointer->GetBoundingBox())
                                )
                            {
                                visibile[ele.id] = true;
                                renderer.RefreshConstantBuffer(*(ele.pointer), idx, ele.id);
                            }
                        }

                        auto intersected = context.GetIntersectingChildren(WorldSpaceBoundingBox);
                        for (OctreeChildNodeRef childRef; !childRef.null; childRef.Advance())
                        {
                            if (intersected.Contains(childRef) && n.HasChild(childRef))
                            {
                                NodeIt.PushChild(childRef);
                            }
                        }
                    }

                    auto &pass_buffer = slight_pass_constant_buffer_->operator[](idx);
                    if (!spotLight.IsCBReady(idx))
                    {
                        PassConstantBuffer constant;
                        constant.InvView = spotLight.MatrixWorld.Transpose();
                        constant.View = spotLight.MatrixInvWorld.Transpose();
                        constant.Proj = spotLight.MatrixProj.Transpose();
                        constant.InvProj = spotLight.MatrixInvProj.Transpose();
                        constant.ViewProj = constant.Proj * constant.View;
                        constant.InvViewProj = constant.InvView * constant.InvProj;
                        pass_buffer.Write(index, constant);

                        SpotLightBuffer slightbuffer;
                        slightbuffer.ShadowMatrix = constant.ViewProj;
                        slightbuffer.LightPosition = spotLight.position_;
                        slightbuffer.Radius = spotLight.radius_;
                        slightbuffer.LightDirection = spotLight.MatrixWorld.Forward();
                        slightbuffer.ShadowMapSize = 512.0f;
                        slightbuffer.Color = spotLight.color_;
                        slightbuffer.InnerFalloff = cos(DirectX::XMConvertToRadians(spotLight.inner_angle_));
                        slightbuffer.OuterFalloff = cos(DirectX::XMConvertToRadians(spotLight.outer_angle_));
                        light_info_buffer_->operator[](idx).Write(index, slightbuffer);
                    }
                    command_list->SetGraphicsRootSignature(shadow_pass_signature_.Get());
                    command_list->SetGraphicsRootConstantBufferView(1, pass_buffer.GetGPUAddressOf(index));

                    ConstantBuffer<ObjectConstantBuffer> &object_buffer = renderer.object_constant_buffer_->operator[](idx);
                    D3D12_GPU_VIRTUAL_ADDRESS object_buffer_base_address = object_buffer.GetResource()->GetGPUVirtualAddress();
                    for (auto it = scene.StaticMeshes.cbegin(); it != scene.StaticMeshes.cend(); it++)
                    {
                        auto dis = std::distance(scene.StaticMeshes.cbegin(), it);
                        if (visibile[dis])
                        {
                            command_list->SetGraphicsRootConstantBufferView(0, object_buffer_base_address + dis * object_buffer.GetSize());
                            it->get()->DrawWithCommand(command_list);
                        }
                    }
                }

                command_list->SetPipelineState(light_pass_pso_.Get());
                command_list->RSSetViewports(1, &renderer.viewport_);
                command_list->RSSetScissorRects(1, &renderer.scissor_rect_);
                command_list->SetGraphicsRootSignature(light_pass_signature_.Get());

                t_shadowmaps[index]->TransitionState(command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                auto color_rtv = renderer.t_color_output_->GetCPURTV();
                command_list->OMSetRenderTargets(1, &color_rtv, FALSE, nullptr);

                command_list->SetGraphicsRootDescriptorTable(0, renderer.t_gbuffer_base_color_->GetGPUSRV());
                command_list->SetGraphicsRootDescriptorTable(1, t_shadowmaps[index]->GetGPUSRV());
                command_list->SetGraphicsRootConstantBufferView(2, light_info_buffer_->operator[](idx).GetGPUAddressOf(index));
                command_list->SetGraphicsRootConstantBufferView(3, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());

                command_list->IASetVertexBuffers(0, 1, &renderer.pp_vertex_buffer_view_);
                command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                command_list->DrawInstanced(3, 1, 0, 0);
            }
        }
    }
} // namespace feng