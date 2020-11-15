#include "scene/camera.hpp"
#include <DirectXMath.h>
#include <cmath>
namespace feng
{

    Camera::Camera(const Vector3 &position, const Vector3 &rotation, float near_distance, float far_distance, float fov, float aspect)
        : Node(position, rotation, Vector3::One), near_(near_distance), far_(far_distance), fov_(fov), aspect_(aspect)
    {
    }

    void Camera::Update([[maybe_unused]]float delta)
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

            cb_ready_ = 0;
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
        float topSlope = tan(DirectX::XMConvertToRadians(fov_) * 0.5f);
        float half_height = topSlope * far_;
        float half_width = half_height * aspect_;
        Box origin = Box(
            Vector3(0, 0, -far_ * 0.5f),
            Vector3(half_width, half_height, far_ * 0.5f));
        box_ = origin.Transform(MatrixWorld);

        frustum_.Far = far_;
        frustum_.Near = near_;
        frustum_.TopSlope = topSlope;
        frustum_.BottomSlope = -frustum_.TopSlope;
        frustum_.RightSlope = frustum_.TopSlope * aspect_;
        frustum_.LeftSlope = -frustum_.RightSlope;
        frustum_.Origin = position_;

        using namespace DirectX;
        XMVECTOR v = XMLoadFloat3(&rotation_);
        v = XMVectorScale(v, XM_PI / 180.0f);
        v = XMQuaternionRotationRollPitchYawFromVector(v);
        XMFLOAT4 RotateY180(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR v2 = XMLoadFloat4(&RotateY180);
        // v2 = XMQuaternionRotationRollPitchYawFromVector(v2);
        XMStoreFloat4(&frustum_.Orientation, DirectX::XMQuaternionMultiply(v2, v));
    }

    Node &Camera::SetScale([[maybe_unused]] const Vector3 &s)
    {
        return *this;
    }
} // namespace feng