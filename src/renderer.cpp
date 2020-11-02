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
        
     
        pass_constant_buffer_ = std::make_unique<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>>(GetDevice(), 1);
        object_constant_buffer_ = std::make_unique<ConstantBufferGroup<ObjectConstantBuffer, BACK_BUFFER_SIZE>>(GetDevice(), scene.StaticMeshes.size());
        simple_.reset(new Simple());
        simple_->Build(*this);

        device_->EndCommand();
        device_->FlushCommand(0);
    }

    void Renderer::Draw(const Scene& scene)
    {
        auto &camera = *(scene.Camera);
        PassConstantBuffer constant;
        // 交换的逆矩阵
        constant.InvView = camera.MatrixWorld.Transpose();
        constant.View = camera.MatrixInvWorld.Transpose();
        constant.Proj = camera.MatrixProj.Transpose();
        constant.InvProj = camera.MatrixInvProj.Transpose();
        constant.ViewProj = constant.Proj * constant.View;
        constant.InvViewProj = constant.InvView * constant.InvProj;

        pass_constant_buffer_->operator[] (render_window_->CurrentFrameIdx()).Write(0, constant);

        for(auto it = scene.StaticMeshes.cbegin(); it != scene.StaticMeshes.cend(); it++)
        {
            auto dis = std::distance(scene.StaticMeshes.cbegin(), it);
            ObjectConstantBuffer buffer;
            buffer.World = it->get()->MatrixWorld.Transpose();
            buffer.InvWorld = it->get()->MatrixInvWorld.Transpose();
            object_constant_buffer_->operator[] (render_window_->CurrentFrameIdx()).Write(dis, buffer);
        }
        simple_->Draw(*this, scene);
    }
} // namespace feng