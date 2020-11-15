#include "scene/light.hpp"

namespace feng
{
    DirectionalLight::DirectionalLight(const Vector3 &rotation, const Color &color) 
        : Node(Vector3::Zero, rotation, Vector3::One), color_(color)
    {
    }

    void DirectionalLight::Update([[maybe_unused]]float deltetime)
    {
        if (dirty_)
        {
            dirty_ = false;
            MatrixWorld = Matrix::CreateFromYawPitchRoll(
                DirectX::XMConvertToRadians(rotation_.y),
                DirectX::XMConvertToRadians(rotation_.x), 0.0f);

            MatrixInvWorld = MatrixWorld.Invert();

            cb_ready_ = 0;
        }
    }
} // namespace feng