#pragma once

#include "dx12/dx12_dyncmic_texture.hpp"
#include "dx12/dx12_static_texture.hpp"
#include "dx12/dx12_constant_buffer.hpp"
#include "render/depth_only.hpp"
#include "render/spot_light_effect.hpp"
#include "ResourceUploadBatch.h"
namespace feng
{
    struct PassConstantBuffer;
    class Renderer;
    class Scene;
    class SkyLight;
    class SkyLightEffect
    {
    public:
        SkyLightEffect(std::shared_ptr<StaticTexture> texture, Renderer &renderer);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx);

    private:
        std::shared_ptr<StaticTexture> cubemap_;
        // For shadowmap generation
        // 512*512
        std::unique_ptr<DynamicTexture> t_irradiance;
        ComPtr<ID3D12RootSignature> convolution_signature_;
        ComPtr<ID3D12PipelineState> shadow_pass_pso_;
        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;

        // For lighting
        ComPtr<ID3D12RootSignature> light_pass_signature_;
        ComPtr<ID3D12PipelineState> light_pass_pso_;
    };
} // namespace feng