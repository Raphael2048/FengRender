#pragma once

#include "dx12/dx12_texture.hpp"
#include "dx12/dx12_buffer.hpp"
#include "render/depth_only.hpp"
#include "render/spot_light_effect.hpp"
namespace feng
{
    struct PassConstantBuffer;
    class Renderer;
    class Scene;
    class SpotLight;
    class SpotLightEffect
    {
    public:
        SpotLightEffect(DepthOnly & depthEffect, Renderer &renderer, const Scene& scene);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    
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
        std::vector<std::unique_ptr<DynamicTexture>> t_shadowmaps;
        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> slight_pass_constant_buffer_;
        ComPtr<ID3D12RootSignature> shadow_pass_signature_;
        ComPtr<ID3D12PipelineState> shadow_pass_pso_;
        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;

        // For lighting 
        ComPtr<ID3D12RootSignature> light_pass_signature_;
        ComPtr<ID3D12PipelineState> light_pass_pso_;
        std::unique_ptr<ConstantBufferGroup<SpotLightBuffer, BACK_BUFFER_SIZE>> light_info_buffer_;

    };
} // namespace feng