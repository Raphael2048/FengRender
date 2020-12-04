#pragma once
#include "dx12/dx12_shader.hpp"
#include "dx12/dx12_texture.hpp"
namespace feng
{
    class Renderer;
    class Scene;

    class BlitEffect : public Uncopyable
    {
    public:
        enum class BlurMode
        {
            Guassion,
            Tent
        };
        friend class DirectionalLightEffect;
        friend class SpotLightEffect;
        friend class PointLightEffect;
        BlitEffect(Renderer &renderer);

        void Blit(ID3D12GraphicsCommandList *command_list, DynamicPlainTexture *from, DynamicPlainTexture *to);
        void Blur(ID3D12GraphicsCommandList *command_list, DynamicPlainTexture *from, DynamicPlainTexture *temp, BlurMode mode);

    private:
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_blit_;
        ComPtr<ID3D12PipelineState> pso_blur_guassion_h_;
        ComPtr<ID3D12PipelineState> pso_blit_guassion_v_;
    };
} // namespace feng