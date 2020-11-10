#pragma once

#include "SimpleMath.h"

namespace feng
{
    typedef DirectX::SimpleMath::Vector2 Vector2;
    typedef DirectX::SimpleMath::Vector4 Vector4;
    typedef DirectX::SimpleMath::Vector3 Vector3;
    typedef DirectX::SimpleMath::Color Color;
    typedef DirectX::SimpleMath::Plane Plane;
    typedef DirectX::SimpleMath::Ray Ray;
    typedef DirectX::SimpleMath::Matrix Matrix;

    struct Box : public DirectX::BoundingBox
    {
        Box() : BoundingBox() {}
        Box(const Box &) = default;
        Box &operator=(const Box &) = default;

        Box(Box &&) = default;
        Box &operator=(Box &&) = default;
        Box(Vector3 center, Vector3 extent) : BoundingBox(center, extent) {}

        Box Transform(const Matrix& m)
        {
            Box out;
            DirectX::BoundingBox::Transform(out, m);
            return out;
        }

        Box Combine(const Box& box)
        {
            Box out;
            DirectX::BoundingBox::CreateMerged(out, *this, box);
            return out;
        }

        bool Intersects(const Box& box) const
        {
            return DirectX::BoundingBox::Intersects(box);
        }
    };

} // namespace feng