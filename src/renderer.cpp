#include "renderer.hpp"

#include "window.hpp"
#include "d3dx12.h"
#include "render/simple.hpp"
#include "scene/scene.hpp"
#include "ResourceUploadBatch.h"
namespace feng
{
    Renderer::Renderer(const Window &window)
    {
        device_ = std::make_unique<Device>();
        render_window_ = std::make_unique<RenderWindow>(*device_, window);
        width_ = window.GetWidth();
        height_ = window.GetHeight();

        viewport_.TopLeftX = 0;
        viewport_.TopLeftY = 0;
        viewport_.Width = static_cast<float>(width_);
        viewport_.Height = static_cast<float>(height_);
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        scissor_rect_ = {0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
    }

    void Renderer::Init(const Scene& scene)
    {
        DirectX::ResourceUploadBatch uploader(device_->GetDevice());
        uploader.Begin();

        for(auto& static_mesh : scene.StaticMeshes)
        {
            static_mesh->Init(GetDevice(), uploader);
        }
        
     
        pass_constant_buffer_ = std::make_unique<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>>(GetDevice(), 1);
        object_constant_buffer_ = std::make_unique<ConstantBufferGroup<ObjectConstantBuffer, BACK_BUFFER_SIZE>>(GetDevice(), scene.StaticMeshes.size());
        
        auto ending_event = uploader.End(device_->GetCommandQueue());
        device_->Signal(0);
        device_->Wait(0);

        ending_event.wait();
        

        t_depth_.reset(
            new DynamicTexture(
               GetDevice(), width_, height_, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT
            )
        );

        simple_.reset(new Simple());
        simple_->Build(*this);

        depth_only_.reset(new DepthOnly());
        depth_only_->Build(*this);
    }

    void Renderer::Draw(const Scene& scene)
    {
        uint8_t idx = render_window_->CurrentFrameIdx();
        GetDevice().Wait(idx);
        auto &camera = *(scene.Camera);
        if (camera.IsCBDirty())
        {
            PassConstantBuffer constant;
            // swaped matrix for camera
            constant.InvView = camera.MatrixWorld.Transpose();
            constant.View = camera.MatrixInvWorld.Transpose();
            constant.Proj = camera.MatrixProj.Transpose();
            constant.InvProj = camera.MatrixInvProj.Transpose();
            constant.ViewProj = constant.Proj * constant.View;
            constant.InvViewProj = constant.InvView * constant.InvProj;

            pass_constant_buffer_->operator[] (idx).Write(0, constant);
        }

        for(auto it = scene.StaticMeshes.cbegin(); it != scene.StaticMeshes.cend(); it++)
        {
            if (it->get()->IsCBDirty())
            {
                auto dis = std::distance(scene.StaticMeshes.cbegin(), it);
                ObjectConstantBuffer buffer;
                buffer.World = it->get()->MatrixWorld.Transpose();
                buffer.InvWorld = it->get()->MatrixInvWorld.Transpose();
                object_constant_buffer_->operator[] (idx).Write(dis, buffer);
            }
        }
        auto command_list = GetDevice().BeginCommand(idx, nullptr);

        ID3D12DescriptorHeap *heaps[] = {
            device_->GetSRVHeap().Heap(), device_->GetDSVHeap().Heap(),
            device_->GetRTVHeap().Heap()
            };
        command_list->SetDescriptorHeaps(1, heaps);

        depth_only_->Draw(*this, scene, command_list, idx);

        simple_->Draw(*this, scene, command_list, idx);

        command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_window_->CurrentBackBuffer(),
                                                                               D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        device_->EndCommand();
        render_window_->Swap();
        // IDX + 1
        device_->Signal(render_window_->CurrentFrameIdx());
    }

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 2>& Renderer::GetStaticSamplers()
    {
        static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 2> samplers = {
            CD3DX12_STATIC_SAMPLER_DESC{0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP},
            CD3DX12_STATIC_SAMPLER_DESC{0, D3D12_FILTER_MIN_MAG_MIP_LINEAR}
        };
        return samplers;
    }
} // namespace feng