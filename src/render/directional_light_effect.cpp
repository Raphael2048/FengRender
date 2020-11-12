#include "render/directional_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"
namespace feng
{
    DirectionalLightEffect::DirectionalLightEffect(DepthOnly &depthEffect, Device &device, const Scene &scene)
    {
        shadow_pass_signature_ = depthEffect.signature_;
        shadow_pass_pso_ = depthEffect.pso_;

        // 这里GPU地址是连续的, 直接用range表示
        t_shadow_split[0].reset(new DynamicTexture(device, 2048, 2048, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_D16_UNORM));
        t_shadow_split[1].reset(new DynamicTexture(device, 2048, 2048, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_D16_UNORM));
        t_shadow_split[2].reset(new DynamicTexture(device, 2048, 2048, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_D16_UNORM));

        auto num = scene.StaticMeshes.size();
        shadow_visibity_split[0].resize(num);
        shadow_visibity_split[1].resize(num);
        shadow_visibity_split[2].resize(num);

        pass_constant_buffer_ = std::make_unique<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>>(device, 3);

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = static_cast<float>(2048);
        viewport_.Height = static_cast<float>(2048);
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, static_cast<LONG>(2048), static_cast<LONG>(2048)};
    }

    void DirectionalLightEffect::Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx)
    {
        command_list->SetPipelineState(shadow_pass_pso_.Get());
        command_list->RSSetViewports(1, &viewport_);
        command_list->RSSetScissorRects(1, &scissor_rect_);

        auto &dlight = *scene.DirectionalLight;
        auto &camera = *scene.Camera;

        float near0 = camera.near_;
        float far0 = camera.near_ * pow(camera.far_ / camera.near_, 1.0f / 3.0f) * 0.5f + ((camera.far_ - camera.near_) / 3.0f + camera.near_) * 0.5f;
        float near1 = far0;
        float far1 = camera.near_ * pow(camera.far_ / camera.near_, 2.0f / 3.0f) * 0.5f + (camera.far_ - (camera.far_ - camera.near_) / 3.0f) * 0.5f;
        float near2 = far1;
        float far2 = camera.far_;

        // 渐变切换
        far0 = (far0 - near0) * 0.1f + far0;
        near2 = near2 - (far2 - near2) * 0.1f;

        float half_width = tan(DirectX::XMConvertToRadians(camera.fov_)) * camera.far_ * 0.5f;
        float half_height = half_width / camera.aspect_ * 0.5f;
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

        Matrix CameraToLight = camera.MatrixWorld * dlight.MatrixInvWorld;
        Matrix LightSapceOrthographicMatrix[3];
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

            MAXP.z = MINP.z + dlight.shadow_distance_;
            // RESERVE-Z
            DirectX::XMStoreFloat4x4(LightSapceOrthographicMatrix + i,
                                     XMMatrixOrthographicOffCenterRH(MINP.x, MAXP.x, MINP.y, MAXP.y, -MINP.z, -MAXP.z));

            MAXPV = XMLoadFloat3(&MAXP);
            //需要绘制物体的区域
            Box LightSpaceBoundingBox;
            XMStoreFloat3(&LightSpaceBoundingBox.Center, XMVectorScale(XMVectorAdd(MAXPV, MINPV), 0.5f));
            XMStoreFloat3(&LightSpaceBoundingBox.Extents, XMVectorScale(XMVectorSubtract(MAXPV, MINPV), 0.5f));
            Box WorldSpaceBoundingBox = LightSpaceBoundingBox.Transform(dlight.MatrixWorld);

            BoundingOrientedBox WorldSpaceOBB(
                LightSpaceBoundingBox.Center, LightSpaceBoundingBox.Extents, Vector4());
            dlight.CalQuaternion(WorldSpaceOBB.Orientation);

            std::vector<bool> visibile;
            visibile.resize(scene.StaticMeshes.size());
            if (WorldSpaceBoundingBox.Intersects(scene.StaticMeshesOctree->GetRootBounds()))
            {
                for (Octree<Scene::StaticMeshProxy>::NodeIterator NodeIt(*(scene.StaticMeshesOctree)); NodeIt.HasPendingNodes(); NodeIt.Advance())
                {
                    auto &n = NodeIt.GetCurrentNode();
                    auto &context = NodeIt.GetCurrentContext();
                    for (const auto &ele : n.GetElements())
                    {
                        if (ele.pointer->GetBoundingBox().Intersects(WorldSpaceBoundingBox))
                        {
                            visibile[ele.id] = true;
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
                constant.Proj = LightSapceOrthographicMatrix[i].Transpose();
                constant.InvProj = constant.Proj.Invert();
                constant.ViewProj = constant.Proj * constant.View;
                constant.InvViewProj = constant.InvView * constant.InvProj;
                auto &pass_buffer = pass_constant_buffer_->operator[](idx);
                pass_buffer.Write(i, constant);

                t_shadow_split[i]->TransitionState(command_list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
                auto depth_descriptor = renderer.GetDevice().GetDSVHeap().GetCpuHandle(t_shadow_split[i]->GetDSVHeapIndex());

                command_list->OMSetRenderTargets(0, nullptr, TRUE, &depth_descriptor);
                command_list->ClearDepthStencilView(depth_descriptor, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

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
                        StaticMesh *static_mesh = it->get();

                        command_list->IASetVertexBuffers(0, 1, &static_mesh->GetVertexBufferView());
                        command_list->IASetIndexBuffer(&static_mesh->GetIndexBufferView());
                        command_list->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                        command_list->SetGraphicsRootConstantBufferView(0, object_buffer_base_address + dis * object_buffer.GetSize());

                        command_list->DrawIndexedInstanced(static_mesh->mesh_->index_count_, 1, 0, 0, 0);
                    }
                }
            }
        }

        // auto& cameraFrustum = camera.GetBoundingFrustrum();
        // Box lightSpaceBox = camera.GetViewSpaceBoundingBox().Transform(CameraToLight);
        // Box
    }
} // namespace feng