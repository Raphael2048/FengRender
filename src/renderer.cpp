#include "renderer.hpp"

#include "window.hpp"
#include "d3dx12.h"
#include "render/simple.hpp"
#include "scene/scene.hpp"
namespace feng
{
    Renderer::Renderer(const Window &window)
    {
        device_ = std::make_unique<Device>();
        render_window_ = std::make_unique<RenderWindow>(*device_, window);
    }

    void Renderer::Init(const Scene& scene)
    {
        auto command_list = device_->BeginCommand(0);

        for(auto& static_mesh : scene.StaticMeshes)
        {
            static_mesh->Init(GetDevice(), command_list);
        }
        
        
        camera_buffer_ = std::make_unique<ConstantBuffer<CameraConstantBuffer>>(GetDevice(), 1, GetDevice().GetCBVHeap());
        simple_.reset(new Simple());
        simple_->Build(*this);

        device_->EndCommand();
        device_->FlushCommand(0);
    }

    void Renderer::Draw(const Scene& scene)
    {
        auto &camera = *(scene.Camera);
        CameraConstantBuffer constant;
        // 交换的逆矩阵
        constant.InvView = camera.MatrixView.Transpose();
        constant.View = camera.MatrixInvView.Transpose();
        constant.Proj = camera.MatrixProj.Transpose();
        constant.InvProj = camera.MatrixInvProj.Transpose();

        camera_buffer_->Write(0, constant);
        simple_->Draw(*this, scene);
    }
} // namespace feng