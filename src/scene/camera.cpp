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
            // 默认情况下看向负Z轴
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

    const DirectX::BoundingFrustum &Camera::GetBoundingFrustrum()
    {
        if (box_dirty_)
        {
            box_dirty_ = false;
            RefreshBoundingBox();
        }
        return frustum_;
    }

    void Camera::RefreshBoundingBox()
    {
        float half_width = tan(DirectX::XMConvertToRadians(fov_)) * far_ * 0.5f;
        float half_height = half_width / aspect_ * 0.5f;
        Box origin = Box(
            Vector3(0, 0, -far_ * 0.5f),
            Vector3(half_width, half_height, far_ * 0.5f));
        box_ = origin.Transform(MatrixWorld);

        DirectX::BoundingFrustum::CreateFromMatrix(frustum_, MatrixProj);
        frustum_.Origin = position_;
        XMStoreFloat4(&frustum_.Orientation, DirectX::XMQuaternionRotationMatrix(MatrixWorld));
    }

    Node &Camera::SetScale([[maybe_unused]] const Vector3 &s)
    {
        return *this;
    }
} // namespace feng