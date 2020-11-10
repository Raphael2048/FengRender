#pragma once
#include "scene/node.hpp"

namespace feng
{

    class Light : public Node
    {
    public:
        Light(const Vector3& position, const Color color):
            Node(position, Vector3::Zero, Vector3::One), color_(color) {}
    private:
        Color color_;

    };

    class DirectionalLight : public Light
    {
    public:
        // HDR linear color, unit : (W/m^2) or (lux)
        DirectionalLight(const Vector3& position, const Vector3& direction, const Color& color);

        virtual void Update(float time) override;

        DirectionalLight& SetDirection(const Vector3& direction)
        {
            direction_ = direction;
            dirty_ = true;
            return *this;
        } 
    private:
        Vector3 direction_;
    };
}