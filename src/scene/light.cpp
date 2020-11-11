#include "scene/light.hpp"

namespace feng
{
    DirectionalLight::DirectionalLight(const Vector3 &position, const Vector3 &direction, const Color &color) : Node(position, Vector3::Zero, Vector3::One), direction_(direction)
    {
    }

    void DirectionalLight::Update(float deltetime)
    {
        if (dirty_)
        {
            MatrixWorld *= Matrix::CreateFromYawPitchRoll(
                DirectX::XMConvertToRadians(rotation_.y),
                DirectX::XMConvertToRadians(rotation_.x), 0.0f);

            MatrixInvWorld = MatrixWorld.Invert();

            cb_dirty_ = BACK_BUFFER_SIZE;
        }
    }
} // namespace feng