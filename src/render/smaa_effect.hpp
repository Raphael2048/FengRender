#pragma once
#include "dx12/dx12_shader.hpp"
#include "DirectXHelpers.h"
#include "dx12/dx12_texture.hpp"
namespace feng
{
    class Renderer;
    class Scene;
    class SMAAEffect : public Uncopyable
    {
    public:
        SMAAEffect(Renderer& renderer);

        void Draw(Renderer &renderer, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    private:
        ComPtr<ID3D12RootSignature> signature_edge_, signature_weight_, signature_final_;
        ComPtr<ID3D12PipelineState> pso_edge_, pso_weight_, pso_final_;
        std::unique_ptr<DynamicPlainTexture> t_edge_, t_weight_, t_final_;
    };
}