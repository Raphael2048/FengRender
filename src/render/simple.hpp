#pragma once

#include "dx12/dx12_shader.hpp"
#include "dx12/dx12_constant_buffer.hpp"
#include "dx12/dx12_root_signature.hpp"
#include "dx12/dx12_buffer.hpp"
#include "dx12/dx12_pipeline_state.hpp"
#include "dx12/dx12_constant_buffer.hpp"
#include "scene/scene.hpp"
#include <array>
namespace feng
{
    class Renderer;
    class Scene;
    class Simple
    {
    public:
        void Build(Renderer &renderer);

        void Draw(Renderer &renderer, const Scene &scene);

    private:
        std::unique_ptr<GraphicsShader> shader;
        std::unique_ptr<RootSignature> signature_;
        std::unique_ptr<PipelineState> pso_;
        std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;


    };
} // namespace feng