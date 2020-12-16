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

    private:
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_accumulate_;
    };
} // namespace feng