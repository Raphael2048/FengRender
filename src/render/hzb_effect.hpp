#pragma once
#include "dx12/dx12_shader.hpp"
#include "DirectXHelpers.h"
namespace feng
{
    class Renderer;
    class Scene;
    class HZBEffect : public Uncopyable
    {
    public:
        HZBEffect(Renderer& renderer);

        void Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list);
    private:
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_;
    };
}