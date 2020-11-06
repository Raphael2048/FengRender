#pragma once

#include "dx12/dx12_shader.hpp"
namespace feng
{
    class Renderer;
    class Scene;
    class EffectBase : public Uncopyable
    {
    protected:
        std::unique_ptr<GraphicsShader> shader;
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_;
    };
} // namespace feng