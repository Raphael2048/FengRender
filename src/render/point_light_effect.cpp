#include "render/point_light_effect.hpp"
#include "renderer.hpp"
#include "scene/scene.hpp"
#include "dx12/dx12_shader.hpp"

namespace feng
{
    PointLightEffect::PointLightEffect(DepthOnly &depthEffect, Renderer &renderer, const Scene &scene)
    {
        shadow_pass_signature_ = depthEffect.signature_;
        shadow_pass_pso_ = depthEffect.pso_;
        for (int i = 0; i < 6; ++i)
        {
            t_shadowmaps[i].reset(new DynamicDepthTexture(renderer.GetDevice(), 128, 128, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT));
        }
    }
} // namespace feng