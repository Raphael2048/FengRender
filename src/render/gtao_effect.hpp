#pragma once
#include "dx12/dx12_shader.hpp"
#include "dx12/dx12_texture.hpp"
namespace feng
{
    class Renderer;
    class Scene;
    class GTAOEffect : public Uncopyable
    {
    public:
        GTAOEffect(Renderer &renderer);

        void Draw(Renderer &renderer, ID3D12GraphicsCommandList *command_list, uint8_t idx);

    private:
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_;
        ComPtr<ID3D12PipelineState> pso2_;
        std::unique_ptr<DynamicPlainTexture> ao_filter_;
    };
} // namespace feng