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
        
        std::array<Vertex, 8> vertices =
        {
            Vertex({Vector3(-1.0f, -1.0f, -1.0f), Vector2(0.0f, 1.0f)}),
            Vertex({Vector3(-1.0f, +1.0f, -1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, +1.0f, -1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, -1.0f, -1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(-1.0f, -1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(-1.0f, +1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, +1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, -1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
        };

        camera_buffer_ = std::make_unique<ConstantBuffer<CameraConstantBuffer>>(GetDevice(), 1, GetDevice().GetCBVHeap());
        // simple_.reset(new Simple());
        // simple_->Build(*this);

        device_->EndCommand();
        device_->FlushCommand(0);
    }

    void Renderer::Draw(const Scene& scene)
    {
        auto &camera = *(scene.Camera);
        CameraConstantBuffer constant;
        constant.View = camera.MatrixView;
        constant.InvView = camera.MatrixInvView;
        constant.Proj = camera.MatrixProj;
        constant.InvProj = camera.MatrixInvProj;

        camera_buffer_->Write(0, constant);
        // simple_->Draw(*this, scene);
    }
} // namespace feng