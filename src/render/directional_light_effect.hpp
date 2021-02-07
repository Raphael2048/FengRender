#pragma once

#include "dx12/dx12_texture.hpp"
#include "dx12/dx12_buffer.hpp"
#include "render/depth_only.hpp"
namespace feng
{
    struct PassConstantBuffer;
    class Renderer;
    class Scene;
    class DirectionalLightEffect
    {
        friend class VolumeEffect;
    public:
        DirectionalLightEffect(DepthOnly & depthEffect, Renderer &renderer, const Scene& scene);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    
    private:
        struct DirectionalLightBuffer
        {
            Matrix ShadowMatrix0;
            Matrix ShadowMatrix1;
            Matrix ShadowMatrix2;
            Vector3 LightDirection;
            float ShadowMapSize;
            Color Color;
        };

        // 2048*2048
        std::unique_ptr<DynamicDepthTexture> t_shadow_split[3];

        // For shadowmap generation
        std::vector<bool> shadow_visibity_split[3];
        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> dlight_pass_constant_buffer_;
        ComPtr<ID3D12RootSignature> shadow_pass_signature_;
        ComPtr<ID3D12PipelineState> shadow_pass_pso_;
        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;


        // For lighting 
        ComPtr<ID3D12RootSignature> light_pass_signature_;
        ComPtr<ID3D12PipelineState> light_pass_pso_;
        std::unique_ptr<ConstantBufferGroup<DirectionalLightBuffer, BACK_BUFFER_SIZE>> light_info_buffer_;

    };
} // namespace feng