#pragma once
#include "dx12/dx12_shader.hpp"
#include "DirectXHelpers.h"
namespace feng
{
    class Renderer;
    class Scene;
    class SSREffect : public Uncopyable
    {
    public:
        SSREffect(Renderer& renderer);

        void Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    private:
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_;
    };
}