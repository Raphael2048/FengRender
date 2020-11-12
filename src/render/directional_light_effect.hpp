#pragma once

#include "dx12/dx12_dyncmic_texture.hpp"
#include "dx12/dx12_constant_buffer.hpp"
#include "render/depth_only.hpp"
namespace feng
{
    struct PassConstantBuffer;
    class Renderer;
    class Scene;
    class DirectionalLightEffect
    {
    public:
        DirectionalLightEffect(DepthOnly & depthEffect, Device& device, const Scene& scene);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    
    private:
        // 2048*2048
        std::unique_ptr<DynamicTexture> t_shadow_split[3];
        std::vector<bool> shadow_visibity_split[3];
        std::unique_ptr<ConstantBufferGroup<PassConstantBuffer, BACK_BUFFER_SIZE>> pass_constant_buffer_;
        
        
        ComPtr<ID3D12RootSignature> shadow_pass_signature_;
        ComPtr<ID3D12PipelineState> shadow_pass_pso_;

        ComPtr<ID3D12RootSignature> light_pass_signature_;
        ComPtr<ID3D12PipelineState> light_pass_pso_;

        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissor_rect_;
    };
} // namespace feng