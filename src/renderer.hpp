#pragma once

#include "dx12/dx12_device.hpp"
#include "dx12/dx12_window.hpp"
#include "dx12/dx12_texture.hpp"
#include "dx12/dx12_buffer.hpp"
#include "scene/camera.hpp"
#include "render/depth_only.hpp"
#include "render/hzb_effect.hpp"
#include "render/gtao_effect.hpp"
#include "render/gbuffer_output.hpp"
#include "render/tone_mapping.hpp"
#include "render/spot_light_effect.hpp"
#include "render/sky_light_effect.hpp"
#include "render/point_light_effect.hpp"
#include "render/directional_light_effect.hpp"
#include "render/ssr_effect.hpp"
#include "render/ssgi_effect.hpp"
#include "render/blit_effect.hpp"
#include "render/smaa_effect.hpp"
#include "render/volume_effect.hpp"
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
        Vector3 CameraPos;
    };

    struct ObjectConstantBuffer
    {
        Matrix World;
        Matrix InvWorld;
    };
    class Window;
    class Scene;
    class StaticMesh;
    class Renderer
    {
    public:
        enum class GIMode
        {
            None,
            ScreenSpace,
            Voxel,
            RayTracing,
        };
        Renderer(const Window &window);
        Device &GetDevice() { return *device_; }

        RenderWindow &GetRenderWindow() { return *render_window_; }
        void Init(const Scene &scene);
        void Draw(Scene &scene);
        void RefreshConstantBuffer(const StaticMesh &mesh, uint8_t idx, ptrdiff_t distance);

        std::array<const CD3DX12_STATIC_SAMPLER_DESC, 2> &GetStaticSamplers();

        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> pass_constant_buffer_;
        std::unique_ptr<ConstantBufferGroup<ObjectConstantBuffer, BACK_BUFFER_SIZE>> object_constant_buffer_;
        UINT width_, height_;
        Vector2 screen_buffer_[2];

        // depth buffer
        std::unique_ptr<DynamicDepthTexture> t_depth_;
        std::unique_ptr<DynamicPlainTexture> t_gbuffer_base_color_, t_gbuffer_normal,
            t_gbuffer_roughness_metallic_, t_color_output_, t_color_output2_, t_ao_, t_hzb_;
        // std::unique_ptr<DynamicPlainTextureMips> t_hzb_;

        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;

        // PostProcessing InputLayout
        D3D12_INPUT_LAYOUT_DESC pp_input_layout_;
        // PostProcessing VertexData
        std::unique_ptr<StaticBuffer> pp_vertex_buffer_;
        D3D12_VERTEX_BUFFER_VIEW pp_vertex_buffer_view_;

        std::unique_ptr<BlitEffect> blit_;
        std::vector<uint32_t> hzb_size_pos_;
    private:
        GIMode gi_mode_ = GIMode::ScreenSpace;
        std::unique_ptr<Device> device_;
        std::unique_ptr<RenderWindow> render_window_;

        // For SreenSpaceGI, GTAO, SSR, SSGI
        std::unique_ptr<HZBEffect> hzb_;
        std::unique_ptr<GTAOEffect> gtao_;
        std::unique_ptr<SSREffect> ssr_;
        std::unique_ptr<SSGIEffect> ssgi_;

        // PostProcessing
        std::unique_ptr<SMAAEffect> smaa_;
        std::unique_ptr<ToneMapping> tone_mapping_;

        std::unique_ptr<DepthOnly> depth_only_;
        std::unique_ptr<GBufferOutput> gbuffer_output_;
        std::unique_ptr<DirectionalLightEffect> directional_light_effect_;
        std::unique_ptr<SpotLightEffect> spot_light_effect_;
        std::unique_ptr<PointLightEffect> point_light_effect_;
        std::unique_ptr<SkyLightEffect> sky_light_effect_;
        std::unique_ptr<VolumeEffect> volume_effect_;
    };

} // namespace feng