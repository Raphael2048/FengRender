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
        BlitEffect(Renderer &renderer);

        void AccumulateTo(Renderer &render, ID3D12GraphicsCommandList *command_list, DynamicPlainTexture *from, DynamicPlainTexture *to);

        void GaussianBlur(Renderer &render, ID3D12GraphicsCommandList* command_list, DynamicPlainTexture *from, DynamicPlainTexture *temp, float kernel = 1.0f);

    private:
        ComPtr<ID3D12RootSignature> signature_, signature_gaussian_;
        ComPtr<ID3D12PipelineState> pso_accumulate_, pso_gaussian_h_, pso_gaussian_v_;
    };
} // namespace feng