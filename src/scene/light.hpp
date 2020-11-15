#pragma once
#include "scene/node.hpp"

namespace feng
{
    // 平行光方向指向负Z轴, 和摄影机一样
    // 三级CSM阴影
    class DirectionalLight : public Node
    {
    public:
        friend class DirectionalLightEffect;
        // HDR linear color, unit : (W/m^2) or (lux)
        DirectionalLight(const Vector3& rotation, const Color& color);

        virtual void Update(float time) override;

        virtual Node& SetScale([[maybe_unused]] const Vector3& scale) override { return *this; }
        virtual Node& SetPosition([[maybe_unused]]const Vector3& pos) override { return *this; }
    private:
        Color color_;
        float shadow_distance_ = 200.0f;
    };

    
}