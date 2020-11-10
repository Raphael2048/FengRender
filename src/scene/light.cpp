#include "scene/light.hpp"

namespace feng
{
    DirectionalLight::DirectionalLight(const Vector3& position, const Vector3& direction, const Color& color):
        Light(position, color), direction_(direction)
    {
    
    }


    void DirectionalLight::Update(float deltetime)
    {
        if (dirty_)
        {
            MatrixWorld *= Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(rotation_.y),
                                                          DirectX::XMConvertToRadians(rotation_.x), DirectX::XMConvertToRadians(rotation_.z));

            MatrixWorld *= Matrix::CreateTranslation(position_);

            MatrixInvWorld = MatrixWorld.Invert();

            cb_dirty_ = BACK_BUFFER_SIZE;
        }
    }
}