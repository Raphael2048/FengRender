#pragma once

#include "dx12/dx12_dyncmic_texture.hpp"
#include "dx12/dx12_static_texture.hpp"
#include "dx12/dx12_buffer.hpp"
#include "render/depth_only.hpp"
#include "render/spot_light_effect.hpp"
#include "ResourceUploadBatch.h"
namespace feng
{
    struct PassConstantBuffer;
    class Renderer;
    class Scene;
    class SkyLight;
    class SkyLightEffect
    {
    public:
        SkyLightEffect(std::shared_ptr<StaticTexture> texture, Renderer &renderer, ID3D12GraphicsCommandList* command_list);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList *command_list, uint8_t idx);

    private:

        std::shared_ptr<StaticTexture> cubemap_;
        // 球谐系数生成
        ComPtr<ID3D12RootSignature> sh_sigature_;
        ComPtr<ID3D12PipelineState> sh_pipeline;
        std::unique_ptr<UAVBuffer> sh_buffer_;
    };
} // namespace feng