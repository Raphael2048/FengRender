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
        };
        SSREffect(Renderer& renderer);

        void Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list, uint8_t idx);

        void Accumate(Renderer &renderer, ID3D12GraphicsCommandList* command_list);
    private:
        ComPtr<ID3D12RootSignature> signature_, signature_blur_;
        ComPtr<ID3D12PipelineState> pso_, pso_blur_h_, pso_blur_v_;
        std::unique_ptr<DynamicPlainTexture> t_ssr_, t_temp_;
        SSRInfo info_;
    };
}