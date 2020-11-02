#pragma once

#include "dx12/dx12_device.hpp"
#include "dx12/dx12_constant_buffer.hpp"
#include "dx12/dx12_render_target.hpp"
#include "scene/camera.hpp"
#include "render/simple.hpp"

namespace feng
{
    struct PassConstantBuffer
    {
        Matrix View;
        Matrix InvView;
        Matrix Proj;
        Matrix InvProj;
        Matrix ViewProj;
        Matrix InvViewProj;
        Vector3 EyePos;
    };

    struct ObjectConstantBuffer
    {
        Matrix World;
        Matrix InvWorld;
    };
    class Window;
    class Scene;
    class Renderer
    {
    public:
        Renderer(const Window &window);
        Device &GetDevice() { return *device_; }

        
        RenderWindow& GetRenderWindow() { return *render_window_; }
        void Init(const Scene& scene);
        void Draw(const Scene& scene);

        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> pass_constant_buffer_;
        std::unique_ptr<ConstantBufferGroup<ObjectConstantBuffer, BACK_BUFFER_SIZE>> object_constant_buffer_;
    private:
        std::unique_ptr<Device> device_;
        std::unique_ptr<RenderWindow> render_window_;
        std::unique_ptr<Simple> simple_;
    };

} // namespace feng