#include "render/directional_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{
    DirectionalLightEffect::DirectionalLightEffect(DepthOnly &depthEffect, Renderer &renderer, const Scene &scene)
    {
        // For Shadow
        shadow_pass_signature_ = depthEffect.signature_;
        shadow_pass_pso_ = depthEffect.pso_;

        // 这里GPU地址是连续的, 直接用range表示
        t_shadow_split[0].reset(new DynamicDepthTexture(renderer.GetDevice(), 2048, 2048, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));
        t_shadow_split[1].reset(new DynamicDepthTexture(renderer.GetDevice(), 2048, 2048, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));
        t_shadow_split[2].reset(new DynamicDepthTexture(renderer.GetDevice(), 2048, 2048, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));

        auto num = scene.StaticMeshes.size();
        shadow_visibity_split[0].resize(num);
        shadow_visibity_split[1].resize(num);
        shadow_visibity_split[2].resize(num);

        dlight_pass_constant_buffer_ = std::make_unique<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>>(renderer.GetDevice(), 3);

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = 2048;
        viewport_.Height = 2048;
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, 2048, 2048};

        // For Lighting
        light_info_buffer_ = std::make_unique<ConstantBufferGroup<DirectionalLightBuffer, BACK_BUFFER_SIZE>>(renderer.GetDevice(), 1);

        auto shader = std::make_unique<GraphicsShader>(L"resources\\shaders\\directional_light.hlsl", nullptr);
        CD3DX12_ROOT_PARAMETER slotRootParameter[4];

        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable, D3D12_SHADER_VISIBILITY_PIXEL);
        CD3DX12_DESCRIPTOR_RANGE cbvTable2;
        cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 4);
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
        psoDesc.DepthStencilState.DepthEnable = false;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        renderer.GetDevice().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&light_pass_pso_));
    }

    void DirectionalLightEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        auto &dlight = *scene.DirectionalLight;
        auto &camera = *scene.Camera;

        float near0 = camera.near_;
        float far0 = camera.near_ * pow(camera.far_ / camera.near_, 1.0f / 3.0f) * 0.5f + ((camera.far_ - camera.near_) / 3.0f + camera.near_) * 0.5f;
        float near1 = far0;
        float far1 = camera.near_ * pow(camera.far_ / camera.near_, 2.0f / 3.0f) * 0.5f + (camera.far_ - (camera.far_ - camera.near_) / 3.0f) * 0.5f;
        float near2 = far1;
        float far2 = camera.far_;

        // 空间冗余
        far0 = (far0 - near0) / 9.0f + far0;
        far1 = (far1 - near1) / 9.0f + far1;

        float half_height = tan(DirectX::XMConvertToRadians(camera.fov_) * 0.5f) * camera.far_;
        float half_width = half_height * camera.aspect_;
        Box box[3];
        new (box) Box(
            Vector3(0, 0, -(near0 + far0) * 0.5f),
            Vector3(half_width * far0 / far2, half_height * far0 / far2, (far0 - near0) * 0.5f));

        new (box + 1) Box(
            Vector3(0, 0, -(near1 + far1) * 0.5f),
            Vector3(half_width * far1 / far2, half_height * far1 / far2, (far1 - near1) * 0.5f));

        new (box + 2) Box(
            Vector3(0, 0, -(near2 + far2) * 0.5f),
            Vector3(half_width, half_height, (far2 - near2) * 0.5f));

        float distances[4] = {near0, far0, far1, far2};

        Matrix CameraToLight = camera.MatrixWorld * dlight.MatrixInvWorld;
        Matrix WorldToLightProjMatrixTransposed[3];

        command_list->SetPipelineState(shadow_pass_pso_.Get());
        command_list->RSSetViewports(1, &viewport_);
        command_list->RSSetScissorRects(1, &scissor_rect_);
        using namespace DirectX;
        for (int i = 0; i < 3; ++i)
        {
            Box lightSpaceBox = box[i].Transform(CameraToLight);
            XMVECTOR v1 = XMLoadFloat3(&lightSpaceBox.Center);
            XMVECTOR v2 = XMLoadFloat3(&lightSpaceBox.Extents);

            XMVECTOR MAXPV = XMVectorAdd(v1, v2);
            XMVECTOR MINPV = XMVectorSubtract(v1, v2);
            XMFLOAT3 MAXP, MINP;
            XMStoreFloat3(&MAXP, MAXPV);
            XMStoreFloat3(&MINP, MINPV);

            MAXP.z = (std::max)(MINP.z + dlight.shadow_distance_, MAXP.z);
            Matrix LightSapceOrthographicMatrix;
            // RESERVE-Z
            DirectX::XMStoreFloat4x4(&LightSapceOrthographicMatrix,
                                     XMMatrixOrthographicOffCenterRH(MINP.x, MAXP.x, MINP.y, MAXP.y, -MINP.z, -MAXP.z));

            MAXPV = XMLoadFloat3(&MAXP);
            //需要绘制物体的区域
            Box LightSpaceBoundingBox;
            XMStoreFloat3(&LightSpaceBoundingBox.Center, XMVectorScale(XMVectorAdd(MAXPV, MINPV), 0.5f));
            XMStoreFloat3(&LightSpaceBoundingBox.Extents, XMVectorScale(XMVectorSubtract(MAXPV, MINPV), 0.51f));
            Box WorldSpaceBoundingBox = LightSpaceBoundingBox.Transform(dlight.MatrixWorld);

            auto currentFrustum = camera.GetBoundingFrustrum();
            currentFrustum.Near = distances[i];
            currentFrustum.Far = distances[i + 1];
            if (WorldSpaceBoundingBox.Contains(currentFrustum) != ContainmentType::CONTAINS)
            {
                FMSG("OOOOPS!");
            }

            BoundingOrientedBox WorldSpaceOBB(WorldSpaceBoundingBox.Center, LightSpaceBoundingBox.Extents, Vector4());
            DirectX::XMStoreFloat4(&WorldSpaceOBB.Orientation, XMQuaternionRotationMatrix(dlight.MatrixWorld));

            std::vector<bool> visibile;
            visibile.resize(scene.StaticMeshes.size());
            t_shadow_split[i]->TransitionState(command_list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            auto depth_descriptor = t_shadow_split[i]->GetCPUDSV();
            command_list->OMSetRenderTargets(0, nullptr, TRUE, &depth_descriptor);
            command_list->ClearDepthStencilView(depth_descriptor, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
            if (WorldSpaceBoundingBox.Intersects(scene.StaticMeshesOctree->GetRootBounds()))
            {
                for (Octree<Scene::StaticMeshProxy>::NodeIterator NodeIt(*(scene.StaticMeshesOctree)); NodeIt.HasPendingNodes(); NodeIt.Advance())
                {
                    auto &n = NodeIt.GetCurrentNode();
                    auto &context = NodeIt.GetCurrentContext();
                    for (const auto &ele : n.GetElements())
                    {
                        if (ele.pointer->GetBoundingBox().Intersects(WorldSpaceBoundingBox) && WorldSpaceOBB.Intersects(ele.pointer->GetBoundingBox()))
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

                PassConstantBuffer constant;
                constant.InvView = dlight.MatrixWorld.Transpose();
                constant.View = dlight.MatrixInvWorld.Transpose();
                constant.Proj = LightSapceOrthographicMatrix.Transpose();
                constant.InvProj = constant.Proj.Invert();
                constant.ViewProj = constant.Proj * constant.View;
                constant.InvViewProj = constant.InvView * constant.InvProj;
                WorldToLightProjMatrixTransposed[i] = constant.ViewProj;
                auto &pass_buffer = dlight_pass_constant_buffer_->operator[](idx);
                pass_buffer.Write(i, constant);

                command_list->SetGraphicsRootSignature(shadow_pass_signature_.Get());
                command_list->SetGraphicsRootConstantBufferView(
                    1, pass_buffer.GetResource()->GetGPUVirtualAddress() + pass_buffer.GetSize() * i);

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
        }

        DirectionalLightBuffer dlightbuffer;
        dlightbuffer.ShadowMatrix0 = WorldToLightProjMatrixTransposed[0];
        dlightbuffer.ShadowMatrix1 = WorldToLightProjMatrixTransposed[1];
        dlightbuffer.ShadowMatrix2 = WorldToLightProjMatrixTransposed[2];
        dlightbuffer.Color = dlight.color_;
        dlightbuffer.ShadowMapSize = 2048.0f;
        dlightbuffer.LightDirection = dlight.MatrixWorld.Forward();
        light_info_buffer_->operator[](idx).Write(0, dlightbuffer);

        command_list->SetPipelineState(light_pass_pso_.Get());
        command_list->RSSetViewports(1, &renderer.viewport_);
        command_list->RSSetScissorRects(1, &renderer.scissor_rect_);
        command_list->SetGraphicsRootSignature(light_pass_signature_.Get());

        t_shadow_split[0]->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);
        t_shadow_split[1]->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);
        t_shadow_split[2]->TransitionState(command_list, D3D12_RESOURCE_STATE_GENERIC_READ);

        auto color_rtv = renderer.t_color_output_->GetCPURTV();
        command_list->OMSetRenderTargets(1, &color_rtv, FALSE, nullptr);
        command_list->SetGraphicsRootDescriptorTable(0, renderer.t_gbuffer_base_color_->GetGPUSRV());
        command_list->SetGraphicsRootDescriptorTable(1, t_shadow_split[0]->GetGPUSRV());
        command_list->SetGraphicsRootConstantBufferView(2, light_info_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());
        command_list->SetGraphicsRootConstantBufferView(3, renderer.pass_constant_buffer_->operator[](idx).GetResource()->GetGPUVirtualAddress());

        command_list->IASetVertexBuffers(0, 1, &renderer.pp_vertex_buffer_view_);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->DrawInstanced(3, 1, 0, 0);
    }
} // namespace feng