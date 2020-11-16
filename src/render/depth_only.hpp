#pragma once

#include "render/effect_base.hpp"

namespace feng
{
    class DepthOnly : public EffectBase
    {
    public:
        friend class DirectionalLightEffect;
        friend class SpotLightEffect;
        DepthOnly(Renderer& renderer);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    };
}