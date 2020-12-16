#pragma once
#include "dx12/dx12_shader.hpp"
#include "DirectXHelpers.h"
#include "dx12/dx12_texture.hpp"
namespace feng
{
    class Renderer;
    class Scene;
    class SSREffect : public Uncopyable
    {
    public:
        struct SSRInfo
        {
            Vector2 ScreenSize;
            Vector2 InvScreenSize;
            Vector2 HZBSize;
            Vector2 InvHZBSize;
        };
        SSREffect(Renderer& renderer);

        void Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    private:
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_;
        std::unique_ptr<DynamicPlainTexture> t_ssr_;
        SSRInfo info_;
    };
}