#pragma once

#include "render/effect_base.hpp"


namespace feng
{
    class GBufferOutput : public EffectBase
    {
    public:
        GBufferOutput(Renderer& renderer);

        void Draw(Renderer &renderer, const Scene &scene, ID3D12GraphicsCommandList* command_list, uint8_t idx);
    };
}