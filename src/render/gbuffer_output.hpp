#pragma once
#include "dx12/dx12_shader.hpp"
#include "DirectXHelpers.h"
#include "dx12/dx12_buffer.hpp"

#define INDIRECT_DRAW 1
namespace feng
{
    class Renderer;
    class Scene;
    class GBufferOutput : public Uncopyable
    {
    public:
        GBufferOutput(Renderer& renderer);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    private:
        std::unique_ptr<GraphicsShader> shader;
        ComPtr<ID3D12RootSignature> signature_;
        ComPtr<ID3D12PipelineState> pso_;

#if INDIRECT_DRAW
        struct IndirectCommand
        {
            D3D12_GPU_VIRTUAL_ADDRESS srv;
            D3D12_GPU_VIRTUAL_ADDRESS cbv;
            D3D12_VERTEX_BUFFER_VIEW vbv;
            D3D12_INDEX_BUFFER_VIEW ibv;
            D3D12_DRAW_ARGUMENTS drawArguments;
        };
        // For IndirectDraw
        ComPtr<ID3D12CommandSignature> command_signature_;
        std::unique_ptr<GenericBuffer<IndirectCommand>> argument_buffer_;
        //std::unique_ptr<GenericBuffer<INT64>> count_buffer_;

#endif
    };
}