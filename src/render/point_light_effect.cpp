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
        light_info_buffer_ = std::make_unique<ConstantBufferGroup<PointLightBuffer, BACK_BUFFER_SIZE>>(renderer.GetDevice(), scene.PointLights.size());
        {
            auto shader = std::make_unique<GraphicsShaderWithGS>(L"resources\\shaders\\point_light_depth.hlsl");
            CD3DX12_ROOT_PARAMETER slotRootParameter[2];
            slotRootParameter[0].InitAsConstantBufferView(0);
            slotRootParameter[1].InitAsConstantBufferView(1);
            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 1, samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

        for (int i = 0; i < scene.PointLights.size(); ++i)
        {
            t_shadowmaps.emplace_back(new DynamicDepthTextureCube(renderer.GetDevice(), SHADOWMAP_WIDTH, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));
        }
        plight_pass_constant_buffer_.reset(new ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>(renderer.GetDevice(), 6));

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = SHADOWMAP_WIDTH;
        viewport_.Height = SHADOWMAP_WIDTH;
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, SHADOWMAP_WIDTH, SHADOWMAP_WIDTH};

        {
            auto shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\point_light.hlsl");
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
    }

    void PointLightEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        auto &CameraFrutum = scene.Camera->GetBoundingFrustrum();
        command_list->SetPipelineState(shadow_pass_pso_.Get());
        command_list->SetGraphicsRootSignature(shadow_pass_signature_.Get());
        command_list->RSSetViewports(1, &viewport_);
        command_list->RSSetScissorRects(1, &scissor_rect_);
        for (auto it = scene.PointLights.begin(); it != scene.PointLights.end(); ++it)
        {
            auto &pointLight = *(*it);
            if (CameraFrutum.Intersects((*it)->GetBoundingBox()))
            {
                auto index = std::distance(scene.PointLights.begin(), it);

                t_shadowmaps[index]->TransitionState(command_list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
                auto depth_descriptor = t_shadowmaps[index]->GetCPUDSV();
                command_list->OMSetRenderTargets(0, nullptr, TRUE, &depth_descriptor);
                command_list->ClearDepthStencilView(depth_descriptor, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

                auto &pass_buffer = light_info_buffer_->operator[](idx);
                if (!pointLight.IsCBReady(idx))
                {
                    PointLightBuffer buffer;
                    buffer.ViewMatrix[0] = Matrix::CreateLookAt(Vector3::Right, Vector3::Zero, Vector3::Up).Invert().Transpose();
                    buffer.ViewMatrix[1] = Matrix::CreateLookAt(Vector3::Left, Vector3::Zero, Vector3::Up).Invert().Transpose();
                    buffer.ViewMatrix[2] = Matrix::CreateLookAt(Vector3::Up, Vector3::Zero, Vector3::Forward).Invert().Transpose();
                    buffer.ViewMatrix[3] = Matrix::CreateLookAt(Vector3::Down, Vector3::Zero, Vector3::Backward).Invert().Transpose();
                    buffer.ViewMatrix[4] = Matrix::CreateLookAt(Vector3::Backward, Vector3::Zero, Vector3::Up).Invert().Transpose();
                    buffer.ViewMatrix[5] = Matrix::CreateLookAt(Vector3::Forward, Vector3::Zero, Vector3::Up).Invert().Transpose();

                    //这里用的是左手系矩阵, 加上180度的旋转
                    buffer.ProjMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), 1, pointLight.radius_, 0.1);
                    buffer.ProjMatrix = buffer.ProjMatrix.Transpose();
                    buffer.Color = pointLight.color_;
                    buffer.ShadowmapSize = 256;
                    buffer.LightPosition = pointLight.position_;
                    buffer.Radius = pointLight.radius_;
                    pass_buffer.Write(index, buffer);
                }
                command_list->SetGraphicsRootConstantBufferView(1, pass_buffer.GetGPUAddressOf(index));

                auto &BoundingBox = pointLight.GetBoundingBox();
                if (BoundingBox.Intersects(scene.StaticMeshesOctree->GetRootBounds()))
                {
                    for (Octree<Scene::StaticMeshProxy>::NodeIterator NodeIt(*(scene.StaticMeshesOctree)); NodeIt.HasPendingNodes(); NodeIt.Advance())
                    {
                        auto &n = NodeIt.GetCurrentNode();
                        auto &context = NodeIt.GetCurrentContext();
                        for (const auto &ele : n.GetElements())
                        {
                            if (ele.pointer->GetBoundingBox().Intersects(BoundingBox) && BoundingBox.Intersects(ele.pointer->GetBoundingBox()))
                            {
                                renderer.RefreshConstantBuffer(*(ele.pointer), idx, ele.id);
                                command_list->SetGraphicsRootConstantBufferView(0, renderer.object_constant_buffer_->operator[](idx).GetGPUAddressOf(index));
                                ele.pointer->DrawWithCommand(command_list);
                            }
                        }

                        auto intersected = context.GetIntersectingChildren(BoundingBox);
                        for (OctreeChildNodeRef childRef; !childRef.null; childRef.Advance())
                        {
                            if (intersected.Contains(childRef) && n.HasChild(childRef))
                            {
                                NodeIt.PushChild(childRef);
                            }
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