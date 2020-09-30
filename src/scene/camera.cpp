#include "scene/camera.hpp"

namespace feng
{

    Camera::Camera(const Vector3 &position, const Vector3 &rotation, float near, float far, float fov, float aspect)
        :Node(position_, rotation_, Vector3::One), near_(near), far_(far), fov_(fov), aspect_(aspect) 
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
        MatrixView = Matrix::CreateFromYawPitchRoll(rotation_.x, rotation_.y, rotation_.z);
        MatrixInvView = MatrixView.Invert();

        MatrixProj = Matrix::CreatePerspectiveFieldOfView(fov_, aspect_, near_, far_);

        MatrixInvProj = MatrixProj.Invert();
    }
} // namespace feng