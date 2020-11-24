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
    class PointLight;
    class PointLightEffect
    {
    public:
        PointLightEffect(Renderer &renderer, const Scene& scene);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    
    private:
        struct PointLightBuffer
        {
            Matrix ShadowMatrix[6];
            Vector3 LightPosition;
            float Radius;
            Color Color;
            float ShadowMapSize;
        };

        // For shadowmap generation
        // 256*256
        std::unique_ptr<DynamicDepthTexture> t_shadowmaps[6];
        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> plight_pass_constant_buffer_;
        ComPtr<ID3D12RootSignature> shadow_pass_signature_;
        ComPtr<ID3D12PipelineState> shadow_pass_pso_;
        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;

        // For lighting 
        ComPtr<ID3D12RootSignature> light_pass_signature_;
        ComPtr<ID3D12PipelineState> light_pass_pso_;
        std::unique_ptr<ConstantBufferGroup<PointLightBuffer, BACK_BUFFER_SIZE>> light_info_buffer_;

    };
} // namespace feng