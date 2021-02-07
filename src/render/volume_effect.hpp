#pragma once
#include "dx12/dx12_shader.hpp"
#include "dx12/dx12_texture.hpp"
#include "DirectXHelpers.h"

namespace feng
{
    class Renderer;
    class Scene;
    class DirectionalLightEffect;
    class VolumeEffect : public Uncopyable
    {
    public:
        VolumeEffect(Renderer& renderer);
        void Draw(Renderer &renderer, DirectionalLightEffect& dlight, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    private:
        ComPtr<ID3D12RootSignature> signature_density_, signature_accumulate_, signature_integration_;
        ComPtr<ID3D12PipelineState> pso_density_, pso_accumulate_, pso_integration_;
        std::unique_ptr<DynamicPlain3DTexture> texture_density_, texture_accumulate_;
        Vector3 inv_size_;
    };
}