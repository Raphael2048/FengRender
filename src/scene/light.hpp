#pragma once
#include "scene/node.hpp"

namespace feng
{
    class DirectionalLight : public Node
    {
    public:
        // HDR linear color, unit : (W/m^2) or (lux)
        DirectionalLight(const Vector3& position, const Vector3& direction, const Color& color);

        virtual void Update(float time) override;

        DirectionalLight& SetDirection(const Vector3& direction);

        //平行光方向指向负Z轴, 和摄影机一样
        virtual Node& SetScale([[maybe_unused]] const Vector3& scale) override { return *this; }
        virtual Node& SetPosition([[maybe_unused]]const Vector3& pos) override { return *this; }
    private:
        Vector3 direction_;
        Color color_;
    };

    
}