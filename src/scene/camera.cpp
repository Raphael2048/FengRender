#include "scene/camera.hpp"
#include <DirectXMath.h>
namespace feng
{

    Camera::Camera(const Vector3 &position, const Vector3 &rotation, float near_distance, float far_distance, float fov, float aspect)
        :Node(position, rotation, Vector3::One), near_(near_distance), far_(far_distance), fov_(fov), aspect_(aspect)
    {
    }

    void Camera::Update(float delta)
    {
        if (dirty_)
        {
            dirty_ = false;
            RefreshMatrix();
        }
    }
    void Camera::RefreshMatrix()
    {
        MatrixWorld = Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(rotation_.y),
            DirectX::XMConvertToRadians(rotation_.x), DirectX::XMConvertToRadians(rotation_.z));
        MatrixWorld *= Matrix::CreateTranslation(position_);
        MatrixInvWorld = MatrixWorld.Invert();

        DirectX::XMStoreFloat4x4(&MatrixProj, DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(fov_), aspect_, near_, far_));
        //MatrixProj = Matrix::CreatePerspectiveFieldOfView(, aspect_, near_, far_);

        MatrixInvProj = MatrixProj.Invert();
    }
} // namespace feng