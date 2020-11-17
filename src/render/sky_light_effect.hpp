#pragma once

#include "dx12/dx12_dyncmic_texture.hpp"
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
        SkyLightEffect(DirectX::ResourceUploadBatch& uploader, Renderer &renderer, const Scene &scene);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx);

    private:
        struct SpotLightBuffer
        {
            Matrix ShadowMatrix;
            Vector3 LightPosition;
            float Radius;
            Vector3 LightDirection;
            float ShadowMapSize;
            Color Color;
            float InnerFalloff;
            float OuterFalloff;
        };

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
        std::unique_ptr<ConstantBufferGroup<SpotLightBuffer, BACK_BUFFER_SIZE>> light_info_buffer_;
    };
} // namespace feng