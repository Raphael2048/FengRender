#include "scene/camera.hpp"
#include <DirectXMath.h>
#include <cmath>
namespace feng
{

    Camera::Camera(const Vector3 &position, const Vector3 &rotation, float near_distance, float far_distance, float fov, float aspect)
        : Node(position, rotation, Vector3::One), near_(near_distance), far_(far_distance), fov_(fov), aspect_(aspect)
    {
    }

    void Camera::Update(float delta)
    {
        if (dirty_)
        {
            dirty_ = false;
            MatrixWorld = Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(rotation_.y),
                                                         DirectX::XMConvertToRadians(rotation_.x), DirectX::XMConvertToRadians(rotation_.z));
            MatrixWorld *= Matrix::CreateTranslation(position_);
            MatrixInvWorld = MatrixWorld.Invert();
            // REVERSE-Z
            DirectX::XMStoreFloat4x4(&MatrixProj, DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(fov_), aspect_, far_, near_));

            MatrixInvProj = MatrixProj.Invert();

            cb_dirty_ = BACK_BUFFER_SIZE;
            box_dirty_ = true;
        }
    }

    void Camera::RefreshBoundingBox()
    {
        float width = tan(DirectX::XMConvertToRadians(fov_)) * far_;
        float height = width / aspect_;
        Box origin = Box(
            Vector3(0, 0, -far_ * 0.5f),
            Vector3(width * 0.5f, height * 0.5f, far_ * 0.5f));
        box_ = origin.Transform(MatrixWorld);
    }

    Node &Camera::SetScale([[maybe_unused]] const Vector3 &s)
    {
        return *this;
    }
} // namespace feng