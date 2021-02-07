#include "scene/light.hpp"

namespace feng
{
    DirectionalLight::DirectionalLight(const Vector3 &direction, const Color &color, float shadow_distance)
        : Node(Vector3::Zero, Vector3::Zero, Vector3::One), color_(color), direction_(direction), shadow_distance_(shadow_distance)
    {
        direction_.Normalize();
    }

    void DirectionalLight::Update([[maybe_unused]] float deltetime)
    {
        if (dirty_)
        {
            dirty_ = false;
            Vector3 Z = -direction_;
            Vector3 X = Vector3::Up.Cross(Z);
            if (X.LengthSquared() == 0.0f)
            {
                X = Vector3::Right;
            }
            else
            {
                X.Normalize();
            }
            Vector3 Y = Z.Cross(X);
            MatrixWorld = Matrix(X, Y, Z);

            MatrixInvWorld = MatrixWorld.Invert();

            cb_ready_ = 0;
        }
    }

    SpotLight::SpotLight(const Vector3 &position, const Vector3 &direction, const Color &color, float radius, float angle, float outer_angle)
        : Node(position, Vector3::Zero, Vector3::One), color_(color), direction_(direction), radius_(radius), inner_angle_(angle), outer_angle_(outer_angle)
    {
        direction_.Normalize();
    }

    void SpotLight::Update([[maybe_unused]] float deltetime)
    {
        if (dirty_)
        {
            dirty_ = false;
            Vector3 Z = -direction_;
            Vector3 X = Vector3::Up.Cross(Z);
            if (X.LengthSquared() == 0.0f)
                X = Vector3::Right;
            else
                X.Normalize();
            Vector3 Y = Z.Cross(X);
            MatrixWorld = Matrix(X, Y, Z);
            MatrixWorld *= Matrix::CreateTranslation(position_);
            MatrixInvWorld = MatrixWorld.Invert();

            DirectX::XMStoreFloat4x4(&MatrixProj, DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(outer_angle_ * 2), 1, radius_, 1.0f));
            MatrixInvProj = MatrixProj.Invert();
            cb_ready_ = 0;
            box_dirty_ = true;
        }
    }

    const DirectX::BoundingFrustum SpotLight::GetBoundingFrustum()
    {
        if (box_dirty_)
        {
            box_dirty_ = false;
            RefreshBoundingBox();
        }
        return frustum_;
    }

    void SpotLight::RefreshBoundingBox()
    {
        float topSlope = tan(DirectX::XMConvertToRadians(outer_angle_));
        float half_height = topSlope * radius_;
        float half_width = half_height;
        Box origin = Box(
            Vector3(0, 0, -radius_ * 0.5f),
            Vector3(half_width, half_height, radius_ * 0.5f));
        box_ = origin.Transform(MatrixWorld);

        frustum_.Far = radius_;
        frustum_.Near = 1.0f;
        frustum_.RightSlope = frustum_.TopSlope = topSlope;
        frustum_.LeftSlope = frustum_.BottomSlope = -frustum_.TopSlope;
        frustum_.Origin = position_;

        using namespace DirectX;
        XMVECTOR v = XMQuaternionRotationMatrix(MatrixWorld);
        XMFLOAT4 RotateY180(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR v2 = XMLoadFloat4(&RotateY180);
        XMStoreFloat4(&frustum_.Orientation, DirectX::XMQuaternionMultiply(v2, v));
    }

    PointLight::PointLight(const Vector3 &position, const Color &color, float radius)
        : Node(position, Vector3::Zero, Vector3::One), color_(color), radius_(radius)
    {
    }

    void PointLight::Update(float time)
    {
        if (dirty_)
        {
            dirty_ = false;
            MatrixWorld = Matrix::CreateTranslation(position_);

            MatrixInvWorld = Matrix::CreateTranslation(-position_);
            cb_ready_ = 0;
            box_dirty_ = true;
        }
    }

    void PointLight::RefreshBoundingBox()
    {
        box_.Center = position_;
        box_.Extents = Vector3(radius_, radius_, radius_);
    }

    SkyLight::SkyLight(const std::wstring &path, float intensity)
        : Node(Vector3::Zero, Vector3::Zero, Vector3::One), path_(path), intensity_(intensity)
    {
    }

} // namespace feng