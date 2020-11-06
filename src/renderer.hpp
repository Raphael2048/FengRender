#pragma once

#include "dx12/dx12_device.hpp"
#include "dx12/dx12_constant_buffer.hpp"
#include "dx12/dx12_render_target.hpp"
#include "dx12/dx12_dyncmic_texture.hpp"
#include "scene/camera.hpp"
#include "render/simple.hpp"
#include "render/depth_only.hpp"
#include "render/gbuffer_output.hpp"
#include <array>
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

        std::array<const CD3DX12_STATIC_SAMPLER_DESC, 2>& GetStaticSamplers();

        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> pass_constant_buffer_;
        std::unique_ptr<ConstantBufferGroup<ObjectConstantBuffer, BACK_BUFFER_SIZE>> object_constant_buffer_;
        UINT width_, height_;


        // depth buffer
        std::unique_ptr<DynamicTexture> t_depth_, t_gbuffer_base_color_, t_gbuffer_normal, t_gbuffer_roughness_metallic_;

        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;
    private:
        std::unique_ptr<Device> device_;
        std::unique_ptr<RenderWindow> render_window_;


        std::unique_ptr<DepthOnly> depth_only_;
        std::unique_ptr<GBufferOutput> gbuffer_output_;
        std::unique_ptr<Simple> simple_;
    };

} // namespace feng